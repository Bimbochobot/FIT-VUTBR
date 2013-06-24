//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 27.10.2012
//// Popis:
//// Subor obsahujuci implementaciu
//// utilitarnych funkcii vyuzitelnych
//// pri dalsich projektoch.
//// Taktiez obsahuje napovedu a text chybovych
//// hlaseni. Pozn: globalne konstanty pouzite
//// na zaklade kostry projektu uvedenej na
//// stranke IZP.

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "proj3util.h"

const char *ECODEMSG[] =
{
  [EOK] = "Vsetko v poriadku.",
  [ECLWRONG] = "Chybne parametry prikazovej riadky!",
  [EWRONGFORMAT] = "Chybny format osemsmerovky!",
  [ENOTFOUND] = "Zadany subor neexistuje!",
  [EBADWORD] = "Zadane slovo obsahuje nepovolene znaky!",
  [ELONGWORD] = "Zadane slovo je prilis dlhe!",
  [ENOSOLUTION] = "V osemsmerovke sa nenaslo zadane slovo.",
  [EALLOC] = "Nebolo mozne alokovat pole pozadovanej velkosti.",
  [EUNKNOWN] = "Nastala neocakavana chyba."
};

const char *HELPMSG =
    "Program Osemsmerovka.\n"
    "Autor: Ivan Sevcik (c) 2012\n\n"
    "Pouzitie: proj3 --help || -h              Zobrazi napovedu.\n"
    "          proj3 --test [subor]            Ak je subor v spravnom formate,\n"
    "                   vypise jeho obsah na vystup (inak chybove hlasenie).\n"
    "          proj3 --search=slovo [subor]    Vyhlada slovo v osemsmerovke\n"
    "                   nacitanej zo subor, vyznaci jeho vyskyt velkymi\n"
    "                   pismenami a vypise na vystup. V pripade, ze slovo\n"
    "                   osemsmerovka neobsahuje, vypise chybove hlasenie.\n"
    "          proj3 --solve [subor1] [subor2] Vyhlada vsetky slova zo\n"
    "                   subor2 v osemsmerovke nacitanej zo subor1 a\n"
    "                   vypise tajnicku pozostavajucu z neoznacenych\n"
    "                   pismen. Ak sa niektore slovo nenaslo, vypise\n"
    "                   chybove hlasenie."
    "";

bool incStrCap(String* str){
    unsigned int newCap = 0;
    char* temp = NULL;

    if(str->cap >= UINT_MAX){
        printError(EALLOC, "presah maximalnej kapacity");
        return false;
    }
    if(str->cap == 0) newCap = 10;
    else if(str->cap > UINT_MAX/2) newCap = UINT_MAX;
    else newCap = str->cap*2;

    temp = realloc(str->data, newCap);
    if(temp == NULL){
        printError(EALLOC, NULL);
        return false;
    }

    str->data = temp;
    str->cap = newCap;

    return true;
}

int getMainParam(MainParam* params, int argc, char* argv[], int offset)
{
    if(offset >= argc)
        return 0;

    if(strcmp(argv[offset], "-h") == 0 || strcmp(argv[offset], "--help") == 0){
        if(!params[HELP].enabled){
            params[HELP] = (MainParam){ true, {NULL, NULL} };
            return 1;
        }
    }

    if(strcmp(argv[offset], "--test") == 0 && offset + 1 < argc){
        if(!params[TEST].enabled){
            params[TEST] = (MainParam){ true, {NULL, NULL} };
            params[TEST].options[0] = argv[offset+1];
            return 2;
        }
    }

    if(strncmp(argv[offset], "--search=", 9) == 0 && offset + 1 < argc){
        if(!params[SEARCH].enabled){
            params[SEARCH] = (MainParam){ true, {NULL, NULL} };
            params[SEARCH].options[0] = argv[offset+1];
            params[SEARCH].options[1] = &(argv[offset][9]);
            return 2;
        }
    }

    if(strcmp(argv[offset], "--solve") == 0 && offset + 2 < argc){
        if(!params[SOLVE].enabled){
            params[SOLVE] = (MainParam){ true, {NULL, NULL} };
            params[SOLVE].options[0] = argv[offset+1];
            params[SOLVE].options[1] = argv[offset+2];
            return 3;
        }
    }

    return 0;
}

bool checkParams(const MainParam* params)
{
    //zisti, ci nenastali konflikty v parametroch
    //v tejto ulohe je povoleny prave 1 parameter
    int paramsEnabled = params[HELP].enabled + params[TEST].enabled
                      + params[SEARCH].enabled + params[SOLVE].enabled;
    if(paramsEnabled != 1)
        return false;

    if(params[SEARCH].enabled && params[SEARCH].options[1][0] == '\0')
        return false;

    return true;
}

bool procMainArg(MainParam* params, int argc, char* argv[])
{
    int argp = 1, ret = 1;
    while(ret != 0 && argp < argc){
        ret = getMainParam(params, argc, argv, argp);
        argp += ret;
    }

    if(argp < argc || !checkParams(params)){
        //zostali nespracovatelne argumenty alebo parametre boli nekorektne
        printError(ECLWRONG, NULL);
        return false;
    }
    else
        return true;
}

void printHelp()
{
    printf("%s", HELPMSG);
}

void printError(int code, const char* detail)
{
    if(detail == NULL)
        fprintf(stderr, "%s\n", ECODEMSG[code]);
    else
        fprintf(stderr, "%s (%s)\n", ECODEMSG[code], detail);
}
