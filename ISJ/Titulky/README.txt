Autor: Ivan Ševčík (xsevci50)

Program na vyhladavanie, stahovanie a porovnanie titulkov.

Pouzitie: 
python3 SubMatch.py [-a ref. hash] [-i ref. imdb ID] [-l ref. jazyk] [-s velkost ref. v byte] [-t jazyk] url

Popis:
Program v zakladnom rezime vyhlada referencne titulky podla URL, vyhlada najlepsiu zhodu v urcenom jazyku, oba titulkove
subory ulozi vo formate SubRip a vytvori subor s dvojicami odpovedajucich si prehovorov. V rozsirenom rezime je mozne
pri zadani znaku - namiesto URL vyhladavat referencne titulky podla inych parametrov.

Program vyuziva databazu opensubtitles.org, dokaze pracovat s MicroDVD (sub) a SubRip (srt) formatom titulkov, 
pre vyhladanie odpovedajucich si prehovorov vyuziva casove okno, dlzku retazcov a pocet ciarok vo vete.
Informacie o titulkoch su ziskavane z XML, na ich prevziatie bolo vyuzite XMLRPC. Najvhodnejsie titulky
su vyberane na zaklade zhodnych imdb ID, pripadne nazvu ak nie je ID dostupne, nasledne filtrovane podla
zhodneho poctu CD a nakoniec sa vyberu titulky, ktore maju rozdiel uvedenej velkosti filmu najmensi voci
referencii. Program by mal byt schopny pracovat s lubovolnym kodovanim titulkov a ukoncovanim riadkov.
