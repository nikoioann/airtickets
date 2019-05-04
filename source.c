#include "header.h"

int A=0;
int number_of_inserter_airlines=0;
int *flags = NULL;
flight_reservation **flights = NULL;
list *lista=NULL;

pthread_barrier_t barrier_phaseA;
pthread_barrier_t barrier_phaseB;
pthread_barrier_t barrier_phaseC;
pthread_barrier_t mybar;
pthread_mutex_t cntr_lock;


void *ticket_booking(void* ptr){
    Reservation *res = (Reservation*)malloc(sizeof(Reservation));
    int booked_tickets = 0;
    int agencies = A*A;

    res->agency_id = (int)(intptr_t)ptr;
    for(int i=0;i<A;i++){
        res->reservation_number = (booked_tickets++)*agencies + res->agency_id;
        if(!insert_stack(res)){
            enq(res);
        }
    }

    free(res);
    pthread_barrier_wait(&barrier_phaseA);
    pthread_exit(NULL);
}

void *controlA(void* ptr){
    pthread_barrier_wait(&barrier_phaseA);
    int cntr = 0,err=0,sum=0,expected_sum=0,inq=0;
    unsigned long long keysum=0,expected_keysum=0;
    stack *s;
    stack_reservation *sr;
    queue_reservation *qr;
    char **errors;

    errors = (char**)malloc(sizeof(char*)*(A+2));
    for(cntr = 0;cntr<(A+2);cntr++){
        errors[cntr] = (char*)malloc(sizeof(char)*512);
    }

    for(cntr=0;cntr<A;cntr++){
        s = flights[cntr]->completed_reservations;
        sum += s->size;

        qr = flights[cntr]->pending_reservations->head->next;
        if(qr) number_of_inserter_airlines++;
        inq=0;
        while(qr!=NULL){
            keysum+= qr->reservation.reservation_number;
            sum++;
            inq++;
            qr = qr->next;
        }
        
        if(s->size <= s->capacity){
            printf("Flight %d :  stack overflow check passed (capacity: %d, found: %d, queue: %d)\n",cntr,s->capacity,s->size,inq);
        }else{
            sprintf(errors[err++],"FAILURE:Flight %d : stack overflow check failed (capacity: %d, found: %d)\n",cntr,s->capacity,s->size);
        }

        sr = s->top;
        while(sr!=NULL){
            keysum += sr->reservation.reservation_number;
            sr = sr->next;
        }
    }

    expected_sum = pow(A,3);
    if(sum == expected_sum){
        printf("total size check passed (expected: %d, found: %d)\n",expected_sum,sum);
    }else{
        sprintf(errors[err++],"FAILURE:total size check failed (expected: %d, found: %d)\n",expected_sum,sum);
    }

    expected_keysum=(pow(A,6) + pow(A,3))/2;
    if(keysum == expected_keysum){
        printf("total keysum check passed (expected: %lld, found: %lld)\n",expected_keysum,keysum);
    }else{
        sprintf(errors[err++],"FAILURE:total keysum check passed (expected: %lld, found: %lld)\n",expected_keysum,keysum);
    }

    if(err){
        printf("\n\n");
        for(cntr=0;cntr<err;cntr++){
            printf("%s",errors[cntr]);
        }
        printf("\nProcess will be terminated\n");
        exit(EXIT_FAILURE);
    }

    pthread_barrier_wait(&barrier_phaseB);
    pthread_exit(NULL);
}

void *manage_reservations(void* ptr){
    pthread_barrier_wait(&barrier_phaseB);
    int airline_id = (int)(intptr_t)ptr;
    queue_reservation *queue_res;
    list_reservation *list_res;
    Reservation *res = (Reservation*)malloc(sizeof(Reservation));

    while(flights[airline_id]->pending_reservations->head->next!=NULL){
        if((queue_res = deq(flights[airline_id]->pending_reservations)) != NULL){
            res->agency_id = queue_res->reservation.agency_id;
            res->reservation_number = queue_res->reservation.reservation_number;
            insert_list(res);
        }
    }

    // pthread_mutex_lock(&cntr_lock);
    // number_of_inserter_airlines--;
    // pthread_mutex_unlock(&cntr_lock);

    flags[airline_id] = 0;
    while(!isfull(flights[airline_id]->completed_reservations) && (!list_finished() || !list_empty() )){
    // while(!isfull(flights[airline_id]->completed_reservations) && (inserter_airlines_finished() || !list_empty())){
        if((list_res = delete_head()) != NULL){
            res->agency_id = list_res->reservation.agency_id;
            res->reservation_number = list_res->reservation.reservation_number;
            if(!insert_in_my_stack(flights[airline_id]->completed_reservations,res)){
                printf("Error with inserting in stack from list.(phaseB)\n");
            }
        }
    }

    free(res);
    pthread_barrier_wait(&barrier_phaseC);
    pthread_exit(NULL);
}


void *controlB(void* ptr){
    pthread_barrier_wait(&barrier_phaseC);
    int cntr = 0,err=0,queueflag=0,sum=0,expected_sum=0;
    unsigned long long keysum=0,expected_keysum=0;
    stack *s;
    stack_reservation *sr;
    queue_reservation *qr;
    char **errors;

    errors = (char**)malloc(sizeof(char*)*(2*A+3));
    for(cntr = 0;cntr<(A+2);cntr++){
        errors[cntr] = (char*)malloc(sizeof(char)*512);
    }
    printf("\n\n");
    for(cntr=0;cntr<A;cntr++){
        s = flights[cntr]->completed_reservations;
        sum += s->size;
        if(s->size <= s->capacity){
            printf("Flight %d :  stack overflow check passed (capacity: %d, found: %d)\n",cntr,s->capacity,s->size);
        }else{
            sprintf(errors[err++],"FAILURE:Flight %d : stack overflow check failed (capacity: %d, found: %d)\n",cntr,s->capacity,s->size);
        }

        sr = s->top;
        while(sr!=NULL){
            keysum += sr->reservation.reservation_number;
            sr = sr->next;
        }

        if(flights[cntr]->pending_reservations->head->next != NULL){
            sprintf(errors[err++],"FAILURE:Flight %d : queue reservation not empty)\n",cntr);
            queueflag = 1;
        }
    }

    expected_sum = pow(A,3);
    if(sum == expected_sum){
        printf("total size check passed (expected: %d, found: %d)\n",expected_sum,sum);
    }else{
        sprintf(errors[err++],"FAILURE:total size check failed (expected: %d, found: %d)\n",expected_sum,sum);
    }

    expected_keysum=(pow(A,6) + pow(A,3))/2;
    if(keysum == expected_keysum){
        printf("total keysum check passed (expected: %lld, found: %lld)\n",expected_keysum,keysum);
    }else{
        sprintf(errors[err++],"FAILURE:total keysum check passed (expected: %lld, found: %lld)\n",expected_keysum,keysum);
    }

    if(!queueflag){
        if(list_empty()){
            printf("reservations completion check passed\n");
        }else{
            sprintf(errors[err++],"FAILURE:Reservation managment center not empty)\n");
        }
    }
    if(err){
        printf("\n\n");
        for(cntr=0;cntr<err;cntr++){
            printf("%s",errors[cntr]);
        }
        printf("\nProcess will be terminated\n");
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}

flight_reservation *new_flight_reservation(int cap){
    flight_reservation *f = (flight_reservation*)malloc(sizeof(flight_reservation));

    f->completed_reservations = (stack*)malloc(sizeof(stack));
        f->completed_reservations->top = NULL;
        f->completed_reservations->size = 0;
        f->completed_reservations->capacity = cap;
        pthread_mutex_init(&(f->completed_reservations->top_lock),0);

    f->pending_reservations = (queue*)malloc(sizeof(queue));
        f->pending_reservations->head = (queue_reservation*)malloc(sizeof(queue_reservation));
        f->pending_reservations->tail = f->pending_reservations->head;
            f->pending_reservations->head->reservation.agency_id = -1;
            f->pending_reservations->head->reservation.reservation_number = -1;
            f->pending_reservations->head->next = NULL;
        pthread_mutex_init(&(f->pending_reservations->head_lock),0);
        pthread_mutex_init(&(f->pending_reservations->tail_lock),0);
    
    return f;
}

int insert_in_my_stack(stack* curr_stack,Reservation* res){
    stack_reservation *stack_res;
    int ret=0;

    pthread_mutex_lock(&curr_stack->top_lock);
    if(curr_stack->size < curr_stack->capacity){ 
        stack_res = (stack_reservation*)malloc(sizeof(stack_reservation));
        stack_res->reservation.agency_id = res->agency_id;
        stack_res->reservation.reservation_number = res->reservation_number;
        curr_stack->size++;
        stack_res->next = curr_stack->top;
    
        curr_stack->top = stack_res;
        ret=1;
    }
    pthread_mutex_unlock(&curr_stack->top_lock);


    return ret;
}

int insert_stack(Reservation* res){
    return insert_in_my_stack(flights[(res->agency_id - 1) % A]->completed_reservations,res);
}

int enq(Reservation* res){
    unsigned which_flight = (res->agency_id - 1) % A;
    queue *curr_queue = flights[which_flight]->pending_reservations;
    queue_reservation *queue_res;
    int ret=0;
  
    queue_res = (queue_reservation*)malloc(sizeof(queue_reservation));
    queue_res->reservation.agency_id = res->agency_id;
    queue_res->reservation.reservation_number = res->reservation_number;
    queue_res->next = NULL;

    pthread_mutex_lock(&curr_queue->tail_lock);
    curr_queue->tail->next = queue_res;
    curr_queue->tail = curr_queue->tail->next;
    ret=1;
    pthread_mutex_unlock(&curr_queue->tail_lock);

    return ret;
}

queue_reservation *deq(queue *curr_queue){
    queue_reservation *queue_res=NULL;

    pthread_mutex_lock(&curr_queue->head_lock);
    if(curr_queue->head->next != NULL){
        queue_res = curr_queue->head->next;
        curr_queue->head = curr_queue->head->next;
    }
    pthread_mutex_unlock(&curr_queue->head_lock);

    return queue_res;
}

int insert_list(Reservation *res) { 
    list_reservation *pred, *curr;
    int result;
    int return_flag = 0;
    
    while (1) {
        pred = lista->head;
        curr = pred->next;
        while (curr->reservation.reservation_number!=-1 && curr->reservation.reservation_number < res->reservation_number) {
            pred = curr;
            curr = curr->next;
        }

        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);
        if (validate(pred, curr)) {
            if (curr->reservation.reservation_number!=-1 && res->reservation_number == curr->reservation.reservation_number) {
                result = 0; 
                return_flag = 1;
            }
            else {
                list_reservation *node = (list_reservation*)malloc(sizeof(list_reservation));
                node->next = curr;
                node->marked = 0;
                node->reservation.agency_id = res->agency_id;
                node->reservation.reservation_number = res->reservation_number;
                pthread_mutex_init(&node->lock,0);
                pred->next = node;
                result = 1;
                return_flag = 1;
            }
        }
        pthread_mutex_unlock(&pred->lock);
        pthread_mutex_unlock(&curr->lock);
        if(return_flag) return result;
    }
}


list_reservation *delete_head() {
    list_reservation *pred=NULL,*curr=NULL,*result=NULL;
    int return_flag = 0;

    while(1){
        pred = lista->head;
        curr = pred->next;
        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);
        if(validate(pred,curr)){
            if(curr->reservation.reservation_number!=-1){
                result = curr;
                curr->marked = 1;
                pred->next = curr->next;
            }
            return_flag = 1;
        }
        pthread_mutex_unlock(&curr->lock);
        pthread_mutex_unlock(&pred->lock);
        if(return_flag)    return result;
    }

} 

int validate(list_reservation *pred, list_reservation *curr) {
    if (pred->marked == 0 && curr->marked == 0 && pred->next == curr) 
        return 1;
    else 
        return 0;
}

int isfull(stack *s){
    return !(s->capacity > s->size);
}

int check_args(int argc,const char **argv){
    int ret,num;
    if(argc<2){
        fprintf(stderr,"Error: Argument A must be provided\n");
        exit(EXIT_FAILURE);
    }
    ret = sscanf(argv[1],"%d",&num);

    if(ret!=1 || A<0){
        fprintf(stderr,"Error: Argument A not suitable\n");
        exit(EXIT_FAILURE);        
    }

    return num;
}

void init(void){
    int cntr;
    
    flights = (flight_reservation**)malloc(A*sizeof(flight_reservation*));
    for(cntr = 0; cntr < A; cntr++){
        flights[cntr] = new_flight_reservation( (  1.5 * (A*A) ) - (A-1-cntr)*A);
    }

    pthread_barrier_init(&barrier_phaseA,NULL, A*A+ 1);
    pthread_barrier_init(&barrier_phaseB,NULL, A  + 1);
    pthread_barrier_init(&barrier_phaseC,NULL, A  + 1);
    pthread_barrier_init(&mybar,NULL, A);

    bookers  = malloc(sizeof(pthread_t) * A*A );
    managers = malloc(sizeof(pthread_t) * A   );
    pthread_mutex_init(&cntr_lock,0);

    flags = malloc(sizeof(int)*A);
    for(cntr=0;cntr<A;cntr++) flags[cntr] = 1;

    lista = (list*)malloc(sizeof(list));
    lista->head = (list_reservation*)malloc(sizeof(list_reservation));
    lista->tail = (list_reservation*)malloc(sizeof(list_reservation));
    lista->head->next = lista->tail;
    lista->head->marked = 0;
    lista->head->reservation.reservation_number = -1;
    pthread_mutex_init(&lista->head->lock,0);
    lista->tail->next = NULL;
    lista->tail->marked = 0;
    lista->tail->reservation.reservation_number = -1;
    pthread_mutex_init(&lista->tail->lock,0);

}

int inserter_airlines_finished(){
    int ret=0;
    pthread_mutex_lock(&cntr_lock);
    if(number_of_inserter_airlines == 0) ret = 1;
    pthread_mutex_unlock(&cntr_lock);
    return ret;
}


int list_finished(){
    int i;
    for(i=0;i<A;i++){
        if(flags[i]) return 0;
    }
    return 1;
}

int list_empty(){
    return lista->head->next == lista->tail;
}

void print_everything(){
    int cntr;
    for(cntr = 0; cntr < A; cntr++){
        printf("%d : %d\n",flights[cntr]->completed_reservations->size,flights[cntr]->completed_reservations->capacity);
        stack_reservation *t = flights[cntr]->completed_reservations->top;
        printf("\tSTACK:\n");
        while(t!=NULL){
            printf("\t\t%d:%d\n",t->reservation.agency_id,t->reservation.reservation_number);
            t=t->next;
        }
        queue_reservation *q = flights[cntr]->pending_reservations->head->next;
        printf("\tQUEUE:\n");
        while(q!=NULL){
            printf("\t\t%d:%d\n",q->reservation.agency_id,q->reservation.reservation_number);
            q=q->next;
        }
    }
    list_reservation *l = lista->head->next;
    printf("\nLIST:\n");
    int i=1;
    while(l!=NULL){
        printf("%d)%d\n",i++,l->reservation.reservation_number);
        l=l->next;
    }
    printf("\n");
}

void free_everything(){
        for(int cntr = 0; cntr < A; cntr++){
        stack_reservation *tmp;
        queue_reservation *tmp2;
        list_reservation *tmp3;
        pthread_mutex_destroy(&(flights[cntr]->completed_reservations->top_lock));
        while((tmp = flights[cntr]->completed_reservations->top) != NULL){
            flights[cntr]->completed_reservations->top=flights[cntr]->completed_reservations->top->next;
            free(tmp);
        }
        free(flights[cntr]->completed_reservations);

        pthread_mutex_destroy(&(flights[cntr]->pending_reservations->head_lock));
        pthread_mutex_destroy(&(flights[cntr]->pending_reservations->tail_lock));
        
        while((tmp2 = flights[cntr]->pending_reservations->head) != NULL){
            flights[cntr]->pending_reservations->head=flights[cntr]->pending_reservations->head->next;
            free(tmp2);
        }
        free(flights[cntr]->pending_reservations);

        while((tmp3 = lista->head) != NULL){
            lista->head = lista->head->next;
            pthread_mutex_destroy(&tmp3->lock);
            free(tmp3);
        }
        // free(lista);
    }

    pthread_mutex_destroy(&cntr_lock);
    pthread_barrier_destroy(&barrier_phaseA);
    pthread_barrier_destroy(&barrier_phaseB);
    pthread_barrier_destroy(&barrier_phaseC);
    free(flags);
    free(bookers);
    free(managers);
}
