/******************************************************************************
 * Projekt - Zaklady pocitacove grafiky - IZG
 * spanel@fit.vutbr.cz
 *
 * $Id:$
 *
 * Opravy a modifikace:
 * -
 */

#ifndef Bmp_H
#define Bmp_H

/******************************************************************************
 * Includes
 */

#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * Prace s grafickym formatem BMP
 */

/* Ulozeni obrazoveho bufferu do souboru ve formatu BMP.
 * filename - jmeno souboru
 * buffer - ukazatel na ukladana obrazova data
 * width, height - sirka a vyska obrazu
 * funkce vraci 0 - chyba, 1 - OK */
int saveBitmap(const char *filename, S_RGBA *buffer, int width, int height);


#ifdef __cplusplus
}
#endif

#endif // Bmp_H

/*****************************************************************************/
/*****************************************************************************/
