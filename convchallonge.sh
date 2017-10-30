date=$(wget $1 -q -O - \
	| grep -o "[0-9]\{4\}-[0-9]\{1,2\}-[0-9]\{1,2\}" | head -n 1 \
	| sort | sed "s/-/ /g" | awk '{print $3, $2, $1}')

wget $1 -q -O - | grep "StoreState" \
	| grep -oP "(\"identifier\":(.*?),)|(\"display_name\":\"(.*?)\",)|(\"games\":\[\[(.*?)\]\],)" \
	| sed "s/\"identifier\"://g" | sed "s/\"display_name\"://g" \
	| sed "s/\"games\"://g" | sed "s/,$//g" \
	| tr -d '\n' | sed "s/\]\]/\]\]\n/g" \
	| sed -e "s/\"\"/\" \"/g" -e "s/\[\[/ /g" -e "s/,/ /g" -e "s/\]\]/ $date/g" \
	| sort -g | sed "s/^[0-9]\+//g"
