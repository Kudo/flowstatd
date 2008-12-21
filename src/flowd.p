/* command.c */
void parseCmd(char *cmd);
/* netflow.c */
int hash(in_addr_t ipaddr);
int isValidNFP(const char *buf, int len);
void PushRecord(char *buf, int recCount);
/* flowd.c */
void ExportRecord(int mode);
void ImportRecord(char *fname);
void Warn(char *msg);
void Diep(const char *s);
int main(int argc, char *argv[]);
/* fttime.c */
struct fttime ftltime(uint32_t sys, uint32_t secs, uint32_t nsecs, uint32_t t);
/* socket.c */
void SendBufToSck(int sckfd, const char *buf, int len);
int BuildUDPSck(in_addr_t listen_ipaddr, uint16_t listen_port);
int BuildTCPSck(in_addr_t listen_ipaddr, uint16_t listen_port);
void Exits(int s);
void SckExit(int s);
