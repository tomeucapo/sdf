
#ifndef  _COMUNICA_H
#define  _COMUNICA_H

#define OP_DIR		0xF001
#define OP_STATF	0xF002
#define OP_READF	0xF003
#define OP_WRITEF	0xF004
#define OP_CREATF	0xF005
#define OP_MKDIR	0xF006
#define OP_UNLINK	0xF007

#define MAX_BUF_MSG	1024

/* Definició d'estructures utilizades pel pas de missatges
 * entre clients i servidors
 */

typedef struct {
        int opcode;
        char nom_fitxer[30];
	int pos_ini;
	int n_bytes;
	char buff[MAX_BUF_MSG];
} s_ordre;

typedef struct {
        long tipus;
        s_ordre comanda;
} s_miss;

int crea_sem(int, int);                 /* Utilizació de semàfors */
void elimina_sem(int);
void init_sem(int, int);
	
void P(int, int, int);                  /* Proveerem (Wait) */
void V(int, int);                       /*           (Signal) */

int agafa_coa(int, int);                      /* Pas de missatges */
void elimina_coa(int);	

#endif 
