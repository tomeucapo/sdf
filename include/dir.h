
#define LEN_LINIA_DIR		45
#define LEN_NAME   		30
#define SDF_FILE_EXISTS		0xE000
#define SDF_DIR_NOT_EXISTS	0xE001
#define DIR_SEPARATE		"/"

/* Representació d'una entrada de directori */

struct directori {                            
       int	p_inode;
       char	nom[LEN_NAME];
};

typedef struct directori str_dir, *ptr_dir;

/* Estructura que dona la informació d'un fitxer */

struct d_stat {                            
       int	f_size_bytes;
       int	f_size_blks;
       time_t	f_t_create;
       time_t	f_t_modify;
};


int montar(char *);
void desmontar();

int gen_creat(char *, unsigned int);
int d_creat(char *);
int d_mkdir(char *);
int d_unlink(char *);
int d_write(char *, int, int, char *);
int d_read(char *, int, int, char *);
struct d_stat d_stat(char *);
int d_list_dir(char *, char *);
