//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 1.4.2012
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
#include <float.h>

#include "util.h"

const char *ECODEMSG[] =
{
  [EOK] = "Vsetko v poriadku.\n",
  [ECLWRONG] = "Chybne parametry prikazovej riadky!\n",
  [EUNKNOWN] = "Nastala neocakavana chyba."
};

const char *HELPMSG =
    "Program Santa a elfovia (aneb 31. marca na snehu).\n"
    "Autor: Ivan Sevcik (c) 2013\n\n"
    "Popis:\n"
    "Program pre riesenie synchronizacneho problemu: Santa Claus Problem\n"
    "http://www.fit.vutbr.cz/study/courses/IOS/public/Lab/projekty/projekt2/projekt2.html\n\n"
    "Pouzitie: santa --help\n"
    "          santa -h         Zobrazi napovedu (zhodne s --help).\n"
    "          santa C E H S    Spusti simulaciu. \n"
    "              C, E, H, S su celociselne arguemtny:\n"
    "              C - Pocet navstev pred dovolenkou. C > 0\n"
    "              E - Pocet skriatkov. E > 0\n"
    "              H - Maximalna doba (v ms) do problemu. H >= 0\n"
    "              S - Maximalna doba (v ms) pre obsluhu u Santu. S >= 0\n"
    "";

int getMainParam(MainParam* params, int argc, char* argv[], int offset)
{
    if(offset >= argc)
        return 0;

    char* readPtr;

    if(strcmp(argv[offset], "-h") == 0 || strcmp(argv[offset], "--help") == 0){
        if(!params[HELP].enabled){
            params[HELP] = (MainParam){ true, {0, 0, 0, 0} };
            return 1;
        }
    }
    else{
        if(!params[SOLVE].enabled && argc - offset >= 3){
            int C = (int)strtol(argv[offset + 0], &readPtr, 10);
            if (*readPtr != 0) return 0;

            int E = (int)strtol(argv[offset + 1], &readPtr, 10);
            if (*readPtr != 0) return 0;

            int H = (int)strtol(argv[offset + 2], &readPtr, 10);
            if (*readPtr != 0) return 0;

            int S = (int)strtol(argv[offset + 3], &readPtr, 10);
            if (*readPtr != 0) return 0;

            params[SOLVE] = (MainParam){ true, {C, E, H, S} };
            return 4;
        }
    }

    return 0;
}

bool checkParams(const MainParam* params)
{
    //zisti, ci nenastali konflikty v parametroch
    //v tejto ulohe je povoleny prave 1 parameter
    int paramsEnabled = params[HELP].enabled + params[SOLVE].enabled;
    if(paramsEnabled != 1)
        return false;

    if(params[SOLVE].enabled && (params[SOLVE].val[0] <= 0 ||
                                 params[SOLVE].val[1] <= 0 ||
                                 params[SOLVE].val[2] < 0  ||
                                 params[SOLVE].val[3] < 0  )) return false;

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
