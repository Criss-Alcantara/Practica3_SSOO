#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <string.h>

/* Definicion de la cola */
typedef struct Queue{
  int capacity; /* capacidad maxima de la cola*/
  int size; /* numero de elementos actuales */
  int front; /* posicion del primer elemento */
  int rear; /* posicion del ultimo elemento */
  struct plane **aviones; /* buffer: array de punteros a estructuras plane */
}Queue;

/* Creamos la cola */
Queue * Q;

/*To create a queue*/
int queue_init(int size){
  /* Reservamos espacio para la cola */
  Q = (Queue *)malloc(sizeof(Queue));
  
  /* Fallo en malloc */
  if(Q == NULL){
    printf ("ERROR queue_init: fallo en el malloc.\n");
    return -1;
  }
  
  /* Declaramos sus parametros, argumento 'size' se corresponde con 
     el numero maximo de aviones que se almacenaran */
  Q->aviones = (struct plane **)malloc(sizeof(struct plane*) * size);
  
  /* Fallo en malloc */
  if(Q->aviones == NULL){
    printf ("ERROR queue_init: fallo en el malloc.\n");
    return -1;
  }
    /* int i; */
  /* for (i=0; i<size; i++){ */
    /* Q->aviones [i] =NULL; */
  /* } */
  
  Q->capacity = size;
  Q->size = 0;
  Q->front = 0;
  Q->rear = 0;
  printf ("[QUEUE] Buffer initialized\n");
  return 0;
}

/* To Enqueue an element*/
int queue_put(struct plane* x) {
  /* Comprobamos que la cola no este llena */
  if(Q->size == Q->capacity){
    printf ("ERROR queue_put: la cola esta llena.\n");
    return -1;
  }
  /* Aumentamos tamaño actual, desplazamos el puntero de cola y actualizamos la cola */
  Q->aviones[Q->rear] = x;
  Q->size++;
  Q->rear = (Q->rear + 1) % Q->capacity;

  printf ("[QUEUE] Storing plane with id %d\n",x->id_number);
  return 0;
}

/* To Dequeue an element.*/
struct plane* queue_get(void) {
  /* Comprobamos que la cola no este vacia */
  if (Q->size==0){
    printf ("ERROR queue_get: la cola esta vacia.\n");
    return NULL;
  }
  
  /* Creamos el elemento que retornaremos y reservamos espacio */
  struct plane* avion;
  avion = (struct plane*)malloc(sizeof(struct plane));
  
  /* Fallo en malloc */
  if(avion == NULL){
    printf ("ERROR queue_get: fallo en el malloc.\n");
    return NULL;
  }
  
  /* Almacenamos en la variable  el primer elemento de la cola, 
     decrementamos el tamaño actual y desplazamos el puntero del primer elemento */
  avion = Q->aviones[Q->front];
  Q->aviones[Q->front] = NULL;
  Q->size--;
  Q->front = (Q->front + 1) % (Q->capacity);

  printf ("[QUEUE] Getting plane with id %d\n",avion->id_number);
  return avion;
}


/*To check queue state*/
int queue_empty(void){
  /* Devuelve 1 si la cola esta vacía y 0 si no lo esta */
  if(Q->size == 0){
    return 1;
  }
  return 0;
}

/*To check queue state*/
int queue_full(void){
  /* Devuelve 1 si la cola esta llena y 0 si no lo esta */
  if(Q->size == Q->capacity){
    return 1;
  }
  return 0;
}

/*To destroy the queue and free the resources*/
int queue_destroy(void){
  /*Liberar los recursos*/
  int i;
  for (i = 0; i < Q->size; i++){
    /* destruye_avion (Q->aviones [(Q->front + i) % Q->capacity]); */
    free (Q->aviones [(Q->front + i) % Q->capacity]);
  }
  free (Q->aviones);
  free (Q); 
  return 0;
}
