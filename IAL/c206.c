	
/* c206.c **********************************************************}
{* Téma: Dvousmìrnì vázaný lineární seznam
**
**                   Návrh a referenèní implementace: Bohuslav Køena, øíjen 2001
**                            Pøepracované do jazyka C: Martin Tuèek, øíjen 2004
**                                             Úpravy: Bohuslav Køena, øíjen 2013
**
** Implementujte abstraktní datový typ dvousmìrnì vázaný lineární seznam.
** U¾iteèným obsahem prvku seznamu je hodnota typu int.
** Seznam bude jako datová abstrakce reprezentován promìnnou
** typu tDLList (DL znamená Double-Linked a slou¾í pro odli¹ení
** jmen konstant, typù a funkcí od jmen u jednosmìrnì vázaného lineárního
** seznamu). Definici konstant a typù naleznete v hlavièkovém souboru c206.h.
**
** Va¹ím úkolem je implementovat následující operace, které spolu
** s vý¹e uvedenou datovou èástí abstrakce tvoøí abstraktní datový typ
** obousmìrnì vázaný lineární seznam:
**
**      DLInitList ...... inicializace seznamu pøed prvním pou¾itím,
**      DLDisposeList ... zru¹ení v¹ech prvkù seznamu,
**      DLInsertFirst ... vlo¾ení prvku na zaèátek seznamu,
**      DLInsertLast .... vlo¾ení prvku na konec seznamu,
**      DLFirst ......... nastavení aktivity na první prvek,
**      DLLast .......... nastavení aktivity na poslední prvek,
**      DLCopyFirst ..... vrací hodnotu prvního prvku,
**      DLCopyLast ...... vrací hodnotu posledního prvku,
**      DLDeleteFirst ... zru¹í první prvek seznamu,
**      DLDeleteLast .... zru¹í poslední prvek seznamu,
**      DLPostDelete .... ru¹í prvek za aktivním prvkem,
**      DLPreDelete ..... ru¹í prvek pøed aktivním prvkem,
**      DLPostInsert .... vlo¾í nový prvek za aktivní prvek seznamu,
**      DLPreInsert ..... vlo¾í nový prvek pøed aktivní prvek seznamu,
**      DLCopy .......... vrací hodnotu aktivního prvku,
**      DLActualize ..... pøepí¹e obsah aktivního prvku novou hodnotou,
**      DLSucc .......... posune aktivitu na dal¹í prvek seznamu,
**      DLPred .......... posune aktivitu na pøedchozí prvek seznamu,
**      DLActive ........ zji¹»uje aktivitu seznamu.
**
** Pøi implementaci jednotlivých funkcí nevolejte ¾ádnou z funkcí
** implementovaných v rámci tohoto pøíkladu, není-li u funkce
** explicitnì uvedeno nìco jiného.
**
** Nemusíte o¹etøovat situaci, kdy místo legálního ukazatele na seznam
** pøedá nìkdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodnì komentujte!
**
** Terminologická poznámka: Jazyk C nepou¾ívá pojem procedura.
** Proto zde pou¾íváme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako
** procedury (v jazyce C procedurám odpovídají funkce vracející typ void).
**/

#include "c206.h"

int solved;
int errflg;

void DLError() {
/*
** Vytiskne upozornìní na to, ¾e do¹lo k chybì.
** Tato funkce bude volána z nìkterých dále implementovaných operací.
**/	
    printf ("*ERROR* This program has performed an illegal operation.\n");
    errflg = TRUE;             /* globální promìnná -- pøíznak o¹etøení chyby */
    return;
}

void DLInitList (tDLList *L)	{
/*
** Provede inicializaci seznamu L pøed jeho prvním pou¾itím (tzn. ¾ádná
** z následujících funkcí nebude volána nad neinicializovaným seznamem).
** Tato inicializace se nikdy nebude provádìt nad ji¾ inicializovaným
** seznamem, a proto tuto mo¾nost neo¹etøujte. V¾dy pøedpokládejte,
** ¾e neinicializované promìnné mají nedefinovanou hodnotu.
**/
    
    L->First = L->Last = L->Act = NULL;
}

void DLDisposeList (tDLList *L)	{
/*
** Zru¹í v¹echny prvky seznamu L a uvede seznam do stavu, v jakém
** se nacházel po inicializaci. Ru¹ené prvky seznamu budou korektnì
** uvolnìny voláním operace free.
**/
	
	while(L->First != L->Last){
	    L->First = L->First->rptr;
	    free(L->First->lptr);
	}
	free(L->First);
	L->First = L->Last = L->Act = NULL;
}

void DLInsertFirst (tDLList *L, int val)	{
/*
** Vlo¾í nový prvek na zaèátek seznamu L.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	
    // Create new list item.
    tDLElemPtr tmp = malloc(sizeof(struct tDLElem));
    if (tmp == NULL){
        DLError();
        return;
    }
    // Empty list.
    else if (L->First == NULL){
        // Set last to be same item.
        L->Last = tmp;
        // Nullify next link.
        tmp->rptr = NULL;
    }
    else{
        // Previous link goes to new first.
        L->First->lptr = tmp;
        // Next link goes to current first.
        tmp->rptr = L->First;
    }
    // Set new first item.
    L->First = tmp;
    // Nullify previous link.
    L->First->lptr = NULL;
    // Set value.
    L->First->data = val;
}

void DLInsertLast(tDLList *L, int val)	{
/*
** Vlo¾í nový prvek na konec seznamu L (symetrická operace k DLInsertFirst).
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/ 	
	
    // Create new list item.
    tDLElemPtr tmp = malloc(sizeof(struct tDLElem));
    if (tmp == NULL){
        DLError();
        return;
    }
    // Empty list.
    else if (L->Last == NULL){
        // Set first to be same item.
        L->First = tmp;
        // Nullify previous link.
        tmp->lptr = NULL;
    }
    else{
        // Next link goes to new last.
        L->Last->rptr = tmp;
        // Previous link goes to current last.
        tmp->lptr = L->Last;
    }
    // Set new first item.
    L->Last = tmp;
    // Nullify next link.
    L->Last->rptr = NULL;
    // Set value.
    L->Last->data = val;
}

void DLFirst (tDLList *L)	{
/*
** Nastaví aktivitu na první prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
	
    L->Act = L->First;
}

void DLLast (tDLList *L)	{
/*
** Nastaví aktivitu na poslední prvek seznamu L.
** Funkci implementujte jako jediný pøíkaz (nepoèítáme-li return),
** ani¾ byste testovali, zda je seznam L prázdný.
**/
	
	
    L->Act = L->Last;
}

void DLCopyFirst (tDLList *L, int *val)	{
/*
** Prostøednictvím parametru val vrátí hodnotu prvního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/

	if(L->First == NULL)
	    DLError();
	else
	    *val = L->First->data;
}

void DLCopyLast (tDLList *L, int *val)	{
/*
** Prostøednictvím parametru val vrátí hodnotu posledního prvku seznamu L.
** Pokud je seznam L prázdný, volá funkci DLError().
**/
	
    if(L->Last == NULL)
        DLError();
    else
        *val = L->Last->data;
}

void DLDeleteFirst (tDLList *L)	{
/*
** Zru¹í první prvek seznamu L. Pokud byl první prvek aktivní, aktivita
** se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
**/

    // Test for empty list.
    if(L->First != NULL){
        // Invalidate active item if needed.
        if(L->Act == L->First)
            L->Act = NULL;

        // Just one item in list.
        if(L->First == L->Last){
            free(L->First);
            L->First = L->Last = NULL;
        }
        else{
            L->First = L->First->rptr;
            free(L->First->lptr);
            L->First->lptr = NULL;
        }
    }
}	

void DLDeleteLast (tDLList *L)	{
/*
** Zru¹í poslední prvek seznamu L. Pokud byl poslední prvek aktivní,
** aktivita seznamu se ztrácí. Pokud byl seznam L prázdný, nic se nedìje.
**/ 
	
    // Test for empty list.
    if(L->Last != NULL){
        // Invalidate active item if needed.
        if(L->Act == L->Last)
            L->Act = NULL;

        // Just one item in list.
        if(L->First == L->Last){
            free(L->Last);
            L->First = L->Last = NULL;
        }
        else{
            L->Last = L->Last->lptr;
            free(L->Last->rptr);
            L->Last->rptr = NULL;
        }
    }
}

void DLPostDelete (tDLList *L)	{
/*
** Zru¹í prvek seznamu L za aktivním prvkem.
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** posledním prvkem seznamu, nic se nedìje.
**/
	
    // Test for inactive list and last in list.
    // Note: if list is empty, it will be inactive.
    if(L->Act != NULL && L->Act->rptr != NULL)
    {
        // Make temporary reference to item to be deleted.
        tDLElemPtr tmp = L->Act->rptr;
        // Link next to next of to be deleted item.
        L->Act->rptr = tmp->rptr;
        // Delete item.
        free(L->Act->rptr);
        // If the next item is NULL set last to active, else
        // link next item to current active.
        if(L->Act->lptr == NULL)
            L->Last = L->Act;
        else
            L->Act->lptr->rptr = L->Act;
    }
}

void DLPreDelete (tDLList *L)	{
/*
** Zru¹í prvek pøed aktivním prvkem seznamu L .
** Pokud je seznam L neaktivní nebo pokud je aktivní prvek
** prvním prvkem seznamu, nic se nedìje.
**/
	
    // Test for inactive list and last in list.
    // Note: if list is empty, it will be inactive.
    if(L->Act != NULL && L->Act->lptr != NULL)
    {
        // Make temporary reference to item to be deleted.
        tDLElemPtr tmp = L->Act->lptr;
        // Link previous to previous of to be deleted item.
        L->Act->lptr = tmp->lptr;
        // Delete item.
        free(L->Act->lptr);
        // If the previous item is NULL set first to active, else
        // link previous to current active.
        if(L->Act->lptr == NULL)
            L->First = L->Act;
        else
            L->Act->lptr->rptr = L->Act;
    }
}

void DLPostInsert (tDLList *L, int val) {
/*
** Vlo¾í prvek za aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	
    // Test for active list.
    if(L->Act != NULL){
        // Create new item.
        tDLElemPtr tmp = malloc(sizeof(struct tDLElem));
        if(tmp == NULL){
            DLError();
            return;
        }

        // Link new item to active one.
        L->Act->rptr = tmp;
        tmp->lptr = L->Act;
        // Set value to new item.
        tmp->data = val;
    }
}

void DLPreInsert (tDLList *L, int val)		{
/*
** Vlo¾í prvek pøed aktivní prvek seznamu L.
** Pokud nebyl seznam L aktivní, nic se nedìje.
** V pøípadì, ¾e není dostatek pamìti pro nový prvek pøi operaci malloc,
** volá funkci DLError().
**/
	
    // Test for active list.
    if(L->Act != NULL){
        // Create new item.
        tDLElemPtr tmp = malloc(sizeof(struct tDLElem));
        if(tmp == NULL){
            DLError();
            return;
        }

        // Link new item to active one.
        L->Act->lptr = tmp;
        tmp->rptr = L->Act;
        // Set value to new item.
        tmp->data = val;
    }
}

void DLCopy (tDLList *L, int *val)	{
/*
** Prostøednictvím parametru val vrátí hodnotu aktivního prvku seznamu L.
** Pokud seznam L není aktivní, volá funkci DLError ().
**/
		
    if(L->Act == NULL)
        DLError();
    else
        *val = L->Act->data;
}

void DLActualize (tDLList *L, int val) {
/*
** Pøepí¹e obsah aktivního prvku seznamu L.
** Pokud seznam L není aktivní, nedìlá nic.
**/
	
	if(L->Act != NULL)
	    L->Act->data = val;
}

void DLSucc (tDLList *L)	{
/*
** Posune aktivitu na následující prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na posledním prvku se seznam stane neaktivním.
**/
	
    if(L->Act != NULL)
        L->Act = L->Act->rptr;
}


void DLPred (tDLList *L)	{
/*
** Posune aktivitu na pøedchozí prvek seznamu L.
** Není-li seznam aktivní, nedìlá nic.
** V¹imnìte si, ¾e pøi aktivitì na prvním prvku se seznam stane neaktivním.
**/
	
    if(L->Act != NULL)
        L->Act = L->Act->lptr;
}

int DLActive (tDLList *L) {		
/*
** Je-li seznam aktivní, vrací true. V opaèném pøípadì vrací false.
** Funkci implementujte jako jediný pøíkaz.
**/
	
	return L->Act != NULL;
}

/* Konec c206.c*/
