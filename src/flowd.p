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
/* multiplex.c */
int selectInitImpl(MultiplexorFunc_t *this);
int selectUnInitImpl(MultiplexorFunc_t *this);
int selectIsActiveImpl(MultiplexorFunc_t *this, int fd);
int selectAddToListImpl(MultiplexorFunc_t *this, int fd);
int selectRemoveFromListImpl(MultiplexorFunc_t *this, int fd);
int selectWaitImpl(MultiplexorFunc_t *this);
MultiplexorFunc_t *selectNewMultiplexor(void);
int selectFreeMultiplexor(MultiplexorFunc_t *this);
/* socket.c */
void SendBufToSock(int sckfd, const char *buf, int len);
int BuildUDPSock(in_addr_t listen_ipaddr, uint16_t listen_port);
int BuildTCPSock(in_addr_t listen_ipaddr, uint16_t listen_port);
void Exits(int s);
void SockExit(int s);
/* global.c */
