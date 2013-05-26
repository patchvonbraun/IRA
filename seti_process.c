/*
 * A program to take FFTW spectral samples from a companion GnuRadio flowgraph via stdin
 *   and analyse those samples in a SETI context.
 *
 *
 * It also "flattens" the spectrum samples, by computing a smoothed, normalized, inverse
 *   function of the incoming spectrum to reduce the "curviness" of the resulting spectrum.
 *   This helps tremendously in calculating the mean and standard deviation.
 */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <forms.h>
#include "integrated_ra_receiver.h"
#include "rcvr_globals.h"


/*
 * Size of the buffer used to compute the spectral flattening function
 */
#define FLATSIZE 4000

/*
 * A simple map of RFI locations--used as zones of avoidance when
 *   calculating de-chirped candidates
 */
short *rfi_map;


/*
 * Input buffer
 */
float *xbuf;
float *wbuf;

/*
 * How big are the spectral samples (typically, over 1 million)
 */
int INSIZE;
int BINWIDTH = 10;

/*
 * Maximum number of hits in a single hit computation
 */
#define NH 100

/*
 * Largest number of "top candidates"
 */

#define NCANDIDATES 2000
struct candidate
{
	double freq;
	int delta;
	int ncand;
} candidates[NCANDIDATES];

#define NPERSIST 30
#define TOPCAND 15

struct persistent
{
	double freq;
	int cnt;
	int delta;
} pfreqs[NPERSIST];

float sigma;

int accum = 0;
int total = 0;
unsigned char *cp;

int inited = 0;
int counter;

int wfall_height;
int wfall_width;
int wfall_allocated = 0;
#define WFALL_LIMIT 5.5
float *wfall_plot;

float w_max = 0;
int donemax = 0;

float shows[4];

int dashtimer = 10;
int dashstate = 1;
float iavg = -1.0;
int
seti_process (sbuf, len)
float *sbuf;
int len;
{
	int i, j, k;
	float start;
	int next = 0;
	time_t now;
	float a;
	float a2;
	float ia;
	int timeto;
	int browser_cleared = 0;
	FL_Coord x, y;
	FL_Coord w, h;
	struct chirp *chip;
	int peaks;
	float w_min, w_max;
	float w_avg;
	w_min = 99999999;
	w_max = 0;
	unsigned short *tp;
	char show_buf[128];
	float l;
	double binwidth;
	
	binwidth = bandwidth / (double)seti_size;
	
	/*
	 * Integration constant, again from command line
	 */
	a = seti_integ;
	a = 1.0 / a;
	
	/*
	 * Just to reduce re-computing this expression on every IIR filter instance
	 */
	ia = 1.0 - a;
	
	/*
	 * We do an analysis run every 'timeto' inputs (seconds, usually)
	 *  we base it on the integration time constant (a)
	 */
	timeto = (int)((1.0/a)*2.5);
	
	xbuf = sbuf;
	if (inited == 0)
	{
		inited = 1;
		
		/*
		 * Get INSIZE from environment
		 */
		INSIZE = seti_size;
		
		/*
		 * Get sigma_k from environment
		 */
		 {
			 char *p;
			 
			 p = getenv ("RCVR_SIGMA_K");
			 if (p != NULL)
			 {
				 sigma_k = atof(p);
			 }
		 }
		
		/*
		 * xbuf
		 */
		xbuf = (float *)sbuf;
		
		/*
		 * Allocate RFI map
		 */

		rfi_map = (short *)malloc(INSIZE*sizeof(short));
		if (rfi_map == NULL)
		{
			fprintf (stderr, "malloc of rfi_map failed\n");
			exit (0);
		}
		memset (rfi_map, 0x00, INSIZE*sizeof(short));
		
		/*
		 * Allocate wfall buf, and analysis buf
		 */
		wbuf = (float *)malloc (INSIZE*sizeof(float));
		if (wbuf == NULL)
		{
			fprintf (stderr, "malloc of wbuf failed\n");
			exit (0);
		}
		memset (wbuf, 0x00, INSIZE*sizeof(float));
	
		/*
		 * Init counters, etc
		 */
		counter = 0;
		
		fl_get_object_bbox(fd_receiver_waterfall->waterfall_display, &x, &y, &w, &h);
		wfall_height = h;
		wfall_width = w;
		
		wfall_plot = (float *)malloc ((wfall_width+2)*sizeof(float));
		if (wfall_plot == NULL)
		{
			fprintf (stderr, "malloc of wfall_plot failed\n");
			exit (0);
		}
		wfall_allocated = 1;
	}
	
	dashtimer--;
	if (dashtimer <= 0)
	{
		dashtimer = 10;
		dashstate = dashstate ? 0 : 1;
	}
	
	/*
	 * I *think* what this weirdness is doing is trying to flatten-out
	 *   the ends of the spectrum, which are always going to be drooped
	 *   relative to the rest, due to CIC filtering in the FPGA of the
	 *   USRP
	 */
	a2 = 0.001;
	if (iavg < 0.0)
	{
		iavg = 0.0;
		for (i = 0; i < 1000; i++)
		{
			iavg += sbuf[i];
		}
		iavg /= 1000;
	}

	for (i = 0; i < INSIZE; i++)
	{
		if (sbuf[i] < (iavg/20.0))
		{
			float rx;
			rx = drand48() * (iavg / 10.0);
			if ((lrand48 () % 2) == 0)
			{
				rx *= -1.0;
			}
			sbuf[i] = iavg;
			sbuf[i] += rx;
		}
		iavg = (sbuf[i] * a2) + ((1.0 - a2) * iavg);
	}
	
	xbuf = sbuf;
	
	/*
	 * Quirks
	 */
	xbuf[0] = xbuf[1];
	xbuf[INSIZE-1] = xbuf[INSIZE-2];
	
	/*
	 * For each bin in the incoming data, calculated smoothed wbuf
	 */
	for (i = 0; i < INSIZE; i++)
	{
		if (wbuf[i] == 0.0)
		{
			wbuf[i] = xbuf[i];
		}
		wbuf[i] = (a * xbuf[i]) + (ia * wbuf[i]);
	}
	
	/*
	 * Compute local (over wfall_width) average, max, min
	 */
	w_avg = 0.0;
	w_max = 0;
	w_min = 1.0e10;
	for (i = 0; i < wfall_width; i++)
	{
		float v;
		int ndx;
		float wd;
		
		/*
		 * Calculate an validate ndx
		 */
		ndx = waterfall_segment-1;
		ndx *= wfall_width;
		ndx += waterfall_fine;
		if (ndx < 0)
		{
			ndx = 0;
		}
		if (ndx >= INSIZE)
		{
			ndx = INSIZE-1;
		}
		if (ndx+i < 0)
		{
			ndx = 0;
		}
		
		v = wbuf[ndx+i];
		w_avg += v;
		
		if (v > w_max)
		{
			w_max = v;
		}
		if (v < w_min)
		{
			w_min = v;
		}
	}
	
	w_avg /= wfall_width;
	if (w_max > (WFALL_LIMIT*w_avg))
	{
		w_max = (WFALL_LIMIT*w_avg);
	}

	if (squash_mode == 0)
	{
		/*
		 * Stuff the plot buffer
		 */
		for (i = 0; i < wfall_width; i++)
		{
			float v;
			float wd;
			int ndx;
			
			ndx = ((waterfall_segment-1)*wfall_width) + i + (int)(waterfall_fine/binwidth);
			if (ndx < 0)
			{
				ndx = 0;
			}
			if (ndx >= INSIZE)
			{
				ndx = INSIZE-1;
			}
			v = wbuf[ndx];
			if (v >= (WFALL_LIMIT*w_min))
			{
				v = WFALL_LIMIT*w_min;
			}

			v -= w_min;
			v /= (w_max-w_min);
			v *= (255.0*waterfall_brightness);

			if (v > (255.0*waterfall_brightness))
			{
				v = (255.0*waterfall_brightness);
			}
			if (v < 0)
			{
				v = 0;
			}
			if (mark_rfi && (rfi_map[ndx] > 0))
			{
				if (dashstate)
				{
					v = (255.0);
				}
				else
				{
					v = 0;
				}
			}
			wfall_plot[i] = v;
		}
	}
	else
	{
		int mod;

		w_min = 1.0e10;
		w_max = 0.0;
		
		mod = INSIZE/wfall_width;
		
		/*
		 * First pass, calculate min/max
		 */
		for (i = 0; i < wfall_width; i++)
		{
			int j;
			float v;
			
			v = 0.0;
			for (j = 0; j < mod; j++)
			{
				v += wbuf[(i * mod) + j];
			}
			v /= mod;
			if (v < w_min)
			{
				w_min = v;
			}
			if (v > w_max)
			{
				w_max = v;
			}
		}
		
		/*
		 * Then do the scaling
		 */
		for (i = 0; i < wfall_width; i++)
		{
			int j;
			float v;
			
			v = 0.0;
			for (j = 0; j < mod; j++)
			{
				v += wbuf[(i * mod) + j];
			}
			v /= mod;
			v -= w_min;
			v /= (w_max - w_min);
			v *= 255.0*waterfall_brightness;
			if (v > (255.0*waterfall_brightness))
			{
				v = 255.0 * waterfall_brightness;
			}
			if (v < 0.0)
			{
				v = 0.0;
			}
			wfall_plot[i] = v;
		}
	}
	
	/*
	 * Statistics stuff (and color key labels)
	 */
	a = 0.2;
	shows[0] = (a * w_min) + ((1.0 - a) * shows[0]);
	sprintf (show_buf, "%8.1f", shows[0]);
	fl_set_object_label (fd_receiver_waterfall->waterfall_show_min, show_buf);
	
	shows[1] = (a * w_max) + ((1.0 - a) * shows[1]);
	sprintf (show_buf, "%8.1f", shows[1]);
	fl_set_object_label (fd_receiver_waterfall->waterfall_show_max, show_buf);
	sprintf (show_buf, "%6.2fdB", 10.0*log10f(shows[1]/shows[0]));
	fl_set_object_label (fd_receiver_waterfall->waterfall_show_avg, show_buf);


	l = 10.0*log10f(w_max/w_min);
	a = 0.1;
	if (l > (10.0*log10f(WFALL_LIMIT)))
	{
		l = (10.0*log10f(WFALL_LIMIT));
	}
	shows[3] = (a * l) + ((1.0 - a) * shows[3]);
	sprintf (show_buf, "%3.1fdB", shows[3]);
	fl_set_object_label (fd_receiver_waterfall->high_db, show_buf);
	
	/*
	 * Plot the waterfall itself
	 */
	wfall_plotit (wfall_plot, wfall_width);

	/*
	 * Time to do an analysis pass?
	 */
	if (counter && ((counter % timeto) == 0))
	{
		int np;
		
		if (browser_cleared == 0)
		{
			fl_clear_browser (fd_receiver_main->seti_hit_browser);
			browser_cleared = 1;
		}
		
		/*
		 * Do the sigma analysis
		 */
		np = analyse_sigma (wbuf, 0, INSIZE, sigma_k);
		sort_peaks(np);

		if (np > 0)
		{
			add_persistents (np);
		}
		for (i = 0; i < NPERSIST  && i < TOPCAND; i++)
		{
			if (pfreqs[i].cnt >= 10)
			{
				int t;
				char tbuf[256];
				char addbuf[256];
				
				sprintf (addbuf, "%12.6f %2d      %2d      ", pfreqs[i].freq/1.0e6,
					pfreqs[i].delta, pfreqs[i].cnt);
				log_data (datadir, "seti", addbuf);
				sprintf (tbuf, "@C%d@b@m%s", FL_GREEN, addbuf);
				fl_add_browser_line(fd_receiver_main->seti_hit_browser, tbuf);
			}
		}
		prune_persistents (np);
	}
	counter++;
}


/*
 * The core analysis function
 */
#define RFI_PERSISTENCE 15

int
analyse_sigma (buf, offs, len, sk)
float *buf;
int offs;
int len;
float sk;
{
	int i;
	double fst;
	int hitcnt;
	double avg_dif;
	int rficnt;
	float mindif;
	float sig_avg;
	int m;
	int nc = 0;
	double binwidth;
	
	binwidth = bandwidth / len;
	
	nc = 0;
	
	
	/*
	 * Compute sigma (standard deviation)
	 */
	sig_avg = 0.0;
	sigma = 0.0;
	
	/*
	 * First, calculate average
	 */
	for (i = 0; i < len; i++)
	{
		sig_avg += buf[i];
	}
	sig_avg /= (float)len;
	
	/*
	 * Then calculate sigma using average
	 */
	for (i = 0; i < len; i++)
	{
		float d;
		d = buf[i] - sig_avg;
		sigma = sigma + (d*d);
	}
	sigma = sqrtf(sigma/(float)len);
	
	/*
	 * Setup start frequency, for recording hit locations
	 */
	fst = sky_freq - (bandwidth / 2.0);
	hitcnt = 0;
	rficnt = 0;
	
	/*
	 * Look for signals that exceed the mean by so many sigma
	 */
	nc = 0;
	for (i = 0; i < len; i++)
	{
		float SK;
		float RSK;
		float d;
		
		d = buf[i] - sig_avg;
		SK = sk * sigma;
		RSK = 3.0 * sk * sigma;
		
		/*
		 * If we aren't ignoring this, and the difference exceeds the mean by sigma_k * sigma, then
		 *   decide if it's RFI, or a potential hit
		 */
		if ((d >= SK))
		{
			/*
			 * If this is a stronger signal, very likely RFI
			 */
			if (d > RSK)
			{
				rfi_map[i] += 1;
				if (rfi_map[i] > RFI_PERSISTENCE)
				{
					rfi_map[i] = RFI_PERSISTENCE;
				}
			}
			
			/*
			 * Was RFI, but has dropped below threshold
			 */
			else if (rfi_map[i] && (d < (RSK*0.85)))
			{
				rfi_map[i]--;
				if (rfi_map[i] < 0)
				{
					rfi_map[i] = 0;
				}
			}
			/*
			 * Not quite as strong, but statistically significant, and not makred as RFI
			 */
			else if ((rfi_map[i] == 0) && (d < RSK))
			{
				float newval;
				
				/*
				 * Add the delta for this hit to this chirps tracking buffer
				 *   Integrate it over 3 samples
				 */
				newval = (d*100.0)/sigma;
				if (nc < (NCANDIDATES-1))
				{
					candidates[nc].freq = fst + (binwidth*(double)i);
					candidates[nc].delta = (int)newval;
					candidates[nc+1].freq = -1;
					candidates[nc+1].delta = -1;
					nc++;
				}
			}
		}
		/*
		 * Track entries into RFI map
		 */
		if (rfi_map[i])
		{
			rficnt++;
		}
	}
	return nc;
}

/*
 * Find candidate signals (that have previously been put there by analyse_sigma)
 *
 * Build a list of candidates, up to NCANDIDATES big, then sort that list into ascending
 *   order.
 */
sort_peaks (int np)
{
	int cancomp();
	
	qsort ((void *)candidates, np, sizeof(struct candidate), cancomp);
	return (np);
}

int
cancomp (one, two)
struct candidate *one, *two;
{
	return (two->delta - one->delta);
}

#define SAMENESS_THRESHOLD 20.0

add_persistents (int np)
{
	int i, j;
	int found;
	
	found = 0;
	
	/*
	 * For all candidates
	 */
	for (i = 0; i < np && i < TOPCAND; i++)
	{
		found = 0;
		/*
		 * Is already in PERSIST?
		 */
		for (j = 0; j < NPERSIST; j++)
		{
			if (fabs(pfreqs[j].freq - candidates[i].freq) < SAMENESS_THRESHOLD)
			{
				pfreqs[j].freq = candidates[i].freq;
				pfreqs[j].delta = candidates[i].delta;
				pfreqs[j].cnt++;
				found++;
fprintf (stderr, "Found a persistent at %lf %d %d\n", candidates[i].freq, candidates[i].delta, pfreqs[i].cnt);
				break;
			}
		}
		if (!found)
		{
			for (j = 0; j < NPERSIST; j++)
			{
				if (pfreqs[j].cnt <= 0)
				{
					pfreqs[j].freq = candidates[i].freq;
					pfreqs[j].delta = candidates[i].delta;
					pfreqs[j].cnt = 2;
					break;
				}
			}
		}
	}
}

prune_persistents (int np)
{
	int i, j;
	
	
	/*
	 * For each entry in the persistence table
	 */
	for (i = 0; i < NPERSIST; i++)
	{
		int found;
		
		found = 0;
		
		/*
		 * Go down candidates, seeing if it's still there.  If not, prune it
		 */
		for (j = 0; j < np && j < TOPCAND; j++)
		{
			if (pfreqs[i].cnt &&
				fabs(pfreqs[i].freq-candidates[j].freq) < SAMENESS_THRESHOLD)
			{
				found++;
				break;
			}
		}
		if (!found && pfreqs[i].cnt)
		{
			pfreqs[i].cnt /= 2;
		}
	}
}
