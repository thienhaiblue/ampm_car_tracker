
#ifndef __FTPC_H__
#define __FTPC_H__

#include "uip.h"
#include "ff.h"
#include "diskio.h"
#include "lib/sys_tick.h"
#define MAX_FILENAMELEN	48

extern uint8_t filePermissionDenied;
extern Timeout_Type tFtpConnectionTimeout;

struct ftp_dataconn {
	uint8_t status;
#define FTP_DATA_NONE 0
#define FTP_DATA_FAIL 1
#define FTP_DATA_COMPLETED 2
  uint8_t conntype;
#define CONNTYPE_LIST 0
#define CONNTYPE_DOWNFILE 1
#define CONNTYPE_UPFILE 	2
	uint8_t socket_status;
  uint16_t port;
  uint8_t filenameptr;
  char filename[MAX_FILENAMELEN];
};

typedef struct{
  uint8_t type;
#define TYPE_CONTROL 1
#define TYPE_DATA    2
#define TYPE_ABORT   3
#define TYPE_CLOSE   4

  uint8_t state;
#define STATE_NONE         0
#define STATE_INITIAL      1
#define STATE_SEND_USER    2
#define STATE_USER_SENT    3
#define STATE_SEND_PASS    4
#define STATE_PASS_SENT    5
#define STATE_SEND_PORT    6
#define STATE_PORT_SENT    7
#define STATE_SEND_OPTIONS 8
#define STATE_OPTION_SENT  9
#define STATE_CONNECTED   10
#define STATE_SEND_NLST   11
#define STATE_NLST_SENT   12
#define STATE_SEND_RETR   13
#define STATE_RETR_SENT   14
#define STATE_SEND_EPSV 15
#define STATE_EPSV_SENT 16
#define STATE_SEND_STOR 17
#define STATE_STOR_SENT	18
#define STATE_SEND_CWD    19
#define STATE_CWD_SENT    20


#define STATE_SEND_CDUP   21
#define STATE_CDUP_SENT   22

#define STATE_SEND_MKD	23
#define STATE_MKD_SENT	24

#define STATE_SEND_QUIT   25
#define STATE_QUIT_SENT   26



  uint8_t socket_status;
#define FTP_SOCKET_DISCONNECTED   0
#define FTP_SOCKET_CONNECTED   1
  struct ftp_dataconn dataconn;
  char code[3];
  uint8_t codeptr;
  uint8_t optionsptr;
	uip_ipaddr_t ipaddr;
	FIL *fatFile;
  char *(*ftp_get_user)(void);
	char *(*ftp_get_pass)(void);
	char *(*ftp_get_file_name)(void);
	void (*ftp_list_file)(char *filename);
}ftp_connection;

void ftpc_init(void);
void ftpc_free_connection(ftp_connection *c);
void *ftpc_connect(ftp_connection *c,uip_ipaddr_t *ipaddr, uint16_t port);
char ftpc_list(void *conn, char *filename);
char ftpc_cwd(void *conn, char *dirname);
char ftpc_mkd(void *conn, char *dirname);
void ftpc_cdup(void *connection);
char ftpc_get(void *connection, char *filename);
char ftpc_put(void *conn, char *filename);
void ftpc_close(void *connection);
char ftpc_update_svr_file_name(void *conn, char *filename);

void ftpc_appcall(void );

#define FTPC_OK        200
#define FTPC_COMPLETED 250
#define FTPC_NODIR     431
#define FTPC_NOTDIR    550

/* Functions to be implemented by the calling module: */
void ftpc_connected(void *connection);
void ftpc_cwd_done(unsigned short status);
char *ftpc_username(void);
char *ftpc_password(void);
void ftpc_closed(void);
void ftpc_aborted(void);
void ftpc_timedout(void);
void ftpc_list_file(char *filename);
void ftpc_data(uint8_t *data, uint16_t len);
void ftp_process(void);

#endif /* __FTPC_H__ */
