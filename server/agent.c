/********************************************************************************
 *
 * agent.c
 * Programa principal que s'encarrega de arrancar i gestionar tota la simulació
 * dels servidors del sistema de fitxers.
 *
 * Last revision: 20/01/2004
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
#include <mm.h>
#include "block.h"
#include "sdf.h"
#include "inodes.h"
#include "bitmap.h"
#include "file.h"
#include "dir.h"
#include "comunica.h"
#include "servidor.h"

#define NUM_SRVS        3 

int coa[2];
int pid_servidor[NUM_SRVS];
int acabats;

void enterrador()
{
	int enterrats = 0;

	while(wait3(NULL, WNOHANG, NULL) > 0) {
		enterrats++;
		acabats++;
	}
	if(enterrats > 0) 
		printf("Servidors acabats: %d\n", acabats);
}

/* Funció general d'inicialització */

int init_agent()
{
     key_t key_q1, key_q2;

     key_q1 = ftok(".", 'a');
     key_q2 = ftok("..", 'a');
    
     /* Incialitzam la memòria compartida */
/*     
     MM_create(sizeof(dev_str) +
               sizeof(sdf_str) +
               sizeof(str_sb) +
               2048, NULL);     

     MM_permission(0666, getgid(), getuid());
*/	     
     /* Inicialitza les dues coes */
     
     if ((coa[0] = agafa_coa(key_q1, IPC_CREAT | 0666)) < 0) {
	 printf("Error al crear la coa 1\n");
	 return -1;
     }
     
     if ((coa[1] = agafa_coa(key_q2, IPC_CREAT | 0666)) < 0) {
	 printf("Error al crear la coa 2\n");
	 return -1;
     }
     
     return 0;
}

/* Bloc principal */

int main(int argc, char **argv)
{
      int i;
      
      setlinebuf(stdout);

      if (argc == 1) {
	  printf("Falta especificar la imatge a emprar!\n");
          exit(-1);
      }
      
      if (init_agent() < 0) {
	  MM_destroy();
	  exit(-1);
      }
     
      printf("Montant imatge ...\n");

      if (montar(argv[1]) < 0) {
          perror("Error montant el sistema d'arxius!");
          MM_destroy();
          exit(-1);
      }
      
      for(i=0;i<NUM_SRVS;i++) {                         /* Creació dels servidors */
	  if((pid_servidor[i] = fork()) == 0) {
	      sprintf(argv[0], "SDF_Server\0");
              servidor_sdf(coa[0], coa[1]);
	      exit(0);
	  }
      }
      
      while(acabats < NUM_SRVS) {
            enterrador();
	    usleep(10000);
      }
     
      /* Destroy all */

      elimina_coa(coa[0]);
      elimina_coa(coa[1]);
      printf("Desmontant imatge ...\n");
      desmontar();
//      MM_destroy();
}
