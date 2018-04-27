/*
*
*DO NOT MODIFY THIS FILE
*
*/


#ifndef HEADER_FILE
#define HEADER_FILE

#include "plane.h"

int queue_init (int size);
int queue_destroy (void);
int queue_put (struct plane* pln);
struct plane * queue_get(void);
int queue_empty (void);
int queue_full(void);

#endif
