#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "md5.h"
#include "license_keys.h"

#define KEYLEN 8
char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
main ()
{
	char inbuf[33];
	int i;
	time_t now;
	MD5_CTX ctx;
	
	srand48 (getpid () ^ time (&now));
	inbuf[KEYLEN] = 0;
	for (i = 0; i < KEYLEN; i++)
	{
		inbuf[i] = alpha[lrand48() % strlen(alpha)];
	}
	
	MD5Init (&ctx);
	MD5Update (&ctx, KEY1, strlen(KEY1));
	MD5Update (&ctx, inbuf, strlen(inbuf));
	MD5Update (&ctx, KEY2, strlen(KEY2));
	MD5Final (&ctx);
	
	fprintf (stdout, "%s ", inbuf);
	for (i = 0; i < 8; i++)
	{
		fprintf (stdout, "%02X", ctx.digest[i]);
	}
	fprintf (stdout, "\n");
}
