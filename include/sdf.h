
#ifndef SDF
#define SDF

#define SDF_FITXER_INCORRECTE       0xAAA1
#define SDF_FITXER_INEXISTENT       0xAA01
#define SDF_BAD_MAGIC		    0xFFFF
#define SDF_EMPTY	            0xFFF1

#define SDF_MAGIC_NUMBER            0x1BADFACE
#define SDF_MIN_BLOCK_SIZE          1024
#define SDF_MAX_BLOCK_SIZE          4096
#define BLK_DEF_SIZE                1024

typedef struct super_block str_sb, *ptr_sb;

struct sdf_struct {
       int 		block_size;               /* Tamany dels blocs */
       int              p_mb;                     /* Punter al primer bloc del mapa de bits */
       int              p_in;                     /* Punter al primer bloc dels i-nodes */
       int              p_dades;                  /* Punter al primer bloc de les dades */
       int		nb_mb;		          /* Nombre de blocs del mapa de bits */
       ptr_sb 		super;                    /* Apuntador al superbloc */
       dev_ptr          dev_blk;                  /* Dispositiu de blocs */
       unsigned char    *mbits;                   /* Copia a memoria del mapa de bits */
};

typedef struct sdf_struct sdf_str, *sdf_ptr;

struct super_block {
       int      magic;               /* Magic number */
       char     nom_volum[16];       /* Nom del volum */
       int      size;                /* Tamany en bytes del sistema de fitxers */
       int      block_size;          /* Mida dels blocs */
       int      inode_size;          /* Mida dels inodes */
       int      blocks_count;        /* Nombre de blocks */
       int      inodes_count;        /* Nombre de i-nodes */
       int      first_inode;         /* Primer i-node no reservat */ 
       time_t   t_create;            /* Data de creació del sistema d'arxius */
       time_t   t_mount;             /* Data del darrer montatge */
};

#define SDF_BLOCK_SIZE(s)    		((s)->block_size)
#define SDF_INODE_SIZE(s)    		((s)->inode_size)
#define SDF_INODES_PER_BLOCK(s)		((s)->block_size / (s)->inode_size)
#define SDF_DATA_BLOCKS(s)   		(((s)->blocks_count-3) * (s)->block_size) / \
                                         ((s)->block_size+((s)->inode_size/2)+(1/8))

#define SDF_INODE_BLOCKS(s)  		((s)->blocks_count - 3 - SDF_DATA_BLOCKS((s)))
	
#define SDF_FIRST_INOD(s)  	        ((s)->first_inode)

sdf_ptr mount_fs(char *, int, int *);
str_sb read_super_block(sdf_ptr);
void write_super_block(sdf_ptr);
int dump_block(sdf_ptr, int);
int umount_fs(sdf_ptr);

#ifndef MALLOC_NOT_SHARED
MM *mm_sdf, *mm_super, *mm_mbits;
#endif


#endif
