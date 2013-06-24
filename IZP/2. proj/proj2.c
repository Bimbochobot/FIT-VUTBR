//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 27.10.2012
//// Popis:
//// Subor obsahujuci funckiu main a funkcie
//// relevantne pre tento konkretny projekt.

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <math.h>

#include "proj2util.h"
#include "proj2math.h"

void procInput(MainParam* params)
{
    int prec = 0;
    if(params[POW].enabled) prec = params[POW].val[0];
    else if(params[TAN].enabled) prec = params[TAN].val[0];
    else if(params[SINH].enabled) prec = params[SINH].val[0];

    double eps = 1;
    for(int i = 0; i < prec; i++)
        eps *= 0.1;

    double x;
    while(feof(stdin) == 0){
        if(scanf("%lf", &x) == EOF) return;
        while(feof(stdin) == 0 && isspace(getchar()) == 0) x = NAN; //vycisti vstup
        if(params[POW].enabled) printf("%.10e\n", m_pow(x, params[POW].val[1], eps));
        else if(params[TAN].enabled) printf("%.10e\n", m_atan(x, eps));
        else if(params[SINH].enabled) printf("%.10e\n", m_argsinh(x, eps));
    }
}

int main(int argc, char* argv[])
{
    //priprav miesto pre vsetky mozne parametre
    MainParam params[PARAM_N];
    for(int i = 0; i < PARAM_N; i++)
        params[i] = (MainParam){false, {0,0}};

    if(!procMainArg(params, argc, argv))
        return 1;

    if(params[HELP].enabled){
        printHelp();
        return 0;
    }

    procInput(params);

    return 0;
}
