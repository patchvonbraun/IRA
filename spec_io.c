#include <stdio.h>
#include <forms.h>
#include "integrated_ra_receiver.h"
#include <math.h>
#include "rcvr_globals.h"

#define DPYSIZE 1024
float *spec_x = NULL;
float *spec_y = NULL;
float *spec_save = NULL;

char dbuf[150000];

int cnt = 0;

time_t lasttime = 0;
time_t lastdpy = 0;

int
inject_spec (buf, cnt, size, abuf, asize)
float *buf;
int cnt;
int size;
float *abuf;
int asize;
{
	int i, j;
	float fst;
	float binwidth;
	float a;
	int csize;
	float val;
	time_t now;
	
	if (spec_x == NULL)
	{
		spec_x = (float *)malloc (spec_fft_size * sizeof(float));
		spec_y = (float *)malloc (spec_fft_size * sizeof(float));
		spec_save = (float *)malloc (spec_fft_size * sizeof(float));
		memset (spec_save, 0x00, sizeof(float) * spec_fft_size);
	}
	
	a = 1.0/((float)spec_integration);
	fst = sky_freq-((float)bandwidth/2.0);
	binwidth = (float)bandwidth / (float)spec_fft_size;
	
	for (i = 0; i < spec_fft_size; i++)
	{
		spec_x[i] = fst/1.0e6;
		fst += binwidth;
		spec_save[i] = (a * buf[i]) + ((1.0 - a) * spec_save[i]);
	}
	
	memcpy (spec_y, spec_save, sizeof(float)*spec_fft_size);
	if (spec_flat_on)
	{
		spec_flatten (spec_y, spec_fft_size);
	}
	
	fl_set_xyplot_data(fd_receiver_spectrum->spectral_plot, spec_x, spec_y, spec_fft_size, "Frequency Spectrum", "MHz", "dB");
	
	time (&now);
	if ((now - lasttime) >= 15)
	{
		char *cp;
		
		lasttime = now;
		cp = dbuf;
		cnt = 0;
		for (i = 0; i < asize; i++)
		{
			sprintf (cp, "%8.1f ", (double)abuf[i]);
			cp += strlen (cp);
			if (i && (i % 8) == 0)
			{
				sprintf (cp, "\n ");
				cp += strlen (cp);
			}
		}
		log_data (datadir, "spec", dbuf);
	}
}
