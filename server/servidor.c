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

/*********************************************************************
  Modul principal del servidor
 *********************************************************************/

  
void servidor_sdf(int coa_lectura, int coa_escritura)
{
     int n_writef, n_readf, n_mkdir, n_creatf, n_unlink, n_statf, n_dir;
     
     void acabar(int s)
     {
	  printf("[SRV: %d] Ha acabat amb aquestes estadístiques:\n", getpid());
	  printf("\tCridades a WRITEF %4d\n",n_writef);
	  printf("\tCridades a READF  %4d\n",n_readf);
	  printf("\tCridades a MKDIR  %4d\n",n_mkdir);
	  printf("\tCridades a CREATF %4d\n",n_creatf);
	  printf("\tCridades a UNLINK %4d\n",n_unlink);
	  printf("\tCridades a STATF  %4d\n",n_statf);
	  printf("\tCridades a DIR    %4d\n\n",n_dir);
	  exit(0);
     }

     s_miss msg,msg2;
     struct d_stat estat_fitxer;
     int r,val_pos, len = 0, res = 0, bytes_enviats, len_buf;
     char buffer[45056];

     n_writef = n_readf = n_mkdir= n_creatf= n_unlink= n_statf= n_dir = 0;

     signal(SIGTERM, acabar);
    
     printf("Iniciat el servidor: %d [Coa RX = %d][Coa TX = %d]\n", getpid(), coa_lectura, coa_escritura);
     while(1) {
	     if((r=msgrcv(coa_lectura, &msg, sizeof(s_ordre) ,0,0))<0)
		printf("[SRV: %d] Error de recepció: %s\n",getpid(),strerror(errno));
	     else {
                //printf("[SRV: %d] ",getpid());
		
	        switch(msg.comanda.opcode) {
               	       case OP_DIR:printf("Client %d DIR [%s]\n", msg.tipus, msg.comanda.nom_fitxer);
				   n_dir++;
				   bytes_enviats = 0;
                                   msg.comanda.opcode = d_list_dir(msg.comanda.nom_fitxer, buffer);
				   len_buf = (msg.comanda.opcode*LEN_LINIA_DIR);
                                   printf("Longitut màxima buffer: %d\n", len_buf);

			           if(buffer[0] == '\0')
			              msg.comanda.n_bytes = 0;
				   else
				      msg.comanda.n_bytes = len_buf; 

				   if(msg.comanda.n_bytes == 0) {
				      msg.comanda.pos_ini = 0;
				      if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                         printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);
				   }

				   while(bytes_enviats < len_buf) {
                                         if ((len_buf-bytes_enviats) < MAX_BUF_MSG)
				             msg.comanda.pos_ini = len_buf-bytes_enviats;
					 else
				             msg.comanda.pos_ini = MAX_BUF_MSG;

					 memcpy(msg.comanda.buff, buffer + bytes_enviats, msg.comanda.pos_ini);
					 bytes_enviats += msg.comanda.pos_ini;

					 if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                             printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);
				   }
				     
				   break;
				  
                    case OP_STATF://printf("Client %d STATF [%s]\n", msg.tipus, msg.comanda.nom_fitxer);

				  n_statf++;
				  
				  estat_fitxer = d_stat(msg.comanda.nom_fitxer);
				  memcpy(msg.comanda.buff, &estat_fitxer, sizeof(estat_fitxer));
				  
				  if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                      printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);
				  
				  break;
				  
                   case OP_UNLINK:printf("Client %d UNLINK [%s]\n", msg.tipus, msg.comanda.nom_fitxer);
				  
				  n_unlink++;
                                  msg.comanda.opcode = d_unlink(msg.comanda.nom_fitxer);
				  
				  if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
				     printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);

				  break;

		   case OP_CREATF://printf("Client %d CREATF [%s]\n", msg.tipus, msg.comanda.nom_fitxer);
				  
                                  n_creatf++;
                                  msg.comanda.opcode = d_creat(msg.comanda.nom_fitxer);

				  if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                     printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);
				  
				  break;
				  
 	            case OP_MKDIR:printf("Client %d MKDIR [%s]\n", msg.tipus, msg.comanda.nom_fitxer);
				  
				  n_mkdir++;
                                  msg.comanda.opcode = d_mkdir(msg.comanda.nom_fitxer);

				  if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                      printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);

				  break;
				  
		    case OP_READF:printf("Client %d READF [%s]\n", msg.tipus, msg.comanda.nom_fitxer);

				  n_readf++;
				  msg.comanda.n_bytes = d_read(msg.comanda.nom_fitxer, 
						                msg.comanda.pos_ini, 
					 	                msg.comanda.n_bytes, 
						                msg.comanda.buff);
				  if (msg.comanda.n_bytes < 0)
				      msg.comanda.opcode = -1;
				  
				  if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                      printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);

				  
				  break;
				  
                   case OP_WRITEF://printf("Client %d WRITEF [%s]\n", msg.tipus, msg.comanda.nom_fitxer);
				 
				  n_writef++;
                                  msg.comanda.n_bytes = d_write(msg.comanda.nom_fitxer, 
						                msg.comanda.pos_ini, 
					 	                msg.comanda.n_bytes, 
						                msg.comanda.buff);
				  if (msg.comanda.n_bytes < 0)
				      msg.comanda.opcode = -1;
				  
				  if((msgsnd(coa_escritura, &msg, sizeof(s_ordre),0))<0)
                                      printf("[SRV: %d] Error enviant: %d\n",getpid(),coa_escritura);
				  
				  break;
				  
			  default:break;
		}
				  
	     }
     }
}
