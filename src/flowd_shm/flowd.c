#include "flowd.h"

inline void usage(char *argv0)
{
  printf("Usage: %s [-v] [-i listen_ip] [-p listen_port] [-s save_path]\n", argv0);
}

void warn(char *msg)
{
  if (verbose)
    fprintf(stderr, "%s\n", msg);
}

void *shm_new(int shmkey, int shmsize, int *shmid)
{
  void *shmptr;

  *shmid = shmget(shmkey, shmsize, 0);
  if (*shmid == -1)
  {
    *shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
    if (*shmid == -1)
    {
      perror("Failed to allocation shared memory (shmget)");
      return NULL;
    }
  }

  shmptr = (void *) shmat(*shmid, NULL, 0);
  if (shmptr == (void *) -1)
  {
    perror("Failed to allocation shared memory (shmid)");
    return NULL;
  }

  return shmptr;
}

int shm_free(int shmid, void *shmptr)
{
  if (shmdt(shmptr) == -1)
  {
    perror("Failed to free shared memory (shmdt)");
    return 0;
  }

  if (shmctl(shmid, IPC_RMID, 0) == -1)
  {
    perror("Failed to free shared memory (shmctl)");
    return 0;
  }

  return 1;
}

int build_socket()
{
  int sockfd;
  struct sockaddr_in sin;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    perror("Error socket()");
    exit(-1);
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = listen_ipaddr;
  sin.sin_port = listen_port;

  if (bind(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
  {
    perror("Error bind() (plz check input ip/port)");
    exit(-1);
  }

  return sockfd;
}

int is_valid(const char *buf, int len)
{
  int record_count;
  struct NF_header *header;

  if ((len - NF_HEADER_SIZE) % NF_RECORD_SIZE != 0)
  {
    warn("Warning: Invalid Netflow V5 packet.");
    return 0;
  }

  record_count = (len - NF_HEADER_SIZE) / NF_RECORD_SIZE;

  header = (struct NF_header *) buf;

  if (ntohs(header->count) != record_count)
  {
    warn("Warning: Invalid Netflow V5 packet.");
    return 0;
  }

  return record_count;
}

static void exits()
{
  shm_free(shmid, hash_table);
  close(sockfd);
  exit(0);
}

int main(int argc, char *argv[])
{
  int ch;
  int plen;
  int n;
  char *buf, *ptr;
  struct sockaddr_in pin;
  int record_count;
  struct NF_header *header;
  struct NF_record *record;
  int hash_value;
  struct fttime ftt;
  struct tm *tm;

  listen_ipaddr = INADDR_ANY;
  listen_port = 0;

  while ((ch = getopt(argc, argv, "i:p:s:v")) != -1)
    switch ((char) ch)
    {
      case 'i':		/* Listen IP */
	listen_ipaddr = inet_addr(optarg);
        break;
      case 'p':		/* Listen Port */
	listen_port = htons(atoi(optarg));
        break;

      case 's':		/* Path prefix to store */
        break;

      case 'v':		/* Verbose mode */
	verbose = 1;
        break;

      case '?':
      default:
        usage(argv[0]);
        return -1;
    }

  buf = (char *) malloc(8192);
  if (buf == NULL)
  {
    fprintf(stderr, "Failed to allocate memory %d bytes\n", 8192);
    return -1;
  }

  hash_table = (struct hostflow *) shm_new(SHMKEY, sizeof(struct hostflow) * IP_NUM, &shmid);
  if (buf == NULL)
  {
    fprintf(stderr, "Failed to allocate memory %d bytes\n", sizeof(struct hostflow) * IP_NUM);
    return -1;
  }
  memset(hash_table, 0, sizeof(struct hostflow) * IP_NUM);

  signal(SIGTSTP, SIG_IGN);
  signal(SIGINT, exits);
  /*
  signal(SIGHUP, flush);
  signal(SIGTERM, save);
  signal(SIGUSR1, save);
  */

  if (listen_port <= 0)
    listen_port = htons(9991);

  sockfd = build_socket();

  setvbuf(stdout, NULL, _IONBF, 0);

  plen = sizeof(struct sockaddr_in);

  while (1)
  {
    n = recvfrom(sockfd, buf, 8192, 0, (struct sockaddr *) &pin, &plen);
    if (n == -1 && verbose)
      perror("Error recvfrom() from peer in");

    record_count = is_valid(buf, n);
    header = (struct NF_header *) buf;
    ptr = buf + NF_HEADER_SIZE;

    for (ch = 0 ; ch < record_count; ch++)
    {
      ptr += NF_RECORD_SIZE;
      record = (struct NF_record *) ptr;

      if (verbose)
      {
	char src_ip[17];
	char dst_ip[17];
	struct in_addr src_addr, dst_addr;

        src_addr.s_addr = record->srcaddr;
	dst_addr.s_addr = record->dstaddr;
	inet_ntop(PF_INET, (void *) &src_addr, src_ip, 16);
	inet_ntop(PF_INET, (void *) &dst_addr, dst_ip, 16);

	printf("SrcIP: %-17.17s 0x%-8.8x\tDstIP: %-17.17s 0x%-8.8x\n", src_ip, src_addr.s_addr, dst_ip, src_addr.s_addr);
      }

      if ((record->srcaddr & 0x0000ffff) == IP_PREFIX)
      {
	hash_value = ntohs(record->srcaddr >> 16);

	ftt = ftltime(ntohl(header->SysUptime), ntohl(header->unix_secs), ntohl(header->unix_nsecs), ntohl(record->First));
	tm = localtime((time_t *) &ftt.secs);

        hash_table[hash_value].hflow[tm->tm_hour][UPLOAD] += record->dOctets;
        hash_table[hash_value].nflow[UPLOAD] += record->dOctets;
      }
      else if ((record->dstaddr & 0x0000ffff) == IP_PREFIX)
      {
	hash_value = ntohs(record->dstaddr >> 16);

	ftt = ftltime(ntohl(header->SysUptime), ntohl(header->unix_secs), ntohl(header->unix_nsecs), ntohl(record->First));
	tm = localtime((time_t *) &ftt.secs);

        hash_table[hash_value].hflow[tm->tm_hour][DOWNLOAD] += record->dOctets;
        hash_table[hash_value].nflow[DOWNLOAD] += record->dOctets;
      }
    }
  }

  shm_free(shmid, hash_table);
  close(sockfd);
  return 0;
}

