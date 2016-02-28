/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/

#include "lib/ringbuf.h"

/**
* \brief init a RINGBUF object
* \param r pointer to a RINGBUF object
* \param buf pointer to a byte array
* \param size size of buf
* \return 0 if successfull, otherwise failed
*/
int32_t RINGBUF_Init(RINGBUF *r, uint8_t* buf, uint32_t size)
{
	if(r == NULL || buf == NULL || size < 2) return -1;
	
	r->pt = buf;
	r->head = 0;
	r->tail = 0;
	r->size = size;
	
	return 0;
}
/**
* \brief put a character into ring buffer
* \param r pointer to a ringbuf object
* \param c character to be put
* \return 0 if successfull, otherwise failed
*/
int32_t RINGBUF_Put(RINGBUF *r, uint8_t c)
{		  
	uint32_t temp;
	temp = r->head;
	temp++;
	if(temp >= r->size) 
	{
		temp = 0;
	}
	if(temp == r->tail)
	{
	 	return -1;		// ring buffer is full
	}
	
	r->pt[r->head] = c;
	r->head = temp;
	
	return 0;
}
/**
* \brief get a character from ring buffer
* \param r pointer to a ringbuf object
* \param c read character
* \return 0 if successfull, otherwise failed
*/
int32_t RINGBUF_Get(RINGBUF *r, uint8_t* c)
{
	if(r->tail == r->head) 
	{
		return -1;				// ring buffer is empty, this should be atomic operation
	}
	*c = r->pt[r->tail];
	r->tail++;
	if(r->tail >= r->size) 
	{
		r->tail = 0;
	}
	return 0;
}

int32_t RINGBUF_GetFill(RINGBUF *r)
{		
   if(r->head >= r->tail)
	{
		return (r->head - r->tail);
	}
	else
	{
	   return( r->size - r->tail + r->head);
	}
}
