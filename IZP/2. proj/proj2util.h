//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
//// Popis:
//// Hlavickovy subor obsahujuci deklaracie
//// vyctov, struktur a funkcii prevazne
//// znovu vyuzitelnych pri dalsich projektoch.

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdbool.h>

typedef enum { HELP, POW, TAN, SINH, PARAM_N} MainParamCodes;
typedef enum { EOK, ECLWRONG, EUNKNOWN } ErrorCodes;

/**
 * @brief Struktura uchovavajuca 1 parameter programu.
 */
typedef struct
{
    bool enabled;
    double val[2];
} MainParam;

/**
 * @brief Ziska 1 parameter zo vstupnych argumentov.
 * @param[out] params Parametre programu.
 * @param argc Celkovy pocet argumentov.
 * @param argv Argumenty programu.
 * @param offset Odsadenie, od ktoreho sa budu argumenty spracovavat.
 * @return Pocet spracovanych argumentov pri ziskavani parametru.
 *         0 zmamena, ze ziadny parameter nebol ziskany.
 */
int getMainParam(MainParam* params, int argc, char* argv[], int offset);

/**
 * @brief Skontroluje ziskane parametre programu.
 * @param[in] params Parametre programu.
 * @return True ak su parametre vporiadku.
 */
bool checkParams(const MainParam* params);

/**
 * @brief Spracuje argumenty do parametrov.
 * @param[out] params Parametre programu.
 * @param argc Celkovy pocet argumentov.
 * @param argv Argumenty programu.
 * @return False ak sa pri spracovavani vyskytla chyba.
 */
bool procMainArg(MainParam* params, int argc, char* argv[]);

/**
 * @brief Vypise napovedu programu.
 */
void printHelp();

#endif // UTIL_H_INCLUDED
