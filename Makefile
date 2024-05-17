TOOLS_DIR=tools
TOOLS_BINARIES=sdf_md sdf_rm sdf_ls sdf_cat 
SDF_BINARIES=ferfs $(TOOLS_BINARIES) llista_inodes llegirfs
DEBUG_ALL=-DDEBUG_MOUNT -DDEBUG_WRITEB -DDEBUG_READB
SUPPORT_MALLOC=-DMBITS_MALLOC
DBG=
INCLUDE=-I./include
#CFLAGS=-DDEBUG_MOUNT -DDEBUG_CREATE_ENTRY $(SUPPORT_MALLOC) $(DBG) $(INCLUDE) 
CFLAGS=$(SUPPORT_MALLOC) $(DBG) $(INCLUDE)
CC=gcc
LIBS=-lmm -lm
COMPILE=$(CC) $(DBG) -DDEBUG_MOUNT $(INCLUDE) $(LIBS)

all: 		$(SDF_BINARIES) server
		mv $(TOOLS_BINARIES) ./tools
		
##############################################################################
# Utilitats bàsiques del sistema d'arxius
 
ferfs:          ferfs.c blocks.o sdf.o bitmap.o inodes.o xmalloc.c
		$(COMPILE) -o ferfs ferfs.c blocks.o sdf.o bitmap.o inodes.o xmalloc.c -DMALLOC_NOT_SHARED

llegirfs:	llegirfs.c blocks.o inodes.o sdf.o bitmap.o file.o dentry_basic.o dir.o xmalloc.o 
		$(COMPILE) -o llegirfs llegirfs.c sdf.o blocks.o inodes.o bitmap.o file.o dentry_basic.o dir.o xmalloc.o server/comunica.o

llista_inodes:	llista_inodes.c blocks.o inodes.o sdf.o bitmap.o xmalloc.c
		$(COMPILE) -o llista_inodes llista_inodes.c blocks.o inodes.o sdf.o bitmap.o xmalloc.c -DMALLOC_NOT_SHARED
		
sdf_md:		$(TOOLS_DIR)/sdf_md.c blocks.o inodes.o sdf.o bitmap.c file.o dentry_basic.o dir.o xmalloc.c server/comunica.o
		$(COMPILE) -o sdf_md $(TOOLS_DIR)/sdf_md.c sdf.o blocks.o inodes.o bitmap.c file.o dentry_basic.o dir.o server/comunica.o xmalloc.c -DMALLOC_NOT_SHARED

sdf_rm:		$(TOOLS_DIR)/sdf_rm.c blocks.o inodes.o sdf.o bitmap.o file.o dentry_basic.o dir.o xmalloc.c
		$(COMPILE) -o sdf_rm $(TOOLS_DIR)/sdf_rm.c sdf.o blocks.o inodes.o bitmap.o file.o dentry_basic.o dir.o server/comunica.o xmalloc.c -DMALLOC_NOT_SHARED
		
sdf_ls:		$(TOOLS_DIR)/sdf_ls.c blocks.o inodes.o sdf.o bitmap.o file.o dentry_basic.o dir.o xmalloc.c
		$(COMPILE) -o sdf_ls $(TOOLS_DIR)/sdf_ls.c sdf.o blocks.o inodes.o bitmap.o file.o dentry_basic.o dir.o server/comunica.o xmalloc.c -DMALLOC_NOT_SHARED

sdf_cat:	$(TOOLS_DIR)/sdf_cat.c blocks.o inodes.o sdf.o bitmap.o file.o dentry_basic.o dir.o xmalloc.c
		$(COMPILE) -o sdf_cat $(TOOLS_DIR)/sdf_cat.c sdf.o blocks.o inodes.o bitmap.o file.o dentry_basic.o dir.o server/comunica.o xmalloc.c -DMALLOC_NOT_SHARED

comunica:	server/comunica.c server/comunica.h
		$(CC) -c server/comunica.c
		
##############################################################################
# Moduls interns del sistema de fitxers SDF

blocks:		blocks.c block.h 
		$(CC) -c blocks.c 
	
bitmap:		bitmap.c bitmap.h block.h sdf.h
		$(CC) -c bitmap.c 
		
inodes:         inodes.c inodes.h sdf.h block.h
		$(CC) -c inodes.c 

sdf:            sdf.c sdf.h
		$(CC) -c sdf.c
		
file:		file.c file.h 
		$(CC) -c file.c

dentry_basic:	dentry_basic.c dentry_basic.h
		$(CC) -c dentry_basic.c
		
dir:		dir.c dir.h
		$(CC) -c dir.c

xmalloc:	xmalloc.c xmalloc.h
		$(CC) -c xmalloc.c


##############################################################################
# Clean-up!

clean:		
		rm -f *.o $(SDF_BINARIES)
