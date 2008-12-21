/* flowd.c */
void warn(char *msg);
void *shm_new(int shmkey, int shmsize, int *shmid);
int shm_free(int shmid, void *shmptr);
int build_socket(void);
int is_valid(const char *buf, int len);
int main(int argc, char *argv[]);
/* fttime.c */
struct fttime ftltime(int sys, int secs, int nsecs, int t);
