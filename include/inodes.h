
#define N_BLOCKS        10                      /* Numero màxim de punters dirèctes */

struct i_node {
       int              i_blocks;               /* Blocks count */
       int              i_size;                 /* Mida en bytes */
       time_t           d_create;               /* Data de creació */
       time_t           d_modify;               /* Data de modificació */
       time_t           d_access;               /* Data d'access */
       unsigned int     i_type;                 /* Tipus de fitxer */
       int              l_blocks[N_BLOCKS];     /* Llista d'apuntadors als blocs */
};

typedef struct i_node i_node_str, *i_node_ptr;

#define SDF_FREE_INODE          0x00
#define SDF_DIRE_INODE          0x01
#define SDF_FILE_INODE          0x02

int write_inode(sdf_ptr, int, i_node_str *);
int read_inode(sdf_ptr, int, i_node_str *);
int mark_inode(sdf_ptr, unsigned int);
int free_inode(sdf_ptr, int);
