if [ ! -d .le/ ]; then
	echo "Zlozka referencnej kopie neexistuje!" 1>&2
	exit 1
fi

# Vytvori regularny vyraz pre ignorovane polozky
if [ -f .le/.config ]; then
    regexp=`grep -E "ignore .+" .le/.config | cut -b 8- | tr '\n' '|'`
else
    regexp=""
fi
# Ignorovanie skrytych suborov
regexp=$regexp"(^|/)\.[^/]*$"

# Vytvori zoznam suborov pre kopirovanie
if [ $# -eq 0 ]; then
    copy_list=`find .le/ -maxdepth 1 -type f | cut -d/ -f 2 | \
		grep -Ev "$regexp" | sed -r "s/([^\n]*)$/.le\/\1/g"`
else
    copy_list=""
    for i in "$@"; do
		i=.le/`echo $i | grep -Ev "$regexp"`
        if [ -f "$i" ]; then
            copy_list="$copy_list$i
"
        fi
    done
fi

IFS="
"
for i in $copy_list; do
    cp "$i" ./
done