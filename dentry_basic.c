/******************************************************************************************
 * dentry_basic.c
 *
 * Mòdul per gestionar les operacions bàsiques del D-Entry (Entrada de directori)
 * cerques, insercions, eliminacions i càlculs de direccions.
 *
 * Last modify: 20/01/2004
 * 
 * Tomeu Capó Capó 2004 (C)
 *****************************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
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

/******************************************************************************************
 * seek_entry - Funció que ens permet cercar una entrada dins el directori donat per l'inode
 * i ens torna la estructura de l'entrada i la posició dins el directori.
 ******************************************************************************************/

int seek_entry(int i_node, char *nom_fitxer, str_dir *t_dir)
{
    int         p, trobat = 0, num_ent;
    i_node_str  file_str;
    str_dir     tmp_dir;
    char        *buffer;
   
    
    file_str = f_stat(i_node);                                    /* Consultam les propietats del directori */
    num_ent = file_str.i_size / sizeof(str_dir);
    
    buffer = (char *) malloc(sizeof(str_dir));

    for(p = 0;p <= num_ent; p++) {      
        if (f_read(i_node, p*sizeof(str_dir), sizeof(str_dir), buffer) < 0) 
	   break;
	else {
           memcpy(&tmp_dir, buffer, sizeof(str_dir));
	   if (strcmp(nom_fitxer, tmp_dir.nom) == 0) {
	       trobat = 1;
	       *t_dir = tmp_dir;
	       break;
	   }
 	}
    }
    free(buffer);
    
    if (trobat)
	return p;
    else
	return -1;
}

/******************************************************************************************
 * cercar_element_dir - Funció que ens permet cercar un element (fitxer/directori) dins
 * el directori i ens torna l'I-NODE aqui on apunta l'element trobat.
 *******************************************************************************************/

int cercar_element_dir(int i_node, char *nom_entrada)
{
    str_dir tmp_dir;
	    
    if (seek_entry(i_node, nom_entrada, &tmp_dir) >= 0) {
        return tmp_dir.p_inode;
    } else
        return -1;
}

/******************************************************************************************
 * namei - Funció que ens torna el i-node del directori base que ens passen
 * ex: /usr/local/bin ens tornaria el i-node del directori bin i la cadena "bin" dins 
 * nom_fitxer.
 ******************************************************************************************/
 
int namei(char *base_dir, char *nom_fitxer)
{
    int i_nou = 0, i_actual = 0, nivells = 0;
    int d_len = strlen(base_dir);
    int retval = 0, nivell = 0;
    char *dir;
    char llista_nivells[255][LEN_NAME], direct[d_len+1];
    char dir_actual[LEN_NAME];
    
    if (d_len) {
	strcpy(direct, base_dir);
	direct[d_len+1] = 0;

        if ((dir = strtok(direct, DIR_SEPARATE)) == NULL) 
           return 0;
	
	while (dir != NULL && nivells <= 255) {
	       strcpy(llista_nivells[nivells], dir);
               dir = strtok(NULL, DIR_SEPARATE);
	       nivells++;
	}
        nivells--;
	
	while (nivell <= nivells-1) {
	       i_nou = cercar_element_dir(i_actual, llista_nivells[nivell]);
	       if (i_nou < 0)
		   break;
  	       
	       i_actual = i_nou;
               nivell++; 
	}
                     
        if (i_nou < 0)
           retval = i_nou;
        else
           retval = i_actual;
	   
        strcpy(nom_fitxer, llista_nivells[nivells]);
    } else
	retval = -1;
    
    free(dir);
    return retval;
}

/******************************************************************************************
 * create_entry - Funció que ens crea una entrada nova al directori
 * depenent de tipus. Podem crear o fitxers o directoris. Creant aixì maiteix el fitxer bàsic
 ******************************************************************************************/

int create_entry(int i_node_base, char *nom_fitxer, unsigned int tipus)
{
     int         retval = 0;
     str_dir     tmp_dir;
     i_node_str  file_str;
     char        buffer[sizeof(str_dir)];
   
     printf("create_entry ---------------------- %s\n", nom_fitxer);
     
     tmp_dir.p_inode = f_create(tipus);          // Cream una entrada nova amb el nom del directori i el punter
                                                 // al inode.
						  
     printf("Ok -------------------------------- %s = %d\n", nom_fitxer, tmp_dir.p_inode);

     if (tmp_dir.p_inode > 0) {
         memset(tmp_dir.nom, 0, LEN_NAME);
         strcpy(tmp_dir.nom, nom_fitxer);

         file_str = f_stat(i_node_base);         // Guardam les dades al directori corresponent
 
         memcpy(buffer, &tmp_dir, sizeof(str_dir));
	 
	 printf("Creant entranda %d %d (%d) %s\n", i_node_base, file_str.i_size, sizeof(str_dir) ,nom_fitxer);
	 
         if (f_write(i_node_base, file_str.i_size, sizeof(str_dir), buffer) < 0)
            retval = -1;
     } else
	 retval=-1;

     return retval;
}

/* Lectura d'una entrada de directori */

int read_entry(int i_node_base, int pos, ptr_dir dentry)
{
    int retval = 0;
    char *buffer = (char *) malloc(sizeof(str_dir));
    
    if(f_read(i_node_base, pos * sizeof(str_dir), sizeof(str_dir), buffer) < 0) 
       retval = -1;
    else 
       memcpy(dentry, buffer, sizeof(str_dir));
 
    free(buffer);
    return retval;
}

/* Copia una entrada dins un altre */

void copy_entry(int i_node_base, int pos_dst, int pos_src)
{
     char *buffer;
     buffer = (char *) malloc(sizeof(str_dir));
     f_read(i_node_base, pos_src, sizeof(str_dir), buffer);
     f_write(i_node_base, pos_dst, sizeof(str_dir), buffer);
     free(buffer);
}

/******************************************************************************************
 * delete_entry - Funció que ens borra una entrada del directori, ALERTA! aqui NO CONTROLAM
 * la posibiliatat de que l'entrada sigui un dirèctori ple i tengui subdirectoris.
 ******************************************************************************************/

int delete_entry(int i_node_base, char *nom_fitxer)
{
    str_dir     tmp_dir;
    i_node_str  file_str;
    int         p, i_node_file, mida_final;
    
    file_str = f_stat(i_node_base);
    mida_final = file_str.i_size - sizeof(str_dir);
   
    if (mida_final <= 0) 
	mida_final = 0;
    
    if ((p = seek_entry(i_node_base, nom_fitxer, &tmp_dir)) >= 0) {
         i_node_file = tmp_dir.p_inode;
	 
	 /* Col.loca la darrera entrada del directori al lloc de la
	  * que borram, si no es la mateixa posició! */
	 
	 p = p * sizeof(str_dir);
	 if (mida_final != p) 
             copy_entry(i_node_base, p, mida_final);
	 
         f_unlink(i_node_file);
         f_trunc(i_node_base, mida_final);
	 return 0;
    }  

    return -1;
}
