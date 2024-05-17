#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <mm.h>
#include "block.h"
#include "sdf.h"
#include "bitmap.h"
#include "inodes.h"
#include "file.h"
#include "server/comunica.h"

static int f_mutex;
static sdf_ptr sdf_f;

int f_mount(char *nom_vol)
{
     int retval;
     key_t clau;                                        
     
     sdf_f = mount_fs(nom_vol, 0, &retval);

     if (!sdf_f) 
	retval = -1;
     else
        read_bbitmap(sdf_f);

     if ((clau=ftok(".",'a'))<0) {
         fprintf(stderr,"[SDF_FILE %d]: Error al crear la clau\n",getpid());
         retval = -1;
     } else {
         f_mutex = crea_sem(clau+1, 1);
         init_sem(f_mutex, 1);
     }
		     
     return retval;
}

int f_umount()
{ 
     if (sdf_f) {
         elimina_sem(f_mutex);
         umount_fs(sdf_f);
	 return 0;
     } else {
	 printf("Res per desmontar!");
         return -1;
     }
}

int f_create(unsigned int tipus)
{
     i_node_str i_node_tmp;	
     int n_inode;

     P(f_mutex, 0, 0);
     
     n_inode = mark_inode(sdf_f, tipus);           /* Reservam el primer inode lliure */
     
     if (n_inode > 0) {
         read_inode(sdf_f, n_inode, &i_node_tmp);
         i_node_tmp.d_create = time(NULL);
         i_node_tmp.d_modify = time(NULL);
         i_node_tmp.d_access = time(NULL);
         i_node_tmp.i_blocks = 0;
         write_inode(sdf_f, n_inode, &i_node_tmp);
     } else n_inode = -1;
     
     V(f_mutex, 0);
     
     return n_inode;
}

int f_unlink(int n_inode)
{
     i_node_str i_node_tmp;
     int i;

     P(f_mutex, 0, 0);
     
     read_inode(sdf_f, n_inode, &i_node_tmp);
     for(i = 0;i < N_BLOCKS;i++) {
         if(i_node_tmp.l_blocks[i] != -1)
            free_block(sdf_f, i_node_tmp.l_blocks[i]);
     }
 
     free_inode(sdf_f, n_inode);
     
     V(f_mutex, 0);
     return 0;
}


int f_write(int n_inode, int offset, int mida, char *buffer)
{
	int pos_inodo, offset_bloque, bytes_escritos, num_bloque, bytes_a_escribir, offset_buffer, i;
	i_node_str i_node;
	char bloque[SDF_BLOCK_SIZE(sdf_f)];

	if (offset >= (SDF_BLOCK_SIZE(sdf_f) * N_BLOCKS)) {
		printf("Out of file!\n");
		return -1;
	}

	// Si hem d'escriure 0 bytes retornam error ?????????

	if (read_inode(sdf_f, n_inode, &i_node) < 0) {
		printf("Error leyendo inodo numero %d\n", n_inode);
		return -1;
	}


	pos_inodo = (offset / SDF_BLOCK_SIZE(sdf_f));
	offset_bloque = (offset % SDF_BLOCK_SIZE(sdf_f));

	bytes_escritos = 0;
	offset_buffer = 0;
	
	while (((mida - bytes_escritos) > 0) && (pos_inodo < N_BLOCKS)) {

		if (i_node.l_blocks[pos_inodo] == -1) {
			
			P(f_mutex, 0, 0);
			num_bloque = seek_free_block(sdf_f);
			printf("Num block lliure = %d\n", num_bloque);
			if (mark_block(sdf_f, num_bloque) < 0) {
				printf("ERROR: No puc reservar un bloc\n");
			        i_node.d_modify = time(NULL);
				if ((offset + bytes_escritos) > i_node.i_size)
				    i_node.i_size = (offset + bytes_escritos);
				
                                write_inode(sdf_f, n_inode, &i_node);
				
			        V(f_mutex, 0);
				return bytes_escritos;
			}
	
			i_node.l_blocks[pos_inodo] = num_bloque;
			i_node.i_blocks ++;
			V(f_mutex, 0);
		}

		// Llegim el bloc on volem escriure per evitar la perdua de dades
		
		if (read_block(sdf_f->dev_blk, i_node.l_blocks[pos_inodo], bloque) < 0) {
			printf("Error leyendo bloque %d en inodo %d\n", i_node.l_blocks[pos_inodo], n_inode);
		
			//P(f_mutex, 0, 0);
			i_node.d_modify = time(NULL);
			if ((offset + bytes_escritos) > i_node.i_size)
			    i_node.i_size = (offset + bytes_escritos);

			write_inode(sdf_f, n_inode, &i_node);
	                //V(f_mutex, 0);
		
			return bytes_escritos;
		}

		if ((offset_bloque + (mida - bytes_escritos)) > SDF_BLOCK_SIZE(sdf_f)) 
	           bytes_a_escribir = (SDF_BLOCK_SIZE(sdf_f) - offset_bloque);
		else
                   bytes_a_escribir = (mida - bytes_escritos);
		

		//Copiam les dades desde el buffer al bloc
		memcpy((bloque + offset_bloque), (buffer + offset_buffer), bytes_a_escribir);
		
		//Escrivim el bloc altre cop
		if (write_block(sdf_f->dev_blk, i_node.l_blocks[pos_inodo], bloque) < 0) {
			printf("Error escribiendo bloque %d\n", i_node.l_blocks[pos_inodo]);	

			i_node.d_modify = time(NULL);
			if ((offset + bytes_escritos) > i_node.i_size)
			   i_node.i_size = (offset + bytes_escritos);

			write_inode(sdf_f, n_inode, &i_node);
			
			return bytes_escritos;
		}
		
		//indicam que, en cas d'haver d'escriure en un altre bloc
		//l'offset ha de ser 0, es a dir, comensar desde el principi pq
		//les dades han d'estar concatenades
		
		offset_bloque = 0;

		//Incrementam l'offset del buffer tants bytes com bytes hem escrit
		
		offset_buffer = offset_buffer + bytes_a_escribir;
		bytes_escritos = bytes_escritos + bytes_a_escribir;

		//passam al seguent punter d'inode per poder escriure al seguent bloc
		pos_inodo++;

		write_inode(sdf_f, n_inode, &i_node);
	}
	
	if ((offset + bytes_escritos) > i_node.i_size)
	    i_node.i_size = (offset + bytes_escritos);
	
	i_node.d_modify = time(NULL);
	write_inode(sdf_f, n_inode, &i_node);

	return bytes_escritos;
}

int f_read(int n_inode, int p_inicial, int mida, char *buffer)
{
     char buff[SDF_BLOCK_SIZE(sdf_f->super)];
     i_node_str i_node_tmp;
     int k=0,j,i;
     int cursor,n_blks,inici;

     /* Llegim l'inode corresponent */
    
     read_inode(sdf_f, n_inode, &i_node_tmp);
     
//   if (i_node_tmp.i_size >= (p_inicial+mida)) {
         inici = p_inicial / SDF_BLOCK_SIZE(sdf_f->super);
	 cursor = p_inicial % SDF_BLOCK_SIZE(sdf_f->super);
	 n_blks = (mida + cursor) / SDF_BLOCK_SIZE(sdf_f->super);
	 if (((mida + cursor) % SDF_BLOCK_SIZE(sdf_f->super)) !=0)
            n_blks++;
	 
	 /* Anam llegint tots els blocs corresponent al fitxer i els
	  * passam al buffer */
	 
	 for(j = 0;j < n_blks & j <= N_BLOCKS; j++) {
#ifdef DEBUG_READF		 
             printf("*** F_READ-%d: I-NODE %d Block Fitxer = %d, Punter Block = %d\n", 
	             getpid(), n_inode, j, i_node_tmp.l_blocks[inici+j]);
#endif
	     if (i_node_tmp.l_blocks[inici+j] >= 0) {
                 read_block(sdf_f->dev_blk, i_node_tmp.l_blocks[inici+j], buff);
  	         for(i = cursor; i < SDF_BLOCK_SIZE(sdf_f->super) & k <= mida; i++) {
		     buffer[k] = buff[i];
		     k++;
	         }
	         cursor = 0;
	     } else 
	       return -1;
         }
	 
	 i_node_tmp.d_access = time(NULL);
	 write_inode(sdf_f, n_inode, &i_node_tmp);
	 
	 return k;
//  }

//  return -1;

}

i_node_str f_stat(int n_inode)
{
    i_node_str i_node_tmp;
 
    P(f_mutex, 0, 0);
    read_inode(sdf_f, n_inode, &i_node_tmp);
    V(f_mutex, 0);

    return i_node_tmp; 
}

int f_trunc(int n_inode, int n_bytes)
{
     int p_block, blocks_used = 0;
     i_node_str i_node_tmp;
    
     read_inode(sdf_f, n_inode, &i_node_tmp);
     
     p_block = n_bytes / SDF_BLOCK_SIZE(sdf_f->super);
     if (n_bytes > 0)
	 p_block++;

     P(f_mutex, 0, 0);
     
     while (p_block < N_BLOCKS) {
            if (i_node_tmp.l_blocks[p_block] != -1) {
		free_block(sdf_f, i_node_tmp.l_blocks[p_block]);
		i_node_tmp.l_blocks[p_block] = -1;
	    }
	    p_block++;
     }
	     
     V(f_mutex, 0);
     
     i_node_tmp.i_size = n_bytes;

     for(p_block=0;p_block<N_BLOCKS;p_block++) 
	 if(i_node_tmp.l_blocks[p_block] != -1)
            blocks_used++;

     		 
     i_node_tmp.i_blocks = blocks_used;
     i_node_tmp.d_modify = time(NULL);
     
     if (write_inode(sdf_f, n_inode, &i_node_tmp) < 0) {
	 printf("Error, no puc guardar l'i-node %d\n", n_inode);
	 return -1;
     }

     return 0;
}
