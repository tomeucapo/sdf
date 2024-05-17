#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <mm.h>
#include "dir.h"

#define SDF_DIRE_INODE		0x01	

int main(int argc, char **argv)
{ 
    int retval = -1, num_entr = 0,i=0,mida, entr=0, tipus;
    char str_date[19];
    char nom_fitxer[LEN_NAME];
    char *buffer;
    
    if (argc == 1) {
	printf("Falta especificar l'imatge a utilizar\n");
	exit(retval);
    }
    
    if (montar(argv[1]) < 0) {
        perror("Error montant el sistema d'arxius!");
        exit(retval);
    }

    if (argv[2]) {
	buffer = (char *) malloc(15192);
        num_entr = d_list_dir(argv[2], buffer);
	if (num_entr >= 0) {
  	   printf("%d Fitxer(s)\n", num_entr);
   	   while (entr <= num_entr-1) {
               sscanf(&buffer[entr*LEN_LINIA_DIR],"%10d;%02d;%30s", &mida, &tipus, nom_fitxer);
	       printf("%10d\t%c\t%s\n",mida
			              ,((tipus == SDF_DIRE_INODE) ? 'D' : 'F')
			              ,nom_fitxer);
	       entr++;
	   }
	} else 
          printf("Error al llistar %s\n", argv[2]);
    } else
	printf("Falta especificar el directori a llistar\n");
    
    desmontar();

    exit(retval);
}
