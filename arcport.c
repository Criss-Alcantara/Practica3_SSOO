#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <pthread.h>
#include <unistd.h>

#define NUM_TRACKS 1 /* Funcionalidad adicional no implementada */
#define DELAY 1.0 /*Sleeps cortos para pruebas*/

pthread_cond_t no_lleno; /* controla el llenado del buffer */
pthread_cond_t no_vacio; /* controla el vaciado del buffer */


pthread_mutex_t mutex_buffer;

pthread_mutex_t mutex_id_creacion;
int id = -1;  /* Variable sera compartida, usara secciones criticas*/
int siguiente = 0;

pthread_mutex_t mutex_insercion;
pthread_cond_t insertado_jefe; /* controla el llenado del buffer */
pthread_cond_t insertado_radar; /* controla el vaciado del buffer */

int ultimo = -1; /* Variable sera compartida, usara secciones criticas*/

pthread_t th_jefe, th_radar, th_control;

/* parametros que dependen del modo de ejecucion */
int n_take_off, t_take_off;
int n_land, t_land;
int max_buffer;

struct plane ** despegues; /*array de punteros a aviones que despegan */
struct plane ** aterrizajes; /*array de punteros a aviones que aterrizan */

void print_banner()
{
  printf("\n*****************************************\n");
  printf("Welcome to ARCPORT - The ARCOS AIRPORT.\n");
  printf("*****************************************\n");
}

void print_bye()
{
  printf("\n********************************************\n");
  printf(" ---> Thanks for your trust in us <---\n");
  printf("********************************************\n\n");
}

/* Cabeceras de las funciones de los hilos */
void jefe_pista (void);
void radar_aereo (void);
void torre_control (void); 

int main(int argc, char ** argv) {

  /* Comprobar los argumentos */
  if ((argc != 1)&&(argc != 6)){
    printf ("usage 1: ./arcport\n");
    printf ("usage 2: ./arcport <n_planes_takeoff> <time_to_takeoff> <n_planes_to_arrive> <time_to_arrive> <size_of_buffer>\n");
    exit (-1);
  }

  /* Asignacion del valor de los parametros segun el modo de ejecucion */
  n_take_off = (argc == 1)? 4 : atoi(argv [1]);
  t_take_off = (argc == 1)? 2 : atoi(argv [2]);
  n_land     = (argc == 1)? 3 : atoi(argv [3]);
  t_land     = (argc == 1)? 3 : atoi(argv [4]);
  max_buffer = (argc == 1)? 6 : atoi(argv [5]);

  /* Comprobar los valores de los argumentos */
  if (max_buffer <= 0){
    printf ("ERROR el tamano del buffer debe ser mayor que 0.\n");
    exit (-1);
  }

  if ((n_take_off < 0)||(t_take_off < 0)||(n_land < 0)||(t_land < 0)){
    printf ("ERROR los argumentos no pueden ser negativos.\n");
    exit (-1);
  }

  /* Caso en que haya 0 aviones que despeguen o aterricen */
  if ((n_take_off == 0)||(n_land == 0)){
    ultimo = 1; /* El ultimo en crearse sera el ultimo global */
  }

  print_banner();
  
  printf ("n_takeoff=%d t_takeoff=%d; n_land=%d t_land=%d; buffer=%d ultimo=%d\n", n_take_off,t_take_off,n_land,t_land,max_buffer,ultimo);/*debug borrar*/
  
  queue_init (max_buffer); /* Inicializacion del buffer circular */

  /* Reserva de memoria para los aviones que van a despegar */
  int i;
  despegues = (struct plane **)malloc(sizeof(struct plane*) * n_take_off);
  if(despegues == NULL){
    printf ("ERROR fallo en el malloc.\n");
    exit (-1);
  }

  for (i=0;i<n_take_off;i++){
    despegues [i] = (struct plane*)malloc(sizeof(struct plane));
    if(despegues [i] == NULL){
      printf ("ERROR fallo en el malloc.\n");
      exit (-1);
    }
  }
  
  /* Reserva de memoria para los aviones que van a aterrizar */
  
  aterrizajes = (struct plane **)malloc(sizeof(struct plane*) * n_land);
  if(aterrizajes == NULL){
    printf ("ERROR fallo en el malloc.\n");
    exit (-1);
  }
  
  for (i=0;i<n_land;i++){
    aterrizajes [i] = (struct plane*)malloc(sizeof(struct plane));
    if(aterrizajes [i] == NULL){
      printf ("ERROR fallo en el malloc.\n");
      exit (-1);
    }
  }
  
  
  pthread_mutex_init(&mutex_buffer, NULL);
  pthread_mutex_init(&mutex_id_creacion, NULL);
  pthread_mutex_init(&mutex_insercion, NULL); 
  pthread_cond_init(&no_lleno, NULL);
  if (pthread_cond_init(&no_vacio, NULL) != 0){
    printf ("ERROR fallo al inicializar la variable condicion.\n");
    exit (-1);
  }
  pthread_cond_init(&insertado_jefe, NULL);
  pthread_cond_init(&insertado_radar, NULL);

  pthread_create(&th_control, NULL,(void *) torre_control, NULL);
  pthread_create(&th_jefe, NULL,(void *) jefe_pista, NULL);
  pthread_create(&th_radar, NULL,(void *) radar_aereo, NULL);
  
  

  pthread_join(th_jefe, NULL);
  pthread_join(th_radar, NULL);
  pthread_join(th_control, NULL);
  

   /* Liberacion de recursos */

  pthread_mutex_destroy(&mutex_buffer);
  pthread_mutex_destroy(&mutex_id_creacion);
    pthread_mutex_destroy(&mutex_insercion);
  pthread_cond_destroy(&no_lleno);
  
  if (pthread_cond_destroy(&no_vacio) != 0){
    printf ("ERROR fallo al destruir la variable condicion.\n");
    exit (-1);
  }
  pthread_cond_destroy(&insertado_jefe);
  pthread_cond_destroy(&insertado_radar);

 
  for (i=0; i<n_take_off;i++){
    free (despegues [i]);
  }
  free (despegues);

  for (i=0; i<n_land; i++){
    free (aterrizajes [i]);
  }
  free (aterrizajes);

  if (queue_destroy() != 0){ /* Liberacion de los recursos del buffer */
    printf ("Error queue_destoy: error en la liberacion de recursos.");
  }

 print_bye ();
 return 0;
}

void jefe_pista (void){
  int i;
  for (i=0; i < n_take_off; i++){
    /* Produccion del avion que va a despegar */

    /* Acceso a las variables globales compartidas: id, ultimo*/
    pthread_mutex_lock (&mutex_id_creacion);
    id++;
    despegues[i]->id_number = id;
    despegues[i]->time_action = t_take_off;
    despegues[i]->action = 0; /* accion 0 = despegue; accion 1 = aterrizaje */
    despegues[i]->last_flight = ((i==n_take_off -1)&&(ultimo >= 0))? 1 : 0;
    if (i==n_take_off -1) ultimo = 0;
    printf ("[TRACKBOSS] Plane with id %d checked\n",despegues[i]->id_number);
    pthread_mutex_unlock (&mutex_id_creacion);
    
    pthread_mutex_lock (&mutex_insercion);
    while (siguiente !=  despegues[i]->id_number)     /* si no se ha insertado el anterior */
      pthread_cond_wait(&insertado_radar, &mutex_insercion); /* se bloquea */
    
    /* Acceso al buffer */
    pthread_mutex_lock(&mutex_buffer);
    while (queue_full ()==1)     /* si buffer lleno */
      pthread_cond_wait(&no_lleno, &mutex_buffer); /* se bloquea */
    queue_put (despegues[i]); /* Almacenar el avion en el hangar */
    pthread_cond_signal(&no_vacio); /* El buffer no esta vacio, tiene al menos 1 elemento*/
    printf ("[TRACKBOSS] Plane with id %d ready to take off\n",despegues[i]->id_number);
    pthread_mutex_unlock(&mutex_buffer); /* Fin de la seccion critica de acceso al buffer */
    
    siguiente = despegues[i]->id_number + 1;
    pthread_cond_signal (&insertado_jefe); 
    pthread_mutex_unlock (&mutex_insercion);
  }
  printf ("+++++trackboss termino\n");/*debug*/
  pthread_exit(0);
}

void radar_aereo (void){
  int i;
  for (i=0; i < n_land; i++){
    /* Produccion de un avion que va a aterrizar */
    
    /* Acceso a las variables globales compartidas: id, ultimo*/
    pthread_mutex_lock (&mutex_id_creacion);
    id++; 
    aterrizajes[i]->id_number = id;  
    aterrizajes[i]->time_action = t_land;
    aterrizajes[i]->action = 1; /* accion 0 = despegue; accion 1 = aterrizaje */
    aterrizajes[i]->last_flight = ((i==n_land -1)&&(ultimo >= 0))? 1 : 0;
    if (i==n_land -1) ultimo = 1;
    printf ("[RADAR] Plane with id %d detected!\n",aterrizajes[i]->id_number);
    pthread_mutex_unlock (&mutex_id_creacion); /* Fin de creacion del avion */
    
    pthread_mutex_lock (&mutex_insercion);
    while (siguiente !=  aterrizajes[i]->id_number)     /* si no se ha insertado el anterior */
      pthread_cond_wait(&insertado_jefe, &mutex_insercion); /* se bloquea */
 
    /* Acceso al buffer */
    pthread_mutex_lock(&mutex_buffer);
    while (queue_full ()==1)     
      pthread_cond_wait(&no_lleno, &mutex_buffer); /* si buffer lleno, se bloquea */
    queue_put (aterrizajes[i]); /* Almacenar el avion en el hangar */
    pthread_cond_signal(&no_vacio); /* El buffer no esta vacio, tiene al menos 1 elemento*/
    printf ("[RADAR] Plane with id %d ready to land\n",aterrizajes[i]->id_number);
    pthread_mutex_unlock(&mutex_buffer); /* Fin de la seccion critica del buffer */
    
    siguiente = aterrizajes[i]->id_number + 1;
    pthread_cond_signal (&insertado_radar);
    pthread_mutex_unlock (&mutex_insercion);   
  }

  printf ("+++ fin radar\n"); /*debug*/
  pthread_exit(0);
}

void torre_control (void){
  int processed = 0;
  int landed = 0;
  int taken_off = 0;
  int i;
  struct plane * avion = NULL;
  for (i=0; i < n_take_off + n_land; i++){
    pthread_mutex_lock(&mutex_buffer); /* acceder al buffer */
    while (queue_empty ()==1){ /* si buffer vacio */
      printf ("[CONTROL] Waiting for planes in empty queue\n");
      pthread_cond_wait(&no_vacio, &mutex_buffer); /* se bloquea */
    }
    avion = queue_get (); /* Recoge un avion del buffer, segun su orden de insercion */
    if (avion==NULL){
      printf ("ERROR al obtener el avion.\n");
    }
    pthread_cond_signal(&no_lleno);     /* buffer no lleno */
    pthread_mutex_unlock(&mutex_buffer); /* Fin de la seccion critica de acceso al buffer */
    
    processed ++;
    if (avion->action == 0){ /* Operaciones si el avion necesita la pista para despegar*/
      printf ("[CONTROL] Putting plane with id %d in track\n",avion->id_number);
      taken_off++;
      if (avion->last_flight == 0){
      sleep (DELAY*avion->time_action);
      printf ("[CONTROL] Plane %d took off after %d seconds\n",avion->id_number,avion->time_action);
      }
      else{ /* si el avion es el ultimo vuelo del dia */
        printf ("[CONTROL] After plane with id %d the airport will be closed\n",avion->id_number);
        sleep (DELAY*avion->time_action);
        printf ("[CONTROL] Plane %d took off after %d seconds\n",avion->id_number,avion->time_action);
        printf ("Airport Closed!\n");
      }
    }
    else{ /* Operaciones si el avion necesita la pista para aterrizar*/
      printf ("[CONTROL] Track is free for plane with id %d\n",avion->id_number);
      landed++;
      if (avion->last_flight == 0){
        sleep (DELAY*avion->time_action);
        printf ("[CONTROL] Plane %d landed in %d seconds\n",avion->id_number,avion->time_action);
      }
      else{  /* si el avion es el ultimo vuelo del dia */
        printf ("[CONTROL] After plane with id %d the airport will be closed\n",avion->id_number);
        sleep (DELAY*avion->time_action);
        printf ("[CONTROL] Plane %d landed in %d seconds\n",avion->id_number,avion->time_action);
        printf ("Airport Closed!\n");
      }
    }    
  }
  FILE * fp;
  fp = fopen ("resume.air", "w"); /* Crea o trunca el fichero para solo escritura */
  if (fp == NULL){
    printf ("ERROR al abrir/crear el fichero \"resume.air\".\n");
    int * res;
    res = (int*)malloc(sizeof(int));
    *res = -1;
    pthread_exit (res);
  }
  fprintf(fp, "\tTotal number of planes processed: %d\n", processed);
  fprintf(fp, "\tNumber of planes landed: %d\n", landed);
  fprintf(fp, "\tNumber of planes taken off: %d\n", taken_off);  
  fclose(fp);
  pthread_exit(0);
}

