#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "forms.h"
#include "integrated_ra_receiver.h"
#include "rcvr_globals.h"

#define PCHARTSIZE 500
float pulsar_buffer[PCHARTSIZE];

float psr_x[PCHARTSIZE];
float psr_y[PCHARTSIZE];

time_t last_psr_log = 0;
time_t last_psr_plot = 0;
char psr_logbuf[16384];

/*
 * Mission elapsed timer, starts at zero, and counts up by 1.0/rate for every
 *   sample.
 */
double met = 0.0;

inject_pulsar (buf, size, rate)
float *buf;
int size;
int rate;
{	
	int i, j, k;
	int fint;
	int cnt;
	int allfilled;
	float a;
	time_t now;
	int bin;
	double tm;
	

	/*
	 * Integration constant
	 */
	a = 1.0 / ((float)pulsar_folding);
	
	if (pulsar_rate <= 0)
	{
		fl_clear_xyplot (fd_receiver_pulsar->pulsar_plot);
	}

	if (pulsar_rate > 0)
	{
		double Tb;
		double x;
		unsigned long long q;
		
		Tb = (1.0/pulsar_rate)/(double)PCHARTSIZE;
		
		/*
		 * For each incoming sample, determine which bin it belongs to
		 */
		for (i = 0; i < size; i++)
		{
			/*
			 * Calculate bin
			 */
			x = met/Tb;
			q = (unsigned long long)x % (unsigned long long)PCHARTSIZE;
			bin = (int)q;
			
			/*
			 * Update that bin
			 */
			pulsar_buffer[bin] = (a * buf[i]) + ((1.0 - a)*pulsar_buffer[bin]);
			
			/*
			 * Update Mission Elapsed Time ON EVERY SAMPLE
			 */
			met += (1.0 / (double)rate);
		}
	
		time (&now);
		
		/*
		 * We dump the pulsar profile once every two seconds
		 */
		if ((now - last_psr_plot) > 1)
		{
			float min;
			float max;
			
			last_psr_plot = now;
			
			/*
			 * Calculate minimum, maximum so we can normalize the bins in the plot
			 */
			min = 99e14;
			for (i = 0; i < PCHARTSIZE; i++)
			{
				if (pulsar_buffer[i] < min)
				{
					min = pulsar_buffer[i];
				}
			}
			
			max = -99e14;
			for (i = 0; i < PCHARTSIZE; i++)
			{
				if ((pulsar_buffer[i]-min) > max)
				{
					max = (pulsar_buffer[i]-min);
				}
			}
			
			/*
			 * Then create the plot buffer
			 */
			for (i = 0; i < PCHARTSIZE; i++)
			{
				psr_x[i] = ((float)i * Tb);
				psr_y[i] = (pulsar_buffer[i] - min)/max;
			}
			fl_set_xyplot_data(fd_receiver_pulsar->pulsar_plot, psr_x, psr_y, PCHARTSIZE, "Pulses", "Time(Sec)", 
				"Rel Pwr");
		}
				
		if ((now - last_psr_log) >= 20)
		{
			float avg;
			
			last_psr_log = now;
			sprintf (psr_logbuf, "%f %d [ ", pulsar_rate, rate);
			avg = 0.0;
			for (i = 0; i  < PCHARTSIZE; i++)
			{
				avg += psr_y[i];
				if (i && ((i % 2) == 0))
				{
					avg /= 2.0;
					sprintf (psr_logbuf+strlen(psr_logbuf), "%8.2f ", avg);
					avg = 0.0;
				}
				if (i && ((i % 16) == 0))
				{
					sprintf (psr_logbuf+strlen(psr_logbuf), "\n");
				}
			}
			sprintf (psr_logbuf+strlen(psr_logbuf), " ]");
			log_data (datadir, "psr", psr_logbuf);
		}
	}
}
