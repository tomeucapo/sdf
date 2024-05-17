#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <mm.h>
#include "block.h"
#include "sdf.h"


void read_bbitmap(sdf_ptr sdf_bm)
{
     int i;
     for(i=0;i<sdf_bm->nb_mb;i++)
         read_block(sdf_bm->dev_blk, i+1, &sdf_bm->mbits[i*SDF_BLOCK_SIZE(sdf_bm->super)]);
/*
#ifndef MALLOC_NOT_SHARED
     printf("read_bbitmap\n");
     mm_display_info(mm_mbits);
     mm_display_info(mm_sdf);
#endif
*/
}

void write_bbitmap(sdf_ptr sdf)
{
     int i;
     for(i=0;i<sdf->nb_mb;i++)
        write_block(sdf->dev_blk, i+1, &sdf->mbits[i*SDF_BLOCK_SIZE(sdf->super)]);
}

int mark_block(sdf_ptr sdf, int n_bloc) 
{
    unsigned maskb = 0x80;   
    int num_blk, num_bit, num_byte;

    num_bit = n_bloc % 8;
    num_byte = (n_bloc / 8) % SDF_BLOCK_SIZE(sdf->super);
    num_blk = n_bloc/(8*SDF_BLOCK_SIZE(sdf->super));
    
    if (num_bit > 0) maskb >>= num_bit;

#ifndef MALLOC_NOT_SHARED
    mm_lock(mm_mbits, MM_LOCK_RW);
#endif
    
    sdf->mbits[num_byte] |= maskb;
    
#ifndef MALLOC_NOT_SHARED
    mm_unlock(mm_mbits);
#endif
    
    write_block(sdf->dev_blk, 1+num_blk, sdf->mbits);
}

/* Cerca el primer bloc lliure dins el mapa de bits
 * tornant el nombre de bloc disponible
 */

int seek_free_block(sdf_ptr sdf)
{
    unsigned char bbyte;
    unsigned char maski = 0x80;
    int i=0,j;
    
    for(j = 0;sdf->mbits[j] == 255;j++);
    bbyte = sdf->mbits[j];
    
    while (bbyte & maski) {
           bbyte<<= 1;
           i++;
    }
   
    return(j*8+i);
}

/* Allibera un bloc en el mapa de bits dels blocs */

void free_block(sdf_ptr sdf, int n_bloc)
{
     unsigned char maski = 0x80;
     int num_blk, num_bit, num_byte;

     num_bit = n_bloc % 8;
     num_byte = (n_bloc / 8) % SDF_BLOCK_SIZE(sdf->super);
     num_blk = n_bloc/(SDF_BLOCK_SIZE(sdf->super)*8);
	     
     if (num_bit > 0)  
	 maski >>= num_bit;
     
#ifndef MALLOC_NOT_SHARED     
     mm_lock(mm_mbits, MM_LOCK_RW);
#endif
     
     sdf->mbits[num_byte] &= ~maski;

#ifndef MALLOC_NOT_SHARED
     mm_unlock(mm_mbits);
#endif
     
     write_block(sdf->dev_blk, 1 + num_blk, sdf->mbits);
}

