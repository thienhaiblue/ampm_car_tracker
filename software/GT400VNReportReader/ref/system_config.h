#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "stm32f10x_flash.h"
#include "hw_config.h"
#include "gps.h"

#define DEBUG_MODE	0

#define FIRMWARE_VERSION	"1.00"

#define HARDWARE_V2

#define DEFAULT_AUTO_ENABLE_SECURITY_TIME		60 //sec


#define DEFAULT_POWER_OFF_DELAY_TIME		(24*3600) //sec
#define DEFAULT_SERVER_SEND_PERIOD	10// 10sec

#define DEFAULT_SPEED_ENGINE_OFF	30 //30km/h

#define GPS_SUPPORT	1
#define GSM_SUPPORT	2
#define VIBRATION_SENSOR_SUPPORT	4
#define RF_MODULE_SUPPORT	8

#define SECURITY_ON	1
#define SECURITY_OFF	0

#define WARNING_TURN_OFF_ENGINE	1
#define	WARNING_HORN						2
#define WARNING_SMS							4
#define WARNING_CALL						8

#define DEFAULT_SECURITY_ON_OFF								SECURITY_ON
#define DEFAULT_SECURITY_ACTION								(WARNING_TURN_OFF_ENGINE | WARNING_HORN | WARNING_SMS | WARNING_CALL)
#define DEFAULT_SOS_KEY_PRESS_TIMES						3
#define DEFAULT_SOS_KEY_PRESS_PERIOD					5 
#define DEFAULT_POWER_LOW_WARNING_PERIOD			(3600) //gio
#define DEFAULT_BATTERY_LOW_WARNING_PERIOD		(3600) //gio
#define DEFAULT_POWER_LOW_WARNING_LEVEL				15 //phan tram
#define DEFAULT_BATTERY_LOW_WARNING_LEVEL			15 //phan tram

#define DEFAULT_MODULE_SUPPORT 								(GPS_SUPPORT | GSM_SUPPORT | VIBRATION_SENSOR_SUPPORT | RF_MODULE_SUPPORT)

#define DEFAULT_BOST_NUMBER	"0978779222"
#define DEFAULT_SMS_PWD			"12345678"
#define DEFAULT_GPSR_APN		"internet"
#define DEFAULT_GPRS_USR		"mms"
#define DEFAULT_GPRS_PWD		"mms"
#define DEFAULT_SMS_MASTER_PWD	"zota"
#define DEFAULT_REMOTE_FW_FILE	"zota_gps.efw"

#define DEFAULT_DSERVER_NAME	"http://atoztracking.vn"
#define DEFAULT_DSERVER_IP0		210
#define DEFAULT_DSERVER_IP1		71
#define DEFAULT_DSERVER_IP2		190
#define DEFAULT_DSERVER_IP3		170
#define DEFAULT_DSERVER_PORT	4049
#define DEFAULT_DSERVER_USEIP	1


#define DEFAULT_FSERVER_NAME	"http://atoztracking.vn"
#define DEFAULT_FSERVER_IP0		210
#define DEFAULT_FSERVER_IP1		71
#define DEFAULT_FSERVER_IP2		190
#define DEFAULT_FSERVER_IP3		170
#define DEFAULT_FSERVER_PORT	50000
#define DEFAULT_FSERVER_USEIP	1

#define DEFAULT_ISERVER_NAME	"http://atoztracking.vn"
#define DEFAULT_ISERVER_IP0		210
#define DEFAULT_ISERVER_IP1		71
#define DEFAULT_ISERVER_IP2		190
#define DEFAULT_ISERVER_IP3		170
#define DEFAULT_ISERVER_PORT	12345
#define DEFAULT_ISERVER_USEIP	1

#define DEFAULT_INFO_SERVER_NAME	"http://atoztracking.vn"
#define DEFAULT_INFO_SERVER_IP0		118
#define DEFAULT_INFO_SERVER_IP1		69
#define DEFAULT_INFO_SERVER_IP2		60
#define DEFAULT_INFO_SERVER_IP3		174
#define DEFAULT_INFO_SERVER_PORT	2802
#define DEFAULT_INFO_SERVER_USEIP	1

#define DEFAULT_SPEED_LIMIT		80
#define DEFAULT_DRIVING_TIME_LIMIT			60*60*4
#define DEFAULT_TOTAL_DRIVING_TIME_LIMIT	60*60*10

#define DEFAULT_SPEED_SENSOR_RATIO			10
#define DEFAULT_REPORT_INTERVAL				10
#define DEFAULT_STOP_REPORT_INTERVAL		60	/* report interval when vehicle stopped, second */

#define DEFAULT_ENABLE_WARNING				1
#define DEFAULT_WHITE_NUMBER				""

#define DEFAULT_INFO_STRING					"Cong ty van tai XYZ\r\n"\
																		"Ngo Quyen – HaNoi\r\n"\


#define DEFAULT_LOGIN_KEY0		0x411F2248
#define DEFAULT_LOGIN_KEY1		0x211D8594
#define DEFAULT_LOGIN_KEY2		0x55284163
#define DEFAULT_LOGIN_KEY3		0x63ECEBFF

#define DEFAULT_SLEEP_TIMER		60


// system definitions
#define CONFIG_HW_VERSION			1
#define CONFIG_FW_VERSION			1
#define CONFIG_RELEASE_DATE			__TIMESTAMP__
#define CONFIG_MAX_DRIVERS			10



#define CONFIG_READ_CHUNK_SIZE		8192
#define CONFIG_WRITE_CHUNK_SIZE		1024

#define CONFIG_REMOTE_FW_FILE		DEFAULT_REMOTE_FW_FILE
#define CONFIG_FW_FILE				"ambo.amb"

#define CONFIG_SPEED_STOP			5

#define CONFIG_SIZE_GPRS_APN		16
#define CONFIG_SIZE_GPRS_USR		16
#define CONFIG_SIZE_GPRS_PWD		16
#define CONFIG_SIZE_SERVER_NAME		31
#define CONFIG_SIZE_SMS_PWD			16
#define CONFIG_SIZE_VIN				20
#define CONFIG_SIZE_PLATE_NO		12
#define CONFIG_SIZE_DRIVER_NAME		32
#define CONFIG_SIZE_LICENSE_NO		12
#define CONFIG_SIZE_ISSUE_DATE		11
#define CONFIG_SIZE_EXPIRY_DATE		11
#define CONFIG_SIZE_PHONE_NUMBER	16


#define FEATURE_GPS_AUTO_AIDING						0x0001
#define FEATURE_GPS_ASSISTNOW_OFFLINE			0x0002
#define FEATURE_GPS_ASSISTNOW_ONLINE			0x0004
#define FEATURE_GPS_ASSISTNOW_AUTONOMOUS	0x0008
#define FEATURE_LOW_BATTERY_WARNING				0x0010
#define FEATURE_LOW_POWER_WARNING					0x0020
#define FEATURE_ACC_WARNING								0x0040
#define FEATURE_TAP_WARNING								0x0080
#define FEATURE_REMOVE_POWER_WARNING			0x0100
#define FEATURE_AUTO_ENABLE_SECURIRY 			0x0200
#define FEATURE_SUPPORT_FIFO_MSG					0x0400
#define FEATURE_SUPPORT_BEEP_WARNING			0x0800

#define CONFIG_SIZE_IP				32


typedef struct __attribute__((packed)){
	int8_t driverName[CONFIG_SIZE_DRIVER_NAME];
	int8_t licenseNo[CONFIG_SIZE_LICENSE_NO];
	int8_t issueDate[CONFIG_SIZE_ISSUE_DATE];
	int8_t expiryDate[CONFIG_SIZE_EXPIRY_DATE];
	int8_t phoneNo[CONFIG_SIZE_PHONE_NUMBER];
}DRIVER_INFO;


typedef struct __attribute__((packed))
{
	int8_t imei[18];
	
	int8_t id[18];
	
	int8_t smsPwd[CONFIG_SIZE_SMS_PWD];					/**< SMS config password */
	
	int8_t whiteCallerNo[CONFIG_SIZE_PHONE_NUMBER];		/**< */
	
	// GPRS config
	int8_t gprsApn[CONFIG_SIZE_GPRS_APN];
	int8_t gprsUsr[CONFIG_SIZE_GPRS_USR];
	int8_t gprsPwd[CONFIG_SIZE_GPRS_PWD];
	uint16_t runReportInterval;
	uint16_t stopReportInterval;
	// current server config
	uint8_t curDServer;
	// primary server config
	uint16_t priDserverIp[2];		/**< ip addr */
	uint8_t  priDserverName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t priDserverPort;	/**< port */	
	// secondary server config
	uint16_t secDserverIp[2];		/**< ip addr */
	uint8_t  secDserverName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t secDserverPort;	/**< port */	
	uint8_t  dServerUseIp;	/**<>**/
	// current firmware server config
	uint8_t curFServer;
	// primary server config
	uint16_t priFserverIp[2];		/**< ip addr */
	uint8_t  priFserverName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t priFserverPort;	/**< port */	
	// secondary server config
	uint16_t secFserverIp[2];		/**< ip addr */
	uint8_t  secFserverName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t secFserverPort;	/**< port */	
	uint8_t  fServerUseIp;	/**<>**/
	
	uint8_t curInfoServer;
	// primary server config
	uint16_t priInfoServerIp[2];		/**< ip addr */
	uint8_t  priInfoServerName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t priInfoServerPort;	/**< port */	
	// secondary server config
	uint16_t secInfoServerIp[2];		/**< ip addr */
	uint8_t  secInfoServerName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t secInfoServerPort;	/**< port */	
	uint8_t  infoServerUseIp;	/**<>**/
	
//  current driver info
	uint8_t driverIndex;
	DRIVER_INFO driverList[CONFIG_MAX_DRIVERS];
	// vehicle info
	int8_t plateNo[CONFIG_SIZE_PLATE_NO];
	int8_t vin[CONFIG_SIZE_VIN];
	uint16_t speedSensorRatio;							/**< pulses/Km/h */
	uint16_t useGpsSpeed;
	
	uint16_t a1Mode;					/**< raw mode = 0, percentage mode = 1 */
	uint16_t a1LowerBound;			/**< lower bound value for percentage mode */
	uint16_t a1UpperBound;			/**< upper bound value for percentage mode */
	
	uint16_t a2Mode;
	uint16_t a2LowerBound;
	uint16_t a2UpperBound;
	
	// current image server config
	uint8_t curIServer;
	// primary image server config
	uint16_t priIserverIp[2];		/**< ip addr */
	uint8_t  priIserverName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t priIserverPort;	/**< port */	
	// secondary image server config
	uint16_t secIserverIp[2];		/**< ip addr */
	uint8_t  secIserverName[CONFIG_SIZE_SERVER_NAME];	/**< domain name */
	uint16_t secIserverPort;	/**< port */	
	uint8_t  iServerUseIp;	/**<>**/
	
	
	uint8_t numCameras;				/**< number of supported cameras */
	uint8_t cameraCompression;		/**< compression value */
	uint8_t cameraWorkingStartTime;
	uint8_t cameraWorkingStopTime;
	uint16_t cameraInterval;
	uint16_t enableWarning;
	uint16_t cameraEvents;			/**< camera capture events map
									bit 0-1: DIN1 assert/deassert event
									bit 2-3: DIN2 assert/deassert event
									bit 4-5: DIN3 assert/deassert event
									bit 6-7: DIN4 assert/deassert event
									bit 8: periodically
									bit 9-15: reserved */
	uint16_t speedLimit;									/**< vehicle speed limit */
	uint32_t drivingTimeLimit;
	uint32_t totalDrivingTimeLimit;
	uint32_t baseMileage;	
									
	uint16_t featureSet;			/**< feature set mapping 
														bit0: GPS Automatic local aiding
														bit1: GPS AssistNow offline
														bit2: GPS AssistNow online
														bit3: GPS AssistNow autonomous
														bit4: Low Battery warning
														bit5: Low Power warning
														bit6: ACC Warning
														bit7: Tap Warning
														bit8: Remove power warning
														bit9: Auto enable security
														*/
	uint16_t lastError;
	uint16_t testMode;
	
	uint32_t loginKey[4];			/**< reporter login key */	
	uint8_t securityOn;
	uint8_t securityAction;
														/**< feature set mapping 
														bit0: Send SMS
														bit1: CALL
														bit2:
														bit3:
														bit4:
														bit5:
														bit6:
														*/
	uint8_t SOSKeyPressTimes;	
	uint8_t SOSKeyPressPeriod;	
	uint32_t sleepTimer;	
	uint32_t powerLowWarningPeriod;
	uint32_t batteryLowWarningPeriod;
	uint8_t powerLowWarningLevel;
	uint8_t batteryLowWarningLevel;
	
	uint32_t autoSecurityTime;	/**<>**/
	uint32_t speedEngineOff; /*Khi bat dc GPS va toc do < speedEngineOff (km/h) thi tat may xe*/
	uint32_t powerOffDelayTime;
	uint8_t accountAlarmCheck;
	
	uint8_t upgradeTimeStamp[20];	/**< firmware upgrade timestamp */
	uint8_t infoString[200];
	uint32_t crc;
}CONFIG_POOL;

extern CONFIG_POOL sysCfg;


void CFG_Save(void);
void CFG_Load(void);

#endif
