
/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "ampm_gsm_common.h"
#include "ampm_gsm_sms.h"
#include "ampm_gsm_call.h"
#include "lib/list.h"
#include "sms/pdu.h"
#include "at_command_parser.h"


uint8_t ampm_GotIncomingNumberFlag = 0;
uint8_t ampm_IncomingCallPhoneNo[16];

uint8_t Ampm_ComparePhoneNumber_1(char* phone1, char* phone2,uint8_t digitThreshold)
{
	uint8_t i = strlen(phone1);
	uint8_t j = strlen(phone2);
	uint8_t l1;
	uint8_t l2;
	uint8_t minL;
	uint8_t count = 0;
	
	l1 = i;
	if (phone1[0] == '+')
	{
		if (i > 4)
		{
			l1 = i - 4;
		}
	}
	else if (phone1[0] == '0')
	{
		if (i > 1)
		{
			l1 = i - 1;
		}
	}
	
	l2 = j;
	if (phone2[0] == '+')
	{
		if (j > 4)
		{
			l2 = j - 4;
		}
	}
	else if (phone2[0] == '0')
	{
		if (j > 1)
		{
			l2 = j - 1;
		}
	}
	
	minL = l1;
	if (l2 < l1)
	{
		minL = l2;
	}
	
	while ((i != 0) && (j != 0))
	{
		i--;
		j--;
		if (phone1[i] == phone2[j])
		{
			count++;
		}
		else
		{
			break;
		}
	}
	
	if ((count >= digitThreshold) && (count >= minL))
	{
		return SAME_NUMBER;
	}
	
	return DIFFERENT_NUMBER;
}


uint8_t Ampm_ComparePhoneNumber(char* phone1, char* phone2)
{
	uint8_t i = strlen(phone1);
	uint8_t j = strlen(phone2);
	uint8_t l1;
	uint8_t l2;
	uint8_t minL;
	uint8_t count = 0;
	
	l1 = i;
	if (phone1[0] == '+')
	{
		if (i > 4)
		{
			l1 = i - 4;
		}
	}
	else if (phone1[0] == '0')
	{
		if (i > 1)
		{
			l1 = i - 1;
		}
	}
	
	l2 = j;
	if (phone2[0] == '+')
	{
		if (j > 4)
		{
			l2 = j - 4;
		}
	}
	else if (phone2[0] == '0')
	{
		if (j > 1)
		{
			l2 = j - 1;
		}
	}
	
	minL = l1;
	if (l2 < l1)
	{
		minL = l2;
	}
	
	while ((i != 0) && (j != 0))
	{
		i--;
		j--;
		if (phone1[i] == phone2[j])
		{
			count++;
		}
		else
		{
			break;
		}
	}
	
	if ((count >= PHONE_DIGIT_THRESHOLD) && (count >= minL))
	{
		return SAME_NUMBER;
	}
	
	return DIFFERENT_NUMBER;
}




