#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "dir.h"

int main(int argc, char **argv)
{ 
    int retval = -1;
    char buf[1024];
    int len, p = 0;
    struct d_stat fitxer_s;
    
    if (argc == 1) {
	printf("Falta especificar l'imatge a utilizar\n");
	exit(retval);
    }
    
    if (montar(argv[1]) < 0) {
        perror("Error montant el sistema d'arxius!");
        exit(retval);
    }
   
    
    if (argv[2]) {
        bzero(&fitxer_s, sizeof(fitxer_s));
        fitxer_s = d_stat(argv[2]);

        if (fitxer_s.f_size_bytes != 0) {
           while ((len = d_read(argv[2], p, sizeof(buf), buf)) > 0) {
                 if (write(1, buf, len) != len) {
                     printf("Error d'escriptura!\n");
	             return 1;
	         } 
	         p = p + sizeof(buf);
           }
        } else len = -1;

        if (len < 0) 
            printf("Error de lectura");
    } else
	printf("Falta especificar el directori a llistar\n");
    
    desmontar();

    exit(retval);
}
