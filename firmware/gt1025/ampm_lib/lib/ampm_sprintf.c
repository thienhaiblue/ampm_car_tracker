/**********************************************************************
Name: Hai Nguyen Van
Cellphone: (84) 97-8779-222
Mail:thienhaiblue@ampm.com.vn 
----------------------------------
AMPM ELECTRONICS EQUIPMENT TRADING COMPANY LIMITED.,
Add: 634/6 Phan Van Suu street , Ward 13, Tan Binh District, HCM City, VN

*********************************************************************/
#include "lib/ampm_sprintf.h"

/* Character code support macros */
#ifndef IsUpper
#define IsUpper(c)	(((c)>='A')&&((c)<='Z'))
#endif
#ifndef IsLower
#define IsLower(c)	(((c)>='a')&&((c)<='z'))
#endif
#ifndef IsDigit
#define IsDigit(c)	(((c)>='0')&&((c)<='9'))
#endif
#ifndef EOF
#define EOF (-1)
#endif

int ampm_f_putc (
	char c,	/* A character to be output */
	char **fp		/* Pointer to the file object */
)
{
	*(*fp) = c;
	(*fp)++;
	*(*fp) = 0;
	return c;
}

int ampm_f_puts (
	const char *str,	/* Pointer to the string to be output */
	char **fp				/* Pointer to the file object */
)
{
	int n;
	for (n = 0; *str; str++, n++) {
		if (ampm_f_putc(*str, fp) == EOF) return EOF;
	}
	return n;
}

/*-----------------------------------------------------------------------*/
/* Put a formatted string to the file                                    */
/*-----------------------------------------------------------------------*/

int ampm_vsprintf (char *fp,
	const char* format,	/* Pointer to the format string */
	va_list arg					/* Optional arguments... */
)
{
	uint8_t f, r;
	uint16_t i, j, w;
	uint32_t v;
	char c, d, s[16], *p;
	int res, chc, cc;

	//va_start(arg, format);

	for (cc = res = 0; cc != EOF; res += cc) {
		c = *format++;
		if (c == 0) break;			/* End of string */
		if (c != '%') {				/* Non escape character */
			cc = ampm_f_putc(c, &fp);
			if (cc != EOF) cc = 1;
			continue;
		}
		w = f = 0;
		c = *format++;
		if (c == '0') {				/* Flag: '0' padding */
			f = 1; c = *format++;
		} else {
			if (c == '-') {			/* Flag: left justified */
				f = 2; c = *format++;
			}
		}
		while (IsDigit(c)) {		/* Precision */
			w = w * 10 + c - '0';
			c = *format++;
		}
		if (c == 'l' || c == 'L') {	/* Prefix: Size is long int */
			f |= 4; c = *format++;
		}
		if (!c) break;
		d = c;
		if (IsLower(d)) d -= 0x20;
		switch (d) {				/* Type is... */
		case 'S' :					/* String */
			p = va_arg(arg, char*);
			for (j = 0; p[j]; j++) ;
			chc = 0;
			if (!(f & 2)) {
				while (j++ < w) chc += (cc = ampm_f_putc(' ', &fp));
			}
			chc += (cc = ampm_f_puts(p, &fp));
			while (j++ < w) chc += (cc = ampm_f_putc(' ', &fp));
			if (cc != EOF) cc = chc;
			continue;
		case 'C' :					/* Character */
			cc = ampm_f_putc((char)va_arg(arg, int), &fp); continue;
		case 'B' :					/* Binary */
			r = 2; break;
		case 'O' :					/* Octal */
			r = 8; break;
		case 'D' :					/* Signed decimal */
		case 'U' :					/* Unsigned decimal */
			r = 10; break;
		case 'X' :					/* Hexdecimal */
			r = 16; break;
		default:					/* Unknown type (pass-through) */
			cc = ampm_f_putc(c, &fp); continue;
		}

		/* Get an argument and put it in numeral */
		v = (f & 4) ? (uint32_t)va_arg(arg, long) : ((d == 'D') ? (uint32_t)(long)va_arg(arg, int) : (uint32_t)va_arg(arg, unsigned int));
		if (d == 'D' && (v & 0x80000000)) {
			v = 0 - v;
			f |= 8;
		}
		i = 0;
		do {
			d = (char)(v % r); v /= r;
			if (d > 9) d += (c == 'x') ? 0x27 : 0x07;
			s[i++] = d + '0';
		} while (v && i < sizeof s / sizeof s[0]);
		if (f & 8) s[i++] = '-';
		j = i; d = (f & 1) ? '0' : ' ';
		chc = 0;
		while (!(f & 2) && j++ < w) chc += (cc = ampm_f_putc(d, &fp));
		do chc += (cc = ampm_f_putc(s[--i], &fp)); while(i);
		while (j++ < w) chc += (cc = ampm_f_putc(' ', &fp));
		if (cc != EOF) cc = chc;
	}

	//va_end(arg);
	return (cc == EOF) ? cc : res;
}

int  ampm_sprintf(char *fp,const char *format, ...)
{
	uint32_t len;
	va_list     vArgs;		    
	va_start(vArgs, format);
	len = ampm_vsprintf(fp,format,vArgs);
	va_end(vArgs);
	return len;
}

