#include <errno.h> 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <linux/module.h>		//says it doesn't exist, so this is just for reference

#define NUM_CUSTOMERS	100		//defined number of customers
#define COMPLEX_TIME	10000000
#define SIMPLE_TIME	5000000

pthread_mutex_t barista1, barista2;	//baristas as represented by mutex locks 
pthread_mutex_t pthreadLock;		//lock to prevent alterations to pthread creations
FILE *simpleFile, *complexFile;		//initializes the two files we'll create


struct queueNode{
	int customerID_Position;
	int orderType;			//0 = simple order, 1 = complex order
	struct queueNode *next;
} *tail, *front, *inner, *rmFront;	//Initializes the structs of these names

struct customer {			//customer structure
	int customerID;
	int orderType;	
};

struct customer customer_array[NUM_CUSTOMERS];		//initializes an array of customers

void *processing(void *input) {
	struct timeval simpleStart, simpleEnd;		//initializes simple time counter
	struct timeval complexStart, complexEnd;	//initializes complex time counter
	int waitCount;					//a wait counter to pass time for when the customer is with the barista
	struct customer *cust;
	cust = (struct customer *) input;		//sets the customer structure to the input taken from the parameter
	
	printf("Customer %d has entered with order %d\n", cust->customerID, cust->orderType);		//just an alert that a customer has entered 
	
	if (cust->orderType == 0) {			//this basically just decides which type of coffee (simple or complex) time counter will start
		gettimeofday(&simpleStart, NULL);
	} else {
		gettimeofday(&complexStart, NULL);
	}

	//This section enques the customer
	pthread_mutex_lock(&pthreadLock);		//locks the pthread such that no one can access while manipulations are being made.
	if (front==NULL) {
		tail = (struct queueNode *)malloc(sizeof(struct queueNode));	//allocates memory
		tail->customerID_Position = cust->customerID;			//sets necessary attributes
		tail->orderType = cust->orderType;
		tail->next = NULL;
		front = tail;							//essentially initializes the first node
	} else {
		inner = (struct queueNode *)malloc(sizeof(struct queueNode));	//allocates memory
		tail->next = inner;						//starts building the linked list
		inner->customerID_Position = cust->customerID;			//sets necessary attributes
		inner->orderType = cust->orderType;
		inner->next = NULL;
		tail = inner;
	}
	pthread_mutex_unlock(&pthreadLock);		//unlocks the pthread to be accessed once more

	//This section checks is the barista are busy or not
	while (1) {
		if (front != NULL && front->customerID_Position == cust->customerID) {
			if (pthread_mutex_trylock(&barista1) == 0){		//checks if barista1 is busy
				printf("Barista 1 is available! Please proceed forward, customer %d.\n", front->customerID_Position);
				deque();					//pops the head of the queue
				if (cust->orderType == 1) {			//this section deals with the passing time according to the type of coffee the customer ordered. 
					for (waitCount=0; waitCount<COMPLEX_TIME; waitCount++)
					{
						waitCount = waitCount;
					}
					gettimeofday(&complexEnd, NULL);	//ends the time counter
					fprintf(complexFile, "%d\n", complexEnd.tv_sec - complexStart.tv_sec);	//records the time data into file
				} else if (cust->orderType == 0) {
					for (waitCount=0; waitCount<SIMPLE_TIME; waitCount++)
					{
						waitCount = waitCount;
					}
					gettimeofday(&simpleEnd, NULL);		//ends the time counter
					fprintf(simpleFile, "%d\n", simpleEnd.tv_sec - simpleStart.tv_sec);	//records the time data into file
				}
				pthread_mutex_unlock(&barista1);		//barista finished "processing" the customer hence is free again.
				break;		//breaks from the loop (meaning customer has been served)
			} else if (pthread_mutex_trylock(&barista2) == 0){	//checks if barista2 is busy
				printf("Barista 2 is available! Please proceed forward, customer %d.\n", front->customerID_Position);
				deque();					//releases next customer
				if (cust->orderType == 1) {			//deals with time-passing according to the cofee the customer ordered
					for (waitCount=0; waitCount<COMPLEX_TIME; waitCount++)
					{
						waitCount = waitCount;
					}
					gettimeofday(&complexEnd, NULL);	//ends the time counter
					fprintf(complexFile, "%d\n", complexEnd.tv_sec - complexStart.tv_sec);	//records time data to file
				} else if (cust->orderType == 0) {
					for (waitCount=0; waitCount<SIMPLE_TIME; waitCount++)
					{
						waitCount = waitCount;
					}
					gettimeofday(&simpleEnd, NULL);		//ends time counter
					fprintf(simpleFile, "%d\n", simpleEnd.tv_sec - simpleStart.tv_sec);	//records time data to file
				}
				pthread_mutex_unlock(&barista2);		//barista finished processing the customer hence is free once more
				break;		//breaks from the loop (meaning customer has been served)
			}
		}
	}
	pthread_exit(NULL);
}

deque(void *input){		//deque metod to pop the head of the linked list
	pthread_mutex_lock(&pthreadLock);	//locks pthread
	rmFront = front;
	printf("Customer %d has just been serviced.\n", rmFront->customerID_Position);
	front = front->next;			//turns the head of the node to equal the next-to-head node
	free(rmFront);				//frees up memory.
	pthread_mutex_unlock(&pthreadLock);	//unlocks pthread for use
}


main(){
	srandom(time(NULL));			//ensures true randomness
	pthread_t customers[NUM_CUSTOMERS];
	int t, check;
	simpleFile = fopen("case1-simple.txt", "a");	//initializes text file 
	complexFile = fopen("case1-complex.txt", "a");	//initializes another text file
	if (simpleFile == NULL || complexFile == NULL) {	//ensures files will open/be created
		printf("Error: unable to open file.\n");
		exit(EXIT_FAILURE);
	}

	for(t=0; t<NUM_CUSTOMERS; t++) {		//creates customers
		customer_array[t].customerID = t;
		customer_array[t].orderType = random()%2;	//randomly generates customer's coffee type
		check = pthread_create(&customers[t], NULL, processing, (void *) &customer_array[t]);	//creates the pthread of the customer that will run concurrently for the rest of the program
		if (check) {
			printf("Error: Starlocks is closed. No customers can enter.\n");
			exit(EXIT_FAILURE);
		}
	}
	for(t=0; t<NUM_CUSTOMERS; t++) {		//ensures that all threads will show up
		check = pthread_join(customers[t], NULL);	//keeps the pthreads in order of (random) creation
		if (check) {
			printf("Error.\n");
			exit(EXIT_FAILURE);
		}
	}
	fclose(simpleFile);				//closes write file
	fclose(complexFile);				//closes write file
}

//MODULE_LICENSE("GPL");
//MODULE_AUTHOR("Emily Chow");
//MODULE_DESCRIPTION("Problem 4 part a ~ original Starlocks customer-barista processing");
