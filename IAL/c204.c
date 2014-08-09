
/* ******************************* c204.c *********************************** */
/*  P�edm�t: Algoritmy (IAL) - FIT VUT v Brn�                                 */
/*  �kol: c204 - P�evod infixov�ho v�razu na postfixov� (s vyu�it�m c202)     */
/*  Referen�n� implementace: Petr P�ikryl, listopad 1994                      */
/*  P�epis do jazyka C: Luk� Mar��k, prosinec 2012                           */
/*  Upravil: Bohuslav K�ena, ��jen 2013                                       */
/* ************************************************************************** */
/*
** Implementujte proceduru pro p�evod infixov�ho z�pisu matematick�ho
** v�razu do postfixov�ho tvaru. Pro p�evod vyu�ijte z�sobn�k (tStack),
** kter� byl implementov�n v r�mci p��kladu c202. Bez spr�vn�ho vy�e�en�
** p��kladu c202 se o �e�en� tohoto p��kladu nepokou�ejte.
**
** Implementujte n�sleduj�c� funkci:
**
**    infix2postfix .... konverzn� funkce pro p�evod infixov�ho v�razu
**                       na postfixov�
**
** Pro lep�� p�ehlednost k�du implementujte n�sleduj�c� funkce:
**    
**    untilLeftPar .... vypr�zdn�n� z�sobn�ku a� po levou z�vorku
**    doOperation .... zpracov�n� oper�toru konvertovan�ho v�razu
**
** Sv� �e�en� ��eln� komentujte.
**
** Terminologick� pozn�mka: Jazyk C nepou��v� pojem procedura.
** Proto zde pou��v�me pojem funkce i pro operace, kter� by byly
** v algoritmick�m jazyce Pascalovsk�ho typu implemenov�ny jako
** procedury (v jazyce C procedur�m odpov�daj� funkce vracej�c� typ void).
**
**/

#include "c204.h"

int solved;


/*
** Pomocn� funkce untilLeftPar.
** Slou�� k vypr�zdn�n� z�sobn�ku a� po levou z�vorku, p�i�em� lev� z�vorka
** bude tak� odstran�na. Pokud je z�sobn�k pr�zdn�, prov�d�n� funkce se ukon��.
**
** Oper�tory odstra�ovan� ze z�sobn�ku postupn� vkl�dejte do v�stupn�ho pole
** znak� postExpr. D�lka p�eveden�ho v�razu a t� ukazatel na prvn� voln�
** m�sto, na kter� se m� zapisovat, p�edstavuje parametr postLen.
**
** Aby se minimalizoval po�et p��stup� ke struktu�e z�sobn�ku, m��ete zde
** nadeklarovat a pou��vat pomocnou prom�nnou typu.
*/
void untilLeftPar ( tStack* s, char* postExpr, unsigned* postLen ) {
    char tmp;
    while(!stackEmpty(s)){
        // Vyjmi znak zo zasobnika
        stackTop(s, &tmp);
        stackPop(s);

        // V pripade lavej zatvorky ukonci cyklus
        if (tmp == '(')
            break;

        // Prirad znak do vyrazu a zvecsi dlzku
        postExpr[(*postLen)++] = tmp;
    }
}

/*
** Pomocn� funkce doOperation.
** Zpracuje oper�tor, kter� je p�ed�n parametrem c po na�ten� znaku ze
** vstupn�ho pole znak�.
**
** Dle priority p�edan�ho oper�toru a p��padn� priority oper�toru na
** vrcholu z�sobn�ku rozhodneme o dal��m postupu. D�lka p�eveden�ho
** v�razu a takt� ukazatel na prvn� voln� m�sto, do kter�ho se m� zapisovat,
** p�edstavuje parametr postLen, v�stupn�m polem znak� je op�t postExpr.
*/
void doOperation ( tStack* s, char c, char* postExpr, unsigned* postLen ) {
    char tmp;
    // Opakuj dokym nebude mozne vlozit operator do zasobnika
    while(1){
        // Zasobnik je prazdny, povolene vlozit operator.
        if(stackEmpty(s))
            break;
        else{
            stackTop(s, &tmp);
            // Na vrchole zasobniku je lava zatvorka, povolene
            // vlozit operator.
            if(tmp == '(')
                break;

            else if(c == '*' || c == '/'){
                // Na vrchole zasobniku je operator s nizsou prioritou,
                // povolene vlozit operator.
                if(tmp == '+' || tmp == '-')
                    break;
            }
        }
        // Na vrchole zasobniku je operator s rovnakou alebo vyssiou
        // prioritou. Vyjmi ho a uloz do vystupneho vyrazu.
        postExpr[(*postLen)++] = tmp;
        stackPop(s);
    }
    stackPush(s, c);
}

/* 
** Konverzn� funkce infix2postfix.
** �te infixov� v�raz ze vstupn�ho �et�zce infExpr a generuje
** odpov�daj�c� postfixov� v�raz do v�stupn�ho �et�zce (postup p�evodu
** hledejte v p�edn�k�ch nebo ve studijn� opo�e). Pam� pro v�stupn� �et�zec
** (o velikosti MAX_LEN) je t�eba alokovat. Volaj�c� funkce, tedy
** p��jemce konvertovan�ho �et�zce, zajist� korektn� uvoln�n� zde alokovan�
** pam�ti.
**
** Tvar v�razu:
** 1. V�raz obsahuje oper�tory + - * / ve v�znamu s��t�n�, od��t�n�,
**    n�soben� a d�len�. S��t�n� m� stejnou prioritu jako od��t�n�,
**    n�soben� m� stejnou prioritu jako d�len�. Priorita n�soben� je
**    v�t�� ne� priorita s��t�n�. V�echny oper�tory jsou bin�rn�
**    (neuva�ujte un�rn� m�nus).
**
** 2. Hodnoty ve v�razu jsou reprezentov�ny jednoznakov�mi identifik�tory
**    a ��slicemi - 0..9, a..z, A..Z (velikost p�smen se rozli�uje).
**
** 3. Ve v�razu m��e b�t pou�it p�edem neur�en� po�et dvojic kulat�ch
**    z�vorek. Uva�ujte, �e vstupn� v�raz je zaps�n spr�vn� (neo�et�ujte
**    chybn� zad�n� v�razu).
**
** 4. Ka�d� korektn� zapsan� v�raz (infixov� i postfixov�) mus� b�t uzav�en
**    ukon�ovac�m znakem '='.
**
** 5. P�i stejn� priorit� oper�tor� se v�raz vyhodnocuje zleva doprava.
**
** Pozn�mky k implementaci
** -----------------------
** Jako z�sobn�k pou�ijte z�sobn�k znak� tStack implementovan� v p��kladu c202.
** Pro pr�ci se z�sobn�kem pak pou��vejte v�hradn� operace z jeho rozhran�.
**
** P�i implementaci vyu�ijte pomocn� funkce untilLeftPar a doOperation.
**
** �et�zcem (infixov�ho a postfixov�ho v�razu) je zde my�leno pole znak� typu
** char, jen� je korektn� ukon�eno nulov�m znakem dle zvyklost� jazyka C.
**
** Na vstupu o�ek�vejte pouze korektn� zapsan� a ukon�en� v�razy. Jejich d�lka
** nep�es�hne MAX_LEN-1 (MAX_LEN i s nulov�m znakem) a tedy i v�sledn� v�raz
** by se m�l vej�t do alokovan�ho pole. Po alokaci dynamick� pam�ti si v�dycky
** ov��te, �e se alokace skute�n� zdra�ila. V p��pad� chyby alokace vra�te nam�sto
** �et�zce konstantu NULL.
*/
char* infix2postfix (const char* infExpr) {
    char *postExpr = malloc(MAX_LEN * sizeof(char));
    unsigned postLen = 0;

    tStack stack;
    stackInit(&stack);

    char tmp;
    for(unsigned i = 0; (tmp = infExpr[i]) != '='; i++){
        switch(tmp){
            case '+':
            case '-':
            case '*':
            case '/':
                // Zpracuj operator
                doOperation(&stack, tmp, postExpr, &postLen);
                break;
            case '(':
                // Pridaj lavu zatvorku do zasobniku.
                stackPush(&stack, '(');
                break;
            case ')':
                // Zpracuj pravu zatvorku.
                untilLeftPar(&stack, postExpr, &postLen);
                break;
            default:
                // Zpracuj hodnotu.
                // isalnum()
                if((tmp >= '0' && tmp <='9') || (tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z'))
                    postExpr[postLen++] = tmp;
                break;
        }
    }
    while(!stackEmpty(&stack)){
        stackTop(&stack, &tmp);
        postExpr[postLen++] = tmp;
        stackPop(&stack);
    }
    postExpr[postLen++] = '=';
    postExpr[postLen] = '\0';

    return postExpr;
}

/* Konec c204.c */
