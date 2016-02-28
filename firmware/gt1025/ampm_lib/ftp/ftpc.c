

#include "ftpc.h"
#include "petsciiconv.h"

#include <string.h>
#include <stdio.h>

#include "memb.h"
#include "tcpip.h"

#include "system_config.h"
#include "lib/sys_tick.h"
#include "lib/ampm_sprintf.h"
#define ISO_nl 0x0a
#define ISO_cr 0x0d

#define DATAPORT 6510

extern uint32_t  DbgCfgPrintf(const uint8_t *format, ...);
#define MAIN_Info(...)  DbgCfgPrintf(__VA_ARGS__)

Timeout_Type tFtpConnectionTimeout;

uint8_t filePermissionDenied = 0;


#define NUM_OPTIONS 1
static const struct {
  uint8_t num;
  char *commands[NUM_OPTIONS];
} options = {
  NUM_OPTIONS,
  {"TYPE I\r\n"}
};

void ftpc_data_appcall(void);
void ftpc_control_appcall(void);
void ftpc_close(void *conn);

extern Timeout_Type tTcpDataIsBusy;

/*---------------------------------------------------------------------------*/
void ftpc_init(void)
{
  
}
/*---------------------------------------------------------------------------*/
void *ftpc_connect(ftp_connection *c,uip_ipaddr_t *ipaddr, uint16_t port)
{
	memset(c,0,sizeof(ftp_connection));
  c->type = TYPE_CONTROL;
  c->state = STATE_INITIAL;
  c->dataconn.port = port;
	c->ipaddr = *ipaddr;
	return tcp_connect(&c->ipaddr, UIP_HTONS(c->dataconn.port),ftpc_control_appcall,c);
}

/*---------------------------------------------------------------------------*/
static void
handle_input(ftp_connection *c)
{
  int code;
	char *pt;
	uint32_t t1;
  code = (c->code[0] - '0') * 100 +
    (c->code[1] - '0') * 10 +
    (c->code[2] - '0');
	if(code == 503/*Bad sequence of commands*/
	|| code == 550 /*Permission denied*/
	|| code == 530 
	)
	{
		if(c->state == STATE_STOR_SENT)
		{
			filePermissionDenied = 1;
		}
		c->optionsptr = 0;
		c->state = STATE_SEND_USER;
	}
	switch(c->state) {
	case STATE_INITIAL:
		 if(code == 220) {
      c->state = STATE_SEND_USER;
			DbgCfgPrintf("FTPC:STATE_SEND_USER\r\n");
    }
	break;
  case STATE_USER_SENT:
		if(code == 331) {
      c->state = STATE_SEND_PASS;
			DbgCfgPrintf("FTPC:STATE_SEND_PASS\r\n");
    }
    break;
  case STATE_PASS_SENT:
		 if(code == 230) {
      c->state = STATE_SEND_OPTIONS;
      c->optionsptr = 0;
			DbgCfgPrintf("FTPC:STATE_SEND_OPTIONS\r\n");
    }
    break;
  
  case STATE_OPTION_SENT:
		if(c->optionsptr >= options.num) {
			c->state = STATE_CONNECTED;
			DbgCfgPrintf("FTPC:STATE_CONNECTED\r\n");
    } else {
      c->state = STATE_SEND_OPTIONS;
			DbgCfgPrintf("FTPC:STATE_SEND_OPTIONS\r\n");
    }
    break;
	case STATE_CONNECTED:
		
    break;
	case STATE_EPSV_SENT:
		 if(code == 229){
			pt = strstr((char *)((char *)uip_appdata),"(|||");
			if(pt){
				sscanf((char *)pt,"(|||%d|)",&t1);
				if(0 < t1 < 0xffff){
					c->dataconn.port = t1;
				}
			}
			if(tcp_connect(&c->ipaddr, UIP_HTONS(c->dataconn.port),ftpc_data_appcall,c) != NULL){
				if(c->dataconn.conntype == CONNTYPE_UPFILE){
					c->state = STATE_SEND_STOR;
				}else if(c->dataconn.conntype == CONNTYPE_DOWNFILE){
					c->state = STATE_SEND_RETR;
				}else if(c->dataconn.conntype == CONNTYPE_LIST){
					c->state = STATE_SEND_NLST;
				}
			}
			else{
				c->optionsptr = 0;
				c->state = STATE_SEND_USER;
			}
		}
		else{
			c->optionsptr = 0;
			c->state = STATE_SEND_USER;
		}
	break;
	case STATE_STOR_SENT:
  case STATE_NLST_SENT:
  case STATE_RETR_SENT:
   if(c->dataconn.socket_status == FTP_SOCKET_DISCONNECTED)
			c->state = STATE_CONNECTED;
    break;
  case STATE_CWD_SENT:
  case STATE_CDUP_SENT:
	case STATE_MKD_SENT:
    c->state = STATE_CONNECTED;
    break;
  case STATE_QUIT_SENT:
    
    break;
  default:
    return;
  }
}
/*---------------------------------------------------------------------------*/
static void
newdata(ftp_connection *c)
{
  uint16_t i;
  uint8_t d;
	((char *)uip_appdata)[uip_datalen()] = 0;
  DbgCfgPrintf("\r\nFTPC:New data:%d Bytes\r\n",uip_datalen());
  for(i = 0; i < uip_datalen(); ++i) {
    d = ((char *)uip_appdata)[i];
    if(c->codeptr < sizeof(c->code)) {
      c->code[c->codeptr] = d;
      ++c->codeptr;
    }

    if(d == ISO_nl) {
      handle_input(c);
      c->codeptr = 0;
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
acked(ftp_connection *c)
{
  switch(c->state) {
  case STATE_SEND_USER:
		DbgCfgPrintf("FTPC:STATE_USER_SENT\r\n");
    c->state = STATE_USER_SENT;
    break;
  case STATE_SEND_PASS:
		DbgCfgPrintf("FTPC:STATE_PASS_SENT\r\n");
    c->state = STATE_PASS_SENT;
    break;
  case STATE_SEND_PORT:
		DbgCfgPrintf("FTPC:STATE_PORT_SENT\r\n");
    c->state = STATE_PORT_SENT;
    break;
  case STATE_SEND_OPTIONS:
		DbgCfgPrintf("FTPC:STATE_OPTION_SENT\r\n");
    ++c->optionsptr;
    c->state = STATE_OPTION_SENT;
    break;
  case STATE_SEND_NLST:
		DbgCfgPrintf("FTPC:STATE_NLST_SENT\r\n");
    c->state = STATE_NLST_SENT;
    break;
  case STATE_SEND_RETR:
		DbgCfgPrintf("FTPC:STATE_RETR_SENT\r\n");
    c->state = STATE_RETR_SENT;
    break;
	case STATE_SEND_EPSV:
		DbgCfgPrintf("FTPC:STATE_EPSV_SENT\r\n");
		c->state = STATE_EPSV_SENT;
		break;
	case STATE_SEND_STOR:
		DbgCfgPrintf("FTPC:STATE_STOR_SENT\r\n");
    c->state = STATE_STOR_SENT;
		break;
  case STATE_SEND_CWD:
		DbgCfgPrintf("FTPC:STATE_CWD_SENT\r\n");
    c->state = STATE_CWD_SENT;
    break;
  case STATE_SEND_CDUP:
		DbgCfgPrintf("FTPC:STATE_CDUP_SENT\r\n");
    c->state = STATE_CDUP_SENT;
    break;
	case STATE_SEND_MKD:
		DbgCfgPrintf("FTPC:STATE_MKD_SENT\t\n");
		c->state = STATE_MKD_SENT;
  case STATE_SEND_QUIT:
		DbgCfgPrintf("FTPC:STATE_QUIT_SENT\r\n");
    c->state = STATE_QUIT_SENT;
    uip_close();
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
senddata(ftp_connection *c)
{
  uint16_t len = 0;
  
  switch(c->state) {
  case STATE_SEND_USER:
    len = 5 + (uint16_t)strlen(c->ftp_get_user()) + 2;
    strcpy(uip_appdata, "USER ");
    strncpy((char *)uip_appdata + 5, c->ftp_get_user(), uip_mss() - 5 - 2);
    strcpy((char *)uip_appdata + len - 2, "\r\n");
    break;
  case STATE_SEND_PASS:
    len = 5 + (uint16_t)strlen(c->ftp_get_pass()) + 2;
    strcpy(uip_appdata, "PASS ");
    strncpy((char *)uip_appdata + 5, c->ftp_get_pass(), uip_mss() - 5 - 2);
    strcpy((char *)uip_appdata + len - 2, "\r\n");
    break;
  case STATE_SEND_PORT:
    len = ampm_sprintf(uip_appdata, "PORT %d,%d,%d,%d,%d,%d\r\n",
		  uip_ipaddr_to_quad(&uip_hostaddr),
		  (c->dataconn.port) >> 8,
		  (c->dataconn.port) & 0xff);
    break;
  case STATE_SEND_OPTIONS:
    len = (uint16_t)strlen(options.commands[c->optionsptr]);
    strcpy(uip_appdata, options.commands[c->optionsptr]);
    break;
	case STATE_SEND_EPSV:
		len = 6;
    strcpy(uip_appdata, "EPSV\r\n");
		break;
  case STATE_SEND_NLST:
		if(c->dataconn.socket_status == FTP_SOCKET_CONNECTED){
			len = 6;
			strcpy(uip_appdata, "NLST\r\n");
		}
    break;
	case STATE_SEND_STOR:
		if(c->dataconn.socket_status == FTP_SOCKET_CONNECTED){
			filePermissionDenied = 0;
			len = ampm_sprintf(uip_appdata, "STOR %s\r\n", c->dataconn.filename);
		}
		break;
  case STATE_SEND_RETR:
		if(c->dataconn.socket_status == FTP_SOCKET_CONNECTED){
			len = ampm_sprintf(uip_appdata, "RETR %s\r\n", c->dataconn.filename);
		}
    break;
	case STATE_STOR_SENT:
  case STATE_NLST_SENT:
  case STATE_RETR_SENT:
   if(c->dataconn.socket_status == FTP_SOCKET_DISCONNECTED)
			c->state = STATE_CONNECTED;
	break;
  case STATE_SEND_CWD:
    len = ampm_sprintf(uip_appdata, "CWD %s\r\n", c->dataconn.filename);
    break;
  case STATE_SEND_CDUP:
    len = 6;
    strcpy(uip_appdata, "CDUP\r\n");
    break;
	case STATE_SEND_MKD:
		len = ampm_sprintf(uip_appdata, "MKD %s\r\n", c->dataconn.filename);
    break;
  case STATE_SEND_QUIT:
    len = 6;
    strcpy(uip_appdata, "QUIT\r\n");
    break;
  default:
    return;
  }
	if(len)
	{
		petsciiconv_toascii(uip_appdata, len);
		uip_send(uip_appdata, len);
	}
}
uint32_t ftpc_control_appcall_cnt = 0;
/*---------------------------------------------------------------------------*/
void ftpc_control_appcall(void)
{
  
  ftp_connection *c = (ftp_connection *)uip_conn->appstate.state;
  ftpc_control_appcall_cnt++;
	if(c == NULL)
	{
		uip_close();
	}
  if(uip_connected()) {
		DbgCfgPrintf("FTPC:ftpc_control_appcall connected\r\n");
    c->socket_status = FTP_SOCKET_CONNECTED;
  }
	InitTimeout(&tFtpConnectionTimeout,SYSTICK_TIME_SEC(30));
  if(c->type == TYPE_ABORT) {
    uip_abort();
    return;
  }

  if(c->type == TYPE_CLOSE) {
    uip_close();
    c->type = TYPE_CONTROL;
    return;
  }

  if(c->type == TYPE_CONTROL) {
    if(uip_closed()) {
      c->socket_status = FTP_SOCKET_DISCONNECTED;
    }
    if(uip_aborted()) {
      c->socket_status = FTP_SOCKET_DISCONNECTED;
    }
    if(uip_timedout()) {
      c->socket_status = FTP_SOCKET_DISCONNECTED;
    }
    if(uip_acked()) {
			
      acked(c);
    }
    if(uip_newdata()) {
		
      newdata(c);
    }
    if(uip_rexmit() ||
       uip_newdata() ||
     uip_acked()) {
      senddata(c);
    } else if(uip_poll()) {
      senddata(c);
    }
  } 
}
uint32_t ftpc_data_appcall_cnt = 0;
uint32_t ftp_data_acked = 1;
uint8_t fileOpened = 0;
void ftpc_data_appcall(void)
{
		int i, t;
		uint32_t res;
		char buf[32],*pt;
		ftp_connection *c = (ftp_connection *)uip_conn->appstate.state;
		ftpc_data_appcall_cnt++;
		if(uip_connected()) {
			ftp_data_acked = 1;
			DbgCfgPrintf("FTPC:ftpc_data_appcall connected\r\n");
			c->dataconn.socket_status = FTP_SOCKET_CONNECTED;
			ampm_sprintf(buf,"IMAGE/%s",c->ftp_get_file_name());
			if(c->dataconn.conntype == CONNTYPE_DOWNFILE){
				if(fileOpened){
					f_close(c->fatFile);
				}
				if(f_open(c->fatFile,buf, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK){
					pt = strstr(buf,".jpg");
					if(pt){
						strcpy(pt,".inf");
						res = f_unlink(buf);
					}
					c->dataconn.socket_status = FTP_SOCKET_DISCONNECTED;
					uip_close();
				}else{
					InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
					fileOpened = 1;
				}
			}else if(c->dataconn.conntype == CONNTYPE_UPFILE){
				if(fileOpened){
					f_close(c->fatFile);
				}
				if(f_open(c->fatFile,buf, FA_READ | FA_OPEN_EXISTING) != FR_OK)	{
					pt = strstr(buf,".jpg");
					if(pt){
						strcpy(pt,".inf");
						res = f_unlink(buf);
					}
					c->dataconn.socket_status = FTP_SOCKET_DISCONNECTED;
					uip_close();
				}else{
					InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
					fileOpened = 1;
				}
			}
		}
		if(c->socket_status == FTP_SOCKET_DISCONNECTED)
		{
			uip_close();
		}
		 if(c->dataconn.conntype == CONNTYPE_LIST) {
      if(uip_newdata()) {
				InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
				for(i = 0; i < uip_datalen(); ++i) {
					t = ((char *)uip_appdata)[i];
					
					if(c->dataconn.filenameptr < sizeof(c->dataconn.filename) - 1 &&
						 t != ISO_cr &&
						 t != ISO_nl) {
						c->dataconn.filename[c->dataconn.filenameptr] = t;
						++c->dataconn.filenameptr;
					}
					
					if(t == ISO_nl) {
						c->dataconn.filename[c->dataconn.filenameptr] = 0;
						petsciiconv_topetscii(c->dataconn.filename, c->dataconn.filenameptr);
									c->ftp_list_file(c->dataconn.filename);
						c->dataconn.filenameptr = 0;
					}
				}
      }
			if(uip_closed()) {
				c->ftp_list_file(NULL);
				c->dataconn.status = FTP_DATA_COMPLETED;
				c->dataconn.socket_status = FTP_SOCKET_DISCONNECTED;
			}
    }else if(c->dataconn.conntype == CONNTYPE_DOWNFILE){
				if(uip_newdata()) {
					InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
					if(f_write(c->fatFile,((uint8_t *)uip_appdata),uip_len,&res) != FR_OK){
						uip_close();
						c->dataconn.socket_status = FTP_SOCKET_DISCONNECTED;
						c->dataconn.status = FTP_DATA_FAIL;
					}
					DbgCfgPrintf("Received %d data bytes:\n",uip_datalen());
				}
				else if(uip_closed() || uip_timedout() || uip_aborted()) {
					if(fileOpened){
						c->dataconn.status = FTP_DATA_COMPLETED;
					}else{
						c->dataconn.status = FTP_DATA_FAIL;
					}
					f_close(c->fatFile);
					fileOpened = 0;
					c->dataconn.socket_status = FTP_SOCKET_DISCONNECTED;
			}
		}else if(c->dataconn.conntype == CONNTYPE_UPFILE){
				if(c->state == STATE_STOR_SENT){
					if(uip_rexmit()){	
						f_lseek(c->fatFile,f_tell(c->fatFile) - 512);
						if(f_read(c->fatFile,((uint8_t *)uip_appdata),512,&res) ==RES_OK){
							uip_send(uip_appdata, res);
							InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
						}
					}
					if(uip_acked()) {
						InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
						if(f_tell(c->fatFile) >= f_size(c->fatFile)){
							c->dataconn.status = FTP_DATA_COMPLETED;
							uip_close();
						}
						ftp_data_acked = 1;
						InitTimeout(&tTcpDataIsBusy,SYSTICK_TIME_SEC(30));
						
					}
					if(uip_poll()) {
						if(ftp_data_acked){
							ftp_data_acked  = 0;
							if(f_read(c->fatFile,((uint8_t *)uip_appdata),512,&res) ==RES_OK){
								uip_send(uip_appdata, res);
							}
						}
					}
				}else{
					ftp_data_acked = 1;
				}
				if(uip_closed() || uip_timedout() || uip_aborted()) {	
					DbgCfgPrintf("\r\nFTPC:UPLOADED FILE COMPLETE!\r\n");
					ftp_data_acked = 1;	
					if((f_tell(c->fatFile) >= f_size(c->fatFile)) && fileOpened){
						DbgCfgPrintf("\r\nFTPC:UPLOADED FILE SUCESS!\r\n");
						c->dataconn.status = FTP_DATA_COMPLETED;
					}else if(c->dataconn.status == FTP_DATA_NONE){
						DbgCfgPrintf("\r\nFTPC:UPLOADED FILE FAIL!\r\n");
						c->dataconn.status = FTP_DATA_FAIL;
					}
					f_close(c->fatFile);
					fileOpened = 0;
					c->dataconn.socket_status = FTP_SOCKET_DISCONNECTED;
				}
		}
}

/*---------------------------------------------------------------------------*/
char
ftpc_list(void *conn, char *filename)
{
  ftp_connection *c;
  c = conn;
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }
	DbgCfgPrintf("\r\nFTPC: ftpc_list();\r\n");
	ampm_sprintf(c->dataconn.filename,"%s",filename);
	petsciiconv_toascii(c->dataconn.filename, strlen(c->dataconn.filename));
  c->state = STATE_SEND_EPSV;
  c->dataconn.conntype = CONNTYPE_LIST;
	c->dataconn.status = FTP_DATA_NONE;
  return 1;
}
/*---------------------------------------------------------------------------*/
char
ftpc_get(void *conn, char *filename)
{
  ftp_connection *c;

  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }
	ampm_sprintf(c->dataconn.filename,"%s/%s",sysCfg.imei,filename);
  petsciiconv_toascii(c->dataconn.filename, strlen(c->dataconn.filename));

  c->state = STATE_SEND_EPSV;
  c->dataconn.conntype = CONNTYPE_DOWNFILE;
	c->dataconn.status = FTP_DATA_NONE;
  return 1;
}

char
ftpc_put(void *conn, char *filename)
{
  ftp_connection *c;

  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }
	DbgCfgPrintf("\r\nFTPC:Start Upload file:%s\r\n",filename);
	ampm_sprintf(c->dataconn.filename,"%s/%s",sysCfg.imei,filename);
  petsciiconv_toascii(c->dataconn.filename, strlen(c->dataconn.filename));

  c->state = STATE_SEND_EPSV;
  c->dataconn.conntype = CONNTYPE_UPFILE;
	c->dataconn.status = FTP_DATA_NONE;
  return 1;
}


char
ftpc_update_svr_file_name(void *conn, char *filename)
{
  ftp_connection *c;

	ampm_sprintf(c->dataconn.filename,"%s/%s",sysCfg.imei,filename);
  petsciiconv_toascii(c->dataconn.filename, strlen(c->dataconn.filename));
  return 1;
}
/*---------------------------------------------------------------------------*/
void
ftpc_close(void *conn)
{
  ftp_connection *c;
  
  c = conn;
  
  if(c == NULL) {
    return;
  }

  c->type = TYPE_CLOSE;
}
/*---------------------------------------------------------------------------*/
char
ftpc_cwd(void *conn, char *dirname)
{
  ftp_connection *c;
  
  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }
  strncpy(c->dataconn.filename, dirname, sizeof(c->dataconn.filename));
  c->state = STATE_SEND_CWD;
	return 1;
}

char
ftpc_mkd(void *conn, char *dirname)
{
  ftp_connection *c;
  
  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return 0;
  }
  strncpy(c->dataconn.filename, dirname, sizeof(c->dataconn.filename));
  c->state = STATE_SEND_MKD;
	return 1;
}

/*---------------------------------------------------------------------------*/
void
ftpc_cdup(void *conn)
{
  ftp_connection *c;
  
  c = conn;
  
  if(c == NULL ||
     c->state != STATE_CONNECTED) {
    return;
  }
  
  c->state = STATE_SEND_CDUP;
}
/*---------------------------------------------------------------------------*/
