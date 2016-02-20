#define MAX_SIZE 101 //keep one more than number of philospher

typedef struct {
pthread_mutex_t lock;
pthread_cond_t nonzero;
unsigned count;
int state[MAX_SIZE];
}semaphore_t;

typedef struct {
pthread_mutex_t lock;
pthread_cond_t done;
int curr_num;
int allowed;
}barrier_t;

semaphore_t *semaphore_create(char *semaphore_name,int sem_val);
semaphore_t *semaphore_open(char *semaphore_name);
void semaphore_post(semaphore_t *semap);
void semaphore_wait(semaphore_t *semap);
void semaphore_close(semaphore_t *semap);
barrier_t* barrier_create(char* barriername, int val);
barrier_t* barrier_open(char* barriername);
void barrier( barrier_t* barriermap);
