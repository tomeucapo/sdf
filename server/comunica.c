/******************************************************************************
 *                                                                            *
 * comunica.c                                                                 *
 *                                                                            *
 * Modul per la comunicació entre processos. Funcions per facilitar           *
 * l'utilització de semafors i missatges.                                     *
 *                                                                            *
 * Date creation: 06/05/2003                                                  *
 * Last modified: 08/05/2003                                                  *
 *                                                                            *
 * Tomeu Capó i Capó                                                          *
 ******************************************************************************/

#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "comunica.h"

/*************************************************************************
 * Funcions per la manipulació de semafors                               *
 *************************************************************************/

int crea_sem(int semid, int num)
{
    int s;
    if ((s=semget(semid, num, IPC_CREAT|0600)) < 0) {
        printf("Error creat el semàfor %d\n",semid);
	exit(0);
    }
    return s;
}

void elimina_sem(int semid)
{
     semctl(semid, 0, IPC_RMID, 0);
}

void init_sem(int semid, int valor)
{
     semctl(semid, 0, SETVAL, valor);
}

void P(int semid, int num, int flg)     /* Espera */
{
     struct sembuf op;

     op.sem_num = num;
     op.sem_op = -1;
     op.sem_flg = flg;                  // Normalment SEM_UNDO
     semop(semid,&op,1);
}

void V(int semid, int num)              /* Senyalitza */
{
     struct sembuf op;

     op.sem_num = num;
     op.sem_op = 1;
     op.sem_flg = 0;
     semop(semid,&op,1);
}

void wait_for_zero(int s, int pos)
{
     struct sembuf sbuf;

     sbuf.sem_num = pos;
     sbuf.sem_op = 0;
     sbuf.sem_flg = 0;
     semop(s, &sbuf, 1);
}

/*************************************************************************
 * Funcions per enviar missatges                                         *
 *************************************************************************/

int agafa_coa(key_t clau, int flags)
{
    int coa;
    if((coa=msgget(clau, flags))<0) {
        perror("Error agafant coa:");
        exit(-1);
    }
    return(coa);
}

void elimina_coa(int coa)
{
    msgctl(coa, IPC_RMID, 0);
}
