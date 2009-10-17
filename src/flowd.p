/* command.c */
void parseCmd(char *cmd);
/* netflow.c */
inline int getIPIdx(in_addr_t ipaddr);
int isValidNFP(const char *buf, int len);
void InsertFlowEntry(char *buf, int recCount);
/* flowd.c */
void ExportRecord(int mode);
int ImportRecord(char *fname);
void Warn(const char *msg);
void Diep(const char *s);
int main(int argc, char *argv[]);
/* fttime.c */
struct fttime ftltime(uint32_t sys, uint32_t secs, uint32_t nsecs, uint32_t t);
/* socket.c */
void SendBufToSock(int sckfd, const char *buf, int len);
int BuildUDPSock(in_addr_t listen_ipaddr, uint16_t listen_port);
int BuildTCPSock(in_addr_t listen_ipaddr, uint16_t listen_port);
void Exits(int s);
void SockExit(int s);
