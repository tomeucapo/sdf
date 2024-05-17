
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include "dir.h"

int main(int argc, char **argv)
{ 
    int retval = -1;
    
    if (argc == 1) {
	printf("Falta especificar l'imatge a utilizar\n");
	exit(retval);
    }
    
    if (montar(argv[1]) < 0) {
        perror("Error montant el sistema d'arxius!");
        exit(retval);
    }

    if (argv[2]) {
        retval = d_mkdir(argv[2]);
        if (retval)
            printf("Error al crear el directori\n");
    } else
	printf("Falta especificar el directori a crear\n");
    
    desmontar();

    exit(retval);
}
