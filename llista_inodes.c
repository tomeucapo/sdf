#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <mm.h>
#include "block.h"
#include "sdf.h"
#include "inodes.h"
#include "bitmap.h"
#include "xmalloc.h"

int main(int argc, char **argv)
{
     int retval = 0, i, in = 0;
     i_node_str i_node_tmp;
     sdf_ptr sdf_f;

     if (argc == 1) {
         printf("Falta especifical la imatge\n");
	 exit(1);
     }

     sdf_f = mount_fs(argv[1], 0, &retval);

     if (!sdf_f) {
         printf("Error al montar el sistema d'arxius: %04x\n", retval);
	 exit(1);
     }else
         read_bbitmap(sdf_f);

	
     for(i=0;i<=sdf_f->super->inodes_count;i++) {
         printf("Inode %d\n",i);
	 read_inode(sdf_f, i, &i_node_tmp);
	 for(in=0;in<=9;in++)
  	     printf("          P%d = %d\n",in, i_node_tmp.l_blocks[in]);
     }

     printf("Seguent inode lliure: %d\n", sdf_f->super->first_inode);
     umount_fs(sdf_f);
     exit(0);
}
