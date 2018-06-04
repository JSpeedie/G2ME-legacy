smashggurl="https://api.smash.gg/phase_group/${1}?expand[]=entrants&expand[]=event&expand[]=phase&expand[]=sets&expand[]=participants&mutations[]=playerData"
json=$(wget $smashggurl -q -O - > .temp)

# Creates output of format:
# [Set Number] [player 1 tournament id] [player 2 tournament id] [player 1 game count] [player 2 game count]
setinfo=$(cat .temp \
	| jq -r '.entities.sets[] | select( .entrant2Id != null ) | select( .entrant1Id != null ) | .entrant1Id,.entrant2Id,.entrant1Score,.entrant2Score,.isGF')
let count=0;
formatted_sets=""
for i in $(echo $setinfo); do
	let count=count+1;
	if [[ count -ge 5 ]]; then
		let count=0;
		formatted_sets+="$(echo "$i")\n"
	else
		formatted_sets+="$(echo "$i ")"
	fi
done

# Creates output of format:
# [player_id in tournament] [player_id_lasting]
IFS=$'\n'
declare -a nameinfo=($(cat .temp \
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

IFS=$'\n'
declare -a id=($(cat .temp \
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

# Creates output of format [player lasting id] [first name][last name]
# Format '"[first name]" "[last name]"' to '[first name][last name]'
player_id_conversion=$(echo "$player_id_conversion" | sed "s/\" \"//g" | sed "s/\"//g")
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

echo -e "$correct_order_sets" | cut -d ' ' -f1-4
