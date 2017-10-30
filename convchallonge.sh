wget $1 -q -O .temp

cat .temp | grep "StoreState" \
	| grep -oP "(\"display_name\":\"(.*?)\",)|(\"games\":\[\[(.*?)\]\],)" \
	| sed "s/\"display_name\"://g" | sed "s/\"games\"://g" | sed "s/,$//g" \
	| tr -d '\n' | sed "s/\]\]/\]\]\n/g" \
	| sed -e "s/\"\"/\" \"/g" -e "s/\[\[/ /g" -e "s/,/ /g" -e "s/\]\]//g"
