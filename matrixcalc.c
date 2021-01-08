#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>

#define NUMBEROFROWCOLL 20

// def of prototypes
void *creator();
void *totalizer();
void takeInput();
void stpHandler(int);

typedef enum
{
    FALSE,
    TRUE
} boolean;

pthread_t th_creator, th_totalizer;                         // definition of thread
pthread_mutex_t mutex_main, mutex_creator, mutex_totalizer; // definition of mutexes
int n;                                                      // user's input for the number of matrixes
int matrix[NUMBEROFROWCOLL][NUMBEROFROWCOLL];               // global matrix
boolean flag = FALSE, inMain = FALSE;                       // for the control of while loop in main

int main(int argc, char const *argv[])
{
    srand(time(NULL));           // for random values
    signal(SIGTSTP, stpHandler); // def of signal
    signal(SIGINT, stpHandler);  // def of signal

    //initialize the mutexes
    pthread_mutex_init(&mutex_creator, NULL);
    pthread_mutex_init(&mutex_totalizer, NULL);
    pthread_mutex_init(&mutex_main, NULL);
    pthread_mutex_lock(&mutex_totalizer);
    pthread_mutex_lock(&mutex_creator);

    // initialize the threads
    if (pthread_create(&th_creator, NULL, creator, NULL) != 0)
    {
        perror("Error to create thread:");
        return 1;
    }
    if (pthread_create(&th_creator, NULL, totalizer, NULL))
    {
        perror("Error to create thread:");
        return 2;
    }

    //pthread_detach(th_creator);
    //pthread_detach(th_totalizer);

    while (1)
    {
        pthread_mutex_lock(&mutex_main);
        if (flag == TRUE)
        {
            break;
        }
        inMain = TRUE;
        takeInput();
        inMain = FALSE;
        pthread_mutex_unlock(&mutex_creator); // when user enters something, unlock mutex of creator
    }
    pthread_mutex_destroy(&mutex_creator);
    pthread_mutex_destroy(&mutex_totalizer);
    pthread_mutex_destroy(&mutex_main);

    return 0;
}

void *creator()
{
    while (1) // infinite loop
    {
        pthread_mutex_lock(&mutex_creator); // if user inputs something, run
        // matrix creation
        for (size_t i = 0; i < NUMBEROFROWCOLL; i++)
        {
            for (size_t j = 0; j < NUMBEROFROWCOLL; j++)
            {
                *(*(matrix + i) + j) = rand() % 10; // fill with random values
                printf("%6d", *(*(matrix + i)));    // print abowe value
            }
            printf("\n");
        }
        pthread_mutex_unlock(&mutex_totalizer); // unlock for senc.
    }
}

void *totalizer()
{
    while (1) // infinite loop
    {
        pthread_mutex_lock(&mutex_totalizer); // if user inputs something, run
        int sum = 0;
        // summation of matrix
        for (size_t i = 0; i < NUMBEROFROWCOLL; i++)
        {
            for (size_t j = 0; j < NUMBEROFROWCOLL; j++)
            {
                sum += *(*(matrix + i) + j);
            }
        }

        printf("\nSum of the matrix: %d\n", sum);
        n--;        // when a matrix is done, reduce # of matrix
        if (n == 0) // if the last matrix is done, throw a signal for main
        {
            pthread_mutex_unlock(&mutex_main);
        }
        else
        {
            pthread_mutex_unlock(&mutex_creator);
        }
    }
}

void takeInput()
{
    printf("\n\nEnter the number of matrix: ");
    scanf("%d", &n);
    if (n == 0 || n < 0)
    {
        exit(0);
    }
}

void stpHandler(int dummy)
{
    printf("\n\n ----Threads will result... \n\n----");
    /* 
    Don't call pthread_exit() or pthread_cancel() from a signal handler! 
    It is not required to be async-signal-safe, see signal-safety.
    In general, you should do as little as possible in a signal handler. 
    The common idiom is to just set a flag that is periodically checked 
    in your main loop like e.g.
    */
    if (inMain == TRUE)
    {
        exit(0); // Program is already in the main func to scan a number or terminate itself.
    }
    else
    {
        /* 
        Thread kullanılan bir uygulamada main() fonksiyonundan return edilirse, 
        tüm thread'ler de sonlandırılır ve kullanılan tüm kaynaklar sisteme geri verilir.
        --FSMVU | OPERARING SYSTEMS COURSE LAB MATERIAL--
        */
        flag = TRUE;
        pthread_mutex_unlock(&mutex_main);
    }
}