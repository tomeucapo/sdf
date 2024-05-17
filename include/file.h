

int f_mount(char *);
int f_umount();
int f_create(unsigned int);
int f_unlink(int);
int f_trunc(int, int);
int f_read(int, int, int, char *);
int f_write(int, int, int, char *);
i_node_str f_stat(int);
