if [ $# -ne 1 ]; then
	echo "Nebola zadana cesta k zlozke projektu!" 1>&2
    exit 1
elif [ ! -d "$1" ]; then
	echo "Zlozka projektu neextistuje!" 1>&2
    exit 1
fi

# Vytvori regularny vyraz pre ignorovane polozky
if [ -f .le/.config ]; then
    regexp=`grep -E "ignore .+" .le/.config | cut -b 8- | tr '\n' '|'`
else
    regexp=""
fi
# Ignorovanie skrytych suborov
hiden_regexp="(^|/)\.[^/]*$"
regexp=$regexp$hiden_regexp

# Vycisti zlozku referencnej kopie
if [ -d .le ]; then
    find .le/ -maxdepth 1 -type f | grep -Ev "$hiden_regexp" | \
    tr '\n' '\0' | xargs -r -0 rm
else
    mkdir .le
fi

#upravi cestu na pouzitie v sed
if ! echo "$1" | grep ".*/$" > /dev/null;	then path="$1"/
											else path="$1"
fi
sedpath=`echo $path | sed 's/\//\\\\\//g'`

# Skopiruje vybrane subory
file_list=`find "$1" -maxdepth 1 -type f | sed "s/[^\n]*\///" | \
			grep -Ev "$regexp" |  sed -r "s/([^\n]*)$/$sedpath\1/g"`
IFS="
"
for i in $file_list; do
	cp "$i" ./
	cp "$i" ./.le/
done

# Zapise aktualny projdir
# Grep vrati 0 (true) ak sa nasla zhoda
touch .le/.config
if grep -E "projdir" .le/.config > /dev/null; then
    sed -i -r "s/projdir.*/projdir $sedpath/g" .le/.config
else
    echo "projdir $path" >> .le/.config
fi