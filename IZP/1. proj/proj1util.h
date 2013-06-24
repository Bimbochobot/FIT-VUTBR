//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
//// Popis:
//// Hlavickovy subor obsahujuci deklaracie
//// vyctov, struktur a funkcii prevazne
//// znovu vyuzitelnych pri dalsich projektoch

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdbool.h>

#define MAX_ARR_SIZE 256

typedef enum { HELP, SORT, EXTRA, PARAM_N} MainParamCodes;
typedef enum { EOK, ECLWRONG, EUNKNOWN } ErrorCodes;

/**
 * @brief Struktura uchovavajuca udaje ohladom 1 znaku histrogramu.
 */
typedef struct
{
    unsigned char ord;
    unsigned char count;
} HistoChar;

/**
 * @brief Struktura uchovavajuca 1 parameter programu.
 */
typedef struct
{
    bool enabled;
    int val;
} MainParam;

/**
 * @brief Prevedie c-string na long.
 * @param[in] str Prevadzany string.
 * @param[out] cor Priznak, ci operacia prebehla korektne.
 * @return Cislo zodpovedajuce c-string.
 */
long strToLC(const char* str, bool* cor);

/**
 * @brief Prevedie c-string na long.
 * @param[in] str Prevadzany string.
 * @return Cislo zodpovedajuce c-string.
 */
long strToL(const char* str);

/**
 * @brief Porovna 2 stringy na zhodnost.
 * @param str1 Prvy porovavany string.
 * @param str2 Druhy porovnavany string.
 * @param[out] cor Priznak, ci operacia prebehla korektne.
 * @return 0 ak boli stringy rovnake.
 *         Kladne cislo, ak bol prvy nezhodujuci sa znak v str1
 *         vecsi ako v str2. Zaporne v opacnom pripade.
 *         Absolutna hodnota oznacuje index+1, v ktorom sa
 *         stringy prvy krat nezhoduju.
 */
int strCmpC(const char* str1, const char* str2, bool* cor);

/**
 * @brief Porovna 2 stringy na zhodnost.
 * @param str1 Prvy porovavany string.
 * @param str2 Druhy porovnavany string.
 * @return 0 ak boli stringy rovnake.
 *         Kladne cislo, ak bol prvy nezhodujuci sa znak v str1
 *         vecsi ako v str2. Zaporne v opacnom pripade.
 *         Absolutna hodnota oznacuje index+1, v ktorom sa
 *         stringy prvy krat nezhoduju.
 */
int strCmp(const char* str1, const char* str2);

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
 * @brief Skontroluje ziskany parameter programu.
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
