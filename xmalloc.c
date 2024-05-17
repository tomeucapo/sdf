/*
 * xmalloc.c
 * Interficie per fer transparent el maneig de memòria dinàmica
 * per si volem utilizar memòria compartida o no.
 * 
 * TCC 2004 
 */

#include "xmalloc.h"

void *sdf_get_mem(MM *mms, size_t size)
{
#ifdef MALLOC_NOT_SHARED
       return (malloc(size));
#else
       return (mm_malloc(mms, size));
#endif
}

void sdf_free_mem(MM *mms, void *ptr)
{
#ifdef MALLOC_NOT_SHARED
       free(ptr);
#else
       mm_free(mms, ptr);
#endif
}

