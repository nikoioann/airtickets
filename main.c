#include "header.h"

int main(int argc, char const *argv[])
{  
    int airlines,agencies;
    int cntr;

    A = check_args(argc,argv);
    airlines = A;
    agencies = A*A;

    init();
    
    for(cntr = 0; cntr < agencies; cntr++){
        pthread_create(&(bookers[cntr]),NULL,ticket_booking,(void*)(intptr_t)(cntr+1));
        if(cntr<airlines)
            pthread_create(&(managers[cntr]),NULL,manage_reservations,(void*)(intptr_t)(cntr));
    }

   pthread_create(&controllerA,NULL,controlA,(void*)(intptr_t)(cntr));
   pthread_create(&controllerB,NULL,controlB,(void*)(intptr_t)(cntr));

   for(cntr = 0; cntr < agencies; cntr++){
        pthread_join(bookers[cntr],NULL);
        if(cntr<airlines) 
            pthread_join(managers[cntr],NULL);
    }
    pthread_join(controllerA,NULL);
    pthread_join(controllerB,NULL);

    // print_everything();
    
    free(bookers);
    //free flights,mutex,barriers

    return 0;
}

