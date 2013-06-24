//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 18.10.2012
//// Popis:
//// Subor obsahujuci implementaciu radiacej
//// a potrebnych obsluznych funckii

#include "proj1sort.h"

int compare(HistoChar val1, HistoChar val2)
{
    int dir = -1;

    //bezne porovnanie hodnot
    if(val1.count < val2.count)
        return 1 * dir;
    else if(val1.count == val2.count)
        return 0;
    else
        return -1 * dir;
}

void merge(HistoChar arr[], HistoChar temp[], int offset, int length)
{
    int cur = offset;
    int l = offset, r = offset + length/2;
    int lm = offset + length/2, rm = offset + length;

    //usporiadaj prelinanim jednotlivych polovic
    while(l < lm && r < rm){
        int cmp = compare(temp[l], temp[r]);
        if(cmp >= 0){
            arr[cur] = temp[l];
            l++;
            cur++;
        }
        else{
            arr[cur] = temp[r];
            r++;
            cur++;
        }
    }

    //ak zvysili prvky v niektorej z polovic, zarad ich nakoniec
    for(; l < lm; l++, cur++)
        arr[cur] = temp[l];

    for(; r < rm; r++, cur++)
        arr[cur] = temp[r];
}

void mergeSortImpl(HistoChar arr[], HistoChar temp[], int offset, int length)
{
    int half = length/2;
    if(half != 0){
        mergeSortImpl(temp, arr, offset, half);
        mergeSortImpl(temp, arr, offset + half, length-half);
    }

    merge(arr, temp, offset, length);
}

void mergeSort(HistoChar arr[], int length)
{
    HistoChar temp[MAX_ARR_SIZE];

    //skopiruj obsah pola do pomocneho pola
    for(int i = 0; i < length; i++)
        temp[i] = arr[i];

    //zorad pole
    mergeSortImpl(arr, temp, 0, length);
}
