#ifndef _HEADER_H_
#define _HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <assert.h>

struct args{
    int id,agencies;
};

typedef struct Reservation{
    int agency_id;
    int reservation_number;
}Reservation;

typedef struct stack_reservation {
    struct Reservation reservation;
    struct stack_reservation *next;
}stack_reservation;

typedef struct queue_reservation {
    struct Reservation reservation;
    struct queue_reservation *next;
}queue_reservation;

typedef struct list_reservation{
    struct Reservation reservation;
    int marked;
    pthread_mutex_t lock;
    struct list_reservation *next;
}list_reservation;

typedef struct stack{
    struct stack_reservation *top;
    pthread_mutex_t top_lock;
    int size;
    int capacity;
}stack;
    
typedef struct queue{
    struct queue_reservation *head;
    struct queue_reservation *tail;
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;
}queue;

typedef struct list{
    struct list_reservation *head;
    struct list_reservation *tail;
}list;

typedef struct flight_reservation{
    stack *completed_reservations;
    queue *pending_reservations;
}flight_reservation;

extern flight_reservation **flights;
extern int A;
extern list *lista;
extern pthread_barrier_t barrier_phaseA;
extern pthread_barrier_t barrier_phaseB;
extern pthread_barrier_t barrier_phaseC;
extern pthread_barrier_t mybar;
extern pthread_mutex_t cntr_lock;
extern int *flags;
extern int number_of_inserter_airlines;
pthread_t *bookers,*managers,controllerA,controllerB;

void *ticket_booking(void*);
void *manage_reservations(void*);
void *controlA(void*);
void *controlB(void*);

flight_reservation *new_flight_reservation(int);
int insert_stack(Reservation*);
int insert_in_my_stack(stack*,Reservation*);

int insert_list(Reservation*);
list_reservation *delete_head(void);
int validate(list_reservation*, list_reservation*);

int enq(struct Reservation*);
queue_reservation *deq(queue*);


void init(void);
int check_args(int,const char**);
int isfull(stack*);
void print_everything(void);
int list_finished(void);
int list_empty(void);
int inserter_airlines_finished();
void free_everything(void);
#endif