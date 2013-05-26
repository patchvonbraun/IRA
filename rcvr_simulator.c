#include <stdio.h>
#include <math.h>
#include <stdlib.h>

main ()
{
	int ssize;
	int psrate;
	float *sbuf;
	float *pbuf;
	FILE *sfifo;
	FILE *pfifo;
	int i;
	
	ssize = atoi(getenv("RCVR_SETI_SIZE"));
	psrate = atoi(getenv("RCVR_PSR_RATE"));
	
	fprintf (stderr, "ssize %d  psrate %d\n", ssize, psrate);
	
	sbuf = (float *)malloc (ssize * sizeof(float));
	pbuf = (float *)malloc (psrate * sizeof(float));
	
	sfifo = fopen ("ra_seti_fifo", "w");
	pfifo = fopen ("ra_psr_fifo", "w");
	
	srand48 (getpid());
	
	for (;;)
	{
		for (i = 0; i < ssize; i++)
		{
			float x;
			
			x = drand48() * 7500.0;
			if ((lrand48 () % 2) == 0)
			{
				x *= -1.0;
			}
			sbuf[i] = 30000 + x;
		}
		for (i = 0; i < psrate; i++)
		{
			pbuf[i] = sbuf[i]*2.0;
		}
		size_t fwrite(const void *ptr, size_t size, size_t nmemb,
                     FILE *stream);

		fwrite (sbuf, sizeof(float), ssize, sfifo);
		fwrite (pbuf, sizeof(float), psrate, pfifo);
		usleep (500000);
	}
}
