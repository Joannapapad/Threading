#include "p3220148-p3220214-p3220081-pizza.h"


pthread_mutex_t screen_mutex;

pthread_mutex_t telephone_mutex;
pthread_cond_t telephone_cond;

pthread_mutex_t oven_mutex;
pthread_cond_t oven_cond;
 
pthread_mutex_t cook_mutex;
pthread_cond_t cook_cond;
 
pthread_mutex_t del_mutex;
pthread_cond_t del_cond;


// global variables 
int available_tel = n_tel;
int available_ovens = n_oven;
int available_cook = n_cook;
int available_del = n_deliverer;


unsigned int seed;
int slept;
int rcScreen;
int fail_orders = 0;
int success_orders = 0;
int income = 0;
int margarita = 0;
int peperoni = 0;
int special = 0; 

struct timespec *order;
int *time_delivered;
int *time_packed;
int *time_cooled;

// a function for the calculation of the percentage 
int percentage(){
	return rand_r(&seed) % 100 + 1;
}

//the function that the created threads use in the main function  
void *start_order(void * order_id){
	
	
	//local variables 
	int tid = *(int *)order_id;
	int rcTel;
	int rcOven;
	int rcCook;
	int rcDel;
	CUSTOMERS cust;
	struct timespec delivered;
	struct timespec packed;
	struct timespec cooled;
	
	//initialization of the customer stuct 
	cust.id = tid;
	cust.number_of_pizzas = rand_r(&seed) % n_order_high + n_order_low;
	cust.time_from_order = order[cust.id-1].tv_sec - order[0].tv_sec;
	
	//calculating the time that the order of every customer starts
	if(tid == 1 ){
		clock_gettime(CLOCK_REALTIME , &order[cust.id - 1]);
	}else{
		sleep(rand_r(&seed) % t_order_high  + t_order_low);
		clock_gettime(CLOCK_REALTIME , &order[cust.id - 1]);	
	}

	
	//telephoner's part 
	rcTel = pthread_mutex_lock(&telephone_mutex);
	if(rcTel != 0){
		printf("Error ocured!! Returned code from pthread_mutex_lock() %d" , rcTel);
		pthread_exit(&rcTel);
	}
	
	while(available_tel == 0){
		pthread_cond_wait(&telephone_cond,&telephone_mutex);
	}

	available_tel --;
	rcTel = pthread_mutex_unlock(&telephone_mutex);
	
	// sleeping a random time for the telephoner to charge the client's card 
	sleep(rand_r(&seed) % t_payment_high + t_payment_low);
	
	rcTel = pthread_mutex_lock(&telephone_mutex);
	if (percentage() > p_fail){

		rcScreen = pthread_mutex_lock(&screen_mutex);
		if(rcScreen != 0){
			printf("Error ocured!! Returned code from pthread_mutex_lock() %d" ,rcScreen);
			pthread_exit(&rcScreen);
		}
		printf("Order with id <%d> was successful!\n", tid);
		pthread_mutex_unlock(&screen_mutex);
		
		available_tel ++;
		success_orders ++;
		
		for(int i =0 ; i < cust.number_of_pizzas; i ++){
		
			if (percentage() <= 35){
				income += c_m;
				margarita++;
			}else if(percentage() <= 60 ){
				income+= c_p;
				peperoni++;
			}else{
				income+= c_s;
				special++;
			}
		}
	} else{
	
		available_tel++;
		fail_orders ++;
		cust.number_of_pizzas = 0;
		
		rcScreen = pthread_mutex_lock(&screen_mutex);
		printf("Order with id <%d> failed!\n", tid);
		rcScreen = pthread_mutex_unlock(&screen_mutex);
		
	}
	
	rcTel = pthread_cond_signal(&telephone_cond);
	rcTel = pthread_mutex_unlock(&telephone_mutex);
	
	//the if statement is used so that the faied orders would not bother the cooker, the ovens and the deliverer 
	//cooker's part 
	if (cust.number_of_pizzas != 0){
		rcCook = pthread_mutex_lock(&cook_mutex);
		if(rcCook != 0){
			printf("Error ocured!! Returned code from pthread_mutex_lock() %d" , rcCook);
			pthread_exit(&rcCook);
		}
		
		while(available_cook == 0){
			pthread_cond_wait(&cook_cond, &cook_mutex);
		}
		
		available_cook--;
		
		rcCook = pthread_mutex_unlock(&cook_mutex);
		
		//the coocker sleeps for the preparation time  
		sleep(t_prep * cust.number_of_pizzas);
		
		rcCook = pthread_mutex_lock(&cook_mutex);
		
		while(pthread_mutex_trylock(&oven_mutex) != 0){
			//	wait
		} 
		
		rcCook = pthread_mutex_unlock(&cook_mutex);
		
		// oven's part 
		while(available_ovens < cust.number_of_pizzas){
			pthread_cond_wait(&oven_cond, &oven_mutex);
		}
		
		available_ovens -= cust.number_of_pizzas;
		
		// the coockers are vailable when the ovens are in use 
		rcCook = pthread_mutex_lock(&cook_mutex);
		available_cook ++;
		rcCook = pthread_cond_signal(&cook_cond);
		rcCook = pthread_mutex_unlock(&cook_mutex);
		
		rcOven = pthread_mutex_unlock(&oven_mutex);
		if(rcOven != 0){
			printf("Error ocured!! Returned code from pthread_mutex_lock() %d" , rcOven);
			pthread_exit(&rcOven);
		}
		
		//the oven sleeps for time bake 
		sleep(t_bake);
		
		//starts counting the time the pizza is ready
		clock_gettime(CLOCK_REALTIME, &cooled);
		
		//deliverer's part 
		rcOven = pthread_mutex_lock(&oven_mutex);
			
		while(pthread_mutex_trylock(&del_mutex) != 0){
			//	wait
		} 
		
		rcOven = pthread_mutex_unlock(&oven_mutex);
		
		while(available_del < 1){
			pthread_cond_wait(&del_cond, &del_mutex);
		}
		
		available_del -= 1;
		
		// the ovens are free when the deliverer stars to be occupied
		rcOven = pthread_mutex_lock(&oven_mutex);
		available_ovens += cust.number_of_pizzas;
		rcOven = pthread_cond_signal(&oven_cond);
		rcOven = pthread_mutex_unlock(&oven_mutex);
		
		rcDel = pthread_mutex_unlock(&del_mutex);
		
		// sleeps until he packs the pizzas 
		sleep(t_pack * cust.number_of_pizzas);
		
		//starts counting the time when the pizzas are packed 
		clock_gettime(CLOCK_REALTIME, &packed);
		
		//calculates the time from the beggining of the order until the pizza is packed 
		time_packed[cust.id - 1] = packed.tv_sec - order[cust.id - 1].tv_sec;
		
		rcScreen = pthread_mutex_lock(&screen_mutex);
		printf("Order with id <%d> is prepared in <%d> minutes\n", cust.id, time_packed[cust.id - 1]);
		rcScreen = pthread_mutex_unlock(&screen_mutex);
		
		int probability_del = rand_r(&seed) % t_del_high + t_del_low;
		sleep(probability_del);
		
		//calculates the time that the orderr delivered to the customers house 
		clock_gettime(CLOCK_REALTIME, &delivered);
		
		//the time that took to delivered 
		time_delivered[cust.id - 1] = delivered.tv_sec - order[cust.id - 1].tv_sec;
		//the time that took for the pizzas to cooled 
		time_cooled[cust.id - 1] = delivered.tv_sec - cooled.tv_sec;
		
		
		rcScreen = pthread_mutex_lock(&screen_mutex);
		printf("Order with id <%d> arrived to destination in <%d> minutes\n", cust.id, time_delivered[cust.id - 1]);
		rcScreen = pthread_mutex_unlock(&screen_mutex);
		
		sleep(probability_del);
		
		rcDel = pthread_mutex_lock(&del_mutex);
		available_del++;
		rcDel = pthread_cond_signal(&del_cond);
		rcDel = pthread_mutex_unlock(&del_mutex);
	
	
	}
	pthread_exit((void*)order_id);

}

int main(int argc, char *argv[]) {


	if(argc != 3){
		printf("Please provide the number of customers to serve and a seed to generate random numbers!");
		exit(-1);
	}
	
	int Ncust = atoi(argv[1]);
	seed = atoi(argv[2]);
	
	if (Ncust <= 0){
		printf("Error number of customers should be a positive number. Not %d", Ncust);
		exit(-1);
	}
		
		
	//local variables 
	int order_num[Ncust];
	int i;
	int rc;	
	
	//threads and conditional variables initializaion 
	pthread_mutex_init(&telephone_mutex, NULL);
	pthread_cond_init(&telephone_cond ,NULL);
	
	pthread_mutex_init(&screen_mutex, NULL);
	
	pthread_cond_init(&oven_cond ,NULL);
	pthread_mutex_init(&oven_mutex, NULL);
	
	pthread_cond_init(&cook_cond ,NULL);
	pthread_mutex_init(&cook_mutex, NULL);

	pthread_cond_init(&del_cond ,NULL);
	pthread_mutex_init(&del_mutex, NULL);
	
	//malloc
	pthread_t *orders = (pthread_t *)malloc(Ncust * sizeof(pthread_t));
	order = malloc(Ncust * sizeof(struct timespec));
	time_delivered = malloc(Ncust * sizeof(int));
	time_packed = malloc(Ncust * sizeof(int));
	time_cooled = malloc(Ncust * sizeof(int));
	
	if((orders == NULL) || (order == NULL) || (time_delivered == NULL) || (time_packed == NULL) || (time_cooled == NULL)){
		printf("Not enough memory!");
	}
	
	//threads creation 
	for(i = 0 ; i < Ncust ; i++){
		order_num[i] = i + 1;
		rc = pthread_create(&orders[i],NULL,start_order, &order_num[i]);
		
		if (rc != 0) {
  			printf("Error occured!! Return code from pthread_create() is %d\n", rc);
     		exit(-1);
     	}		
		
	}
	
	// threads joins
	void *status;
	for (i = 0; i < Ncust; i++) {
		rc = pthread_join(orders[i], &status);
		
		if (rc != 0) {
			printf("Error occured!! Return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
	}
	
	//the calculation of the avarage and maximum time of deliver and of colling pizzas 
	int Max_Delivered = time_delivered[0];
	int Max_cooled = time_cooled[0];
	
	int Avg_Delivered =0;
	int Avg_cooled =0;
	
	for(int i = 0; i < Ncust ; i++){
		if(time_delivered[i] > Max_Delivered){
			Max_Delivered = time_delivered[i];
		}
		Avg_Delivered += time_delivered[i];
		
		if(time_cooled[i] > Max_cooled){
			Max_cooled = time_cooled[i];
			
		}
		Avg_cooled += time_cooled[i];
	}
	
	
	double average = (double) Avg_Delivered/(double)success_orders;
	double average_cooled = (double) Avg_cooled/(double)success_orders;
	
	
	rcScreen = pthread_mutex_lock(&screen_mutex);
	printf("-------------WELCOME TO OUR STORE-------------\n");
	printf("TOTAL INCOME: %d\n", income);
    printf("TOTAL SUCCESSFUL ORDERS: %d\n", success_orders);
    printf("TOTAL FAILED ORDERS: %d\n", fail_orders);
    printf("TOTAL MARGARITA PIZZAS: %d\n", margarita);
    printf("TOTAL PEPERONI PIZZAS: %d\n", peperoni);
    printf("TOTAL SPECIAL PIZZAS: %d\n", special);
    printf("AVERAGE CUSTOMERS SERVICE TIME: %f\n",average);
    printf("MAXIMUM CUSTOMERS SERVICE TIME: %d\n",Max_Delivered);
    printf("AVERAGE COOLED TIME: %f\n",average_cooled);
    printf("MAXIMUM COOLED TIME: %d\n",Max_cooled);
    printf("--------HOPE TO SEE YOU AGAIN!! GOODBYE--------\n");
    
    rcScreen = pthread_mutex_unlock(&screen_mutex);
    
	//threads and conditional variables destroy 
	pthread_mutex_destroy(&oven_mutex);
	pthread_mutex_destroy(&telephone_mutex);
	pthread_mutex_destroy(&screen_mutex);
	pthread_cond_destroy(&telephone_cond);
	pthread_cond_destroy(&oven_cond);
	
	pthread_mutex_destroy(&cook_mutex);
	pthread_cond_destroy(&cook_cond);
	
	pthread_mutex_destroy(&del_mutex);
	pthread_cond_destroy(&del_cond);
	
	// free memory 
	free(orders);
	free(order);
	free(time_delivered);
	free(time_packed);
	free(time_cooled);
	
	return 0;
}