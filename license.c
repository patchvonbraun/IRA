#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "md5.h"
#include "exptime.h"
#include "license_keys.h"

char exptime[128];

check_license (info)
char *info;
{
	char *key;
	time_t now;
	MD5_CTX ctx;
	char inbuf[128];
	char hashbuf[128];
	char hexstr[128];
	int i;
	
	strcpy (exptime, EXPTIME);
	key = getenv ("RCVR_LICENSE_KEY");
	if (key != NULL && (strcmp (key, "none") != 0))
	{
		sscanf (key, "%s %s", inbuf, hashbuf);
		MD5Init (&ctx);
		MD5Update (&ctx, KEY1, strlen(KEY1));
		MD5Update (&ctx, inbuf, strlen(inbuf));
		MD5Update (&ctx, KEY2, strlen(KEY2));
		MD5Final (&ctx);
		hexstr[0] = '\0';
		for (i = 0; i < 8; i++)
		{
			sprintf (hexstr+strlen(hexstr), "%02X", ctx.digest[i]);
		}
		if (strcmp (hashbuf, hexstr) == 0)
		{
			sprintf (info, "License Key: %s %s", inbuf, hashbuf);
			return (1);
		}
		else
		{
			return (0);
		}
	}
	else
	{
		time (&now);
		if (strcmp (exptime, "NONE") == 0)
		{
			return (0);
		}
		else
		{
			time_t exp;
			char *fmt = "%Y/%m/%d-%H:%M:%S";
			struct tm ltp;
			
			time (&now);
			memset (&ltp, 0x00, sizeof(ltp));
			strptime (exptime, fmt, &ltp);
			exp = mktime(&ltp);
			
			if (exp > now)
			{
				sprintf (info, "License: expires at %04d/%02d/%02d",
					1900+ltp.tm_year, ltp.tm_mon+1, ltp.tm_mday);
				return (1);
			}
			return (0);
		}
	}
	
}
