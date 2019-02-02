# Example:
# I have a tournament: https://smash.gg/tournament/toronto-stock-exchange-7/events
# $ sh dlsmashgg.sh toronto-stock-exchange-7
# All that's needed now is to add the date for every set

# Take a tournament "url name" and print all the brackets that could
# be converted and let the user choose
echo "fetching group IDs..."
smashggurl="https://api.smash.gg/tournament/${1}?expand[]=phase&expand[]=groups&expand[]=event"
json=$(wget $smashggurl -q -O -)
groupIds=$(echo "$json" \
	| jq -r '.entities.groups[] | .id');
echo "$groupIds"
printf "Please enter one of the IDs listed above: "
read -a chosenGroupId

if echo "$groupIds" | grep "^${chosenGroupId}$"; then
	echo "Chosen group: $chosenGroupId"
	echo "fetching bracket data..."
else
	echo "ERROR: Invalid groupId. Exiting..."
	exit 1
fi

# The rest of this script requires 1 argument: a phase id for the bracket on
# smash.gg this can be found by going to the api page:
# https://api.smash.gg/tournament/[tournament name here]?expand[]=phase&expand[]=groups&expand[]=event
# and going to 'entities' > 'groups'. Each event (melee singles, melee doubles,
# project m, etc) will have it's own number. Expand 1 of them, 'id' is the value
# you will need for this script.
# Example:
# I have a tournament: https://smash.gg/tournament/toronto-stock-exchange-7/events
# I visit the api page:
# https://api.smash.gg/tournament/toronto-stock-exchange-7?expand[]=phase&expand[]=groups&expand[]=event
# After findidng the correct phase id (in this example, melee singles is
# 577720) I run the rest of this script like
# All that's needed now is to add the date for every set
smashggurl="https://api.smash.gg/phase_group/${chosenGroupId}?expand[]=entrants&expand[]=event&expand[]=phase&expand[]=sets&expand[]=participants&mutations[]=playerData"
json=$(wget $smashggurl -q -O -)

# Creates output of format:
# [Set Number] [player 1 tournament id] [player 2 tournament id] [player 1 game count] [player 2 game count]
date=$(date "+%d %m %Y")
formatted_sets=$(echo "$json" \
	| jq -jr --arg date "$date" '.entities.sets[] | select( .entrant2Id != null ) | select( .entrant1Id != null ) | .id, " ", .entrant1Id, " ", .entrant2Id, " ", .entrant1Score, " ", .entrant2Score, " ", .isGF, " ", $date, "\n"')

# Sort the sets to be in the correct order
# aka winners rounds 1 to x, then losers rounds 1 to x
formatted_sets=$(echo -e -n "$formatted_sets" | sort -nk1 | cut -d ' ' -f2-9)

# Creates output of format '[player lasting id] "[name]" "[gamerTag]"' with
# no spaces in the names or gamerTags
IFS=$'\n'
declare -a id=($(echo "$json" \
	| jq '.entities.entrants[] | .id, .mutations.players[].name, .mutations.players[].gamerTag'));
unset IFS
player_id_conversion=""
for (( i = 0; i < ${#id[@]}; ++i )); do
	let count=count+1;
	if [[ count -ge 3 ]]; then
		let count=0;
		player_id_conversion+="$(echo "${id[$i]}" | sed "s/ //g")\n"
	else
		player_id_conversion+="$(echo "${id[$i]}" | sed "s/ //g") "
	fi
done

# Change players names who didn't enter their names to a place holder
player_id_conversion=$(echo "$player_id_conversion" | sed "s/\"\"/PLACEHOLDER/g" \
	| sed "s/null/PLACEHOLDER/g" | sed "s/\"//g")

# Create output of format '[player lasting id] "[name or gamerTag]"'
# The purpose of this is to set a players name to their gamerTag if they
# didn't provide a name or provided a blank one
IFS=$'\n'
player_id_conversion_safe=""
for i in $(echo -e -n "${player_id_conversion[@]}"); do
	lasting_id=$(echo "$i" | cut -d ' ' -f1)
	name=$(echo "$i" | cut -d ' ' -f2)
	gamerTag=$(echo "$i" | cut -d ' ' -f3)
	# If the user did not provide a name
	if [[ "$name" == "PLACEHOLDER" ]]; then
		name="$gamerTag"
	fi
	player_id_conversion_safe+="$(echo -e "$lasting_id $name")\n";
done
unset IFS

# Converts '[player lasting id] [first name][last name]'
#       to '[player temp id] [first name][last name]'
IFS=$'\n'
tourn_id_name=$tourn_id_lasting_id
for i in $(echo -e -n "$player_id_conversion_safe"); do
	tourn_player_id=$(echo "$i" | cut -d ' ' -f1)
	lasting_player_id=$(echo "$i" | cut -d ' ' -f2)
	tourn_id_name=$(echo -e -n "$tourn_id_name" | sed "s/${tourn_player_id}/${lasting_player_id}/g")
done
unset IFS

# Creates output of format
# [Set Number] [player 1 lasting id] [player 2 lasting id] [player 1 game count] [player 2 game count]
IFS=$'\n'
sets_with_player_names=$formatted_sets
for i in $(echo -e -n "${player_id_conversion_safe[@]}"); do
	lasting_id=$(echo "$i" | cut -d ' ' -f1)
	name=$(echo "$i" | cut -d ' ' -f2)
	sets_with_player_names=$(echo -e -n "$sets_with_player_names" \
		| sed "s/${lasting_id}/${name}/g")
done
unset IFS

# Move any grand finals sets to the end of the output. Smashgg by default
# places them at the end of the winner's bracket which is technically correct,
# but would calculate the glicko2 data in the wrong order so it must be
# rearranged
grand_finals_sets=$(echo -e "$sets_with_player_names" | grep "true")
correct_order_sets=$(echo -e "$sets_with_player_names" | grep -v "true"; echo -e $grand_finals_sets;)

echo -e "$correct_order_sets" | cut -d ' ' -f1-4,6-8
