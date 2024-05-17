

int seek_entry(int, char *, str_dir *);         // Cerca una entrada dins un directori i ens torna la seva posició
int cercar_element_dir(int, char *);		// Cerca una entrada a un directori i ens torna l'inode aqui on apunta
int namei(char *, char *);			// Ens torna el inode d'un directori base
int read_entry(int, int, ptr_dir);		// Ens llegeix una entrada del directori
int create_entry(int, char *, unsigned int);	// Genera una entrada nova a un directori
void copy_entry(int, int, int);			// Duplica una entrada dins el directori (només!)
int delete_entry(int, char *);			// Elimina una entrada d'un directori	

