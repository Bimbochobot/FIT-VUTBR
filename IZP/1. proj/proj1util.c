//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
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

#include "proj1util.h"

const char *ECODEMSG[] =
{
  [EOK] = "Vsetko v poriadku.\n",
  [ECLWRONG] = "Chybne parametre prikazovej riadky!\nPre zobrazenie napovedy pouzite parameter --help\n",
  [EUNKNOWN] = "Nastala neocakavana chyba."
};

const char *HELPMSG =
    "Program Tvorba histogramu.\n"
    "Autor: Ivan Sevcik (c) 2012\n\n"
    "Popis:\n"
    "Program vytvori histogram vyskytu jednotlivych znakov na vstupe.\n\n"
    "Pouzitie: proj1 --help\n"
    "          proj1 -h         Zobrazi napovedu (zhodne s --help).\n"
    "          proj1 N          Vypise N najcastejsich znakov.\n"
    "          proj1 --extra    Vypise honodty v prehladnej tabulke.\n"
    "                           Parameter je kompatibilny s parametrom N.\n"
    "";

long strToLC(const char* str, bool* cor)
{
    if(str == NULL){
        if(cor != NULL)
            *cor = false;
        return 0;
    }

    long num = 0;
    int i = 0;
    while(str[i] != 0)
    {
        num *= 10;
        int digit = str[i] - '0';
        if(digit >= 0 && digit <= 9)
            num += digit;
        else{
            if(cor != NULL)
                *cor = false;
            return 0;
        }
        i++;
    }

    if(cor != NULL)
        *cor = true;
    return num;
}

long strToL(const char* str)
{
    return strToLC(str, NULL);
}

int strCmpC(const char* str1, const char* str2, bool* cor)
{
    if(str1 == NULL || str2 == NULL){
        if(cor != NULL)
            *cor = false;
        return 0;
    }

    if(cor != NULL)
        *cor = true;

    bool end = false;
    int i = 0;
    while(!end)
    {
        if(str1[i] == str2[i]){
            if(str1[i] == 0)
                end = true;
            i++;
        }
        else if (str1[i] > str2[i])
            return (i+1);
        else
            return -(i+1);
    }

    return 0;
}

int strCmp(const char* str1, const char* str2)
{
    return strCmpC(str1, str2, NULL);
}

int getMainParam(MainParam* params, int argc, char* argv[], int offset)
{
    if(offset >= argc)
        return 0;

    bool check;

    if(strCmp(argv[offset], "-h") == 0 || strCmp(argv[offset], "--help") == 0){
        if(!params[HELP].enabled){
            params[HELP] = (MainParam){ true, 0 };
            return 1;
        }
    }
    else if(strCmp(argv[offset], "--extra") == 0){
        if(!params[EXTRA].enabled){
            params[EXTRA] = (MainParam){ true, 0 };
            return 1;
        }
    }
    else{
        int N = (int)strToLC(argv[offset], &check);
        if(check && !params[SORT].enabled){
            params[SORT] = (MainParam){ true, N };
            return 1;
        }
    }

    return 0;
}

bool checkParams(const MainParam* params)
{
    if(params[SORT].enabled && (params[SORT].val > MAX_ARR_SIZE || params[SORT].val < 1)){
        //zadane N nebolo z dovoleneho intervalu
        return false;
    }

    //zisti, ci nenastali konflikty v parametroch
    if(params[HELP].enabled && (params[SORT].enabled || params[EXTRA].enabled))
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
        fprintf(stderr, "%s", ECODEMSG[ECLWRONG]);
        return false;
    }
    else
        return true;
}

void printHelp()
{
    printf("%s", HELPMSG);
}
