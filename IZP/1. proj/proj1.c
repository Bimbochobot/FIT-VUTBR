//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
//// Popis:
//// Subor obsahujuci funckiu main a funkcie
//// relevantne pre tento konkretny projekt.

#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#include "proj1util.h"
#include "proj1sort.h"

/**
 * @brief Inicializuje histogram do vychodiskoveho stavu.
 * @param[out] histogram Histogram urceny k inicializacii.
 */
void initHisto(HistoChar histogram[])
{
    for(int i = 0; i < MAX_ARR_SIZE; i++)
        histogram[i] = (HistoChar){i, 0};
}

/**
 * @brief Precita vstup a zaplni histogram.
 * @param[out] histogram Histogram urceny k zaplneniu.
 */
int analyzeInput(HistoChar histogram[])
{
    int c = 0;
    int total = 0;
    while( (c = getchar()) != EOF){
        if(histogram[c].count <= UCHAR_MAX-1){
            histogram[c].count++;
            total++;
        }
    }

    return total;
}

/**
 * @brief Vypise hlavicku grafickej tabulky
 */
void printHeader()
{
    //pridaj par riadkov medzi vstup a vystup
    printf("\n\n\n\n\n");
    printf("+ORD+CH+CNT+");
    for(int i = 0; i < 21; i++)
        putchar('-');
    printf("HISTOGRAM");
    for(int i = 0; i < 20; i++)
        putchar('-');
    printf("+PERC+\n");
}

/**
 * @brief Vypise petu grafickej tabulky
 */
void printFooter()
{
    printf("+---+--+---+");
    for(int i = 0; i < 50; i++)
        putchar('-');
    printf("+----+\n");
}

/**
 * @brief Vypise 1 riadok histogramu v pozadovanom formate.
 * @param histoChar Znak z histogramu, ktory sa ma vypisat.
 * @param testRelevant Prepina ci sa ma testovat relevantnost riadku
 * @param total Obsahuje celkovy pocet nacitanych znakov pre rozsirenie
 */
void printHistoLine(HistoChar histoChar, bool testRelevant, int total)
{
    if(testRelevant && histoChar.count == 0)
        return;

    if(total > 0){ //vytlac tabulku

        //vytlac informacie
        printf("|%3d| %c|", histoChar.ord,
               isprint(histoChar.ord) ? histoChar.ord : ' ');
        if(histoChar.count > UCHAR_MAX-1)
            printf("NNN|");
        else
            printf("%3d|", histoChar.count);

        //znazorni histogram
        int fill = (histoChar.count*50)/total;
        for(int i = 0; i < fill; i++)
            putchar('#');
        for(int i = fill; i < 50; i++)
            putchar(' ');

        //vytlac percenta cislom
        printf("|%3d%%|\n", (histoChar.count*100)/total);
    }
    else{ //vypis podla zadania
        if(isprint(histoChar.ord))
            printf("%d '%c': ", histoChar.ord, histoChar.ord);
        else
            printf("%d: ", histoChar.ord);


        if(histoChar.count > UCHAR_MAX-1)
            printf("NNN\n");
        else
            printf("%d\n", histoChar.count);
    }
}

/**
 * @brief Vypise histogram na standardny vystup.
 * @param histogram Histogram urceny k vypisu.
 * @param count Pocet riadkov k vypisu.
 * @param testRelevant Prepina ci sa ma testovat relevantnost riadku
 * @param total Obsahuje celkovy pocet nacitanych znakov pre rozsirenie
 */
void printHisto(HistoChar histogram[], int count, bool testRelevant, int total)
{
    if(total > 0) printHeader();

    for(int i = 0; i < count; i++)
        printHistoLine(histogram[i], testRelevant, total);

    if(total > 0) printFooter();
}

/**
 * @brief Najde dalsii najpocetnejsi znak v histograme.
 * @param[in] histogram Histogram, v ktorom sa hlada.
 * @param length Dlzka histogramu.
 * @param lastMax Pozicia posledneho maxima.
 *        Zadanim -1 sa vyhlada prve maximum v histograme.
 * @return Vrati poziciu dalsieho najpocetnejsieho znaku.
 */
int findNextMax(HistoChar histogram[], int length, int lastMax)
{
    int max = 0;
    unsigned char limit;
    if(lastMax < 0)
        limit = UCHAR_MAX;
    else
        limit = histogram[lastMax].count;

    for(int i = 0; i < length; i++)
    {
        if(histogram[i].count == limit && i > lastMax)
            return i;

        if(histogram[i].count < limit && histogram[i].count > histogram[max].count)
            max = i;
    }

    return max;
}

int main(int argc, char* argv[])
{
    //priprav miesto pre vsetky mozne parametre
    MainParam params[PARAM_N];
    for(int i = 0; i < PARAM_N; i++)
        params[i] = (MainParam){false, 0};

    if(!procMainArg(params, argc, argv))
        return 1;

    if(params[HELP].enabled){
        printHelp();
        return 0;
    }

    HistoChar histogram[MAX_ARR_SIZE];
    initHisto(histogram);
    int total = analyzeInput(histogram);

    total = params[EXTRA].enabled ? total : -1;
    if(params[SORT].enabled){
        //pre N vecsie ako 8 je vyhodnejsie zotriedit cele pole (n * log(n))
        if(params[SORT].val > 8){
            mergeSort(histogram, MAX_ARR_SIZE);
            printHisto(histogram, params[SORT].val, true, total);
        }
        else{
            int max = -1;
            if(params[EXTRA].enabled) printHeader();
            for(int i = 0; i < params[SORT].val; i++){
                max = findNextMax(histogram, MAX_ARR_SIZE, max);
                if(max == 0)
                    break;
                printHistoLine(histogram[max], false, total);
            }
            if(params[EXTRA].enabled)printFooter();
        }
    }
    else
        printHisto(histogram, MAX_ARR_SIZE, true, total);

    return 0;
}
