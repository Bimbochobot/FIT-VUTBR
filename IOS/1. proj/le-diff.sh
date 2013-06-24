# Vytvori regularny vyraz pre ignorovane polozky
if [ -f .le/.config ]; then
    regexp=`grep -E "ignore .+" .le/.config | cut -b 8- | tr '\n' '|'`
else
	echo "Neexistuje konfiguracny subor - neznama cesta k zlozke projektu." 1>&2
    exit 1
fi
# Ignorovanie skrytych suborov
regexp=$regexp"(^|/)\.[^/]*$"

proj_dir=`grep -E "projdir" .le/.config | cut -d' ' -f 2-`
# Zaisti slash na konci cesty
if ! echo "$proj_dir" | grep "/$" > /dev/null; then
    proj_dir=$proj_dir"/"
fi

if [ $# -eq 0 ]; then
    # Vytvori zoznam suborov pre porovnanie zlucenim oboch zloziek
    file_list=`find "$proj_dir" -maxdepth 1 -type f | rev | cut -d'/' -f 1 | \
               rev`
	file_list="$file_list
"`find ./ -maxdepth 1 -type f | cut -d'/' -f 2-`
               
    # Odstrani duplikaty a ignorovane
    file_list=`echo -n "$file_list" | sort | uniq | grep -Ev "$regexp"`
else
    # Prepise jednotlive argumenty na samostatny riadok
    file_list=""
    for i in "$@"; do
		if [ -f "$i" ]; then
			file_list="$file_list$i
"
		fi
    done
    file_list=`echo -n "$file_list" | grep -Ev "$regexp"`
fi

# Porovnaj zlozky podla zoznamu
IFS="
"
for i in $file_list; do
    if [ -f $proj_dir$i ]; then
        if [ -f ./$i ]; then
             if diff -u $proj_dir$i ./$i; then
                echo ".: $i"
             fi
        else
            echo "C: $i"
        fi
    elif [ -f ./$i ]; then
        echo "D: $i"
    fi
done