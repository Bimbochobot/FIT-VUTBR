//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
//// Popis:
//// Hlavickovy subor obsahujuci deklaraciu
//// radiaceho algoritmu

#ifndef SORT_H_INCLUDED
#define SORT_H_INCLUDED

#include "proj1util.h"

/**
 * @brief Zotriedi pole pomocou merge sortu. Najhorsia zlozitost O(n*log(n)).
 * @param arr Pole, ktore bude sa bude triedit.
 * @param length Velkost pola.
 * @todo Zovseobecnit pouzitim ukazatela na funkciu porovnania.
 */
void mergeSort(HistoChar arr[], int length);

#endif // SORT_H_INCLUDED
