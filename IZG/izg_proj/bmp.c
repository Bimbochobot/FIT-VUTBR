/******************************************************************************
 * Projekt - Zaklady pocitacove grafiky - IZG
 * spanel@fit.vutbr.cz
 *
 * $Id:$
 *
 * Opravy a modifikace:
 * -
 */

#include "bmp.h"

#include <math.h>
#include <stdio.h>

/* Platform-specific includes */
#ifdef _WIN32
#   include <windows.h>
#else
#   include <sys/types.h>
#endif


/****************************************************************************
 * Struktury pro praci s graficky formatem BMP
 */

/* Zarovnani scanline na 4 byty */
#define DIB_SCAN_LINE(width) (((width) & 0xfffffffc) + (((width) & 0x3) ? 4 : 0))

/* Windows maji nasledujici struktury definovany ve windows.h */
#ifndef _WIN32

    #pragma pack(2)        /* zarovnani pameti na 2byty */

    typedef __int16_t WORD;     /* 2 byte */
    typedef __int32_t DWORD;    /* 4 byte */

    /* Hlavicka BMP souboru */
    typedef struct
    {
        WORD           bfType;           /* oznaceni BM - typ souboru - 2byte */
        DWORD          bfSize;           /* velikost souboru - 4byte */
        WORD           bfReserved1;      /* rezervovane */
        WORD           bfReserved2;      /* rezervovane */
        DWORD          bfOffBits;        /* offset na obrazova data */
    } BITMAPFILEHEADER;

    /* Informace o formatu ulozeneho obrazu */
    typedef struct
    {
        DWORD          biSize;           /* velikost informacni hlavicky */
        DWORD          biWidth;          /* sirka obrazu */
        DWORD          biHeight;         /* vyska obrazu */
        WORD           biPlanes;         /* pocet barevnych rovin */
        WORD           biBitCount;       /* pocet bitu na pixel */
        DWORD          biCompression;    /* typ pouzite komprese */
        DWORD          biSizeImage;      /* velikost obrazovych dat */
        DWORD          biXPelsPerMeter;  /* pocet X pixelu na metr */
        DWORD          biYPelsPerMeter;  /* pocet Y pixelu na metr */
        DWORD          biClrUsed;        /* pocet pouzitych barev */
        DWORD          biClrImportant;   /* pocet dulezitych barev */
    } BITMAPINFOHEADER;

    #pragma pack()         /* default memory alignment */

    /* Mozne hodnoty biCompression - typ pouzite komprese dat */
    #define BI_RGB       0               /* zadna komprese */
    #define BI_RLE8      1               /* 8-bit RLE */
    #define BI_RLE4      2               /* 4-bit RLE */
    #define BI_BITFIELDS 3               /* RGB bitove pole s maskou */

#endif // _WIN32


/****************************************************************************
 * Ulozeni obsahu obrazovky do souboru ve formatu BMP
 */ 

int saveBitmap(const char * filename, S_RGBA * buffer, int width, int height)
{
    BITMAPFILEHEADER head;
    BITMAPINFOHEADER info;
    int x, y, zar;
    FILE * fw;

    fw = fopen(filename, "wb");
    if( fw == NULL )
    {
        fprintf(stderr, "<%s, %d> Error: Nelze ulozit obrazek do souboru %s", __FILE__, __LINE__, filename);
        return 0;
    }
    
    head.bfType = 0x4d42;
    head.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + DIB_SCAN_LINE(width * 3) * height;
    head.bfReserved1 = 0;
    head.bfReserved2 = 0;
    head.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    info.biSize = sizeof(BITMAPINFOHEADER);
    info.biWidth = width;
    info.biHeight = height;
    info.biPlanes = 1;
    info.biBitCount = 24;
    info.biCompression = 0;
    info.biSizeImage = DIB_SCAN_LINE(width * 3) * height;
    info.biXPelsPerMeter = 0;
    info.biYPelsPerMeter = 0;
    info.biClrUsed = 0;
    info.biClrImportant = 0;
    fwrite(&head, sizeof(BITMAPFILEHEADER), 1, fw);
    fwrite(&info, sizeof(BITMAPINFOHEADER), 1, fw);
    
    zar = DIB_SCAN_LINE(info.biWidth * 3) - info.biWidth * 3;
    for( y = info.biHeight - 1; y >= 0; --y )
    {
        for( x = 0; x < info.biWidth; ++x )
        {
            S_RGBA color = *(buffer + y * width + x);
            fputc(color.blue, fw);
            fputc(color.green, fw);
            fputc(color.red, fw);
        }
        for( x = 0; x < zar; ++x )
        {
            fputc(0, fw);
        }
    }
    fclose(fw);
    
    return 1;
}


/*****************************************************************************/
/*****************************************************************************/
