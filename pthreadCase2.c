#include <errno.h> 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#include <linux/module.h>		//says it doesn't exist, so this is just for reference

#define NUM_CUSTOMERS	100		//defined number of customers
#define COMPLEX_TIME	10000000
#define SIMPLE_TIME	5000000
#define PAY_TIME	3000000
#define COMPLEX_PRICE	5		//price of complex coffee
#define SIMPLE_PRICE	2		//price of simple coffee

pthread_mutex_t barista1, baristaCashier;	//baristas as represented by mutex locks
pthread_mutex_t pot1, pot2, pot3;		//self-serve coffee pots as re 
pthread_mutex_t complexPthreadLock, simplePthreadLock, cashierPthreadLock;	//lock to prevent alterations to pthread creations,
int cashierBox;				//counts the total amount of money
FILE *simpleFile, *complexFile;		//initializes the two files we'll create
struct timeval simpleStart, simpleEnd;		//initializes simple time counter
struct timeval complexStart, complexEnd;	//initializes complex time counter


struct complexQueueNode{			//initializes a stuct for the complex nodes later created
	int customerID_Position;
	struct complexQueueNode *next;
} *complexTail, *complexFront, *complexInner, *rmComplexFront;	//Initializes the structs of these names

struct simpleQueueNode{				//initializes a struct for the simple nodes later created 
	int customerID_Position;
	struct simpleQueueNode *next;
} *simpleTail, *simpleFront, *simpleInner, *rmSimpleFront;	//Initializes the structs of these names

struct payingQueueNode{				//initializes a struct for the paying nodes 
	int customerID_Position;
	struct payingQueueNode *next;
} *payingTail, *payingFront, *payingInner, *rmPayingFront;

struct customer {			//customer structure
	int customerID;
	int orderType;			//0 = simple order, 1 = complex order
};

struct customer customer_array[NUM_CUSTOMERS];		//initializes an array of customers

void *processing(void *input) {
	int waitCount;
	struct customer *cust;
	cust = (struct customer *) input;		//sets the local customer to the input

	printf("Customer %d has entered with order %d\n", cust->customerID, cust->orderType);

	if (cust->orderType == 0){
		//~~~~~~~~SECTION: enques to simple queue
		pthread_mutex_lock(&simplePthreadLock);		//locks the pthread such that no one can access while manipulations are being made.
		gettimeofday(&simpleStart, NULL);		//get current time value
		if (simpleFront==NULL) {			//if there is no linked list with simpleQueueNodes, then we start making one
			simpleTail = (struct simpleQueueNode *)malloc(sizeof(struct simpleQueueNode));	//allocate memory for the queue node
			simpleTail->customerID_Position = cust->customerID;			//set the customer id in the node
			simpleTail->next = NULL;						//set the next to null because its the head and we have nothing for it yet
			simpleFront = simpleTail;						//set the head to the tail
		} else {
			simpleInner = (struct simpleQueueNode *)malloc(sizeof(struct simpleQueueNode));	//allocates memory for the queue node
			simpleTail->next = simpleInner;						//set the next 
			simpleInner->customerID_Position = cust->customerID;			//set the customer id
			simpleInner->next = NULL;						//set the next to null
			simpleTail = simpleInner;
		}
		pthread_mutex_unlock(&simplePthreadLock);	//unlocks the pthread such that it can be accessed again. 
		while (1) {
		if (simpleFront != NULL && simpleFront->customerID_Position == cust->customerID) {
			if (pthread_mutex_trylock(&pot1) == 0){		//checks if pot1 is busy
				printf("1st self-service pot is available! Please proceed forward, customer %d.\n", simpleFront->customerID_Position);
				simpleDeque();					//pops the head of the queue
				if (cust->orderType == 0) {
					//the following 'for' loop kills time 
					for (waitCount=0; waitCount<SIMPLE_TIME; waitCount++) {
						waitCount = waitCount;
					}
					fprintf(simpleFile, "%d\n", simpleEnd.tv_sec - simpleStart.tv_sec);	//records the time data into file
				}
				pthread_mutex_unlock(&pot1);		//barista finished "processing" the customer hence is free again.
				break;		//breaks from the loop (meaning customer has been served)
			} else if (pthread_mutex_trylock(&pot2) == 0){	//checks if pot2 is busy
				printf("2nd self-service pot is available! Please proceed forward, customer %d.\n", simpleFront->customerID_Position);
				simpleDeque();					//releases next customer
				if (cust->orderType == 0) {
					//the following 'for' loop kills time
					for (waitCount=0; waitCount<SIMPLE_TIME; waitCount++) {
						waitCount = waitCount;
					}
					fprintf(simpleFile, "%d\n", simpleEnd.tv_sec - simpleStart.tv_sec);	//records time data to file
				}
				pthread_mutex_unlock(&pot2);		//barista finished processing the customer hence is free once more
				break;		//breaks from the loop (meaning customer has been served)
			} else if (pthread_mutex_trylock(&pot3) == 0){	//checks if pot3 is busy
				printf("3rd self-service pot is available! Please proceed forward, customer %d.\n", simpleFront->customerID_Position);
				simpleDeque();					//releases next customer
				if (cust->orderType == 0) {
					//the following 'for' loop kills time
					for (waitCount=0; waitCount<SIMPLE_TIME; waitCount++) {
						waitCount = waitCount;
					}
					fprintf(simpleFile, "%d\n", simpleEnd.tv_sec - simpleStart.tv_sec);	//records time data to file
				}
				pthread_mutex_unlock(&pot3);		//barista finished processing the customer hence is free once more
				break;		//breaks from the loop (meaning customer has been served)
			}
		}
	}
	} else {
		//~~~~~~~~SECTION: enques to complex queue
		pthread_mutex_lock(&complexPthreadLock);		//locks the pthread such that no one can access while manipulations are being made.
		gettimeofday(&complexStart, NULL);			//get the current time
		if (complexFront==NULL) {
			complexTail = (struct complexQueueNode *)malloc(sizeof(struct complexQueueNode));	//allocate memory
			complexTail->customerID_Position = cust->customerID;			//set for equiv IDs
			complexTail->next = NULL;
			complexFront = complexTail;
		} else {
			complexInner = (struct complexQueueNode *)malloc(sizeof(struct complexQueueNode));	//allocate memory
			complexTail->next = complexInner;
			complexInner->customerID_Position = cust->customerID;
			complexInner->next = NULL;
			complexTail = complexInner;
		}
		pthread_mutex_unlock(&complexPthreadLock);		//unlocks the pthread
		//~~~~~~~~SECTION: checks if barista is busy or not~~~~~~~~~~~~~~
		while (1) {
			if (complexFront != NULL && complexFront->customerID_Position == cust->customerID) {
				if (pthread_mutex_trylock(&barista1) == 0){		//checks if barista1 is busy
					printf("Barista is available! Please proceed forward, customer %d.\n", complexFront->customerID_Position);
					complexDeque();					//pops the head of the queue
					if (cust->orderType == 1) {			//this section ensures that coffee type is complex
						for (waitCount=0; waitCount<COMPLEX_TIME; waitCount++) {
							waitCount = waitCount;
						}
						fprintf(complexFile, "%d\n", complexEnd.tv_sec - complexStart.tv_sec);	//records the time data into file
					} 
					pthread_mutex_unlock(&barista1);		//barista finished "processing" the customer hence is free again.
					break;		//breaks from the loop (meaning customer has been served)
				}
			}
		}
	}
	//~~~~~~~~SECTION: enques to cashier queue
	pthread_mutex_lock(&cashierPthreadLock);		//locks the pthread such that no one can access while manipulations are being made.
	if (payingFront==NULL) {
		payingTail = (struct payingQueueNode *)malloc(sizeof(struct payingQueueNode));
		payingTail->customerID_Position = cust->customerID;
		payingTail->next = NULL;
		payingFront = payingTail;
	} else {
		payingInner = (struct payingQueueNode *)malloc(sizeof(struct payingQueueNode));
		payingTail->next = payingInner;
		payingInner->customerID_Position = cust->customerID;
		payingInner->next = NULL;
		payingTail = payingInner;
	}
	pthread_mutex_unlock(&cashierPthreadLock);		//unlocks the pthread for use
	//~~~~~~~~SECTION: checks if the payment barista is busy or not~~~~~~~~~~~~~~~~
	while (1) {
		if (payingFront != NULL && payingFront->customerID_Position == cust->customerID) {
			if (pthread_mutex_trylock(&baristaCashier) == 0){		//checks if barista1 is busy
				printf("Barista is available for payment! Please proceed forward, customer %d.\n", payingFront->customerID_Position);
				payingDeque();					//pops the head of the queue
				if (cust->orderType == 1) {			//this section ensures that coffee type is complex
					for (waitCount=0; waitCount<PAY_TIME; waitCount++)
					{
						waitCount = waitCount;
					}
					cashierBox = cashierBox + COMPLEX_PRICE;	//adds cash to the till
					gettimeofday(&complexEnd, NULL);	//gets end time
					fprintf(complexFile, "%d\n", complexEnd.tv_sec - complexStart.tv_sec);	//records the time data into file
				} else {
					for (waitCount=0; waitCount<PAY_TIME; waitCount++) {
						waitCount = waitCount;
					}
					cashierBox = cashierBox + SIMPLE_PRICE;		//adds cash to the till
					gettimeofday(&simpleEnd, NULL);		//gets end time
					fprintf(simpleFile, "%d\n", simpleEnd.tv_sec - simpleStart.tv_sec);	//records the time data into file
				}
				pthread_mutex_unlock(&baristaCashier);		//barista finished "processing" the customer hence is free again.
				break;		//breaks from the loop (meaning customer has been served)
			}
		}
	}
	pthread_exit(NULL);		//exits the pthread
}

//dequing the linked list for complex orders
complexDeque(void *input){
	pthread_mutex_lock(&complexPthreadLock);	//locks to ensure that its not being altered
	rmComplexFront = complexFront;			//gets details for deletion
	printf("Customer %d has just been serviced.\n", rmComplexFront->customerID_Position);
	complexFront = complexFront->next;		//sets the next head
	free(rmComplexFront);				//deletes the old head
	pthread_mutex_unlock(&complexPthreadLock);	//unlocks the mutex lock
}

//dequing the linked list for simple orders
simpleDeque(void *input){
	pthread_mutex_lock(&simplePthreadLock);
	rmSimpleFront = simpleFront;
	printf("Customer %d has just been serviced.\n", rmSimpleFront->customerID_Position);
	simpleFront = simpleFront->next;
	free(rmSimpleFront);
	pthread_mutex_unlock(&simplePthreadLock);
}

//dequing the linked list for paying customers
payingDeque(void *input){
	pthread_mutex_lock(&cashierPthreadLock);
	rmPayingFront = payingFront;
	printf("Customer %d has just paid for their coffee.\n", rmPayingFront->customerID_Position);
	payingFront = payingFront->next;
	free(rmPayingFront);
	pthread_mutex_unlock(&cashierPthreadLock);
}


main(){
	srandom(time(NULL));			//setting the random seed
	pthread_t customers[NUM_CUSTOMERS];	//initialize an array of pthreads
	int t, check;
	simpleFile = fopen("case2-simple.txt", "a");	//opens simple file
	complexFile = fopen("case2-complex.txt", "a");	//opens complex file
	if (simpleFile == NULL || complexFile == NULL) {
		printf("Error: unable to open file.\n");
		exit(EXIT_FAILURE);
	}

	for(t=0; t<NUM_CUSTOMERS; t++) {
		customer_array[t].customerID = t;
		customer_array[t].orderType = random()%2;
		check = pthread_create(&customers[t], NULL, processing, (void *) &customer_array[t]);
		if (check) {
			printf("Error: no customers can enter.\n");
			exit(EXIT_FAILURE);
		}
	}
	for(t=0; t<NUM_CUSTOMERS; t++) {			//ensures that all threads will show up
		check = pthread_join(customers[t], NULL);	//keeps the pthreads in order of (random) creation
		if (check) {
			printf("Error.\n");
			exit(EXIT_FAILURE);
		}
	}
	printf("Today Starlocks made $%d\n", cashierBox);	//just a random checking of amount 
	fclose(simpleFile);				//closes simple file
	fclose(complexFile);				//closes complex file
}

//MODULE_LICENSE("GPL");
//MODULE_AUTHOR("Emily Chow");
//MODULE_DESCRIPTION("Problem 4 part c ~ original Starlocks customer-barista processing but with angry, angry customers");
