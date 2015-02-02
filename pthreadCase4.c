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
		while (1) {
			if (pthread_mutex_trylock(&pot1) == 0){		//checks if pot1 is busy
				printf("1st self-service pot is available! Please proceed forward, customer %d.\n", cust->customerID);
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
				printf("2nd self-service pot is available! Please proceed forward, customer %d.\n", cust->customerID);
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
				printf("3rd self-service pot is available! Please proceed forward, customer %d.\n", cust->customerID);
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
	} else {
		//~~~~~~~~SECTION: checks if barista is busy or not~~~~~~~~~~~~~~
		while (1) {
			if (pthread_mutex_trylock(&barista1) == 0){		//checks if barista1 is busy
				printf("Barista is available! Please proceed forward, customer %d.\n", cust->customerID);
				if (cust->orderType == 1) {			//this section ensures that coffee type is complex
					for (waitCount=0; waitCount<COMPLEX_TIME; waitCount++) {
						waitCount = waitCount;
					}
				} 
				pthread_mutex_unlock(&barista1);		//barista finished "processing" the customer hence is free again.
				break;		//breaks from the loop (meaning customer has been served)
			}
		}
	}
	//~~~~~~~~SECTION: checks if the payment barista is busy or not~~~~~~~~~~~~~~~~
	while (1) {
		if (pthread_mutex_trylock(&baristaCashier) == 0){		//checks if barista1 is busy
			printf("Pay for your coffee, customer %d.\n", cust->customerID);
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
	pthread_exit(NULL);		//exits the pthread
}


main(){
	srandom(time(NULL));			//setting the random seed
	pthread_t customers[NUM_CUSTOMERS];	//initialize an array of pthreads
	int t, check;
	simpleFile = fopen("case4-simple.txt", "a");	//opens simple file
	complexFile = fopen("case4-complex.txt", "a");	//opens complex file
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
