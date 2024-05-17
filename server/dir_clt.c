#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "comunica.h"
#include "dir.h"                       // Només per l'estructura d_stat
#include "dir_clt.h"

int coa_rx, coa_tx;

void assigna_coes(int q_rx, int q_tx)
{
     coa_rx = q_rx;
     coa_tx = q_tx;
}

int c_creat(char *nom_fitxer)
{
    s_miss msg, msg2;

    if(strlen(nom_fitxer) > 0) {
       msg.tipus = getpid();
       msg.comanda.opcode = OP_CREATF;
       strcpy(msg.comanda.nom_fitxer, nom_fitxer);
    } else 
      return -1;
    
    if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
        printf ("C_CREAT: Error enviament PID=%d, COA=%d, STRERROR=%s\n", getpid(), coa_tx, strerror(errno));
        return -1;
    }

    if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
       printf ("C_CREAT: Error recepció PID=%d, COA=%d, STRERROR=%s\n", getpid(), coa_rx, strerror(errno));	
       return -1;
    }
    
    return 0;
}

int c_mkdir(char *nom_fitxer)
{
    s_miss msg, msg2;

    if(strlen(nom_fitxer) > 0) {
       msg.tipus = getpid(); 
       msg.comanda.opcode = OP_MKDIR;
       strcpy(msg.comanda.nom_fitxer, nom_fitxer);
    } else
       return -1;

    if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
       printf ("Error enviament %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
       return -1;
    }

    if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
       printf ("Error recepció %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
       return -1;
    }

    return 0;
}

int c_unlink(char *nom_fitxer)
{
    s_miss msg, msg2;

    if(strlen(nom_fitxer) > 0) {
       msg.tipus = getpid();
       msg.comanda.opcode = OP_UNLINK;
       strcpy(msg.comanda.nom_fitxer, nom_fitxer);
    } else
       return -1;

    if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
       printf ("Error enviament %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
       return -1;
    }

    if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
       printf ("Error recepció %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
       return -1;
    } else
       if(msg2.comanda.opcode < 0)
	  return -1;
    
    return 0;
}

int c_write(char *nom_fitxer, int inici, int mida, char *buffer)
{
    int midab, len, bytes_enviats = 0;
    s_miss msg, msg2;

    if (mida > MAX_BUF_MSG) 
	midab = MAX_BUF_MSG;
    else
	midab = mida;

    if(strlen(nom_fitxer) > 0) {
       msg.tipus = getpid();
       msg.comanda.opcode = OP_WRITEF;
       strcpy(msg.comanda.nom_fitxer, nom_fitxer);
    } else
       return -1;

    while (bytes_enviats < mida) {
	   
           msg.comanda.pos_ini = inici + bytes_enviats;
           msg.comanda.n_bytes = midab;
           memcpy(msg.comanda.buff, buffer, midab);

           if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
              printf ("Error enviament %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
              return -1;
           }

	   if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
               printf ("Error recepció %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
               return -1;
	   } else {
               if (msg2.comanda.opcode < 0) 
		   return bytes_enviats;

	       bytes_enviats += msg2.comanda.n_bytes;
	       
	       if((mida - bytes_enviats) > MAX_BUF_MSG)
		  midab = MAX_BUF_MSG;
	       else
		  midab = mida - bytes_enviats;
	   }
    }

    return bytes_enviats;
}

int c_read(char *nom_fitxer, int inici, int mida, char *buffer)
{
    s_miss msg, msg2;
    int midab, bytes_rebuts;

    if (mida > MAX_BUF_MSG) 
	midab = MAX_BUF_MSG;
    else
	midab = mida;

    if(strlen(nom_fitxer) > 0) {
       msg.tipus = getpid();
       msg.comanda.opcode = OP_READF;
       msg.comanda.pos_ini = inici;
       msg.comanda.n_bytes = midab;
       strcpy(msg.comanda.nom_fitxer, nom_fitxer);
    } else
       return -1;


    if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
       printf ("C_READ: Error enviament %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
       return -1;
    }

    if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
       printf ("Error recepció %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
       return -1;
    } else {
       bytes_rebuts += msg2.comanda.n_bytes;
       memcpy(buffer, msg2.comanda.buff, bytes_rebuts);
       while (bytes_rebuts < mida) {
              if ((mida - bytes_rebuts) > MAX_BUF_MSG)
		   midab = MAX_BUF_MSG;
	      else
		   midab = mida;

	      msg.comanda.pos_ini = inici + bytes_rebuts;
	      msg.comanda.n_bytes = midab;
	      if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
                 printf ("C_READ: Error enviament %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
                 return -1;
              }
	      
              if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
                 printf ("Error recepció %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
                 return -1;
              } else {
                 if (msg2.comanda.opcode < 0)
	             return bytes_rebuts;

		 memcpy(buffer + bytes_rebuts, msg2.comanda.buff, msg2.comanda.pos_ini);
		 bytes_rebuts += msg2.comanda.n_bytes;
	      }
       }
       return bytes_rebuts;
    } 
}

struct d_stat c_stat(char *nom_fitxer)
{
    s_miss msg, msg2;
    struct d_stat stat_fitxer;
    
    if(strlen(nom_fitxer) > 0) {
       msg.tipus = getpid();
       msg.comanda.opcode = OP_STATF;
       strcpy(msg.comanda.nom_fitxer, nom_fitxer);
    } else
       return;

    if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
       printf ("Error envio %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
       return;
    }

    if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
       printf ("Error recepción %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
       return;
    }

    memcpy(&stat_fitxer, msg2.comanda.buff, sizeof(stat_fitxer));

    return stat_fitxer; 
}

int c_list_dir(char *nom_dir, char *buffer)
{
    s_miss msg, msg2;
    int n_fitxers, mida, bytes_llegits;

    if(strlen(nom_dir) > 0) {
       msg.tipus = getpid();
       msg.comanda.opcode = OP_DIR;
       strcpy(msg.comanda.nom_fitxer, nom_dir);
    } else
       return -1;

    if(msgsnd(coa_tx, &msg, sizeof(s_ordre), 0)<0) {
       printf ("C_DIR: Error envio %d,%d, %s\n", getpid(), coa_tx, strerror(errno));
       return -1;
    }

    if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
       printf ("Error recepción %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
       return -1;
    } else {
       n_fitxers = msg2.comanda.opcode;
       mida = msg2.comanda.n_bytes;
       bytes_llegits = msg2.comanda.pos_ini;
       memcpy(buffer, msg2.comanda.buff, bytes_llegits);
       
       printf("N BYTES A LLEGIR ---> %d\n", mida);
       while (bytes_llegits < mida) {
              if(msgrcv(coa_rx, &msg2, sizeof(s_ordre), getpid(), 0)<0) {
                 printf ("Error recepción %d,%d, %s\n", getpid(), coa_rx, strerror(errno));
                 return -1;
	      } else {
                 memcpy(buffer + bytes_llegits, msg2.comanda.buff, msg2.comanda.pos_ini); 
		 bytes_llegits += msg2.comanda.pos_ini;
	      }
       }
       return n_fitxers;
    }
    
}
