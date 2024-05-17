INCLUDE=-I../include
SDF_OBJS=../blocks.o ../inodes.o ../sdf.o ../bitmap.o ../file.o ../dentry_basic.o ../dir.o ../xmalloc.o
COMPILE=gcc $(INCLUDE) -lmm -lm
CFLAGS=$(INCLUDE) -O2

all:		agent simulador
	
agent:		agent.c servidor.o comunica.o $(SDF_OBJS)
		$(COMPILE) -o agent agent.c servidor.o comunica.o $(SDF_OBJS) 
		
simulador:	simulador.c client.o comunica.o dir_clt.o
		$(COMPILE) -o simulador simulador.c client.o comunica.o dir_clt.o
		
comunica: 	comunica.h comunica.c
		cc -c comunica.c

servidor:	servidor.c servidor.h
		cc -c servidor.c

client:		client.c client.h
		cc -c client.c

dir_clt:	dir_clt.c dir_clt.h
		cc -c dir_clt.c

clean:
		rm -f *.o agent simulador
		killall -TERM agent
		sh clear_ipcs
