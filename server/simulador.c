/********************************************************************************
 *
 * simulador.c
 * Programa principal que s'encarrega de arrancar i gestionar tota la simulació
 * dels clients que fan peticions als servidors del SDF.
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
#include "dir.h"           
#include "comunica.h"
//#include "client.h"

//#define NUM_CLIENTS    

int coa[2];
int acabats;

void enterrador()
{
	int enterrats = 0;

	while(wait3(NULL, WNOHANG, NULL) > 0) {
		enterrats++;
		acabats++;
	}
	if(enterrats > 0) 
		printf("Acabats: %d\n", acabats);
}

/* Funció general d'inicialització */

int init_simul()
{
     key_t key_q1, key_q2;

     key_q1 = ftok(".", 'a');
     key_q2 = ftok("..", 'a');
     
     if ((coa[0] = agafa_coa(key_q1, 0666)) < 0) {
	 printf("Error al enganxar amb els servidors\n");
         return -1;
     }
     
     if ((coa[1] = agafa_coa(key_q2, 0666)) < 0) {
	 printf("Error al enganxar amb els servidors\n");
         return -1;
     }

     return 0;
}

/* Bloc principal */

int main(int argc, char **argv)
{
      int NUM_CLIENTS = 0;
      int i, j, mida, tipus, num_files;
      char dir[45056];
      char nom_fitxer[LEN_NAME];
      
      setlinebuf(stdout);

      if (argc == 1) {
          printf("Usage: simulador [num_clients]\n");	  
	  exit(1);
      } else
	  NUM_CLIENTS = atoi(argv[1]);

      if (init_simul() < 0) 
	  exit(1);

      assigna_coes(coa[1], coa[0]);

      /* Borram tots els fitxers del directori arrel */
      
      num_files = c_list_dir("/", dir);
      for (i=0;i<num_files;i++) {
	   sscanf(&dir[i*LEN_LINIA_DIR],"%10d;%02d;%30s", &mida, &tipus, nom_fitxer);
           printf("fichero a borrar: %s\n", nom_fitxer);
           if (tipus == 2) c_unlink(nom_fitxer);
      }
     
      /* Creació dels clients */
      
      for(i=0;i<NUM_CLIENTS;i++) {               
	  if(fork()==0) {
             sprintf(argv[0], "SDF_Client\0");
	     client(coa[0], coa[1]);
	     exit(0);
	  }
	  usleep(10000);
	  enterrador();
      }

      while(acabats < NUM_CLIENTS) {
            enterrador();
            usleep(10000);
      }

}
