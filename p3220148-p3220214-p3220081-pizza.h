#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// Number of employees
#define n_tel 2      
#define n_cook 2      
#define n_oven 10     
#define n_deliverer 10 

// Order time range
#define t_order_low 1
#define t_order_high 5

// Number of orders per time range
#define n_order_low 1
#define n_order_high 5

// Order type probabilities
#define p_m 35      
#define p_p 25      
#define p_s 40     

// Payment time range
#define t_payment_low 1
#define t_payment_high 3

// Failure probability
#define p_fail 5   

// Costs per order type
#define c_m 10        
#define c_p 11        
#define c_s 12        

// Preparation and delivery times
#define t_prep 1      
#define t_bake 10     
#define t_pack 1     
#define t_del_low 5   
#define t_del_high 15 

// our customers struct 
typedef struct customers {
		int id;
		int number_of_pizzas;
		int time_from_order;
}CUSTOMERS;

