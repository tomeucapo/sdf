/*****************************************************************************************
 * dir.c
 *
 * Mòdul per gestionar les operacions a nivell de directoris, abstraguent el concepte de
 * I-Node cap el de nom de fitxer/directori.
 *
 * Last modify: 20/01/2004
 *
 * Tomeu Capó Capó 2004 (C)
 *****************************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <mm.h>
#include "block.h"
#include "sdf.h"
#include "inodes.h"
#include "file.h"
#include "dir.h"
#include "dentry_basic.h"
#include "server/comunica.h"

static int mutex;

/******************************************************************************************
 * Operacions bàsiques per montar - desmontar el sistema d'arxius
 * Utilizant les primitives f_mount i f_umount
 ******************************************************************************************/

int montar(char *nom_sdf)
{
    key_t clau;                                             /* Generam l'identificador */

    if ((clau=ftok(".",'a'))<0) {
         fprintf(stderr,"[SDF_DIR %d]: Error al crear la clau\n",getpid());
         return(-1);
    }

    mutex = crea_sem(clau, 1); 
    init_sem(mutex, 1);
    return(f_mount(nom_sdf));
}

void desmontar()
{
    elimina_sem(mutex);
    f_umount();
}

/******************************************************************************************
 * gen_creat - Funció genèrica per crear tot tipus de fitxers dins el sistema d'arxius
 * depenent de tipus. Podem crear o fitxers o directoris
 ******************************************************************************************/

int gen_creat(char *nom_fitxer, unsigned int tipus)
{
    int i_node_base, retval = 0, i_node_trobat;
    char nomes_nom_fitxer[LEN_NAME];
    
    bzero(&nomes_nom_fitxer, LEN_NAME);
   
    P(mutex, 0, 0);
    i_node_base = namei(nom_fitxer, nomes_nom_fitxer);		// Miram si el directori base existeix
                                                   		// si existeix ens torna el seu i-node.
    if (i_node_base < 0)
	retval = SDF_DIR_NOT_EXISTS;
    else {
	    
      // Miram si aquest fitxer ja existeix!
      
      if (strlen(nomes_nom_fitxer) > 0) {
	 i_node_trobat = cercar_element_dir(i_node_base, nomes_nom_fitxer);

	 if (i_node_trobat < 0) {
             retval = create_entry(i_node_base, nomes_nom_fitxer, tipus);
	 } else
             retval = SDF_FILE_EXISTS;
      } else 
	 retval = SDF_DIR_NOT_EXISTS;
    } 
    
    V(mutex, 0);
    return retval;
}

/****************************************************************************************
 * d_creat - Ens permet crear un fitxer nou a partir d'un directori base
 * especificat. Si no existeix el directori base ens torna un codi d'error.
 ****************************************************************************************/

int d_creat(char *nom_fitxer)
{
    return (gen_creat(nom_fitxer, SDF_FILE_INODE));
}

/****************************************************************************************
 * d_mkdir - Ens permet crear un subdirectori nou a partir d'un directori base
 * especificat. Si no existeix el directori base ens torna un codi d'error.
 ****************************************************************************************/

int d_mkdir(char *nom_dir)
{
    return (gen_creat(nom_dir, SDF_DIRE_INODE));
}

/****************************************************************************************
 * d_unlink - Ens permet borrar un fitxer/directori "buid"
 ****************************************************************************************/

int d_unlink(char *nom_fitxer)
{
    int i_node_f, i_node_base, retval = 0;
    i_node_str i_node_tmp;
    char nomes_nom_fitxer[LEN_NAME];

    P(mutex, 0, 0);
    i_node_base = namei(nom_fitxer, nomes_nom_fitxer);     // Miram si el directori base existeix
                                                           // si existeix ens torna el seu i-node.
    if (i_node_base >= 0) {
	// Miram si aquest fitxer existeix!
        i_node_f = cercar_element_dir(i_node_base, nomes_nom_fitxer);
	
	if (i_node_f >= 0) {

	   i_node_tmp = f_stat(i_node_f);

	   if (i_node_tmp.i_type == SDF_FILE_INODE)
               retval = delete_entry(i_node_base, nomes_nom_fitxer);
	   else
	       if ((i_node_tmp.i_type == SDF_DIRE_INODE) && (i_node_tmp.i_size == 0))
		   retval = delete_entry(i_node_base, nomes_nom_fitxer);
	       else
                   retval = -1;
	   
        } else
           retval = -1;
    } else
      retval = -1;

    V(mutex, 0);
    return retval;
}

/****************************************************************************************
 * d_write - Ens permet escriure dades dins un fitxer determinat. Ens torna el nombre
 * de bytes escrits, en cas contrari un -1.
 ****************************************************************************************/

int d_write(char *nom_complet, int inici, int mida, char *buff)
{
    int i_node_f, i_node_base, retval = 0;
    i_node_str i_node_tmp;
    char nomes_nom_fitxer[LEN_NAME];

    P(mutex, 0, 0);
    i_node_base = namei(nom_complet, nomes_nom_fitxer);     // Miram si el directori base existeix
                                                            // si existeix ens torna el seu i-node.
    if (i_node_base >= 0) {
	    
	// Miram si aquest fitxer existeix!
	
        i_node_f = cercar_element_dir(i_node_base, nomes_nom_fitxer);
        
	V(mutex, 0);

	if (i_node_f >= 0) {
           i_node_tmp = f_stat(i_node_f);
	   if (i_node_tmp.i_type == SDF_FILE_INODE) {
               retval = f_write(i_node_f, inici, mida, buff);
	   } else
               retval = -1;
        } else
           retval = -1;
    } else {
      retval = -1;
      V(mutex, 0);
    }
    
    return retval;
}

/****************************************************************************************
 * d_read - Ens permet llegir dades d'un fitxer determinat. Ens torna el nombre
 * de bytes llegits, en cas contrari un -1.
 ****************************************************************************************/

int d_read(char *nom_complet, int inici, int mida, char *buff)
{
    int i_node_f, i_node_base, retval = 0;
    i_node_str i_node_tmp;
    char nomes_nom_fitxer[LEN_NAME];
    
    i_node_base = namei(nom_complet, nomes_nom_fitxer);     // Miram si el directori base existeix
                                                            // si existeix ens torna el seu i-node.
    if (i_node_base >= 0) {
	    
	// Miram si aquest fitxer existeix!
	
	i_node_f = cercar_element_dir(i_node_base, nomes_nom_fitxer);
	
	if (i_node_f >= 0) {
	   
           i_node_tmp = f_stat(i_node_f);
	   if (i_node_tmp.i_type == SDF_FILE_INODE) {
               retval = f_read(i_node_f, inici, mida, buff);
	   } else
               retval = -1;
        } else
           retval = -1;
	
    } else {
      retval = -1;
    }

    return retval;
}

/****************************************************************************************
 * d_stat - Ens torna la metainformació referent al fitxer
 ****************************************************************************************/

struct d_stat d_stat(char *nom_fitxer)
{
    int i_node_f, i_node_base;
    i_node_str i_node_tmp;
    struct d_stat stat_file;
    char nomes_nom_fitxer[LEN_NAME];

    i_node_base = namei(nom_fitxer, nomes_nom_fitxer);     // Miram si el directori base existeix
                                                           // si existeix ens torna el seu i-node.
    bzero(&stat_file, sizeof(stat_file));

    if (i_node_base >= 0) {
	// Miram si aquest fitxer existeix!

	i_node_f = cercar_element_dir(i_node_base, nomes_nom_fitxer);
	
	if (i_node_f >= 0) {
           i_node_tmp = f_stat(i_node_f);
	   stat_file.f_size_bytes = i_node_tmp.i_size;
	   stat_file.f_size_blks = i_node_tmp.i_blocks;
	   stat_file.f_t_create = i_node_tmp.d_create;
	   stat_file.f_t_modify = i_node_tmp.d_modify;
	} 
           
    } 
    
    return stat_file;
}

/****************************************************************************************
 * d_list_dir - Ens torna un buffer aqui on hi ha les entrades
 ****************************************************************************************/

int d_list_dir(char *nom_dir, char *buffer)
{
    int i_node_f = 0, i_node_base, i, num_ent;
    i_node_str i_node_tmp;
    str_dir tmp_dir;
    char linia[LEN_LINIA_DIR];
    char nomes_nom_fitxer[LEN_NAME];
    
    bzero(&nomes_nom_fitxer, LEN_NAME);

    i_node_base = namei(nom_dir, nomes_nom_fitxer);                 // Miram si el directori base existeix
                                                                    // si existeix ens torna el seu i-node.
    if (i_node_base >= 0) {
	// Miram si aquest fitxer existeix!

	if (strlen(nomes_nom_fitxer) > 0) 
           i_node_f = cercar_element_dir(i_node_base, nomes_nom_fitxer);
		
	if(i_node_f >= 0) {
           i_node_tmp = f_stat(i_node_f);
           num_ent = i_node_tmp.i_size / sizeof(str_dir);
	} else 
           return -1;
	   
        if (i_node_tmp.i_type == SDF_FILE_INODE) { 
            printf("L'objecte que vol llistar (%d) no és del tipus directori\n", i_node_f);
            return -1;
	}
	

        if (num_ent == 0) 
            return 0;
	else {
	    for(i = 0;i <= num_ent-1; i++) {
                read_entry(i_node_f, i, &tmp_dir);   
	        i_node_tmp = f_stat(tmp_dir.p_inode);
                if (strlen(tmp_dir.nom) > 0) {
	            sprintf(linia, "%10d;%02d;%30s",i_node_tmp.i_size, 
						    i_node_tmp.i_type,
				                    tmp_dir.nom);

		    memcpy(&buffer[i*LEN_LINIA_DIR], linia, LEN_LINIA_DIR);
	        }
            }
	    return num_ent;
	}
    } else 
      return -1;
    
}


