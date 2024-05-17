#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <mm.h>
#include "block.h"
#include "sdf.h"
#include "inodes.h"
#include "bitmap.h"
#include "file.h"
#include "dir.h"

int d_printf(char *nom_fitxer, char *cadena)
{
    int l_cad = strlen(cadena);
    struct d_stat file_stat;
    
    file_stat = d_stat(nom_fitxer);
    return (d_write(nom_fitxer, file_stat.f_size_bytes, strlen(cadena), cadena));
}

fill(int pid)
{
    int l,i = 0, len = 0, len_i = 0;
    time_t clock;
    struct tm *tm;
    char linia[40];
    char nom_fitxer[20];
    
    sprintf(nom_fitxer,"/fill-%4d%02d.dat",getpid(),pid);
    d_creat(nom_fitxer);
    
    sprintf(linia, "Inici log client %d\n",getpid());
    d_printf(nom_fitxer, linia);
    
    for(l=0;l<=99;l++) {
	time(&clock);
	tm = gmtime(&clock);
	
        sprintf(linia, "Linia número %d\n", l);
	d_printf(nom_fitxer, linia);
    }

    sprintf(linia, "Fi log client %d\n",getpid());
    d_printf(nom_fitxer, linia);
}

int main(int argc, char **argv)
{  
    int pid_fill[2];
    int n_in;
    char msg[20]="Això és un missatge";
         
    if(argc == 1) { 
       printf("Falta un paràmetre\n");
       exit(-1);
    }

    printf("Montant la imatge...\n");
    if (montar(argv[1]) < 0) {
	perror("Error montant el sistema d'arxius!");
	MM_destroy();
        exit(-1);
    }

    printf("Creant fitxers de processos ...\n");
    
    for(n_in=0;n_in<=99;n_in++)
	fill(n_in);
    
    printf("Desmontat la imatge...\n");


    desmontar();

  //  MM_destroy();
    exit(0);
}

