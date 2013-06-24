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
    file_list="$file_list
"`find .le/ -maxdepth 1 -type f | cut -d'/' -f 2-`
               
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
    # Najskor testuj existenciu suborov
    if [ ! -f "$proj_dir$i" ]; then
        if [ ! -f ".le/$i" ]; then
            continue
        else
            # Case 7
            echo "D: $i"
            rm ".le/$i"
            if [ -f "./$i" ]; then rm "./$i"; fi
            continue
        fi
    else
        if [ ! -f "./$i" ]; then
            # Case 6
            echo "C: $i"
            cp "$proj_dir$i" ".le/"
            cp "$proj_dir$i" "./"
            continue
        elif [ ! -f ".le/$i" ]; then
            continue
        fi
    fi
    
    # Porovnaj zhodnost suborov
    if diff "$proj_dir$i" "./$i" > /dev/null; then
        if diff "$proj_dir$i" ".le/$i" > /dev/null; then
            # Case 1
            echo ".: $i"
        elif [ $? -eq 1 ]; then
            # Case 3
            echo "UM: $i"
            cp "$proj_dir$i" ".le/"
        fi
    else
		patchfile=`mktemp ./XXXXXXXX`
        if diff -u ".le/$i" "$proj_dir$i" > "$patchfile"; then
            # Case 2
            echo "M: $i"
        elif [ $? -eq 1 ]; then
            if diff "./$i" ".le/$i" > /dev/null; then
                # Case 4
                echo "U: $i"
                cp "$proj_dir$i" ".le/"
                cp "$proj_dir$i" "./"
            elif [ $? -eq 1 ]; then
                # Case 5
				tmp=`mktemp -d ./XXXXXXXX`
                if patch "./$i" -i "$patchfile" -o "$tmp/patched" -r "$tmp/rj" \
				-s -B "$tmp/" > /dev/null 2>&1; then
                    echo "M+: $i"
                    mv "$tmp/patched" "./$i"
                    cp "$proj_dir$i" ".le/"
                else
                    echo "M!: $i conflict!"
                fi
				rm -r "$tmp"
            fi
        fi
        rm "$patchfile"
    fi
done