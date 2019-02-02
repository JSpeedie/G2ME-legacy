date=$(wget $1 -q -O - \
	| grep -o "[0-9]\{4\}-[0-9]\{1,2\}-[0-9]\{1,2\}" | head -n 1 \
	| sort | sed "s/-/ /g" | awk '{print $3, $2, $1}')

wget $1 -q -O - | grep -oP "\[\'TournamentStore\'\] =.*?\};" \
	| sed "s/\['TournamentStore'\] = //g" | sed "s/\};$/\}/g" \
	| jq -j --arg date "$date" '.matches_by_round[] | .[] |  .player1.display_name, " ", .player2.display_name, " ", .scores[0], " ", .scores[1], " ", $date, "\n"'
