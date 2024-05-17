
#define DEV_NAME_LEN  20

struct io_manager_str {
       int fd;
       int b_size;
       int n_blocks;
       char nom_dev[DEV_NAME_LEN];
};

typedef struct io_manager_str dev_str, *dev_ptr; 

dev_ptr open_dev_blk(char *, int, int);
void block_size(dev_ptr, int);
void blocks_count(dev_ptr, int);
int write_block(dev_ptr, int, char *);
int read_block(dev_ptr, int, char *);
int close_dev_blk(dev_ptr);
