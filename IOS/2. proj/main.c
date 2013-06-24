//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 1.4.2013
//// Popis:
//// Subor obsahujuci funckiu main a funkcie
//// relevantne pre tento konkretny projekt.

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>

#include <sys/types.h>

#include "util.h"

#define SEM_FTOK_ID 10
#define SHM_FTOK_ID 100

typedef struct{
    int activeElves;
    int waitingElves;
    int lineCounter;
} Shared;

union semunn{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

enum {SEMN_ENTER = 0, SEMN_REQUEST, SEMN_EWAIT, SEMN_DECIDE, SEMN_GENERAL, SEMN_END, SEMN_N};

/**
 * @brief Caka na uvolnenie semaforu a potom ho uzamkne. (Jedna sa o 1 atomicku operaciu.)
 * @param semid ID semaforovej skupiny.
 * @param semnum Cislo semaforu v skupine.
 * @param weight Hodnota, o ktoru sa zvysi stav semaforu.
 * @return True ak sa vsetky operacie vykonali uspesne.
 */
bool lockSemaphore(int semid, int semnum, int weight)
{
    struct sembuf sops[2];

    sops[0].sem_num = semnum;
    sops[0].sem_op = 0;
    sops[0].sem_flg = 0;

    sops[1].sem_num = semnum;
    sops[1].sem_op = weight;
    sops[1].sem_flg = 0;

    if (semop(semid, sops, 2) < 0){
         fprintf(stderr, "Nebolo mozne uzamknut semafor (id: %i, num: %i)\n", semid, semnum);
         return false;
    }
    return true;
}

/**
 * @brief Caka na uvolnenie semaforu.
 * @param semid ID semaforovej skupiny.
 * @param semnum Cislo semaforu v skupine.
 * @return True ak sa vsetky operacie vykonali uspesne.
 */
bool waitSemaphore(int semid, int semnum)
{
    struct sembuf sop;

    sop.sem_num = semnum;
    sop.sem_op = 0;
    sop.sem_flg = 0;

    if (semop(semid, &sop, 1) < 0){
         fprintf(stderr, "Operacia cakania na semfore zlyhala (id: %i, num: %i)\n", semid, semnum);
         return false;
    }
    return true;
}

/**
 * @brief Uvolni semafor.
 * @param semid ID semaforovej skupiny.
 * @param semnum Cislo semaforu v skupine.
 * @param weight Hodnota, o ktoru sa znizi stav semaforu.
 * @return True ak sa vsetky operacie vykonali uspesne.
 */
bool releaseSemaphore(int semid, int semnum, int weight)
{
    struct sembuf sop;

    sop.sem_num = semnum;
    sop.sem_op = -weight;
    sop.sem_flg = 0;

    if (semop(semid, &sop, 1) < 0){
         fprintf(stderr, "Nebolo mozne uvolnit semafor (id: %i, num: %i)\n", semid, semnum);
         return false;
    }
    return true;
}

/**
 * @brief Kodova vetva pre proces santu.
 * @param file Vystupny subor pre zapisovanie udalosti.
 * @param semid ID semaforovej skupiny.
 * @param shmid ID zdielanej pamete.
 * @param S Maximalna doba pre obsluhu elfov (ms).
 */
void santaProcess(FILE* file, int semid, int shmid, int S)
{
    void* shmAddress = shmat(shmid, NULL, 0);
    if (shmAddress == NULL){
        fprintf(stderr, "santa: Namapovanie zdielanej pamete zlyhalo\n");
        exit(1);
    }
    Shared* shared = (Shared*)shmAddress;
    srand((int)getpid());

    if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
    fprintf(file, "%i: santa: started\n", shared->lineCounter);
    shared->lineCounter++;
    if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

    // Kym este nejaky elfovia pracuju
    while(shared->activeElves > 0){
        shared->waitingElves = 0;
        if(!lockSemaphore(semid, SEMN_EWAIT, 1)) exit(1);
        int bufferSize = shared->activeElves > 3 ? 3 : 1;

        // Spristupni priechod elfom
        if(!releaseSemaphore(semid, SEMN_ENTER, 1)) exit(1);

        // Spristupnuj buffer kym ho elfovia nezaplnia
        while(true){
            // Cakaj kym elf zaziada o obsluhu
            if(!lockSemaphore(semid, SEMN_REQUEST, 1)) exit(1);

            // Nechaj posledneho elfa v buffri blokovat vstup
            if(shared->waitingElves == bufferSize){
                if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
                fprintf(file, "%i: santa: checked state: %i: %i\n", shared->lineCounter,
                    shared->activeElves, shared->waitingElves);
                shared->lineCounter++;
                if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
                break;
            }

            // Umozni dalsiemu elfovi narvat sa pred santu
            if(!releaseSemaphore(semid, SEMN_ENTER, 1)) exit(1);
        }

        // Zacni obsluhu elfov
        if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
        fprintf(file, "%i: santa: can help\n", shared->lineCounter);
        shared->lineCounter++;
        if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

        // Pohraj sa s elfmi
        if(S > 0) usleep(rand() % (S * 1000));

        // Nastav semafor, aby sa kazdy elf musel rozhodnut
        // (1 dekrementacia per elf)
        if(!lockSemaphore(semid, SEMN_DECIDE, bufferSize)) exit(1);

        // Vypusti skriatkov z obsluhy
        if(!releaseSemaphore(semid, SEMN_EWAIT, 1)) exit(1);

        // Pockaj, kym sa vsetcia rozhodnu, co so zivotom
        if(!waitSemaphore(semid, SEMN_DECIDE)) exit(1);
    }
    shared->waitingElves = 0;
    fprintf(file, "%i: santa: checked state: %i: %i\n",
            shared->lineCounter, shared->activeElves, shared->waitingElves);
    shared->lineCounter++;
    fprintf(file, "%i: santa: finished\n", shared->lineCounter);
    shared->lineCounter++;
    if(!releaseSemaphore(semid, SEMN_END, 1)) exit(1);

    if(shmdt(shmAddress) < 0) exit(1);
    exit(0);
}

/**
 * @brief Kodova vetva pre proces elfa.
 * @param file Vystupny subor pre zapisovanie udalosti.
 * @param elfid Poradove cislo elfa, zacina sa od 0.
 * @param semid ID semaforovej skupiny.
 * @param shmid ID zdielanej pamete.
 * @param C Pocet navstev pred dovolenkou.
 * @param H Maximalna doba do problemu (ms).
 */
void elfProcess(FILE* file, int elfid, int semid, int shmid, int C, int H)
{
    elfid++;
    void* shmAddress = shmat(shmid, NULL, 0);
    if (shmAddress == NULL){
        fprintf(stderr, "elf: %i: Namapovanie zdielanej pamete zlyhalo\n", elfid);
        exit(1);
    }
    Shared* shared = (Shared*)shmAddress;
    srand((int)getpid());

    if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
    fprintf(file, "%i: elf: %i: started\n", shared->lineCounter, elfid);
    shared->lineCounter++;
    if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

    int visits = 0;
    // Makaj kym nemas dost navstev
    while(visits < C){
        // Trocha zamakaj nez pojdes na navstevu
        if(H > 0) usleep(rand() % (H * 1000));

        // Houstne, mame problem
        if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
        fprintf(file, "%i: elf: %i: needed help\n", shared->lineCounter, elfid);
        shared->lineCounter++;
        if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

        // Narvi sa pred santu
        if(!lockSemaphore(semid, SEMN_ENTER, 1)) exit(1);

        // Zvys pocet v elf buffer
        shared->waitingElves++;

        if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
        fprintf(file, "%i: elf: %i: asked for help\n", shared->lineCounter, elfid);
        shared->lineCounter++;
        if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

        // Vyziadaj si obsluhu (santa zisti, ze niekto prisiel)
        if(!releaseSemaphore(semid, SEMN_REQUEST ,1)) exit(1);

        // Pockaj na dokoncenie obsluhy santom
        if(!waitSemaphore(semid, SEMN_EWAIT)) exit(1);
        visits++;

        if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
        fprintf(file, "%i: elf: %i: got help\n", shared->lineCounter, elfid);
        shared->lineCounter++;
        // Rozhodni sa, ci pojdes na dovolenku
        if(visits == C){
            shared->activeElves--;
            fprintf(file, "%i: elf: %i: got a vacation\n", shared->lineCounter, elfid);
            shared->lineCounter++;
        }
        if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

        // Informuj santu, ze pocet pracujucich elfov je
        // uz aktualizovany po tvojom rozhodnuti.
        if(!releaseSemaphore(semid, SEMN_DECIDE, 1)) exit(1);
    }
    if(!waitSemaphore(semid, SEMN_END)) exit(1);

    if(!lockSemaphore(semid, SEMN_GENERAL, 1)) exit(1);
    fprintf(file, "%i: elf: %i: finished\n", shared->lineCounter, elfid);
    shared->lineCounter++;
    if(!releaseSemaphore(semid, SEMN_GENERAL, 1)) exit(1);

    if(shmdt(shmAddress) < 0) exit(1);
    exit(0);
}

/**
 * @brief Vytvori semaforovu skupinu.
 * @param count Pocet semaforov, ktore bude skupina obsahovat.
 * @return ID semaforovej skupiny, ak sa vsetky operacie vykonali uspesne.
 *         Inak -1.
 */
int createSempahores(int count)
{
    key_t key = ftok(".", SEM_FTOK_ID);
    int semid = semget(key, count, 0666 | IPC_CREAT | IPC_EXCL);
    if(semid < 0){
        fprintf(stderr, "Vytvorenie semaforu zlyhalo\n");
        return semid;
    }
    return semid;
}

/**
 * @brief Inicializuje semaforovu skupinu.
 * @param semid ID semaforovej skupiny.
 * @param count Pocet semaforov v skupine.
 * @param values Inicializacne hodnoty.
 * @return True, ak sa vsetky operacie vykonali uspesne.
 */
bool initializeSemaphores(int semid, int count, int* values)
{
    union semunn def;
    for(int i = 0; i < count; i++){
        def.val = values[i];
        if(semctl(semid, i, SETVAL, def) < 0)
            return false;
    }
    return true;
}

/**
 * @brief Vytvori zdielanu pamet pre IPC.
 * @param size Pozadovana velkost v bytoch. Bude zaokruhlena nahor k PAGE_SIZE.
 * @return ID zdielanej pamete, ak sa vsetky operacie vykonali uspesne.
 *         Inak -1.
 */
int createSharedMemory(size_t size)
{
    key_t key = ftok(".", SHM_FTOK_ID);
    int shmid = shmget(key, size, 0666 | IPC_CREAT | IPC_EXCL);
    if(shmid < 0){
        fprintf(stderr, "Vytvorenie zdielanej pamete zlyhalo\n");
        return shmid;
    }

    return shmid;
}

/**
 * @brief Inicializuje zdielanu pamet.
 * @param shmid ID zdielanej pamete.
 * @param E Pocet aktivnych elfov na zaciatku.
 * @return True, ak sa vsetky operacie vykonali uspesne.
 */
bool initializeSharedMemory(int shmid, int E)
{
    void* shmAddress = shmat(shmid, NULL, 0);
    if (shmAddress == NULL){
        printf("Namapovanie zdielanej pamete zlyhalo\n");
        return false;
    }
    Shared* shared = (Shared*)shmAddress;
    shared->activeElves = E;
    shared->waitingElves = 0;
    shared->lineCounter = 1;
    return shmdt(shmAddress) == 0;
}

/**
 * @brief Ukonci vytvorene procesy.
 * @param santaPid PID santu.
 * @param elfPid PID elfov.
 * @param elfNum Pocet elfov.
 */
void killChilds(pid_t santaPid, pid_t* elfPid, int elfNum)
{
    if(santaPid > 0) kill(santaPid, SIGTERM);
    for(int i = 0; elfPid != NULL && i < elfNum; i++){
        if(elfPid[i] > 0) kill(elfPid[i], SIGTERM);
    }
}

/**
 * @brief Vytvori podprocesy pre santu a elfov.
 * @param file Vystupny subor pre zapisovanie udalosti.
 * @param[out] santaPid Pid pre proces santu.
 * @param[out] elfPid Pole Pid pre procesy elfov. Pole alokuje funkcia, vstupna hodnota musi byt NULL.
 * @param params Parametre programu.
 * @param semid ID semaforovej skupiny.
 * @param shmid ID zdielanej pamete.
 * @return True, ak sa vsetky operacie vykonali uspesne.
 */
bool createSubprocesses(FILE* file, pid_t* santaPid, pid_t** elfPid, MainParam params, int semid, int shmid)
{
    setbuf(file, NULL);
    int E = params.val[1];
    if(*elfPid != NULL){
        fprintf(stderr, "Vstupna hodnota parametru elfPid musi byt NULL\n");
        return false;
    }
    *elfPid = (pid_t*) malloc(sizeof(pid_t)*E);
    if(*elfPid == NULL) return false;

    *santaPid = fork();
    if(*santaPid < 0) return false;
    if(*santaPid == 0)
        santaProcess(file, semid, shmid, params.val[3]);
    else{
        int pid;
        for(int i = 0; i < E; i++){
            pid = fork();
            if(pid < 0){
                // Zahraj sa na masoveho vraha
                killChilds(*santaPid, *elfPid, i);
                return false;
            }
            if(pid == 0)
                elfProcess(file, i, semid, shmid, params.val[0], params.val[2]);
            else
                (*elfPid)[i] = pid;
        }
    }
    return true;
}

/**
 * @brief Riesi Santa Claus Problem.
 * @param params Parametre programu.
 * @return 0, ak sa vsetky operacie vykonali uspesne.
 *         Inak 2.
 */
int solve(MainParam params)
{
    int ret = 0;
    FILE* file;
    pid_t santaPid = 0;
    pid_t* elfPid = NULL;
    int semid = 0, shmid = 0;
    int defSemValues[] = {1, 1, 0, 0, 0, 1};

    // Vytvor vsetky potrebne procesy a zdielane zdroje
    if((file = fopen("santa.out", "w")) != NULL &&
       (semid = createSempahores(SEMN_N)) >= 0 &&
       initializeSemaphores(semid, SEMN_N, defSemValues) &&
       (shmid = createSharedMemory(sizeof(Shared))) >= 0 &&
       initializeSharedMemory(shmid, params.val[1]) &&
       createSubprocesses(file, &santaPid, &elfPid, params, semid, shmid)){
        int status;
        for(int i = 0; i < params.val[1] + 1;){
            // Cakaj na zmenu stavu ktorehokolvek childa
            waitpid(-1, &status, 0);
            if(WIFEXITED(status)){
                // Child sa ukoncil
                i++;
                // Nastala chyba?
                if(WEXITSTATUS(status) != 0){
                    // Jeden za vsetkych, vsetcia za jednoho
                    // Zlyha jeden proces -> zlyha cela synchronizacia
                    killChilds(santaPid, elfPid, params.val[1]);
                    ret = 2;
                    break;
                }
            }
        }
    }
    else
        ret = 2;

    if(semid >= 0) semctl(semid, 0, IPC_RMID);
    if(shmid >= 0) shmctl(shmid, IPC_RMID, NULL);
    fclose(file);
    free(elfPid);
    return ret;
}

int main(int argc, char* argv[])
{
    // Priprav miesto pre vsetky mozne parametre
    MainParam params[PARAM_N];
    for(int i = 0; i < PARAM_N; i++)
        params[i] = (MainParam){false, {0, 0, 0, 0}};

    if(!procMainArg(params, argc, argv))
        return 1;

    if(params[HELP].enabled){
        printHelp();
        return 0;
    }

    return solve(params[SOLVE]);
}
