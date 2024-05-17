/****/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <mm.h>
#include <stdio.h>
#include "block.h"
#include "sdf.h"
#include "inodes.h"

/* Guarda un inode determinat */

int write_inode(sdf_ptr sdf, int n_inode, i_node_str *inode)
{
    char buffer[SDF_BLOCK_SIZE(sdf)];
    int blk, offset, i;

    blk = sdf->p_in + n_inode / SDF_INODES_PER_BLOCK(sdf->super);
    i = n_inode % SDF_INODES_PER_BLOCK(sdf->super);
    offset = i * SDF_INODE_SIZE(sdf->super);
   
    if (n_inode <= sdf->super->inodes_count) {
        if (read_block(sdf->dev_blk, blk, buffer) < 0)
            return -1;
	
        memcpy(&buffer[offset], inode, SDF_INODE_SIZE(sdf->super));
        write_block(sdf->dev_blk, blk, buffer);
	return 0;
    } else return -1;
}

/* Llegeix un inode del sistema de fitxers i ens torna l'estructura */

int read_inode(sdf_ptr sdf, int n_inode, i_node_str *inode)
{
    char buffer[SDF_BLOCK_SIZE(sdf)];
    int blk, offset, i;

    blk = sdf->p_in + n_inode / SDF_INODES_PER_BLOCK(sdf->super); 
    i = n_inode % SDF_INODES_PER_BLOCK(sdf->super);
    offset = i * SDF_INODE_SIZE(sdf->super); 

    if (n_inode <= sdf->super->inodes_count) {    
        read_block(sdf->dev_blk, blk, buffer);
        memcpy(inode, &buffer[offset], SDF_INODE_SIZE(sdf->super));
    } else
        return -1;
    
#ifdef DEBUG_READI    
    printf("\nINODE_BLOCK = %d:%d\n", blk, offset);
    printf("i_blocs = %d\n",inode->i_blocks);
    printf("i_size = %d\n",inode->i_size);
#endif

    return 0;
}

int mark_inode(sdf_ptr sdf, unsigned int i_tipus)
{
    i_node_str inode_t;
    int i, j = sdf->super->first_inode;
 
    read_inode(sdf, j, &inode_t);
    
    inode_t.i_type = i_tipus;

#ifndef MALLOC_NOT_SHARED
    mm_lock(mm_super, MM_LOCK_RW);
#endif
    
    sdf->super->first_inode = inode_t.l_blocks[0];    /* ALERTA!!! Només ho modificam 
							 dins memoria, necesitam guardar-ho!!! */
#ifndef MALLOC_NOT_SHARED
    mm_unlock(mm_super);
#endif
      
    printf("PROXIM INODE DISPONIBLE = %d\n", sdf->super->first_inode);
    
    if (inode_t.l_blocks[0] < 0) 
	return -1;
    else if (inode_t.l_blocks[0] > sdf->super->inodes_count)
	     return -1;
	
    for(i=0;i<N_BLOCKS;i++)
	inode_t.l_blocks[i] = -1;
    
    write_inode(sdf, j, &inode_t);
       
    return j;
}

int free_inode(sdf_ptr sdf, int n_inode)
{
    i_node_str inode_t;	
    
    read_inode(sdf, n_inode, &inode_t);
    inode_t.i_type = SDF_FREE_INODE;
    inode_t.i_size = 0;
    inode_t.l_blocks[0] = sdf->super->first_inode;
    write_inode(sdf, n_inode, &inode_t);

#ifndef MALLOC_NOT_SHARED
    mm_lock(mm_super, MM_LOCK_RW);
#endif
    
    sdf->super->first_inode = n_inode;

#ifndef MALLOC_NOT_SHARED
    mm_unlock(mm_super);
#endif
    
}

