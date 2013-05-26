#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "forms.h"
#include "integrated_ra_receiver.h"
#include "rcvr_globals.h"


float *inter_x;
float *inter_y;

static int inited = 0;
float real = 0.0;
float imag = 0.0;
int inter_cnt = 0;
int realnext = 1.0;
inject_inter (buf, size, rate)
float *buf;
int size;
int rate;
{
	float a;
	int i, j;
	
	if (!inited)
	{
		/* fl_set_object_dblbuffer (fd_receiver_interferometer->interferometer_chart, 1); */
		fl_set_xyplot_ybounds (fd_receiver_interferometer->interferometer_chart, -1.0*interferometer_span, 
			interferometer_span);
		inited = 1;
		fl_set_xyplot_ytics (fd_receiver_interferometer->interferometer_chart, 10, 1);
		fl_set_xyplot_ygrid (fd_receiver_interferometer->interferometer_chart, FL_GRID_MINOR);
		fl_set_xyplot_xgrid (fd_receiver_interferometer->interferometer_chart, FL_GRID_MINOR);
	}
	
	
	a = 1.0 / (rate * 15.0);
	i = 0;
	while (i < size)
	{
		float g;
		
		g = interferometer_gain;
		if (realnext)
		{
			real = (a * (g*buf[i++])) + ((1.0 - a) * real);
			realnext = 0;
		}
		else
		{
			if (i < size)
			{
				imag = (a * (g*buf[i++])) + ((1.0 - a) * imag);
				realnext = 1;
			}
		}
		if (inter_cnt++ >= (rate*2))
		{
			char str[64];
			time_t now;
			struct tm *ltp;
			char sid[10];
			FL_Coord x, y, w, h;
			
			fl_get_object_bbox (fd_receiver_interferometer->interferometer_chart, &x, &y, &w, &h);
			w -= 90;
			
			if (inter_x == NULL)
			{
				inter_x = (float *)malloc (w*sizeof(float));
				inter_y = (float *)malloc (w*sizeof(float));
				memset (inter_x, 0x00, w*sizeof(float));
				memset (inter_y, 0x00, w*sizeof(float));
				for (j = 0; j < w; j++)
				{
					inter_x[j] = (float)j;
				}
			}
			
			sprintf (str, "%f %f", real, imag);
			
			for (j = w-1; j > 0; j--)
			{
				inter_y[j] = inter_y[j-1];
			}
			inter_y[0] = real;
			fl_set_xyplot_data(fd_receiver_interferometer->interferometer_chart, inter_x, inter_y, w,  "East x West",
				"Time(secs)", "Intensity");
			log_data (datadir, "inter", str);
			
			
			time(&now);
			ltp = gmtime (&now);
			get_sid_time (sid, longitude, ltp);
			sprintf (str, "%02d:%02d:%02d", sid[0], sid[1], sid[2]);
			fl_set_object_label (fd_receiver_interferometer->lmst_low, str);
			
			now -= w;
			ltp = gmtime (&now);
			get_sid_time (sid, longitude, ltp);
			sprintf (str, "%02d:%02d:%02d", sid[0], sid[1], sid[2]);
			fl_set_object_label (fd_receiver_interferometer->lmst_high, str);
			
			
			inter_cnt = 0;
		}
	}
}
