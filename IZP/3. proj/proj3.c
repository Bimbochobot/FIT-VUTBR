//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 27.10.2012
//// Popis:
//// Subor obsahujuci funckiu main a funkcie
//// relevantne pre tento konkretny projekt.

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "proj3util.h"


/**
 * @brief Vypise maticu v predposanom formate.
 * @param matrix Matica k vypisu.
 */
void printMatrix(const Matrix* matrix)
{
    char c;
    bool big;

    printf("%u %u\n", matrix->rows, matrix->cols);
    for(unsigned int i = 0; i < matrix->rows; i++){
        for(unsigned int j = 0; j < matrix->cols; j++){
            c = matrix->data[idxMat2(j,i,matrix->cols)];
            big = matrix->mark[idxMat2(j,i,matrix->cols)];
            if(c < ALPHABET_SIZE){
                putchar(c + (big ? 'A' : 'a') - 1);
            }
            else{
                switch(c){
                case CHAR_CH:
                    putchar(big ? 'C' : 'c');
                    putchar(big ? 'H' : 'h');
                    break;
                }
            }
            putchar(' ');
        }
        putchar('\n');
    }
}

/**
 * @brief Vypise riesenie osemsmerovky.
 * @param matrix Matica s osemsmerovkou.
 */
void printSolution(const Matrix* matrix){
    char c;

    for(unsigned int i = 0; i < matrix->size; i++){
        c = matrix->data[i];
        if(matrix->mark[i] == false){
            if(c < ALPHABET_SIZE){
                putchar(c + 'a' - 1);
            }
        }
    }
    putchar('\n');
}

/**
 * @brief Nacita osemsmerovku zo suboru do matice.
 * @param matrix Matica pre ulozenie dat.
 * @param file Subor obsahujuci osemsmerovku.
 */
bool loadMatrix(Matrix* matrix, FILE* file)
{
    if(file == NULL) return false;

    if(fscanf(file, "%u%u", &matrix->rows, &matrix->cols) != 2){
        // Chyba v rozmeroch.
        printError(EWRONGFORMAT, NULL);
        return false;
    }

    // Alokacia poli.
    matrix->size = matrix->rows * matrix->cols;
    matrix->data = malloc(matrix->size * sizeof(char));
    matrix->mark = malloc(matrix->size * sizeof(bool));
    if(matrix->data == NULL || matrix->mark == NULL){
        printError(EALLOC, NULL);
        return false;
    }

    int c;
    unsigned int idx = 0;
    while((c = fgetc(file)) != EOF && idx < matrix->size){
        if(isspace(c)) continue;

        c = c - 'a' + 1;
        if(c <= 0 || c >= ALPHABET_SIZE){
            printError(EWRONGFORMAT, NULL);
            return false;
        }

        matrix->data[idx] = c;
        // Citanie viacznakovych pismen.
        while((c = fgetc(file)) != EOF && !isspace(c)){
            switch(matrix->data[idx]){
            case ('c'-'a'+1):
                if(c == 'h'){
                    matrix->data[idx] = CHAR_CH;
                    break;
                }
            default:
                printError(EWRONGFORMAT, NULL);
                return false;
            }
        }
        matrix->mark[idx] = false;
        idx++;
    }

    if(idx != matrix->size || feof(file) == 0){
        printError(EWRONGFORMAT, NULL);
        return false;
    }

    return true;
}

/**
 * @brief Otestuje osemsmerovku. Ak je v spravnom formate, vypise
 *        ju nezmenenu na vystup, inak vypise chybove hlasenie.
 * @param params Parametre programu.
 */
void test(MainParam* params)
{
    Matrix matrix = (Matrix){0,0,0,NULL,NULL};
    FILE* file = fopen(params[TEST].options[0], "r");
    if(file == NULL){
        printError(ENOTFOUND, NULL);
        return;
    }
    if(loadMatrix(&matrix, file)) printMatrix(&matrix);

    fclose(file);
    free(matrix.data); matrix.data = NULL;
    free(matrix.mark); matrix.mark = NULL;
}

/**
 * @brief Najde index v matici, na ktorom je dalsie pismeno
 *        v zadanom smere. Taktiez testuje ohranicenie osemsmerovky.
 * @param matrix Matica s osemsmerovkou.
 * @param pos Predchadzajuca pozicia, po uspesnom posunuti nova pozicia.
 * @param dir Smer posunu.
 * @return False v pripade prekrocenia okrajov osemsmerovky.
 */
bool nextPos(const Matrix* matrix, unsigned int* pos, Directions dir){
    unsigned int derefPos = *pos;
    switch(dir){
    case DIRU:
        if(derefPos < matrix->cols) return false;
        *pos = derefPos - matrix->cols;
        break;
    case DIRUR:
        if(derefPos < matrix->cols || (derefPos+1) % matrix->cols == 0) return false;
        *pos = derefPos - matrix->cols + 1;
        break;
    case DIRR:
        if((derefPos+1) % matrix->cols == 0) return false;
        *pos = derefPos + 1;
        break;
    case DIRDR:
        if((derefPos + matrix->cols) >= matrix->size || (derefPos+1) % matrix->cols == 0)
            return false;
        *pos = derefPos + matrix->cols + 1;
        break;
    case DIRD:
        if((derefPos + matrix->cols) >= matrix->size) return false;
        *pos = derefPos + matrix->cols;
        break;
    case DIRDL:
        if((derefPos + matrix->cols) >= matrix->size || derefPos % matrix->cols == 0)
            return false;
        *pos = derefPos + matrix->cols - 1;
        break;
    case DIRL:
        if(derefPos % matrix->cols == 0) return false;
        *pos = derefPos - 1;
        break;
    case DIRUL:
        if(derefPos < matrix->cols || derefPos % matrix->cols == 0) return false;
        *pos = derefPos - matrix->cols - 1;
        break;
    default:
        return false;
    }
    return true;
}

/**
 * @brief Vyhlada slovo v osemsmerovke.
 * @param matrix Matica s osemsmerovkou.
 * @param word Slovo, ktore ma byt najdene.
 * @param lastVec Posledny najdeny vyskyt.
 * @return Dalsi najdeny vyskyt slova. Ak slovo nebolo najdene, pozicia
 *         vyskytu bude vecsia rovna velkosti matice, a smer nastaveny
 *         na DIRNULL.
 */
WordVector wordSearch(const Matrix* matrix, const char* word, WordVector lastVec){
    lastVec.dir++;
    unsigned int pos, i;
    for(; lastVec.pos < matrix->size; lastVec.pos++){
        pos = lastVec.pos;
        i = 0;

        if(word[i] == matrix->data[pos]){
            for(; lastVec.dir < DIR_N; lastVec.dir++){
                pos = lastVec.pos;
                i = 1;

                // Porovnavanie znakov dokym neni najdene cele slovo
                // alebo hrana matice.
                while(word[i] != '\0' &&
                      nextPos(matrix, &pos, lastVec.dir) &&
                      word[i] == matrix->data[pos])
                      i++;

                if(word[i] == '\0') return lastVec;
            }
        }
        lastVec.dir = DIRU;
    }

    lastVec.dir = DIRNULL;
    return lastVec;
}

/**
 * @brief Zakoduje slovo do zhodneho formatu s nacitanou osemsmerovkou.
 * @param original Povodne slovo v ASCII.
 * @param converted Zakodovane slovo.
 * @param maxlen Obmedzenie maximalnej dlzky slova.
 * @return Dlzku skonvertovaneho slova v pripade uspesnej konverzie, inak 0.
 *         Po neuspesnej konverzii je obsah converted nedefinovany.
 */
unsigned int encodeWord(const char* original, char* converted,
                        unsigned int maxlen){
    unsigned int j = 0;

    for(unsigned int i = 0; original[i] != 0;){
        converted[j] = original[i] - 'a' + 1;
        if(converted[j] <= 0 || converted[j] >= ALPHABET_SIZE){
            printError(EBADWORD, NULL);
            return 0;
        }
        i++;
        if(converted[j] == ('c'-'a'+1) && original[i] == 'h'){
            converted[j] = CHAR_CH;
            i++;
        }
        j++;
        if(j >= maxlen){
            printError(ELONGWORD, NULL);
            return 0;
        }
    }
    converted[j] = '\0';
    return j+1;
}

/**
 * @brief Dekoduje slovo z formatu osemsmerovky do ASCII.
 * @param original Zakodovane slovo.
 * @param converted Slovo v ASCII.
 * @param maxlen Obmedzenie maximalnej dlzky slova.
 * @return Dlzku skonvertovaneho slova v pripade uspesnej konverzie, inak 0.
 *         Po neuspesnej konverzii je obsah converted nedefinovany.
 */
unsigned int decodeWord(const char* original, char* converted,
                        unsigned int maxlen){
    unsigned int i = 0, j = 0;
    unsigned int len = 1;

    for(; original[i] != 0;){
        if(original[i] >= ALPHABET_SIZE){
            switch(original[i]){
            case CHAR_CH:
                len++;
                break;
            default:
                return 0;
            }
        }
        len++;
        i++;
    }
    if(len == 0 || len > maxlen) return 0;

    j = len-1;
    converted[j] = '\0';
    while(j > 0){
        i--;
        j--;
        if(original[i] < ALPHABET_SIZE)
            converted[j] = original[i] + 'a' - 1;
        else{
            switch(original[i]){
            case CHAR_CH:
                converted[j] = 'h';
                j--;
                converted[j] = 'c';
                break;
            default:
                return 0;
            }
        }
    }

    return len;
}

/**
 * @brief Precita lubovolne dlhe slovo zo suboru.
 * @param file Subor, z ktoreho sa slovo cita.
 * @param word Precitane slovo.
 * @param maxlen Maximalna dlzka nacitaneho slova. Ak je 0, dlzka
 *               sa nekontroluje.
 * @return True, ak sa uspesne nacitalo nove slovo.
 */
bool readWord(FILE* file, String* word, unsigned int maxlen){
    unsigned int len = 0;
    bool reading = true;
    int c;

    //precitaj biele znaky
    do{
        c = fgetc(file);
    }
    while(c != EOF && isspace(c));

    while(reading){
        while(len + BUFFER_SIZE > word->cap){
            if(!incStrCap(word))
                return false;
        }
        for(int i = 0; i < BUFFER_SIZE; i++){
            if(c == EOF || isspace(c)){
                word->data[len] = '\0';
                len++;
                reading = false;
                break;
            }
            word->data[len] = c;
            len++;
            c = fgetc(file);
        }

        //Test na maximalnu dlzku slova
        if(maxlen > 0 && len > maxlen){
            printError(ELONGWORD, NULL);
            return false;
        }
    }

    //slovo nemoze obsahovat iba ukoncovaciu '\0'
    return len > 1;
}

/**
 * @brief Vyhlada slovo predane cez parameter programu v
 *        osemsmerovke.
 * @param params Parametre programu.
 */
void search(MainParam* params)
{
    Matrix matrix = (Matrix){0,0,0,NULL,NULL};
    FILE* file = fopen(params[SEARCH].options[0], "r");
    if(file == NULL){
        printError(ENOTFOUND, NULL);
        return;
    }

    unsigned int pos;
    char* word = NULL;

    if(loadMatrix(&matrix, file)){
        WordVector vec = (WordVector){0, DIRNULL};

        // Maximalna dlzka slova moze byt hlavna uhlopriecka
        // stvorca mensieho rozmeru, sirka alebo vyska.
        // Kedze po ulohpriecke sa pohybujem taktiez vzdy o
        // vzdialenost 1, staci uvazovat iba o sirke a vyske.
        unsigned int maxlen = 1 + (matrix.cols > matrix.rows?
                                   matrix.cols : matrix.rows);

        word = malloc(maxlen*sizeof(char));
        if(word == NULL)
            printError(EALLOC, NULL);

        else{
        if(encodeWord(params[SEARCH].options[1], word, maxlen)){
            bool found = false;
            while(vec.pos < matrix.size){
                vec = wordSearch(&matrix, word, vec);
                if(vec.pos < matrix.size){
                    //zaznacenie najdeneho slova
                    found = true;
                    pos = vec.pos;
                    for(unsigned int i = 0; word[i] != 0; i++){
                        matrix.mark[pos] = true;
                        nextPos(&matrix, &pos, vec.dir);
                    }
                }
            }
            if(found) printMatrix(&matrix);
            else printError(ENOSOLUTION, params[SEARCH].options[1]);
        }
        }
    }

    fclose(file);
    free(word); word = NULL;
    free(matrix.data); matrix.data = NULL;
    free(matrix.mark); matrix.mark = NULL;
}

/**
 * @brief Vylusti osemsmerovku pomocou slov dodanych v subore.
 * @param params Parametre programu.
 */
void solve(MainParam* params)
{
    Matrix matrix = (Matrix){0, 0, 0, NULL, NULL};
    String word = (String){0, NULL};
    WordVector vec;
    unsigned int pos;
    FILE* file = fopen(params[SOLVE].options[0], "r");
    if(file == NULL){
        printError(ENOTFOUND, NULL);
        return;
    }

    if(loadMatrix(&matrix, file)){
        fclose(file); file = NULL;
        file = fopen(params[SOLVE].options[1], "r");
        if(file == NULL)
            printError(ENOTFOUND, NULL);

        else{

        // Maximalna dlzka slova moze byt hlavna uhlopriecka
        // stvorca mensieho rozmeru, sirka alebo vyska.
        // Kedze po ulohpriecke sa pohybujem taktiez vzdy o
        // vzdialenost 1, staci uvazovat iba o sirke a vyske.
        unsigned int maxlen = 1 + (matrix.cols > matrix.rows?
                                   matrix.cols : matrix.rows);

        //maxlen*2 pre pripad, ze by vsetky znaky boli ch
        //while(feof(file) == 0 && readWord(file, &word, maxlen*2)){
        while(readWord(file, &word, maxlen*2)){
            if(encodeWord(word.data, word.data, maxlen)){
                bool found = false;
                vec = (WordVector){0, DIRNULL};
                while(vec.pos < matrix.size){
                    vec = wordSearch(&matrix, word.data, vec);
                    if(vec.pos < matrix.size){
                        //zaznacenie najdeneho slova
                        found = true;
                        pos = vec.pos;
                        for(unsigned int i = 0; word.data[i] != 0; i++){
                            matrix.mark[pos] = true;
                            nextPos(&matrix, &pos, vec.dir);
                        }
                    }
                }
                if(!found){
                    if(decodeWord(word.data, word.data, maxlen*2) != 0)
                        printError(ENOSOLUTION, word.data);
                    else
                        printError(ENOSOLUTION, NULL);
                    break;
                }
            }
            else
                break;
        }

        if(feof(file) != 0)
            printSolution(&matrix);

        }
    }

    fclose(file); file = NULL;
    free(word.data); word.data = NULL;
    free(matrix.data); matrix.data = NULL;
    free(matrix.mark); matrix.mark = NULL;
}

int main(int argc, char* argv[])
{
    //priprav miesto pre vsetky mozne parametre
    MainParam params[PARAM_N];
    for(int i = 0; i < PARAM_N; i++)
        params[i] = (MainParam){false, {0,0}};

    if(!procMainArg(params, argc, argv))
        return 1;

    if(params[HELP].enabled) printHelp();
    else if(params[TEST].enabled) test(params);
    else if(params[SEARCH].enabled) search(params);
    else if(params[SOLVE].enabled) solve(params);

    return 0;
}
