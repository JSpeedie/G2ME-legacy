# This script requires 1 argument: a phase id for the bracket on smash.gg
# this can be found by going to the api page:
# https://api.smash.gg/tournament/[tournament name here]?expand[]=phase&expand[]=groups&expand[]=event
# and going to 'entities' > 'groups'. Each event (melee singles, melee doubles,
# project m, etc) will have it's own number. Expand 1 of them, 'id' is the value
# you will need for this script.
#
# Example:
# I have a tournament: https://smash.gg/tournament/toronto-stock-exchange-7/events
# I visit the api page:
# https://api.smash.gg/tournament/toronto-stock-exchange-7?expand[]=phase&expand[]=groups&expand[]=event
# After findidng the correct phase id (in this example, melee singles is
# 577720) I run this script like
# $ sh dlsmashgg.sh 577720
# All that's needed now is to add the date for every set
smashggurl="https://api.smash.gg/phase_group/${1}?expand[]=entrants&expand[]=event&expand[]=phase&expand[]=sets&expand[]=participants&mutations[]=playerData"
json=$(wget $smashggurl -q -O -)

# Creates output of format:
# [Set Number] [player 1 tournament id] [player 2 tournament id] [player 1 game count] [player 2 game count]
setinfo=$(echo "$json" \
	| jq -r '.entities.sets[] | select( .entrant2Id != null ) | select( .entrant1Id != null ) | .id,.entrant1Id,.entrant2Id,.entrant1Score,.entrant2Score,.isGF,.completedAt')
let count=0;
formatted_sets=""
for i in $(echo $setinfo); do
	let count=count+1;
	if [[ count -ge 7 ]]; then
		let count=0;
		formatted_sets+="$(date -d @${i} "+%d %m %Y")\n"
	else
		formatted_sets+="$(echo "$i ")"
	fi
done

# Sort the sets to be in the correct order
# aka winners rounds 1 to x, then losers rounds 1 to x
formatted_sets=$(echo -e -n "$formatted_sets" | sort -nk1 | cut -d ' ' -f2-9)

# Creates output of format:
# [player_id in tournament] [player_id_lasting]
IFS=$'\n'
declare -a nameinfo=($(echo "$json" \
	| jq '.entities.entrants[] | .id,.participantIds[0]'))
unset IFS
let count=0;
tourn_id_lasting_id=""
for (( i = 0; i < ${#nameinfo[@]}; ++i )); do
	let count=count+1;
	if [[ count -ge 2 ]]; then
		let count=0;
		tourn_id_lasting_id+="$(echo "${nameinfo[$i]}")\n"
	else
		tourn_id_lasting_id+="$(echo "${nameinfo[$i]} ")"
	fi
done

# Creates output of format '[player lasting id] "[first name]" "[last name]"'
IFS=$'\n'
declare -a id=($(echo "$json" \
	| jq '.entities.participants[] | .id,.contactInfo.nameFirst,.contactInfo.nameLast'))
unset IFS
player_id_conversion=""
for (( i = 0; i < ${#id[@]}; ++i )); do
	let count=count+1;
	if [[ count -ge 3 ]]; then
		let count=0;
		# Remove spaces in names that are made of multiple names Ex. "Martinez Gracey"
		player_id_conversion+="$(echo "${id[$i]}" | sed "s/ //g")\n"
	else
		player_id_conversion+="$(echo "${id[$i]}" | sed "s/ //g") "
	fi
done


# Change players names who didn't enter their names to a place holder
player_id_conversion=$(echo "$player_id_conversion" | sed "s/\"\"/PLACEHOLDER/g")
# Converts '[player lasting id] "[first name]" "[last name]"'
#       to '[player lasting id] [first name][last name]'
player_id_conversion=$(echo "$player_id_conversion" | sed "s/\" \"//g" | sed "s/\"//g")

# Converts '[player lasting id] [first name][last name]'
#       to '[player temp id] [first name][last name]'
IFS=$'\n'
tourn_id_name=$tourn_id_lasting_id
for i in $(echo -e -n "$player_id_conversion"); do
	tourn_player_id=$(echo "$i" | cut -d ' ' -f1)
	lasting_player_id=$(echo "$i" | cut -d ' ' -f2)
	tourn_id_name=$(echo -e -n "$tourn_id_name" | sed "s/${tourn_player_id}/${lasting_player_id}/g")
done
unset IFS

# Creates output of format
# [Set Number] [player 1 lasting id] [player 2 lasting id] [player 1 game count] [player 2 game count]
IFS=$'\n'
sets_with_player_names=$formatted_sets
for i in $(echo -e -n "$tourn_id_name"); do
	lasting_id=$(echo "$i" | cut -d ' ' -f1)
	name=$(echo "$i" | cut -d ' ' -f2)
	sets_with_player_names=$(echo -e -n "$sets_with_player_names" | sed "s/${lasting_id}/${name}/g")
done
unset IFS

# Move any grand finals sets to the end of the output. Smashgg by default
# places them at the end of the winner's bracket which is technically correct,
# but would calculate the glicko2 data in the wrong order so it must be
# rearranged
grand_finals_sets=$(echo -e "$sets_with_player_names" | grep "true")
correct_order_sets=$(echo -e "$sets_with_player_names" | grep -v "true"; echo -e $grand_finals_sets;)

echo -e "$correct_order_sets" | cut -d ' ' -f1-4,6-8
