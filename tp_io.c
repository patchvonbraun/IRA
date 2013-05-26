#include <stdio.h>
#include <stdlib.h>
#include <forms.h>
#include "integrated_ra_receiver.h"
#include <math.h>
#include "rcvr_globals.h"
#include <time.h>

float current_tp = 0;
float current_sec_tp = 0;
float current_ref = 0;
float current_sec_ref = 0;
int dicke_state = 0;

int tp_counter = 0;
int sec_tp_counter = 0;

float *tp_x = NULL;
float *tp_y = NULL;
float *sec_tp_y = NULL;
time_t last_tp;

inject_tp (buf, size, rate, which)
float *buf;
int size;
int rate;
int which;
{
	float a;
	int i, j;
	unsigned char sidbuf[6];
	struct tm *ltp;
	char dbuf[64];
	time_t now;
	char *prefstr;

	time (&now);
	a = 1.0/(float)((tp_integration*rate)/2);
	for (i = 0; i < size; i++)
	{
		float val;
		
		if (dicke_state)
		{
			if (which == 0)
			{
				current_ref = (a * buf[i]) + ((1.0 - a) * current_ref);
			}
			else
			{
				current_sec_ref = (a * buf[i]) + ((1.0 - a) * current_sec_ref);
			}
		}
		else
		{
			if (which == 0)
			{
				current_tp = (a * buf[i]) + ((1.0 - a) * current_tp);
				tp_counter++;
			}
			else
			{
				current_sec_tp = (a * buf[i]) + ((1.0 - a) * current_sec_tp);
				sec_tp_counter++;
			}
		}
					
		if ((now - last_tp) >= (1*tp_time_scale))
		{
			char dstr[64];
			FL_Coord x, y, w, h;
			double whichtp;
			double whichref;
			
			last_tp = now;
				
			whichtp = (which == 0) ? current_tp : current_sec_tp;
			whichref = (which == 0) ? current_ref : current_sec_ref;
			val = (double)whichtp-(whichref*refmult);
			val = val * ((float)dc_gain/(float)5);
			val *= dc_mult;
			val = val + (float)(dc_offset*100000*dc_mult);
			
			sprintf (dstr, "%9.1f", val);
			
			fl_get_object_bbox (fd_receiver_continuum->tp_chart, &x, &y, &w, &h);
			w -= 130;
			
			if (tp_x == NULL)
			{
				tp_x = (float *)malloc (w * sizeof(float));
				tp_y = (float *)malloc (w * sizeof(float));
				sec_tp_y = (float *)malloc (w * sizeof(float));
				memset (tp_x, 0x00, sizeof(float)*w);
				memset (tp_y, 0x00, sizeof(float)*w);
				memset (sec_tp_y, 0x00, sizeof(float)*w);

			}
			for (j = 0; j < w; j++)
			{
				tp_x[j] = (float)j*(float)tp_time_scale;
			}

			if (which == 0)
			{
				/*
				 * Shift 'n stuff
				 */
				for (j = w-1; j > 0; j--)
				{
					tp_y[j] = tp_y[j-1];
				}
				tp_y[0] = val;
			}
			else
			{
				/*
				 * Shift 'n stuff
				 */
				for (j = w-1; j > 0; j--)
				{
					sec_tp_y[j] = sec_tp_y[j-1];
				}
				sec_tp_y[0] = val;
			}
			
			prefstr = (which == 0) ? "tp" : "tp2";
			log_data (datadir, prefstr, dstr);
			
			if (which == 0)
			{
				fl_set_object_label(fd_receiver_main->tp_current_display, dstr);
				fl_set_object_label(fd_receiver_continuum->tp_current_display, dstr);
				if (val > detector_peak)
				{
					detector_peak = val;
					sprintf (dstr, "%9.1f", detector_peak);
					fl_set_object_label (fd_receiver_main->detector_peak, dstr);
				}

			}
			if (which == 0)
			{
				fl_set_xyplot_data(fd_receiver_continuum->tp_chart, tp_x, tp_y, (int)w, 
			    "Continuum", "Time(secs)", "Intensity");
			}
			else
			{
				fl_add_xyplot_overlay(fd_receiver_continuum->tp_chart, 2, tp_x, sec_tp_y,
                             (int)w, FL_GREEN);
			}
			if (which == 0)
			{
				tp_counter = 0;
			}
			else
			{
				sec_tp_counter = 0;
			}
		
			if (which == 0)
			{
				time (&now);
				ltp = gmtime(&now);
				get_sid_time (sidbuf, longitude, ltp);
				sprintf (dbuf, "%02d:%02d:%02d", sidbuf[0], sidbuf[1], sidbuf[2]);
				fl_set_object_label (fd_receiver_continuum->chart_lmst_low, dbuf);
				
				now -= (w*tp_time_scale)/2;
				ltp = gmtime(&now);
				get_sid_time (sidbuf, longitude, ltp);
				sprintf (dbuf, "%02d:%02d:%02d", sidbuf[0], sidbuf[1], sidbuf[2]);
				fl_set_object_label(fd_receiver_continuum->chart_lmst_mid, dbuf);
				
				
				now -= (w*tp_time_scale)/2;
				ltp = gmtime (&now);
				get_sid_time (sidbuf, longitude, ltp);
				sprintf (dbuf, "%02d:%02d:%02d", sidbuf[0], sidbuf[1], sidbuf[2]);
				fl_set_object_label(fd_receiver_continuum->chart_lmst_high, dbuf);
			}
		}
	}
}


#define TRATE 200
#define NTRANS 1500*(TRATE/100)
float transbuf[NTRANS*2];
float validbuf[NTRANS*2];
int transcnt = 0;
int validcnt = 0;
int tcnt = 0;
int vcnt = 0;
float tval = 0;
float vval = 0;
inject_transients (buf, size, rate, which)
float *buf;
int size;
int rate;
int which;
{
	int i;
	int j;
	int nfull;
	int resid;
	
	nfull = size/TRATE;
	resid = size % TRATE;

	for (i = 0; i < size; i++)
	{
		if (which == 0)
		{
			tval += buf[i];
			tcnt++;
		}
		else
		{
			vval += buf[i];
			vcnt++;
		}
		if (((which == 0) && tcnt >= (rate/TRATE) )||
			((which == 1) && vcnt >= (rate/TRATE) ) )
		{
			if (which == 0)
			{
				tval /= (rate/TRATE);
				tcnt = 0;
			}
			else
			{
				vval /= (rate/TRATE);
				vcnt = 0;
			}
			if (which == 0)
			{
				transbuf[transcnt++] = tval;
				tval = 0.0;
			}
			else
			{
				validbuf[validcnt++] = vval;
				vval = 0.0;
			}
			if (!split_mode)
			{
				if (transcnt >= NTRANS)
				{
					analyse_transients (transbuf, NTRANS, NULL);
					transcnt = 0;
				}
			}
			else
			{
				if (validcnt >= NTRANS && transcnt >= NTRANS)
				{
					analyse_transients (transbuf, transcnt > validcnt ? validcnt : transcnt, validbuf);
					validcnt = 0;
					transcnt = 0;
				}
			}
		}
	}
}

float mvavg = -99.0;
float mvavgvalid = -99.0;

#define CONTEXT 10

char databuf[40000];

analyse_transients (buf, size, vbuf)
float *buf;
int size;
float *vbuf;
{
	char lmst[64];
	int i, j, k;
	float max = 0;
	float min = 99e15;
	time_t now;
	struct tm *ltp;
	float a;
	int length;
	float tavg;
	float vavg;
	int tstarted;
	int vstarted;
	int vlength;
	int vtriggered;
	int ttriggered;
	int vsize;
	int tstop;
	float tpeak;
	
	a = 1.0 / 75.0;
	if (mvavg < -10.0)
	{
		mvavg = 0.0;
		for (i = 0; i < 100; i++)
		{
			mvavg += buf[i];
		}	
		mvavg /= 100.0;
	}
	if (vbuf != NULL && (mvavgvalid < -10.0))
	{
		mvavgvalid = 0.0;
		for (i = 0; i < 100; i++)
		{
			mvavgvalid += vbuf[i];
		}
		mvavgvalid /= 100.0;
	}
	
	length = 0;
	tavg = 0.0;
	tstarted = -1;
	time (&now);
	tpeak = 0.0;
	for (i = 0; i < size; i++)
	{
		float thresh;
		
		thresh = transient_threshold*mvavg;
		while ((buf[i] > thresh) && (i < size))
		{
			if (tstarted < 0)
			{
				tstarted = i;
			}
			tavg += buf[i];
			if (buf[i] > tpeak)
			{
				tpeak = buf[i];
			}
			length++;
			i++;
			thresh = (transient_threshold*mvavg)/2;
		}
		if (i >= size)
		{
			break;
		}
		if (length > 0)
		{
			break;
		}
		mvavg = (a * buf[i]) + ((1.0 - a) * mvavg);
	}
	tstop = i;
	
	/*
	 * Validation buffer
	 */
	vlength = 0;
	vavg = 0.0;
	vstarted = -1;
	vsize = size;
	if (vbuf == NULL)
	{
		vsize = 0;
	}
	for (i = 0; i < vsize; i++)
	{
		float thresh;
		
		if (vbuf == NULL)
		{
			break;
		}
		
		thresh = transient_threshold*mvavgvalid;
		while ((vbuf[i] > thresh) && (i < size))
		{
			if (tstarted < 0)
			{
				vstarted = i;
			}
			vavg += vbuf[i];
			vlength++;
			i++;
			thresh = (transient_threshold*mvavgvalid)/2;
		}
		if (i >= size)
		{
			break;
		}
		if (vlength > 0)
		{
			break;
		}
		mvavgvalid = (a * vbuf[i]) + ((1.0 - a) * mvavgvalid);
	}
	
	
	time (&now);
	now -= (NTRANS/TRATE);
	now += (tstop/TRATE);
	now -= ((length/2)/TRATE);
	ltp = gmtime (&now);
	sprintf (lmst, "%02d:%02d:%02d",
		ltp->tm_hour,
		ltp->tm_min,
		ltp->tm_sec);
		
	if (((split_mode == 0) && (length > transient_duration)) ||
		(split_mode && (vbuf != NULL) && (vlength < transient_duration/2) && (length > transient_duration)) )
	{
		int lower;
		int upper;
		char tbuf[256];
		
		tavg /= length;
		sprintf (databuf, "%s %3d %4.2f %4.2f", lmst, length, tpeak/mvavg, tavg/mvavg);
		sprintf (tbuf, "@C%d@b@m%s", FL_GREEN, databuf);
		fl_add_browser_line (fd_receiver_main->transients_browser, tbuf);
		sprintf (databuf+strlen(databuf), " [ ");
		
		/*
		 * Setup to record transient profile
		 */
		lower = tstarted-CONTEXT;
		if (lower < 0)
		{
			lower = 0;
		}
		upper = (tstarted+length)+CONTEXT;
		if (upper >= size)
		{
			upper = size;
		}
		for (i = lower; i < upper; i++)
		{
			sprintf (databuf+strlen(databuf), "%8.2f ", buf[i]);
			if (i && ((i % 5) == 0))
			{
				sprintf (databuf+strlen(databuf), "\n");
			}
		}
		sprintf (databuf+strlen(databuf), " ]");
		
		if (vbuf != NULL)
		{
			sprintf (databuf+strlen(databuf), " + {");
		}
		for (i = lower; i < upper; i++)
		{
			if (vbuf == NULL)
			{
				break;
			}
			sprintf (databuf+strlen(databuf), "%8.2f ", vbuf[i]);
			if (i && ((i % 5) == 0))
			{
				sprintf (databuf+strlen(databuf), "\n");
			}
		}
		if (vbuf != NULL)
		{
			sprintf (databuf+strlen(databuf), " }");
		}
		
		log_data (datadir, "trns", databuf);
	}	
}

change_dicke_state (s)
int s;
{
	dicke_state = s;
}
