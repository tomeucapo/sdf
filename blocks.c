#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include "xmalloc.h"
#include "block.h"

/****************************************************************************************
 * Funció per obrir el sistema de blocks.
 * Si no existeix el fitxer el crea buid amb uns paràmetres per defècte
 ****************************************************************************************/

#ifndef MALLOC_NOT_SHARED
static MM *mm_dev_blk;
#endif

dev_ptr open_dev_blk(char *nom, int bsize, int nblocks)
{
    int fd;
    dev_ptr dev_g;

#ifndef MALLOC_NOT_SHARED
    mm_dev_blk = mm_create(sizeof(dev_str), NULL);
#endif
    
    dev_g = (dev_ptr) sdf_get_mem(mm_dev_blk, sizeof(dev_str));
    
    dev_g->fd = open(nom, O_RDWR);
    dev_g->b_size = bsize;
    dev_g->n_blocks = nblocks;
    strcpy(dev_g->nom_dev, nom);
   
    if (dev_g->fd < 0) { 
	/* Si no existeix el cream buid */
	dev_g->fd = open(nom, O_CREAT | O_WRONLY, 0644);
        
	if (dev_g->fd < 0) {
	    sdf_free_mem(mm_dev_blk, dev_g);
            return(NULL);
	}
    } 
    return(dev_g);
}

void block_size(dev_ptr dev_g, int bsize)
{
     dev_g->b_size = bsize;
}

void blocks_count(dev_ptr dev_g, int count)
{
     dev_g->n_blocks = count;
}

int write_block(dev_ptr dev_g, int n_block, char *buffer)
{
    int size;

    if (n_block <= dev_g->n_blocks) {
       lseek(dev_g->fd, n_block*dev_g->b_size, SEEK_SET);
       size = write(dev_g->fd, buffer, dev_g->b_size);
    } else  
       size = -1;
    
#if DEBUG_WRITEB
    printf("[%d] WRITE_BLOCK %d: %d Bytes\n", dev_g->fd, n_block,size);
#endif
    return size;
}

int read_block(dev_ptr dev_g, int n_block, char *buffer)
{
    int size;
    
    if (n_block <= dev_g->n_blocks) {
       lseek(dev_g->fd, n_block * dev_g->b_size, SEEK_SET);
       size = read(dev_g->fd, buffer, dev_g->b_size);
    } else
       size = -1;
	    
#if DEBUG_READB
    printf("[%d] READ_BLOCK %d: %d Bytes\n", dev_g->fd, n_block,size);
#endif
    return size;
}

int close_dev_blk(dev_ptr dev_g)
{
    close(dev_g->fd);     /* Tancam el sistema de fitxers i alliberam espai */
    
    sdf_free_mem(mm_dev_blk, dev_g);

#ifndef MALLOC_NOT_SHARED    
    mm_destroy(mm_dev_blk);
#endif

}
