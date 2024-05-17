/********************************************************************************
 *
 * client.c
 * Codi que de cada client, programa que genera un fitxer amb un nombre de
 * linies determinat.
 *
 * Tomeu Capó Capó 2004
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include "comunica.h"
#include "dir.h"          /* Només per l'estructura d_stat */
#include "dir_clt.h"

#define MAX_LIN		100

static int pos_act = 0;

int fprint(char *nom_fitxer, char *cadena)
{
    int retval;
	
    retval = c_write(nom_fitxer, pos_act, strlen(cadena), cadena);
    pos_act = pos_act + strlen(cadena);

    return retval;
}

void client(int coa_escritura, int coa_lectura)
{
     int i,l;
     time_t clock;
     struct tm *tm;
     char linia[40];
     char nom_fitxer[30];
     
     assigna_coes(coa_lectura, coa_escritura);          /* Assigna les coes per a les operacions */

     sprintf(nom_fitxer,"/client-%d.dat", getpid());
     
//   printf("*** Soc el client %d creant l'arxiu %s\n", getpid(), nom_fitxer);

     c_creat(nom_fitxer);

//   printf("*** Iniciat el log del client %d\n", getpid());
     
     sprintf(linia, "Inici del log client %d\n", getpid());
     
     fprint(nom_fitxer, linia);
     
     for(l = 0; l <= MAX_LIN; l++) {
         clock = time(NULL);
         tm = localtime(&clock);
    
         sprintf(linia, "%02d:%02d:%02d Linia número %d\n", tm->tm_hour, tm->tm_min, tm->tm_sec , l);
         fprint(nom_fitxer, linia);
     }

     sprintf(linia, "Fi del log client %d\n", getpid());
     fprint(nom_fitxer, linia);
}
