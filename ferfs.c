#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <mm.h>
#include "block.h"
#include "sdf.h"
#include "inodes.h"

const char * program_name = "ferfs";
static ptr_sb super;
static dev_ptr d_blk;

static void usage(void)
{
       fprintf(stderr, "Usage: %s  [-b block-size] device [blocks-count]\n", program_name);
       exit(1);
}

void crea_sb(char *nom_volum, int n_blocks, int t_bloc)
{
    char *buffer;
    buffer = (char *) malloc (t_bloc);
    
    super->magic = SDF_MAGIC_NUMBER; 
    super->block_size = t_bloc;
    super->inode_size = sizeof(i_node_str);
    super->size = n_blocks * t_bloc;
    super->blocks_count = n_blocks;
    super->inodes_count = SDF_INODE_BLOCKS(super) * SDF_INODES_PER_BLOCK(super);
    super->first_inode = 0;                      /* El zero està reservat per l'arrel (/) */
    time(&super->t_create);
    
    strcpy(super->nom_volum, nom_volum);

    memcpy(buffer, super, sizeof(str_sb));
    write_block(d_blk, 0, buffer);
    free(buffer);
}

int crea_bitmap()
{  
    int n_b = (super->blocks_count/8)+1;           // Numero de bytes que ocupen els blocs
    int i, b;
    char *buffer;
    buffer = (char *) malloc(SDF_BLOCK_SIZE(super));
   
    for(i=0; i <= SDF_BLOCK_SIZE(super);i++) 
        buffer[i] = 0;
    
    for (b=1;b<=(n_b/SDF_BLOCK_SIZE(super))+1;b++) 
	 write_block(d_blk, b, buffer);
 
    free(buffer);
    
    if((super->blocks_count/8)%SDF_BLOCK_SIZE(super) != 0)  
      return (n_b/SDF_BLOCK_SIZE(super))+2;
    else 
      return (n_b/SDF_BLOCK_SIZE(super))+1;
    
}

int crea_inodes(int inici_bloc) 
{
    i_node_str empty_inode;
    char buff_ino[SDF_BLOCK_SIZE(super)];
    int inodes_per_block, blk, i,j,offset;
    int process = 0, k = 0, l;
   
    inodes_per_block = SDF_BLOCK_SIZE(super) / sizeof(i_node_str);

    for(blk = inici_bloc;blk <= SDF_INODE_BLOCKS(super) + 1;blk++) {
        memset(&buff_ino, 0, SDF_BLOCK_SIZE(super));
	for(i=0;i<=inodes_per_block-1;i++) {
  	    empty_inode.i_type = SDF_FREE_INODE;
	    empty_inode.i_blocks = 0;
	    empty_inode.i_size = 0;
	    empty_inode.l_blocks[0] = k+1;
            for(l=1;l<=N_BLOCKS-1;l++) empty_inode.l_blocks[l] = -1;
	    
	    j = i % inodes_per_block;
	    offset = j * SDF_INODE_SIZE(super);
	    
	    memcpy(&buff_ino[offset], &empty_inode, SDF_INODE_SIZE(super));
	    k++;
        }
	write_block(d_blk, blk, buff_ino);
	printf("%d ",blk);
    }

    printf("\nNombre de inodes: %d\n", k); //SDF_INODE_BLOCKS(super) * inodes_per_block);
}

int main(int argc, char **argv)
{  
    sdf_ptr n_sdf;
    i_node_str i_node_tmp;
    int nb_bmp, i, t_bloc, num_blocs, parm = 1,blk,retval, n_inode;
    char nom_fitxer[255];
        
    if (argc <= 2) 
        usage();   

    if (strncmp(argv[1],"-b",2)==0) {
        t_bloc = atoi(argv[2]);
        if (!t_bloc) {
           fprintf(stderr, "Falta especificar la mida dels blocs en bytes\n");
           exit(1);
        } else
           if (t_bloc < SDF_MIN_BLOCK_SIZE) {
               fprintf(stderr, "La mida minima del bloc és de %d Bytes\n",SDF_MIN_BLOCK_SIZE);
	       exit(1);
	   } else
             if (t_bloc > SDF_MAX_BLOCK_SIZE) {
                fprintf(stderr, "La mida màxima del bloc és de %d Bytes\n",SDF_MAX_BLOCK_SIZE);
	        exit(1);
	     }

	parm += 2;
    } else
        t_bloc = SDF_MIN_BLOCK_SIZE;
    
    if (argv[parm]) 
       strcpy(nom_fitxer, argv[parm]);
     
    if (argv[parm+1]) {
       num_blocs = atoi(argv[parm+1]);
       if(!num_blocs) {
          fprintf(stderr, "Falta especificar el nombre de blocs\n");
          exit(1);
       }   
    } else {
      fprintf(stderr, "Falta especificar el nombre de blocs\n");
      exit(1);
    }
    
    d_blk = open_dev_blk(nom_fitxer, t_bloc, num_blocs);
    
    if (!d_blk) {
         fprintf(stderr, "Error al crear el sistema d'arxius\n");
 	 exit(-1);
    }
   
    super = (ptr_sb) malloc(sizeof(str_sb));         /* Reservam memoria pel superbloc */

    printf("Creant superbloc...\n");
    crea_sb("SDF_IMG", num_blocs, t_bloc);      /* Cream el superbloc al dispositiu */

    printf("Creat el mapa de bits dels blocs ...\n");
    nb_bmp = crea_bitmap();                       /* Cream el BITMAP dels blocs */

    printf("Creat els inodes ...\n");
    crea_inodes(nb_bmp);                          /* Cream els inodes */
  
    printf("Inicialitzan els blocs de dades ...\n");
    
    char *buffer = (char *) malloc(SDF_BLOCK_SIZE(super));
    memset(buffer, 0, SDF_BLOCK_SIZE(super));
    for(blk = SDF_INODE_BLOCKS(super)+2;blk <= SDF_INODE_BLOCKS(super)+SDF_DATA_BLOCKS(super)+2;blk++) {
	printf(".");
	write_block(d_blk, blk, buffer);
    }
    printf("\n");
    printf("Nombre de blocs de dades: %d\n", (SDF_INODE_BLOCKS(super)+SDF_DATA_BLOCKS(super)) -
		                             SDF_INODE_BLOCKS(super));
    close_dev_blk(d_blk);
    free(super);
    free(buffer);

    /* Acaba de perfilar el SDF **************************************************/
   
    printf("Marcant blocs utilitzats pel sistema ...\n");
    n_sdf = mount_fs(nom_fitxer, 0, &retval);

    if (!n_sdf) {
        printf("Error montant el sistema d'arxius (%04x)\n",retval);
        exit(-1);
    }
   
    read_bbitmap(n_sdf);       // Carregam el mapa de bits a memoria
    
    // Marcam el bloc 0 i els dels inodes com a ocupats
    
    for(blk=0;blk<=n_sdf->nb_mb;blk++)
	mark_block(n_sdf, blk); 
   
    for(blk=n_sdf->nb_mb;blk<=SDF_INODE_BLOCKS(n_sdf->super)+2;blk++)
	mark_block(n_sdf, blk);
  
    
    n_inode = mark_inode(n_sdf, SDF_DIRE_INODE);    
    read_inode(n_sdf, n_inode, &i_node_tmp);
    time(&i_node_tmp.d_create);
    write_inode(n_sdf, n_inode, &i_node_tmp);

			      
    umount_fs(n_sdf);
    exit(0);
}
