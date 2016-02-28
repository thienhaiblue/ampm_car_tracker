/******************************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 22 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN
******************************************************************************/

#ifndef __AMPM_GSM_RING_H__
#define __AMPM_GSM_RING_H__

/*RI pin chang status*/
#define AMPM_RINGING_FLAG					0x01
/* +CLIP: "0978779222",161,,,,0 */
#define AMPM_RING_GOT_AT_CLIP	0x02
/* RING*/
#define AMPM_RING_GOT_AT_RING		0x04
/* +CMTI: */
#define AMPM_RING_GOT_AT_CMTI		0x08

typedef enum{
	AMPM_GSM_RING_IDLE_PHASE = 0,
	AMPM_GSM_RING_IS_DETECTING,
	AMPM_GSM_RING_IS_A_SMS,
	AMPM_GSM_RING_IS_A_CALL
}AMPM_GSM_RING_PHASE;

extern uint8_t ampm_MissCall;
extern uint8_t ampm_NewSmsFlag;

AMPM_GSM_RING_PHASE Ampm_Ringing_GetPhase(void);
void Ampm_RingingTimer(void);
void Ampm_SetRinging(void);
void Ampm_RingingReset(void);
void Ampm_SetAtRing(void);
void Ampm_SetAtClip(void);
void Ampm_RingingProcess(void);
void Ampm_SetAtCmti(void);
void Ampm_RingingDisable(void);
void Ampm_RingingEnable(void);
#endif
