//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
//// Popis:
//// Hlavickovy subor obsahujuci deklaracie
//// vyctov, struktur a funkcii prevazne
//// znovu vyuzitelnych pri dalsich projektoch.

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdbool.h>

//zakladna velkost abecedy + '\0' pre ukoncovanie retazcov
#define ALPHABET_SIZE 27
//rozsirenie abecedy
#define CHAR_CH 27

//jednotna velkost pre buffer
#define BUFFER_SIZE 50

//macro na konverziu suradnic matice do pola
#define idxMat2(c, r, cols) (c + r*cols)

typedef enum { HELP, TEST, SEARCH, SOLVE, PARAM_N} MainParamCodes;
typedef enum { EOK, ECLWRONG, EWRONGFORMAT, ENOTFOUND, EBADWORD,
               ELONGWORD, ENOSOLUTION , EALLOC, EUNKNOWN } ErrorCodes;
typedef enum { DIRNULL, DIRU, DIRUR, DIRR, DIRDR, DIRD, DIRDL, DIRL,
               DIRUL, DIR_N} Directions;

/**
 * @brief Struktura uchovavajuca 1 parameter programu.
 */
typedef struct
{
    bool enabled;
    const char* options[2];
} MainParam;

/**
 * @brief Struktura uchovavajuca osemsmerovku.
 */
typedef struct
{
    unsigned int cols, rows, size;
    char* data;
    bool* mark; //Kompaktnejsie riesenie by bolo cez bitove pole
} Matrix;

/**
 * @brief Struktura uchovavajuca informacie o polohe slova.
 */
typedef struct
{
    unsigned int pos;
    Directions dir;
} WordVector;

/**
 * @brief Struktura uchovavajuca dynamicky string.
 */
typedef struct
{
    unsigned int cap;
    char* data;
} String;


/**
 * @brief Zvysi dostupnu kapacitu dynamickeho stringu.
 * @param str Dynamicky string
 */
bool incStrCap(String* str);

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

/**
 * @brief Vypise chybovu hlasku.
 * @param code Identifikator chybovej hlasky.
 * @param detail Doplnujuce informacie k chybe.
 */
void printError(int code, const char* detail);

#endif // UTIL_H_INCLUDED
