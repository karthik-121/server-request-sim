#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>

#define BACKLOG 8
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;

struct connection {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

int server(char *port);
void *echo(void *arg);
int readInput(FILE* fpin, FILE* fpout);
char* returnKey(char* stringL);
char* returnValue(char* stringL);
void* workerFun(void* A);




struct node{
	struct node* next;
	char* key;
	char* value;
};


struct node* allocate(char* str, char* str2){
	struct node* temp = malloc(sizeof(struct node));
	temp->key = malloc(strlen(str)+1);
	strcpy(temp->key, str);
	temp->value = malloc(strlen(str2) + 1);
	strcpy(temp->value, str2);
	temp->next = 0;
	return temp; 
}

struct queue{

	struct node* front;
	struct node* back;
	int size;
};

struct queue* makeQueue(){

	struct queue* q = malloc(sizeof(struct queue));
	q->front = 0; 
	q->back = 0;
	q->size = 0;
	return q;
}


struct queue* list;

void insert(struct queue* q, char* key, char* value){
		
	pthread_mutex_lock(&lock1);
	if(q->size == 0){
        
        	struct node* head = allocate(key,value);

        	q->front = head;
        	q->back = head;
        	q->size++;
		pthread_mutex_unlock(&lock1);
        	return;
    	}
	else{
        	struct node* ptr = q->front;
        	struct node* last = 0;
        	while(ptr != 0){
            		if(strcmp(ptr->key, key) == 0){
				free(ptr->value);
                		ptr->value = value;
				pthread_mutex_unlock(&lock1);
                		return;
           		}
            		last = ptr;
            		ptr = ptr->next;

        	}
        	
            	struct node* temp = allocate(key,value);
            	last->next = temp;
            	q->back = temp;
            	q->size++;

        	
        
   	}
	pthread_mutex_unlock(&lock1);
    	return;


}

void del(struct queue* q, char* key){
	
	pthread_mutex_lock(&lock1);
    
	if(q->front == 0){
		pthread_mutex_unlock(&lock1);
		return;

	}

	struct node* ptr = q->front;
	struct node* last = 0;

	if(strcmp(ptr->key,key) == 0){
		struct node* temp = q->front;
		q->front = q->front->next;
		free(temp->key);
		free(temp->value);
		free(temp);
		q->size--;
		pthread_mutex_unlock(&lock1);
		return;
	}
	
	//struct node* temp = ptr;
	while(ptr != 0){
		if(strcmp(ptr->key,key) == 0){
			
		    	break;
		}

		last = ptr;
		ptr = ptr->next;
		//temp = ptr;
	}

	if(ptr == 0){
		pthread_mutex_unlock(&lock1);
		return;
	}


	last->next = ptr->next;
	q->size--;
	free(ptr->key);
	free(ptr->value);
	free(ptr);
	pthread_mutex_unlock(&lock1);
	return;
}

struct t_args{

	FILE* fpin;
	FILE* fpout;
	int connection;
	int connection2;


};


int main(int argc, char** argv){
	
	if ( argc != 2){
		printf("wrong number of arguments for %s", argv[0]);
		exit(EXIT_FAILURE);

	}
	list = makeQueue();
	(void) server(argv[1]);
	
	return EXIT_SUCCESS;


}



int server(char* port){

	struct addrinfo hint, *info_list, *info;
	//int connection;
	int error, sfd;
	pthread_t tid;
	
	memset(&hint, 0, sizeof(struct addrinfo));
	
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_PASSIVE;
	
	
	error = getaddrinfo(NULL, port, &hint, &info_list);
	if ( error != 0){

		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
		return -1;
	}

	for ( info = info_list; info != NULL; info = info->ai_next){
		sfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

		if ( sfd == -1){
			continue;
		}
		
		if ((bind(sfd, info->ai_addr, info->ai_addrlen) == 0) && (listen(sfd, BACKLOG) == 0)) {
            		break;
        	}
		
		close(sfd);

	}

	if ( info == NULL){
		fprintf(stderr, "could not bind socket, try again or use diff port\n");
		freeaddrinfo(info_list);	
		return -1;
	}
		
	freeaddrinfo(info_list);
	printf("Waiting for connection\n");
	for (;;){
		
		struct t_args* args = malloc(sizeof(struct t_args)); 
		args->connection = accept(sfd, NULL, NULL);
		printf("Waiting for input...\n");
		args->connection2 = dup(args->connection);
		error = pthread_create(&tid, NULL, workerFun, args);

		if (error != 0) {
           		fprintf(stderr, "Unable to create thread: %d\n", error);
            		close(args->connection);
            		free(args);
            		continue;
        	}
		if ( args->connection2 != -1){

			pthread_detach(tid);

			

		}

		


	}

}

char* getVal(struct queue* q, char* key){
	
	pthread_mutex_lock(&lock1);
	struct node* ptr = q->front;
	while( ptr != 0){
		
		if ( strcmp(ptr->key, key) == 0){
			pthread_mutex_unlock(&lock1);
			return ptr->value;
		}
		ptr = ptr->next;
	}
	//key was not found
	pthread_mutex_unlock(&lock1);
	return "KNF";

}


int readInput(FILE* fpin, FILE* fpout){
	char c[4];
	char f;
	int check = 0;
	while ( check == 0){
		f = getc(fpin);
		if ( f != '\n'){
			c[0] = f;
			check = 1;
		}

	}
	int i = 1;
	
	for ( int j = 0; j < 2; j++){
		
		c[i] = getc(fpin);
		
		i++;

	}
	c[3] = '\0';
	printf("%s\n", c);
	if ( strcmp(c, "GET") == 0){
		//loop through and do get method
		//char* getArray = malloc(sizeof(char) * INT_MAX);
		getc(fpin);
		int length;
		fscanf(fpin, "%d", &length);
		if ( length == 0){
			fprintf(fpout, "ERR\nBAD\n");
			fflush(fpout);
			return -1;
		}

		char t;
		
		t = getc(fpin);
		if ( t != '\n'){
			fprintf(fpout, "ERR\nBAD\n");
			fflush(fpout);
			return -1;
		}
		
		
		char* d = malloc(sizeof(char) * (length + 1));
		int nlnum = 0;
		int k;
		for ( k = 0; k < length; k++){
			d[k] = getc(fpin);

			if (d[k] == '\n'){
				nlnum++;

			}
			if ( k == length - 1){

				if ( d[k] != '\n'){
					fprintf(fpout, "ERR\nLEN\n");
					fflush(fpout);
					free(d);
					return -1;
					
				}
			}
			
			if ( nlnum == 1 && k < length - 1){
				
				fprintf(fpout, "ERR\nLEN\n");
				fflush(fpout);
				free(d);
				return -1;

			}

			
		}
		d[k] = '\0';
		char* key = returnKey(d);
		
		char* get = getVal(list, key);
		if ( strcmp(get, "KNF") == 0){
			fprintf(fpout, "KNF\n");
			fflush(fpout);
		}else{
			fprintf(fpout, "OKG\n%ld\n%s\n", strlen(get) + 1, get);
			fflush(fpout);

		}
		
		free(d);
		
	}
	else {
		if (strcmp(c, "SET") == 0){
			//loop through and do set method
		
			getc(fpin);
			int length;
			fscanf(fpin, "%d", &length);
			if ( length == 0){
				fprintf(fpout, "ERR\nBAD\n");
				fflush(fpout);
				return -1;
			}

			
			char t;
		
			t = getc(fpin);
			if ( t != '\n'){
				fprintf(fpout, "ERR\nBAD\n");
				fflush(fpout);
				return -1;
			}
			
			char* d = malloc(sizeof(char) * (length + 1));
			int nlnum = 0;
			int k;
			for ( k = 0; k < length; k++){
				d[k] = getc(fpin);

				if (d[k] == '\n'){
					nlnum++;

				}
				if ( k == length - 1){

					
					if ( d[k] != '\n' || nlnum == 1){
					
						fprintf(fpout, "ERR\nLEN\n");
						fflush(fpout);
						free(d);
						return -1;
					}

				}
				
				
				if (nlnum == 2 && k < length - 1){
					
					fprintf(fpout, "ERR\nLEN\n");
					fflush(fpout);
					free(d);
					return -1;
				}

			}
			d[k] = '\0';
			
			char* key = returnKey(d);
			char* value = returnValue(d);
			insert(list, key, value);
		
			
			fprintf(fpout, "OKS\n");
			fflush(fpout);
			free(d);

		}

		else {
			
			if (strcmp(c, "DEL") == 0){
				//loop through and do del method
				getc(fpin);
				int length;
				fscanf(fpin, "%d", &length);

				if ( length == 0){
					fprintf(fpout, "ERR\nBAD\n");
					fflush(fpout);
					return -1;
				}
				
				char t;
		
				t = getc(fpin);
				if ( t != '\n'){
					fprintf(fpout, "ERR\nBAD\n");
					fflush(fpout);
					return -1;
				}
				char* d = malloc(sizeof(char) * (length + 1));
				int nlnum = 0;
				int k;
				for ( k = 0; k < length; k++){
					d[k] = getc(fpin);

					if (d[k] == '\n'){
						nlnum++;

					}
					if ( k == length - 1){

						if ( d[k] != '\n'){
							fprintf(fpout, "ERR\nLEN\n");
							fflush(fpout);
							free(d);
							return -1;
							
						}
					}
					
					if ( nlnum == 1 && k < length - 1){
						
						fprintf(fpout, "ERR\nLEN\n");
						fflush(fpout);
						free(d);
						return -1;

					}

			
				}
				d[k] = '\0';
				char* key = returnKey(d);
				
				char* get = getVal(list, key);
				if ( strcmp(get, "KNF") == 0){
					fprintf(fpout, "KNF\n");
					fflush(fpout);
				}else{
					fprintf(fpout, "OKD\n%ld\n%s\n", strlen(get) + 1, get);
					fflush(fpout);

				}
				
				del(list, key);
				free(d);
				
				

			}
			else {

				fprintf(fpout, "ERR\nBAD\n");
				fflush(fpout);
				
				return -1;

			}

		}

	}
	
	
	return 0;
	
	


}









char* returnKey(char* stringL){

	int i = 0;
	for ( i = 0; i < strlen(stringL); i++){
		if ( stringL[i] == '\n'){
			break;
		}

	}
	char* key = malloc(sizeof(char) * (i + 1));
	
	for ( int j = 0; j < i; j++){
		key[j] = stringL[j];
	}
	key[i] = '\0';
    	
	return key;
	
		
	
	
}

char* returnValue(char* stringL){

	int i = 0;
	for ( i = 0; i < strlen(stringL); i++){
		if ( stringL[i] == '\n'){
			break;
		}

	}
	
    
	
	int x = strlen(stringL) - (i + 1);
	char* value = malloc(sizeof(char) * x);
	for ( int j = 0; j < x - 1; j++){
		value[j] = stringL[j + i + 1];
	}
		
	value[x - 1] = '\0';
	    
	   		
	return value;
		
	
	
}

void* workerFun(void* A){
	
	struct t_args* args = (struct t_args*) A;
	args->fpin = fdopen(args->connection, "r");
	args->fpout = fdopen(args->connection2, "w");
	while ( args->fpin != NULL && args->fpout != NULL){
		int x = readInput(args->fpin, args->fpout);
		if ( x != 0){
			close(args->connection);
			return NULL;
		}

	}


	return NULL;

}














