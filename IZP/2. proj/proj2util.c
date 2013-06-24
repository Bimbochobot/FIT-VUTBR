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
#include <float.h>

#include "proj2util.h"

const char *ECODEMSG[] =
{
  [EOK] = "Vsetko v poriadku.\n",
  [ECLWRONG] = "Chybne parametry prikazovej riadky!\n",
  [EUNKNOWN] = "Nastala neocakavana chyba."
};

const char *HELPMSG =
    "Program Iteracne vypocty.\n"
    "Autor: Ivan Sevcik (c) 2012\n\n"
    "Popis:\n"
    "Program prepocita hodnoty na vstupe pomocou zvolenej funkcie.\n\n"
    "Pouzitie: proj1 --help\n"
    "          proj1 -h         Zobrazi napovedu (zhodne s --help).\n"
    "          proj1 --powxa [sigdig, a] Pouzije exponencialnu funkciu\n"
    "                                    (umocni cisla na exponent [a]).\n"
    "          proj1 --arctg [sigdig]    Pouzije inverznu funkciu\n"
    "                                    k tangens (arcus tangens).\n"
    "          proj1 --argsinh [sigdig]  Pouzije funckiu argument\n"
    "                                    sinusu hyperbolickeho.\n"
    "Hodnota sigdig urcuje presnost vypoctov ako pocet platnych cifier.\n"
    "Za vstupnu hodnotu je povazovany obsah medzi seriami bielych znakov.\n"
    "Ak v obsahu bude nepovoleny znak alebo nespravny format cisla,\n"
    "hodnota bude nastavena na NAN a prejde sa na najblizsi biely znak.\n"
    "";

int getMainParam(MainParam* params, int argc, char* argv[], int offset)
{
    if(offset >= argc)
        return 0;

    char* readPtr;
    double p, exp;

    if(strcmp(argv[offset], "-h") == 0 || strcmp(argv[offset], "--help") == 0){
        if(!params[HELP].enabled){
            params[HELP] = (MainParam){ true, {0, 0} };
            return 1;
        }
    }
    if(strcmp(argv[offset], "--powxa") == 0 && offset + 2 < argc){
        if(!params[POW].enabled){

            p = (double)strtol(argv[offset+1], &readPtr, 10);
            if (*readPtr != 0) return 0;

            exp = strtod(argv[offset+2], &readPtr);
            if (*readPtr != 0) return 0;

            params[POW] = (MainParam){ true, {p, exp} };
            return 3;
        }
    }
    if(strcmp(argv[offset], "--arctg") == 0 && offset + 1 < argc){
        if(!params[TAN].enabled){

            p = (double)strtol(argv[offset+1], &readPtr, 10);
            if (*readPtr != 0) return 0;

            params[TAN] = (MainParam){ true, {p, 0} };
            return 2;
        }
    }
    if(strcmp(argv[offset], "--argsinh") == 0 && offset + 1 < argc){
        if(!params[SINH].enabled){

            p = (double)strtol(argv[offset+1], &readPtr, 10);
            if (*readPtr != 0) return 0;

            params[SINH] = (MainParam){ true, {p, 0} };
            return 2;
        }
    }

    return 0;
}

bool checkParams(const MainParam* params)
{
    //zisti, ci nenastali konflikty v parametroch
    //v tejto ulohe je povoleny prave 1 parameter
    int paramsEnabled = params[HELP].enabled + params[POW].enabled
                      + params[TAN].enabled + params[SINH].enabled;
    if(paramsEnabled != 1)
        return false;

    if(params[POW].enabled && (params[POW].val[0] <= 0 ||
                               params[POW].val[0] > DBL_DIG)) return false;
    if(params[TAN].enabled && (params[TAN].val[0] <= 0 ||
                               params[TAN].val[0] > DBL_DIG)) return false;
    if(params[SINH].enabled && (params[SINH].val[0] <= 0 ||
                                params[SINH].val[0] > DBL_DIG)) return false;

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
