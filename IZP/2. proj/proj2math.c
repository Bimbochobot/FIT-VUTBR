//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 27.10.2012
//// Popis:
//// Subor obsahujuci implementacie matematickych
//// funkcii a potrebne konstanty.

#include "proj2math.h"
#include <stddef.h>
#include <stdlib.h>
#include <float.h>

const double IZP_E = 2.7182818284590452354;        // e
const double IZP_PI = 3.14159265358979323846;      // pi
const double IZP_2PI = 6.28318530717958647692;     // 2*pi
const double IZP_PI_2 = 1.57079632679489661923;    // pi/2
const double IZP_PI_4 = 0.78539816339744830962;    // pi/4

bool m_equal(double x, double y)
{
    double absX = m_abs(x), absY = m_abs(y);
    return m_abs(x-y) <= ((absX < absY ? absX : absY) * DBL_EPSILON);
}

double m_signOf(double x){
    return x < 0 ? -1 : x > 0 ? 1 : 0;
}

double m_abs(double x)
{
    return x > 0 ? x : -x;
}

//Pouzita takzvana babylonska metoda, odvoditelna z Newtonovej.
//Velmi dobre konverguje - pocet platnych cislic sa v kazdom kroku
//priblizne zdvojnasobuje
double m_sqrt(double x, double eps)
{
    if(isnan(x)) return NAN;
    if(isinf(x)) return INFINITY;
    if(m_equal(x, 0)) return 0;
    if(m_equal(x, 1)) return 1;
    if(x < 0) return NAN;

    double y, nextMem = 1;
    do{
        y = nextMem;
        nextMem = (y + x/y) * 0.5;
    }while(m_abs(nextMem - y) > m_abs(eps*y));

    return y;
}

//Matematicka implementacia binarneho logaritmu je
//pomerne pomala. Pri pouziti znalosti o zapise
//desatinneho cisla by bolo mozne dosiahnut ovela
//lepsiu efektivitu.
double m_log2(double x, double eps)
{
    if(isnan(x)) return NAN;
    if(isinf(x)) return INFINITY;
    if(x <= 0) return NAN;
    if(m_equal(x, 1)) return 0;

    double y = 0, nextMem = 0, fraction = 1;
    //najdi celociselnu cast logaritmu
    while(x >= 2){
        x *= 0.5;
        y++;
    }
    while(x < 1){
        x *= 2;
        y--;
    }

    //najdi logaritmus desatinnej casti ( x patri <1,2) )
    nextMem = 0;
    do{
        y += nextMem;

        //skontroluj, ci x == 1 (nasledoval by nekonecny cyklus)
        if(m_equal(x, 1)) return y;

        //umocnuj x pokym nie je z intervalu <2,4)
        while(x < 2){
            x *= x;
            fraction *= 0.5;
        }
        nextMem = fraction;

        //vrat x do intervalu <1,2)
        x *= 0.5;
    }while(m_abs(nextMem) > m_abs(eps*y));

    return y;
}

double m_ln(double x, double eps)
{
    if(isnan(x)) return NAN;
    if(isinf(x)) return INFINITY;
    if(x <= 0) return NAN;
    if(m_equal(x, 1)) return 0;

    //x treba upravit do intervalu konvergencie
    //rozvoj konverguje v rozsahu (0, 2) a najlepsie
    //okolo bodu x = 1
    int add = 0;
    while(x > 1.5){
        x /= IZP_E;
        add += 1.0;
    }
    while(x < 0.5){
        x *= IZP_E;
        add -= 1.0;
    }

    double y = 0, b = (x-1)/(x+1), a = b*b, nextMem = 0;
    double n = 1.0;
    do{
        y += nextMem;
        nextMem = b / n;
        b *= a;
        n += 2.0;
    }while(m_abs(nextMem) > m_abs(eps*y));

    return (2*y) + add;
}

double m_pow(double x, double exp, double eps)
{
    if(m_equal(exp, 0)) return 1;
    if(isnan(x)) return NAN;
    if(m_equal(x, 1)) return 1;
    if(isnan(exp)) return NAN;
    if(m_equal(x, 0)){
        if(exp < 0) return INFINITY;
        else return 0;
    }
    if(exp == INFINITY){
        if(m_equal(x, -1)) return 1;
        else return INFINITY;
    }
    if(exp == -INFINITY){
        if(x < 0 && !isinf(x)) return NAN;
        else return 0;
    }
    if(m_equal(exp, 1)) return x;
    if(isinf(x)){
        if(exp < 0) return 0;
        else return INFINITY;
    }

    bool neg = false;
    if(x < 0){
        //test na celociselnost exponentu
        //ak je cislo rovne zaokruhlenemu cislu, tak je cele
        if(m_equal(exp, (int)(exp+m_signOf(exp)*0.5))){
            x = -x;
            neg = ((int)exp)%2;
        }
        else
            return NAN;
    }

    //vypocet exponencialnej funckie
    double multiplier = 1;
    exp = exp * m_ln(x, eps);

    //vypocet celej casti exponentu
    while(exp > 1){
        multiplier *= IZP_E;
        if(isinf(multiplier)) return INFINITY;
        exp--;
    }
    while(exp < -1){
        multiplier /= IZP_E;
        if(multiplier == 0) return 0;
        exp++;
    }

    //dopocitanie desatinnej casti pomocou rozvoja
    double y = 0, nextMem = 1;
    int n = 1;
    do{
        y += nextMem;
        nextMem *= (exp/n);
        n++;
    }while(m_abs(nextMem) > m_abs(eps*y));

    y *= multiplier;
    return neg ? -y : y;
}

double m_atan(double x, double eps)
{
    if(isnan(x)) return NAN;
    if(m_equal(x, 0)) return 0;
    if(m_equal(x, 1)) return IZP_PI_4;
    if(m_equal(x, -1)) return -IZP_PI_4;

    double y = 0, sqrX, a, nextMem;
    double n = 1.0;

    //Taylorov rozvoj je dostatocne presny, ale pre male X
    //Zvolena hranica je 0.2 - konverguje rozumne rychlo
    //Pre x >= 5 sa pouzije vztah atan(x) = PI_2 - atan(1/x)
    //Pre x < 5 je potrebne dostat x do intervalu (0; 0,2)
    //a to dosiahnem pomocou atan(x) = 2*atan(x/(1+sqrt(1+x*x)))

    if(m_abs(x) < 5){
        double multiplier = 1;
        while(m_abs(x) > 0.2){
            x = x/(1.0 + m_sqrt(1.0+x*x, eps));
            multiplier *= 2.0;
        }

        sqrX = x*x;
        a = x, nextMem = x;

        //opravit rozvoj
        do{
            y += nextMem;
            a *= -sqrX;
            n += 2.0;
            nextMem = a / n;
        }while(m_abs(nextMem) > m_abs(eps*y));

        y = multiplier * y;
    }
    else{
        sqrX = x*x;
        a = -x, nextMem = m_signOf(x)*IZP_PI_2 - 1.0/x;

        do{
            y += nextMem;
            a *= -sqrX;
            n += 2.0;
            nextMem = 1.0 / (n * a);
        }while(m_abs(nextMem) > m_abs(eps*y));
    }

    return y;
}

double m_argsinh(double x, double eps)
{
    //pre male x, ktore by sa nezmestili do presnosti double
    //vyuzijem vztahu argsinh(x) ~ x
    if(m_equal(x + 1.0, 1.0))
        return x;

    //pre velke x, ktore by naopak vracali po umocneni nekonecno
    //zanedbam odmocninu s pripocitanim 1 a pocitam priamo logaritmus
    if(x > m_sqrt(DBL_MAX, eps)) // v buducnosti moze byt konstanta
        return m_ln( 2 * x, eps);
    else
        return m_ln( x + m_sqrt(x*x + 1, eps), eps );
}
