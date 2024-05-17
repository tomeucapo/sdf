/*****************************************************************************************
 * sdf.c
 *
 * Mòdul bàsic el qual perment montar un sistema de fitxers determinat del format
 * propi (SDF) i interpretar-lo, per el posterior maneig de les capes superiors.
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
#include "xmalloc.h" 
#include "block.h"
#include "sdf.h"

sdf_ptr mount_fs(char *nom, int flags, int *errors)
{
    char *buff;
    int res;
    sdf_ptr sdf_nou;
    
#ifndef MALLOC_NOT_SHARED
    mm_sdf = mm_create(sizeof(sdf_str), NULL);
    mm_super = mm_create(sizeof(str_sb), NULL);
#endif
    
    sdf_nou = (sdf_ptr) sdf_get_mem(mm_sdf, sizeof(sdf_str));
    
    /* Intentam obrir el sistema d'arxius */
   
    sdf_nou->dev_blk = open_dev_blk(nom, sizeof(str_sb), 1);
     
    if (sdf_nou->dev_blk) {
	    
#ifdef DEBUG_MOUNT
       printf("\n\nFS_DESC = %s %d\n\n",nom, sdf_nou->dev_blk->fd);
#endif

        sdf_nou->super = (ptr_sb) sdf_get_mem(mm_super, sizeof(str_sb));
	if(!sdf_nou->super) {
           printf("mount_fs: Error al reservar memòria pel super-bloc\n");
           sdf_free_mem(mm_sdf, sdf_nou);
	   *errors = 1;
	   return NULL;
        }
	
	/* Si existeix el sistema d'arxius llegim el super-bloc per comprovar que
	 * es el sistema correcte */
	
	buff = (char *) malloc(sizeof(str_sb));
	res = read_block(sdf_nou->dev_blk, 0, buff);
	memcpy(sdf_nou->super, buff, sizeof(str_sb));
	free(buff);
	
	if ( res >= 0 ) {
           if (sdf_nou->super->magic != SDF_MAGIC_NUMBER) { 
	      *errors = SDF_BAD_MAGIC;
	      sdf_free_mem(mm_sdf, sdf_nou);
	      return NULL;
	   }

	   /* Modificar la data del darrer montatge */
	   
	   sdf_nou->block_size = sdf_nou->super->block_size;
	   sdf_nou->super->t_mount = time(NULL);
	   write_super_block(sdf_nou);
	   
	   /* Configurar el dispositiu de blocs amb el tamany de bloc adecuat i el
	    * nombre màxim de blocs */
	   
	   block_size(sdf_nou->dev_blk, sdf_nou->block_size);
	   blocks_count(sdf_nou->dev_blk, sdf_nou->super->blocks_count);
	  
	   /* Calculam els punters necesaris per treballar amb el sistema de fitxers */
	   
	   sdf_nou->p_in = (sdf_nou->super->blocks_count / 8) / SDF_BLOCK_SIZE(sdf_nou) + 1;
	   
	   if ((sdf_nou->super->blocks_count / 8) % SDF_BLOCK_SIZE(sdf_nou) != 0) 
	      sdf_nou->p_in ++;
	   
	   sdf_nou->p_mb = 1;
	   sdf_nou->p_dades = SDF_INODE_BLOCKS(sdf_nou->super) + 2; 
           sdf_nou->nb_mb = sdf_nou->p_in - sdf_nou->p_mb;
	  
	   /* Reservam espai a memòria pel mapa de bits */
	   
#ifndef MALLOC_NOT_SHARED
	   mm_mbits = mm_create(sdf_nou->nb_mb * SDF_BLOCK_SIZE(sdf_nou), NULL);
#endif
	   
	   sdf_nou->mbits = (unsigned char *) sdf_get_mem(mm_mbits, sdf_nou->nb_mb * SDF_BLOCK_SIZE(sdf_nou)); 
	   if(!sdf_nou->mbits) {
              printf("mount_fs: Error al reservar memòria pel mapa de bits\n");
              *errors=1;
	      return NULL;
           }
          
#ifdef DEBUG_MOUNT 
	   printf("FS Size = %d Bytes\n", sdf_nou->super->size);
	   printf("Block Size = %d Bytes\n", sdf_nou->super->block_size);
	   printf("Create date = %s", ctime(&sdf_nou->super->t_create));
	   printf("Last mount date = %s", ctime(&sdf_nou->super->t_mount));
	   printf("Primer bloc dels inodes = %d\n", sdf_nou->p_in);
	   printf("Primer bloc de les dades = %d\n", sdf_nou->p_dades);
	   printf("I-Nodes count = %d\n", sdf_nou->super->inodes_count);
#endif	
	} else {
	   *errors = SDF_EMPTY;
	   return NULL;
	}
    } else {  
      *errors=SDF_FITXER_INEXISTENT;
      return NULL;
    }
    
    *errors = 0;
    return sdf_nou;
}


int dump_block(sdf_ptr sdf_g, int n_block)
{
    unsigned char *buf;
    int i,j=0;
   
    buf = (unsigned char *) malloc(SDF_BLOCK_SIZE(sdf_g)+1);
    if(!buf) {
       printf("dump_block: Error al reservar memoria\n");
       exit(1);
    }
    
    read_block(sdf_g->dev_blk, n_block, buf);

    for (i=0;i<= SDF_BLOCK_SIZE(sdf_g);i++) {
	if (j<=30) {
           if (j == 0) printf("%04d: ",i);
           printf("%02x ",buf[i]);
	   if (j == 15) printf(": ");
   	   j++;
	} else {
           j=0;i--;
	   printf("\n");
	}
    }
    printf("\n");
    free(buf);
}

str_sb read_super_block(sdf_ptr sdf_g)
{
     str_sb super;
     memcpy(&super, sdf_g->super, sizeof(str_sb));
     return(super);
}

void write_super_block(sdf_ptr sdf_g)
{
     char buffer[SDF_BLOCK_SIZE(sdf_g)];
     memset(buffer, 0, SDF_BLOCK_SIZE(sdf_g));
     memcpy(buffer, sdf_g->super, sizeof(str_sb));
     write_block(sdf_g->dev_blk, 0, buffer);
}

int umount_fs(sdf_ptr sdf_g)
{
    write_super_block(sdf_g);
    close_dev_blk(sdf_g->dev_blk);     /* Tancam el sistema de fitxers i alliberam espai */
    
    sdf_free_mem(mm_mbits, sdf_g->mbits);
    sdf_free_mem(mm_super, sdf_g->super);
    sdf_free_mem(mm_sdf, sdf_g);

#ifndef MALLOC_NOT_SHARED
    mm_destroy(mm_mbits);
    mm_destroy(mm_super);
    mm_destroy(mm_sdf);
#endif
    
}

