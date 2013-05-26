#include "forms.h"
#include "integrated_ra_receiver.h"
#include "rcvr_globals.h"
#include "Ira_xpm.h"
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

int seti_fd, handle_seti_io();
int pulsar_fd, handle_pulsar_io();
int dicke_fd, handle_dicke_io();
int inter_fd, handle_inter_io();
int validation_fd, handle_validation_io();

int spectral_plot_post();
int pulsar_plot_post();
int continuum_plot_post();

int spec_integration;
int spec_method = 2;
int spec_flat_on = 0;
int current_smax = 80;
int current_span = 60;
int tp_integration;
double refmult = 1.0;
double detector_peak = 0.0;
double waterfall_integration;
int seti_integ;
int mark_rfi = 0;
int squash_mode = 0;
double pulsar_rate = -1.0;
int pulsar_folding = 5;
double frequency;
double sky_freq;
int sky_locked = 1;
double bandwidth;
int rf_gain;
double gc_a = 1.0;
double gc_b = 1.0;
double dc_gain = 1.0;
double dc_offset = 0.0;
double dc_mult = 1.0;
double declination;
char dec_mode[128];
double ra;
char ra_mode[128];
int seti_size;
int bin_width = 10;
int waterfall_segment = 1;
int waterfall_fine = 0;
double waterfall_brightness = 1.0;
int tp_rate;
int tp_time_scale = 1;
int psr_rate;
int spec_fft_size;
double tp_maxval = 30000;
double tp_minval = 20000;
double tp_span = 20000.0;
double sigma_k = 2.5;
double pulsar_dm = 0.1;
double pulsar_delay = 0.0;
char datadir[512];
char rcvr_mode[128];
double interferometer_gain = 2.5;
double interferometer_phase = 0.0;
double interferometer_delay = 0.0;
double interferometer_span = 100.0;
double transient_threshold= 2.5;
int transient_duration = 3;
int split_mode = 0;

#define NNOTCHES 64
double notches[NNOTCHES];
int notch_length = 80;

time_t last_io_time;

int rcvr_runrun = 1;
int no_more_input = 0;

extern double longitude;
unsigned char current_lmst[6];

char *copyright = "Copyright(c) 2009 Science Radio Laboratories and Marcus Leech";

int gr_fd;

FD_receiver_main *fd_receiver_main;
FD_receiver_pulsar *fd_receiver_pulsar;
FD_receiver_spectrum *fd_receiver_spectrum;
FD_receiver_continuum *fd_receiver_continuum;
FD_receiver_waterfall *fd_receiver_waterfall;
FD_receiver_interferometer *fd_receiver_interferometer;
FD_receiver_info *fd_receiver_info;
FD_receiver_lproblem *fd_receiver_lproblem;
FD_receiver_error *fd_receiver_error;
FD_receiver_shutdown *fd_receiver_shutdown;

/*
 * We need to "push" these variables onto a temporary
 *   "stack" of variables that will be written-back to
 *   the Gnu Radio flow-graph (via XMLRPC) after we detect
 *   that the flow-graph has properly started.
 */
struct pushed_variables
{
	double value;
	char *name;
	int inuse;
};

struct pushed_variables pushed_list[128];
int pushed_next = 0;

#define PUSHVAR(y,x) {pushed_list[pushed_next].value = x;\
					  pushed_list[pushed_next].name = y;\
					  pushed_list[pushed_next].inuse++;\
					  pushed_next++;}


char license_info[128];
char version_info[128];
char *ira_info[] = 
{
	"Copyright(c) 2009 Science Radio Laboratories and Marcus Leech",
	"THIS IS BETA SOFTWARE",
	"NOT INTENDED FOR MASS DISTRIBUTION",
	"There is no warranty of any kind for this software, including but not",
	"limited to, fitness for a particular purpose.",
	" ",
	version_info,
	license_info,
	"http://www.science-radio-labs.com",
	NULL
};


#include "version.h"

time_t started_at;
int main(int argc, char *argv[])
{
	void lmst_update (FL_OBJECT *, long);
	int i;
	void dismiss_psr();
	void spectrum_hide();
	void tp_hide ();
	void hide_waterfall();
	void hide_interferometer();
	void hide_info();
	void receiver_leave();
	int close_sub ();
	int close_main ();
	char *p;
	char fntstr[128];
	char d[128];
	int binwidth;
	
	/*
	 * Init xforms library
	 */
   fl_initialize(&argc, argv, "Ira", 0, 0);
   fl_get_app_resources (NULL, 0);
   
   time (&started_at);
   
   /*
    * Create various windows, including the main one
	*/
   fd_receiver_main = create_form_receiver_main();
	fl_set_form_atclose(fd_receiver_main->receiver_main, close_main, 0);
	
   fd_receiver_pulsar = create_form_receiver_pulsar();
   fl_set_form_atclose (fd_receiver_pulsar->receiver_pulsar, close_sub, dismiss_psr);
   
   fd_receiver_spectrum = create_form_receiver_spectrum();
   fl_set_form_atclose (fd_receiver_spectrum->receiver_spectrum, close_sub, spectrum_hide);
   
   fd_receiver_continuum = create_form_receiver_continuum();
   fl_set_form_atclose (fd_receiver_continuum->receiver_continuum, close_sub, tp_hide);
   
   fd_receiver_waterfall = create_form_receiver_waterfall();
   fl_set_form_atclose (fd_receiver_waterfall->receiver_waterfall, close_sub, hide_waterfall);
   
   fd_receiver_info = create_form_receiver_info();
   fl_set_form_atclose (fd_receiver_info->receiver_info, close_sub, hide_info);
   
   fd_receiver_lproblem = create_form_receiver_lproblem ();
   fd_receiver_error = create_form_receiver_error ();
   
   fd_receiver_shutdown = create_form_receiver_shutdown ();
   
   
   flps_init();
   fl_free_pixmap_pixmap(fd_receiver_main->ira_xpm_button);
   fl_set_pixmap_data(fd_receiver_main->ira_xpm_button, Ira_xpm);
   
   /*
    * Check to make sure that they're licensed to use the software
	*/
   if (check_license (license_info) == 0)
   {
	   fl_show_form(fd_receiver_lproblem->receiver_lproblem,FL_PLACE_CENTER,FL_FULLBORDER,"IRA License Problem");
	   while(fl_do_forms())
         ;
		exit (1);
	}
	sprintf (version_info, "Ver: %s  (BETA)", VERSION);
   
   /*
    * Stuff lines in info window
	*/
   for (i = 0; ; i++)
   {
	   if (ira_info[i] == NULL)
	   {
		   break;
	   }
	   fl_add_browser_line (fd_receiver_info->info_browser, ira_info[i]);
   }
   fl_set_browser_fontsize(fd_receiver_info->info_browser, 14);
   
   /* fill-in form initialization code */
   fl_set_object_label (fd_receiver_main->startup_text, "PLEASE WAIT.........");
   
   /* show the first form */
   fl_show_form(fd_receiver_main->receiver_main,FL_PLACE_CENTER,FL_FULLBORDER,"IRA Control Panel");
   fl_check_forms();
   
   /*
    * Now we check a raft of environment variables, and use those to initialize
	*   various settable values
	*/
	
	refmult = 1.0;
	if ((p = getenv ("RCVR_REF_MULT")) != NULL)
	{
   		refmult = atof(p);
	}
   fl_set_slider_value (fd_receiver_main->refmult_slider, refmult);
   
   seti_integ = 15;
   if ((p = getenv ("RCVR_SETI_INTEG")) != NULL)
   {
   		seti_integ = (int)atof(p);
	}
   fl_set_slider_value (fd_receiver_main->seti_integ_slider, (float)seti_integ);
   
   strcpy (datadir, ".");
   if ((p = getenv ("RCVR_DATA_DIR")) != NULL)
   {
   		strcpy (datadir, p);
	}
  
  	/*
	 * Set the input field for freq, as well as the actual frequency
	 */
   if ((p = getenv ("RCVR_INITIAL_FREQ")) != NULL)
   {
	   fl_set_input (fd_receiver_main->frequency_input, p); 
	   frequency = atof(p);
	   sky_freq = frequency;
	   sky_locked = 1;
   }
   
   /*
    * Start out with sky_freq unavailable for input
	*/
   fl_deactivate_object (fd_receiver_main->sky_freq_input);
   fl_set_input (fd_receiver_main->sky_freq_input, "--------");
   
   /* If there's a sky_freq parameter, use it, and re-activate the
    *  sky_freq_input control
	*/
   if ((p = getenv ("RCVR_SKY_FREQ")) != NULL)
   {
	   if (abs(atof(p) - frequency) > 100.0)
	   {
			   
		   sky_locked = 0;
		   fl_set_input (fd_receiver_main->sky_freq_input, p);
			sky_freq = atof(p);
			fl_activate_object (fd_receiver_main->sky_freq_input);
			fl_set_button (fd_receiver_main->sky_lock_button, 0);
		}
		else
		{
			sky_freq = atof(p);
			fl_set_button (fd_receiver_main->sky_lock_button, 1);
			sky_locked = 1;
		}
	}
	PUSHVAR("ifreq", frequency);
	PUSHVAR("skyfreq", sky_freq);
   /*
    * 
    * And again for RF gain
	*/
	if ((p = getenv ("RCVR_RF_GAIN")) != NULL)
	{
   		rf_gain = atoi(p);
	}
   fl_set_slider_value (fd_receiver_main->rf_gain_slider, rf_gain);
	PUSHVAR("igain", rf_gain);
   /*
    * Gain correction values for A and B sides
	*/
   if ((p = getenv ("RCVR_COR_A")) != NULL)
   {
	   gc_a = atof(p);
	  
   }
   sprintf (d, "%f", gc_a);
   fl_set_input (fd_receiver_main->gc_a, d);
   
   if ((p = getenv ("RCVR_COR_B")) != NULL)
   {
	   gc_b = atof(p);
   }
   sprintf (d, "%f", gc_b);
   fl_set_input (fd_receiver_main->gc_b, d);
   
	/*
	 * Set bounds/values for DC gain
	 */
	if ((p = getenv ("RCVR_DC_GAIN")) != NULL)
	{
		dc_gain = atof(p);
	}
   fl_set_slider_value (fd_receiver_main->dc_gain_control, dc_gain);

   
   /*
    * And again for DC offset
	*/
	if ((p = getenv ("RCVR_DC_OFFSET")) != NULL)
	{
		dc_offset = atof(p);
	}
   fl_set_slider_value (fd_receiver_main->dc_offset_control, dc_offset);
   /*
    * Receiver DC Gain multiplier
	*/
   if ((p = getenv ("RCVR_DC_MULT")) != NULL)
   {
	   int which;
	   
	   dc_mult = (double)atoi(p);
	   which = 1;
	   /*
	    * It's a choice widget, so we need to set 'which' appropriately
		*/
	   switch ((int)dc_mult)
	   {
		case 1:
			which = 1;
			break;
		case 5:
			which = 2;
			break;
		case 10:
			which = 3;
			break;
		case 15:
			which = 4;
			break;
		case 20:
			which = 5;
			break;
		case 25:
			which = 6;
			break;
		case 30:
			which=7;
			break;
		case 35:
			which = 8;
			break;
		case 40:
			which = 9;
			break;
		}
		fl_set_choice (fd_receiver_main->mult_choice, which);
   } 
   
   /*
    * Total power integration value
	*/
	tp_integration = 5;
	if ((p = getenv ("RCVR_TP_INTEG")) != NULL)
	{
		tp_integration = atoi(p);
	}
   fl_set_slider_value (fd_receiver_main->continuum_int, (double)atof(getenv("RCVR_TP_INTEG")) );
   
   /*
    * Spectral integration
	*/
	spec_integration = 15;
	if ((p = getenv ("RCVR_SPEC_INTEG")) != NULL)
	{
		spec_integration = atoi(p);
	}
   fl_set_slider_value (fd_receiver_main->spec_int_slider, (double)atof(getenv("RCVR_SPEC_INTEG")) );
   
   /*
    * Sigma_K for SETI analysis
	*/
	sigma_k = 2.5;
	if ((p = getenv ("RCVR_SIGMA_K")) != NULL)
	{
   		sigma_k = atof(p);
	}
   fl_set_slider_value (fd_receiver_main->sigma_k_slider, sigma_k);
   
   /*
    * Check desired receiver mode
	*/
   if (getenv("RCVR_MODE") != NULL)
   {
   	strcpy (rcvr_mode, getenv("RCVR_MODE"));
	}
	else
	{
		strcpy (rcvr_mode, "unknown");
	}
	
	/*
	 * Interferometer?  Create the interferometer window
	 */
   if (strcmp (rcvr_mode, "interferometer") == 0)
   {
	   fd_receiver_interferometer = create_form_receiver_interferometer();
	   fl_set_form_atclose (fd_receiver_interferometer->receiver_interferometer, close_sub, 
	   	hide_interferometer);
   }
   /*
    * Otherwise, delete the "show interferograms" control
	*/
   else
   {
	   fl_delete_object  (fd_receiver_main->interferometer_button);
   }
   
   /*
    * Various values
	*/
	declination = -28.3;
	if ((p = getenv ("RCVR_DECLINATION")) != NULL)
	{
		declination = atof(p);
	}
	fl_set_input (fd_receiver_main->declination_input, getenv("RCVR_DECLINATION"));
	
	longitude = 0.0;
	if ((p = getenv ("RCVR_LONGITUDE")) != NULL)
	{
		longitude = atof(p);
	}
	seti_size = 500000;
	if ((p = getenv ("RCVR_SETI_SIZE")) != NULL)
	{
   		seti_size = atoi (p);
	}

    bandwidth = 5000000;
    if ((p = getenv ("RCVR_BANDWIDTH")) != NULL)
    {
		bandwidth = atoi (p);
	}

   psr_rate = 10000;
   if ((p = getenv ("RCVR_PSR_RATE")) != NULL)
   {
   		psr_rate = atoi (getenv ("RCVR_PSR_RATE"));
	}
   for (i = 0; i < NNOTCHES; i++)
   {
	   notches[i] = -1.0;
   }
   if ((p = getenv ("RCVR_NOTCHES")) != NULL)
   {
	   char *tp;
	   char pcopy[128];
	   FILE *fp;
	   
	   strcpy (pcopy, p);
	   
	   tp = strtok (pcopy, ",");
	   notches[0] = atof(tp);
	   for (i = 1; i < NNOTCHES; i++)
	   {
		   tp = strtok (NULL, ",");
		   if (tp == NULL)
		   {
			   break;
		   }
		   notches[i] = atof(tp);
	   }
   }
   if ((p = getenv ("RCVR_NOTCH_SIZE")) != NULL)
   {
	   notch_length = atoi(p);
   		fl_set_slider_value (fd_receiver_spectrum->notch_slider, (double)notch_length);
	}
	if ((p = getenv ("RCVR_DM")) != NULL)
	{
		pulsar_dm = atof(p);
		fl_set_slider_value (fd_receiver_main->dm_input, (double)pulsar_dm);
	}
	PUSHVAR("idm", pulsar_dm);
	
	if ((p = getenv ("PULSAR_RATE")) != NULL)
	{
		pulsar_rate = atof(p);
		fl_set_input (fd_receiver_main->pulsar_rate_input, p);
	}
	if ((p = getenv ("PULSAR_FOLDING")) != NULL)
	{
		pulsar_folding = atoi(p);
		fl_set_choice (fd_receiver_main->pulsar_choice, pulsar_folding/5);
	}
	/*
	 * Set spec_fft_size based on width of spectral plot display
	 */
   {
	   FL_Coord x, y, w, h;
	   
   		fl_get_object_bbox (fd_receiver_spectrum->spectral_plot, &x, &y, &w, &h);
   		spec_fft_size = w-130;
	}
	
	tp_maxval = 100000;
	tp_span = 20000;
	
	/*
	 * Establish parameters for TP plot
	 */
	if ((p = getenv("RCVR_TP_MAXVAL")) != NULL)
	{
		tp_maxval = (double)atoi(p);
	}
	if ((p = getenv ("RCVR_TP_SPAN")) != NULL)
	{
		tp_span = (double)atoi(p);
	}
	tp_minval = tp_maxval - tp_span;
	fl_set_slider_value (fd_receiver_continuum->tp_max_slider, (double)tp_maxval);
	fl_set_slider_value (fd_receiver_continuum->tp_span_slider, (double)tp_span);  
	fl_set_xyplot_ybounds(fd_receiver_continuum->tp_chart, (double)tp_minval, (double)tp_maxval);
   fl_set_xyplot_ytics(fd_receiver_continuum->tp_chart, 10, 1);
   fl_set_xyplot_xgrid (fd_receiver_continuum->tp_chart, FL_GRID_MINOR);
   fl_set_xyplot_ygrid (fd_receiver_continuum->tp_chart, FL_GRID_MINOR);
   fl_set_object_posthandler(fd_receiver_continuum->tp_chart, continuum_plot_post);
   
   /*
    * Set a post handler for inteferometer display
	*/
   if (strcmp (rcvr_mode, "interferometer") == 0)
   {
   		fl_set_object_posthandler(fd_receiver_interferometer->interferometer_chart, continuum_plot_post);
		if ((p = getenv ("RCVR_INT_GAIN")) != NULL)
		{
			interferometer_gain = atof(p);
		}
		if ((p = getenv ("RCVR_INT_SPAN")) != NULL)
		{
			interferometer_span = atof(p);
		}
		if ((p = getenv ("RCVR_PHCORR")) != NULL)
		{
			interferometer_phase = atof(p);
		}
		if ((p = getenv ("RCVR_DELAY")) != NULL)
		{
			interferometer_delay = atof(p);
		}
		fl_set_xyplot_ytics (fd_receiver_interferometer->interferometer_chart, 10, 1);
		fl_set_xyplot_xgrid (fd_receiver_interferometer->interferometer_chart, FL_GRID_MINOR);
		fl_set_xyplot_ygrid (fd_receiver_interferometer->interferometer_chart, FL_GRID_MINOR);
		fl_set_slider_value (fd_receiver_interferometer->int_gain_slider, interferometer_gain);
		fl_set_slider_value (fd_receiver_interferometer->int_span_slider, interferometer_span);
		fl_set_slider_value (fd_receiver_interferometer->phase_adjust, interferometer_phase);
		fl_set_slider_value (fd_receiver_interferometer->delay_adjust, interferometer_delay);
		fl_set_xyplot_ybounds (fd_receiver_interferometer->interferometer_chart, -1*interferometer_span,
			interferometer_span);
	}
   
   fl_add_timeout (1000.0, (FL_TIMEOUT_CALLBACK)lmst_update, 0);
   
   /*
    * Setup parameters for spectral plot
	*/
	if ((p = getenv ("RCVR_SPEC_MAX")) != NULL)
	{
		current_smax = atoi(p);
	}
	if ((p = getenv ("RCVR_SPEC_SPAN")) != NULL)
	{
		current_span = atoi(p);
	}
	if ((p = getenv ("RCVR_SPEC_FLAT")) != NULL)
	{
		spec_flat_on = atoi(p);
		fl_set_button (fd_receiver_spectrum->flaten_button, spec_flat_on);
	}
	if ((p = getenv ("RCVR_SPEC_METHOD")) != NULL)
	{
		spec_method = atoi (p);
	}
	fl_set_xyplot_xgrid(fd_receiver_spectrum->spectral_plot, FL_GRID_MINOR);
	fl_set_xyplot_ygrid(fd_receiver_spectrum->spectral_plot, FL_GRID_MINOR);
	fl_set_xyplot_ybounds(fd_receiver_spectrum->spectral_plot, (double)(current_smax-current_span), (double)
		current_smax);
	fl_set_xyplot_ytics(fd_receiver_spectrum->spectral_plot, 10, 1);
	fl_set_xyplot_xtics(fd_receiver_spectrum->spectral_plot, 10, 1);
	
	fl_set_object_posthandler(fd_receiver_spectrum->spectral_plot, spectral_plot_post);
	fl_set_choice (fd_receiver_spectrum->spec_method_choice, spec_method);
	fl_set_choice_fontsize (fd_receiver_spectrum->spec_method_choice, 14);
	
	fl_set_slider_value (fd_receiver_spectrum->spec_max_slider, (double)current_smax);
	fl_set_slider_value (fd_receiver_spectrum->spec_span_slider, (double)current_span);
	
	/*
	 * Set post handler for pulsar display
	 */
	fl_set_object_posthandler(fd_receiver_pulsar->pulsar_plot, pulsar_plot_post);
	
	/*
	 * Set parameters for waterfall (SETI) display
	 */
	{
		FL_Coord x, y;
		FL_Coord w, h;
		fl_get_object_bbox(fd_receiver_waterfall->waterfall_display, &x, &y, &w, &h);
		fl_set_slider_bounds (fd_receiver_waterfall->wfall_seg_slider, 1.0, (float)seti_size/w);
		fl_set_object_dblbuffer(fd_receiver_waterfall->waterfall_display, 1);
		
		if ((p = getenv ("RCVR_WFALL_SEGMENT")) != NULL)
		{
			waterfall_segment = atoi(p);
			fl_set_slider_value (fd_receiver_waterfall->wfall_seg_slider, (double)waterfall_segment);
		}
		if ((p = getenv ("RCVR_WFALL_FINE")) != NULL)
		{
			waterfall_fine = atoi(p);
			fl_set_slider_value (fd_receiver_waterfall->fine_segment, waterfall_fine);
		}
		if ((p = getenv ("RCVR_WFALL_BRIGHTNESS")) != NULL)
		{
			double w;
			
			w = atof(p);
			if (fabsf(w-1.0) < 0.1)
			{
				fl_set_choice (fd_receiver_waterfall->wfall_brightness, 1);
				w = 1.0;
			}
			if (fabsf(w-0.75) < 0.1)
			{
				fl_set_choice (fd_receiver_waterfall->wfall_brightness, 2);
				w = 0.75;
			}
			if (fabsf(w-0.66) < 0.1)
			{
				fl_set_choice (fd_receiver_waterfall->wfall_brightness, 3);
				w = 0.66;
			}
			if (fabsf(w-0.50) < 0.1)
			{
				fl_set_choice (fd_receiver_waterfall->wfall_brightness, 4);
				w = 0.50;
			}
			waterfall_brightness = (float)w;
		}
	}
	if ((p = getenv ("RCVR_TRANS_THRESH")) != NULL)
	{
		transient_threshold = atof(p);
		fl_set_slider_value (fd_receiver_main->trans_thr_slider, transient_threshold);
	}
	if ((p = getenv ("RCVR_TRANS_DUR")) != NULL)
	{
		transient_duration = atof(p);
		fl_set_slider_value (fd_receiver_main->trans_dur_slider, transient_duration);
	}
	/*
	 * Open various FIFOs--that's where we get our data from
	 */
	if ((seti_fd = open ("ra_seti_fifo", O_RDONLY|O_NONBLOCK)) > 0)
   {
	   fcntl (seti_fd, F_SETFL, 0);
	   fl_add_io_callback (seti_fd, FL_READ, (FL_IO_CALLBACK)handle_seti_io, fd_receiver_main);
   }
   if ((pulsar_fd = open ("ra_psr_fifo", O_RDONLY|O_NONBLOCK)) > 0)
   {
	    fcntl (pulsar_fd, F_SETFL, 0);
		fl_add_io_callback (pulsar_fd, FL_READ, (FL_IO_CALLBACK)handle_pulsar_io, fd_receiver_main);
   }
   if ((dicke_fd = open ("ra_switching_fifo", O_RDONLY|O_NONBLOCK)) > 0)
   {
	    fcntl (dicke_fd, F_SETFL, 0);
	    fl_set_object_label (fd_receiver_main->dicke_mode, "DICKE: ON");
		fl_add_io_callback (dicke_fd, FL_READ, (FL_IO_CALLBACK)handle_dicke_io, fd_receiver_main);
   }
   if (strcmp (rcvr_mode, "interferometer") == 0)
   {
	   if ((inter_fd = open ("ra_inter_fifo", O_RDONLY|O_NONBLOCK)) > 0)
	   {
		    fcntl (inter_fd, F_SETFL, 0);
	    	fl_add_io_callback (inter_fd, FL_READ, (FL_IO_CALLBACK)handle_inter_io, fd_receiver_main);
		}
	}
	if (strcmp (rcvr_mode, "split") == 0)
	{
		if ((validation_fd = open ("ra_validation_fifo", O_RDONLY|O_NONBLOCK)) > 0)
		{
			fcntl (validation_fd, F_SETFL, 0);
			split_mode = 1;
			fl_add_io_callback (validation_fd, FL_READ, (FL_IO_CALLBACK)handle_validation_io, fd_receiver_main);
		}
	}
	fl_set_oneliner_font (FL_FIXEDBOLDITALIC_STYLE, FL_MEDIUM_FONT);
	fl_set_oneliner_color (FL_GREEN, FL_BLACK);

   while(fl_do_forms())
         ;
   return 0;
}

int startup_cleared = 0;

/*
 * Called once per second to update LMST display
 *  and also to update parameters file
 */
int writeback_flag = 0;
int lm_cnt = 3;

void
lmst_update (FL_OBJECT *ob, long data)
{
	struct tm *ltp;
	time_t now;
	unsigned char sid[6];
	char lmst_buf[64];
	FILE *fp;
	int i, j;
	FL_Coord x, y;
	FL_Coord w, h;
	double high, low;
	char *p;
	int delta;
	int hours, mins, secs;
	
	fl_add_timeout (1000.0, (FL_TIMEOUT_CALLBACK)lmst_update, 0);
	
	/*
	 * Determine current LMST
	 */
	time (&now);
	ltp = gmtime (&now);
	get_sid_time (sid, longitude, ltp);
	memcpy (current_lmst, sid, 3);
	sprintf (lmst_buf, "%02d:%02d:%02d", sid[0], sid[1], sid[2]);
	fl_set_object_label(fd_receiver_main->lmst_display, lmst_buf);
	
	delta = now - started_at;
	hours = delta / 3600;
	mins = (delta - (hours * 3600)) / 60;
	secs = (delta - ((hours * 3600) + (mins*60)));
	sprintf (lmst_buf, "%02d:%02d:%02d", hours, mins, secs);
	fl_set_object_label (fd_receiver_main->runtime_text, lmst_buf);
	
	ra_mode[0] = 0x00;
	dec_mode[0] = 0x00;
	if ((p = getenv ("RCVR_RA_MODE")) != NULL)
	{
		strcpy (ra_mode, p);
		if (strcmp (p, "transit") == 0)
		{
			ra = (float)sid[0] + (float)sid[1]/60.0 + (float)sid[2]/3600.0;
			sprintf (lmst_buf, "%f", ra);
			fl_set_input (fd_receiver_main->ra_input, lmst_buf);
		}
		else if (strncmp (p, "external", 8) == 0)
		{
			char keyword[128], filename[128];
			
			sscanf (p, "%s %s", keyword, filename);
			if ((fp = fopen (filename, "r")) != NULL)
			{
				fscanf (fp, "%f", &ra);
				sprintf (lmst_buf, "%f", ra);
				fl_set_input (fd_receiver_main->ra_input, lmst_buf);
				fclose(fp);
			}
		}
	}
	if ((p = getenv ("RCVR_DEC_MODE")) != NULL)
	{
		strcpy (dec_mode, p);
		if (strncmp (p, "external", 8) == 0)
		{
			char keyword[128], filename[128];
			
			sscanf (p, "%s %s", keyword, filename);
			if ((fp = fopen (filename, "r")) != NULL)
			{
				fscanf (fp, "%f", &declination);
				sprintf (lmst_buf, "%f", declination);
				fl_set_input (fd_receiver_main->declination_input, lmst_buf);
				fclose(fp);
			}
		}
	}
	
	dump_params ();
	
	time (&now);
	if ((startup_cleared > 0) && (last_io_time > 1000) && ((now - last_io_time) > 30))
	{
		if (no_more_input == 0)
		{
			fprintf (stderr, "Leaving from %s\n", __FUNCTION__);
			receiver_leave (NULL, -10);
		}
	}
	if (startup_cleared > 0)
	{
		time (&now);
		
		/*
		 * We know that the python script has started and can now take
		 *     XMLRPC commands, because it's sending us data, so update
		 *     some variables, *once*.
		 */
		if (writeback_flag == 0)
		{
			void write_pushed_variables(void);
			
			writeback_flag = 1;
			
			/*
			 * Write (via XMLRPC) all our "pushed" variables
			 *   into the Gnu Radio flowgraph.
			 */
			write_pushed_variables();
			write_notches ();
		}
		
		/*
		 * Set the warning levels for I/O lag from Gnu Radio flowgraph
		 */
		if ((now - last_io_time) <= 1)
		{
			fl_set_object_color (fd_receiver_main->io_status, FL_GREEN, FL_GREEN);
		}
		else if ((now - last_io_time) < 8)
		{
			fl_set_object_color (fd_receiver_main->io_status, FL_YELLOW, FL_YELLOW);
		}
		else if ((now - last_io_time) >= 8)
		{
			fl_set_object_color (fd_receiver_main->io_status, FL_RED, FL_RED);
		}
	}
}

dump_params ()
{
	FILE *fp;
	int i;
	
	fp = fopen ("rcvr_param.pdump", "w");
	if (fp != NULL)
	{
		
	
		fprintf (fp, "export RCVR_DC_GAIN=%d\n", (int)dc_gain);
		fprintf (fp, "export RCVR_DC_OFFSET=%lf\n", dc_offset);
		fprintf (fp, "export RCVR_DC_MULT=%d\n", (int)dc_mult);
		fprintf (fp, "export RCVR_DECLINATION=%lf\n", declination);
		fprintf (fp, "export RCVR_RA=%f\n", ra);
		fprintf (fp, "export RCVR_INITIAL_FREQ=%lf\n", frequency);
		fprintf (fp, "export RCVR_REF_MULT=%lf\n", refmult);
		fprintf (fp, "export RCVR_RF_GAIN=%d\n", rf_gain);
		fprintf (fp, "export RCVR_SETI_INTEG=%d\n", (int)seti_integ);
		fprintf (fp, "export RCVR_SIGMA_K=%lf\n", sigma_k);
		fprintf (fp, "export RCVR_SPEC_INTEG=%d\n", spec_integration);
		fprintf (fp, "export RCVR_TP_INTEG=%d\n", tp_integration);
		fprintf (fp, "export RCVR_LONGITUDE=%lf\n", longitude);
		fprintf (fp, "export RCVR_COR_A=%lf\n", gc_a);
		fprintf (fp, "export RCVR_COR_B=%lf\n", gc_b);
		if (ra_mode[0] != 0x00)
		{
			fprintf (fp, "export RCVR_RA_MODE=\"%s\"\n", ra_mode);
		}
		if (dec_mode[0] != 0x00)
		{
			fprintf (fp, "export RCVR_DEC_MODE=\"%s\"\n", dec_mode);
		}
		fprintf (fp, "export RCVR_SPEC_SPAN=%d\n", current_span);
		fprintf (fp, "export RCVR_SPEC_MAX=%d\n", current_smax);
		fprintf (fp, "export RCVR_SPEC_FLAT=%d\n", spec_flat_on);
		fprintf (fp, "export RCVR_TP_MAXVAL=%d\n", (int)tp_maxval);
		fprintf (fp, "export RCVR_TP_SPAN=%d\n", (int)tp_span);
		fprintf (fp, "export RCVR_SPEC_METHOD=%d\n", spec_method);
		fprintf (fp, "export RCVR_INT_GAIN=%lf\n", interferometer_gain);
		fprintf (fp, "export RCVR_INT_SPAN=%lf\n", interferometer_span);
		fprintf (fp, "export RCVR_PHCORR=%lf\n", interferometer_phase);
		fprintf (fp, "export RCVR_PDELAY=%lf\n", interferometer_delay);
		fprintf (fp, "export RCVR_WFALL_SEGMENT=%d\n", waterfall_segment);
		fprintf (fp, "export RCVR_WFALL_FINE=%d\n", waterfall_fine);
		fprintf (fp, "export RCVR_WFALL_BRIGHTNESS=%lf\n", waterfall_brightness);
		if (sky_locked == 0)
		{
			fprintf (fp, "export RCVR_SKY_FREQ=%lf\n", sky_freq);
		}
		if (notches[0] > 0.0)
		{
			fprintf (fp, "export RCVR_NOTCHES=");
			for (i = 0; i < NNOTCHES; i++)
			{
				if (notches[i] < 0.0)
				{
					break;
				}
				fprintf (fp, "%f", notches[i]);
				if (notches[i+1] > 0.0)
				{
					fprintf (fp, ",");
				}
			}
			fprintf (fp, "\n");
		}
		fprintf (fp, "export RCVR_NOTCH_SIZE=%d\n", notch_length);
		fprintf (fp, "export RCVR_TRANS_THRESH=%lf\n", transient_threshold);
		fprintf (fp, "export RCVR_TRANS_DUR=%d\n", transient_duration);
		fprintf (fp, "export RCVR_DM=%f\n", pulsar_dm);
		fprintf (fp, "export PULSAR_RATE=%12.10lf\n", pulsar_rate);
		fprintf (fp, "export PULSAR_FOLDING=%d\n", pulsar_folding);
		fclose (fp);
	}
}

int
set_rf_gain (ga)
int ga;
{
	rf_gain = ga;
	write_rcvr_params("igain", rf_gain);
}

int
set_rf_freq (fq)
double fq;
{
	double delta;
	char d[128];
	
	delta = fq - frequency;
	if (sky_locked)
	{
		sky_freq = fq;
	}
	else
	{
		sky_freq = sky_freq + delta;
	}
	sprintf (d, "%f", sky_freq);
	fl_set_input (fd_receiver_main->sky_freq_input, d);
	frequency = fq;
	write_rcvr_params("ifreq", frequency);
	write_rcvr_params("skyfreq", sky_freq);
}


void
write_rcvr_params (char *pname, double parm)
{
	char sys_str[256];
	
	sprintf (sys_str, "python -c \"import xmlrpclib;xmlrpclib.Server('http://localhost:11420').set_%s(%.9le)\"", pname, parm);
	system (sys_str);
}

float *pulsar_inbuf = NULL;
int psr_buf_size = 0;
int
handle_pulsar_io ()
{
	int c;
	time_t now;
	
	if (no_more_input > 0)
	{
		fl_remove_io_callback(pulsar_fd, FL_READ,
                              (FL_IO_CALLBACK)handle_pulsar_io);
		return (0);
	}
	if (pulsar_inbuf == NULL)
	{
		psr_buf_size = 2 * psr_rate;
		pulsar_inbuf = (float *)malloc (psr_buf_size*sizeof(float));
	}

	c = read_floats (pulsar_fd, pulsar_inbuf, psr_rate);
	
	if ((c > 0) && startup_cleared == 0)
	{
		char txt[64];

		startup_cleared = 1;
		sprintf (txt, "%3.1fMsps %s", (float)((float)bandwidth/(float)1000000), rcvr_mode);
		if (strcmp (rcvr_mode, "single") == 0)
		{
			sprintf (txt+strlen(txt), "-%s", getenv("RCVR_SUBDEV"));
		}
		if (getenv("RCVR_CARD") != NULL)
		{
			sprintf (txt+strlen(txt), "(%s)", getenv("RCVR_CARD"));
		}
		fl_set_object_label (fd_receiver_main->startup_text, txt);
	}
	inject_pulsar (pulsar_inbuf, c, psr_rate);
	inject_tp (pulsar_inbuf, c, psr_rate, 0);
	inject_transients (pulsar_inbuf, c, psr_rate, 0);
}

float *validation_inbuf = NULL;
int validation_buf_size = 0;
handle_validation_io ()
{
	int c;
	
	if (validation_inbuf == NULL)
	{
		validation_buf_size = 8 * (psr_rate/2);
		validation_inbuf = (float *)malloc (psr_buf_size*sizeof(float));
	}
	c = read_floats (validation_fd, validation_inbuf, validation_buf_size/4);
	if (c > 0)
	{
		inject_transients (validation_inbuf, c, psr_rate, 1);
		inject_tp (validation_inbuf, c, psr_rate, 1);
	}
}

#define INTER_SIZE 40
float inter_buf[INTER_SIZE];

handle_inter_io ()
{
	int c;
	
	if (no_more_input > 0)
	{
		fl_remove_io_callback(inter_fd, FL_READ,
                              (FL_IO_CALLBACK)handle_inter_io);
		return (0);
	}
	c = read_floats (inter_fd, inter_buf, INTER_SIZE);
	inject_inter (inter_buf, c, 20);
}

handle_dicke_io ()
{
	unsigned char cmdbuf[128];
	int s;

	if (no_more_input > 0)
	{
		fl_remove_io_callback(dicke_fd, FL_READ,
                              (FL_IO_CALLBACK)handle_dicke_io);
		return (0);
	}
	if (read (dicke_fd, cmdbuf, 128) > 0)
	{
		sscanf (cmdbuf, "%d", &s);
	}
	change_dicke_state (s);
}

float *seti_buf = NULL;
float *seti_tbuf = NULL;

int cmap_allocated = 0;
#define NCOLORS 256
char *cmap[NCOLORS];
char ascii_map[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

char pixmapheader[128];
char *pixmapdata[NCOLORS*4];
float *plotptr = NULL;

extern int wfall_allocated;
extern short *wfall_rows[];
float spec_buf[2048];


#define SPEC_ANAL_SIZE 6000
float spec_anal_buf[SPEC_ANAL_SIZE];
int seti_c = 0;
unsigned long long int then, now;
handle_seti_io ()
{
	int c;
	int i, j;
	float v;
	struct timeval tv;
	
	if (no_more_input > 0)
	{
		fl_remove_io_callback(seti_fd, FL_READ,
                              (FL_IO_CALLBACK)handle_seti_io);
		return (0);
	}
	
	if (seti_buf == NULL)
	{
		seti_buf = (float *)malloc (seti_size*sizeof(float));
		seti_tbuf = (float *)malloc ((seti_size/2)*sizeof(float));
	}

	seti_c += read_floats (seti_fd, seti_buf+seti_c, seti_size-seti_c);
	
	if (seti_c >= seti_size)
	{
		float a;
		
		if (seti_c > seti_size)
		{
			fprintf (stderr, "seti_c is larger than needed! %d\n", seti_c);
		}
		
		
		/*
		 * Undo the trippy ordering that FFTW uses
		 */
		memcpy (seti_tbuf, seti_buf, (seti_size/2)*sizeof(float));
		memcpy (seti_buf, seti_buf+(seti_size/2), (seti_size/2)*sizeof(float));
		memcpy (seti_buf+(seti_size/2), seti_tbuf, (seti_size/2)*sizeof(float));
		
		if (strcmp (rcvr_mode, "interferometer") == 0)
		{
			for (i = 0; i < seti_size; i++)
			{
				seti_buf[i] /= 1.e6;
			}
		}
		
		seti_c = 0;
		
		/*
		 * First compress for display
		 */
		for (i = 0; i < spec_fft_size; i++)
		{
			float v;
			float v2;
			float min;
			float max;
			
			/*
			 * Compute average over this "clump"
			 */
			v = 0.0;
			min = 99.0e17;
			max = -99.0e17;
			for (j = 0; j < (seti_size/spec_fft_size); j++)
			{
				v2 = seti_buf[(i*(seti_size/spec_fft_size))+j];
				v += v2;
				if (v2 > max)
				{
					max = v2;
				}
				if (v2 < min)
				{
					min = v2;
				}
			}
			if (isnan(v) || isinf(v))
			{
				fprintf (stderr, "1 v is %f at %d\n", v, i);
			}
			/*
			 * Scale into log scale, and adjust
			 *
			 */
			if (spec_method == 0 || spec_method == 2)
			{
				v /= (seti_size/spec_fft_size);
			}
			else if (spec_method == 1)
			{
				v = max;
			}
			else if (spec_method == 3)
			{
				v = min + (max-min/2);
			}

			v = 20.0*log10f(v);
			v -= 20.0*log10f((double)seti_size/spec_fft_size);
			if (isnan(v) || isinf(v))
			{
				fprintf (stderr, "2 v is %f at %d\n", v, i);
				fprintf (stderr, "setisize is %d spec_fft_size is %d\n",
					seti_size, spec_fft_size);
			}
			spec_buf[i] = v;
		}
		
		/*
		 * Compress for non-seti analysis
		 */

		for (i = 0; i < SPEC_ANAL_SIZE; i++)
		{
			
			/*
			 * Compute average over all bins in this "clump"
			 */
			v = 0;
			for (j = 0; j < (seti_size/SPEC_ANAL_SIZE); j++)
			{
				v += seti_buf[(i*(seti_size/SPEC_ANAL_SIZE))+j];
			}
			v /= (seti_size/SPEC_ANAL_SIZE);
			
			/*
			 * Insert into analysis buf, along with integration
			 */
			spec_anal_buf[i] = v;
		}
		
		/*
		 * Send it off to the spectral display
		 */
		inject_spec (spec_buf, spec_fft_size, spec_fft_size, spec_anal_buf, SPEC_ANAL_SIZE);
		
		gettimeofday (&tv, NULL);
		now = (tv.tv_sec * (1000*1000)) + tv.tv_usec;
		if ((now - then) >= (1000*1000))
		{
			then = now;
			seti_process(seti_buf, seti_size);
		}
	}
}

#include "colormap.h"

float last_brightness = 0;
char *wc_pixmap[1+NCOLORS+128];
char wc_header[128];
wfall_plotit (buf, len)
float *buf;
int len;
{
	int i, j, k;
	FL_Coord x, y, w, h;
	char lbuf[128];
	char *tp;
	double low, mid, high;
	int v;
	double binwidth;
	
	binwidth = (double)bandwidth / (double)seti_size;
		
	/*
	 * Handle update of waterfall Pixmap
	 */
	if (cmap_allocated == 0)
	{
		int pa, pb;
		int r, g, b;
		int rgb;
		
		pa = pb  = 0;
		cmap_allocated = 1;
		b = 128;
		r = g = 0;
		for (i = 0; i < NCOLORS; i++)
		{
			cmap[i] = malloc(64);	
			rgb = 0;
			r = red[i];
			g = green[i];
			b = blue[i];
			rgb = r<<16 | g << 8 | b;
			sprintf (cmap[i], "%c%c c #%06X ", ascii_map[pb], ascii_map[pa], rgb);
#ifdef ORIG_CMAP
			/*
			 * Increment rule for Red
			 */
			if (i > (int)(0.425*256.0) && i <= (int)(0.625*256.0))
			{
				r += 3;
			}
			if (i > (int)(0.625 * 256.0) && i <= (int)(0.8125*256.0))
			{
				r = 255;
			}
			if (i > (int)(0.8125*256.0))
			{
				r -= 3;
			}
			if (r < 0)
			{
				r = 0;
			}
			
			/*
			 * Incremenet rule for Green
			 */
			if (i > (int)(0.125 * 256.0) && i <= (int)(0.4125*256.0))
			{
				g += 3;
			}
			if (i > (int)(0.4125*256.0) && i <= (int)(0.625*256.0))
			{
				g = 255;
			}
			if (i > (int)(0.625 * 256.0))
			{
				g -= 3;
			}
			if (g < 0)
			{
				g = 0;
			}
			
			/*
			 * Increment rule for Blue
			 */
			b += 2;
			if (i > (int)(0.125 * 256.0) && i <= (int)(0.4125 * 256.0))
			{
				b = 255;
			}
			if (i > (int)(0.4125 * 256.0))
			{
				b -= 2;
			}
			if (b < 0)
			{
				b = 0;
			}
#endif
			pb ++;
			if (pb >= strlen(ascii_map))
			{
				pa++;
				pb = 0;
			}
		}

		fl_get_object_bbox(fd_receiver_waterfall->waterfall_display, &x, &y, &w, &h);
		pixmapdata[0] = pixmapheader;
		sprintf (pixmapdata[0], "%d %d 256 2 ", w, h);
		for (i = 0; i < NCOLORS; i++)
		{
			pixmapdata[i+1] = cmap[i];
		}
		for (i = 1+NCOLORS; i < (1+NCOLORS+h); i++)
		{
			pixmapdata[i] = (char *)malloc ((w*2)+1);
			memset (pixmapdata[i], 0x00, (w*2)+1);
			for (j = 0; j < w; j++)
			{
				strncpy(pixmapdata[i]+(j*2), cmap[0], 2);
			}
		}
		
		fl_get_object_bbox(fd_receiver_waterfall->colormap_pixmap, &x, &y, &w, &h);
		wc_pixmap[0] = wc_header;
		sprintf (wc_header, "%d %d 256 2 ", w, h);
		for (i = 0; i < NCOLORS; i++)
		{
			wc_pixmap[i+1] = cmap[i];
		}
		for (i = 0; i < h; i++)
		{
			wc_pixmap[i+NCOLORS+1] = malloc ((2*w)+1);
			for (j = 0; j < w; j++)
			{
				int t;
				
				t = (NCOLORS/w)*j;
				if (t > 255)
				{
					t = 255;
				}
				strncpy (wc_pixmap[i+NCOLORS+1]+(j*2), cmap[t], 2);
			}
		}
		fl_free_pixmap_pixmap (fd_receiver_waterfall->colormap_pixmap);
		fl_set_pixmap_data (fd_receiver_waterfall->colormap_pixmap, wc_pixmap);
	}
	
	/*
	 * Get chart pixmap geometry
	 */
	fl_get_object_bbox(fd_receiver_waterfall->waterfall_display, &x, &y, &w, &h);	
	
	/*
	 * Shift pixmap buffer
	 */
	tp = pixmapdata[NCOLORS+1+(h-1)];
	for (j = NCOLORS+1+(h-1); j > NCOLORS+1; j--)
	{
		pixmapdata[j] = pixmapdata[j-1];
	}
	pixmapdata[NCOLORS+1] = tp;
	
	/*
	 * Update line zero of the pixmap buffer
	 */
	memset (pixmapdata[NCOLORS+1], 0x00, (w*2)+1);
	v = 0;
	for (i = 0; i < w; i++)
	{
		char *p;

		v = buf[i];
		if (v < 0)
		{
			v = 0;
		}
		if (v >= 255)
		{
			v = 255;
		}
		p = pixmapdata[1+NCOLORS];
		strncpy (p+(i*2), cmap[v], 2);
	}
	pixmapdata[1+NCOLORS+h] = " ";
	
	if (squash_mode == 0)
	{
		low = (double)sky_freq-((double)bandwidth/2);
		low += (double)((waterfall_segment-1)*(len*binwidth));
		low += (waterfall_fine);
		if (low < sky_freq-(bandwidth/2))
		{
			low = sky_freq-(bandwidth/2);
		}
		if (low > sky_freq+(bandwidth/2))
		{
			low = sky_freq+(bandwidth/2);
		}
		mid = low + (double)(len/2)*binwidth;
		if (mid > sky_freq+(bandwidth/2))
		{
			mid = sky_freq+(bandwidth/2);
		}
		
		high = low + (double)(len*binwidth);
		if (high > frequency+(bandwidth/2))
		{
			high = sky_freq+(bandwidth/2);
		}
	}
	else
	{
		low = sky_freq - (bandwidth/2);
		mid = sky_freq;
		high = sky_freq + (bandwidth/2);
	}
		
	
	sprintf (lbuf, "%11.6fMHz", low/1.0e6);
	fl_set_object_label(fd_receiver_waterfall->freq_low, lbuf);
	
	sprintf (lbuf, "%11.6fMHz", mid/1.0e6);
	fl_set_object_label(fd_receiver_waterfall->freq_mid, lbuf);
	
	sprintf (lbuf, "%11.6fMHz", high/1.0e6);
	fl_set_object_label(fd_receiver_waterfall->freq_high, lbuf);
	
	/*
	 * Free old pixmap, set new one
	 */
	fl_free_pixmap_pixmap(fd_receiver_waterfall->waterfall_display);
	fl_set_pixmap_data(fd_receiver_waterfall->waterfall_display, pixmapdata);
	
	if (last_brightness != waterfall_brightness)
	{
		last_brightness = waterfall_brightness;
		update_wfall_colormap ();
	}
}

update_wfall_colormap ()
{
	int maxc;
	int i, j;
	FL_Coord x, y, w, h;
	
	fl_get_object_bbox (fd_receiver_waterfall->colormap_pixmap, &x, &y, &w, &h);
	
	maxc = (int)(255.0 * waterfall_brightness);
    if (maxc > w)
	{
		for (i = 0; i < h; i++)
		{
			for (j = 0; j < w; j++)
			{
				int t;
				
				t = (maxc/w)*j;
				if (t > maxc)
				{
					t = maxc;
				}
				strncpy (wc_pixmap[i+NCOLORS+1]+(j*2), cmap[t], 2);
			}
		}
	}
	else
	{
		for (i = 0; i < h; i++)
		{
			float t;
			
			t = 0.0;
			for (j = 0; j < w; j++)
			{
				t += ((float)maxc / (float)w);
				if (t > maxc)
				{
					t = (float)maxc;
				}
				strncpy (wc_pixmap[i+NCOLORS+1]+(j*2), cmap[(int)t], 2);
			}
		}
	}
	
	fl_free_pixmap_pixmap (fd_receiver_waterfall->colormap_pixmap);
	fl_set_pixmap_data (fd_receiver_waterfall->colormap_pixmap, wc_pixmap);
}

wfall_export (fp)
FILE *fp;
{
	FL_Coord x, y, w, h;
	int i;
	char *p;
	
	fl_get_object_bbox (fd_receiver_waterfall->waterfall_display, &x, &y, &w, &h);
	fprintf (fp, "#define wfall_format 1\n");
	fprintf (fp, "#define wfall_width %d\n", w);
	fprintf (fp, "#define wfall_height %d\n", h);
	fprintf (fp, "#define wfall_ncolors 256\n");
	fprintf (fp, "#define wfall_chars_per_pixel 2\n");
	fprintf (fp, "static char *wfall_colors[] = {\n");
	for (i = 0; i < NCOLORS; i++)
	{
		char csym[5], cval[16];
		char copy[128];
		
		strcpy (copy, pixmapdata[i+1]);
		
		p = strtok (copy, " ");
		strcpy (csym, p);
		strtok (NULL, " ");
		p = strtok (NULL, " ");
		strcpy (cval, p);
		fprintf (fp, "    \"%s\", \"%s\",\n", csym, cval);
	}
	fprintf (fp, "};\n");
	fprintf (fp, "static char *wfall_pixels[] = {\n");
	for (i = 0; i < h; i++)
	{
		fprintf (fp, "   \"%s\",\n", pixmapdata[i+1+NCOLORS]);
	}
	fprintf (fp, "};\n");
	fclose (fp);
}

int
read_floats (fd, buf, size)
int fd;
float *buf;
int size;
{
	int accum;
	unsigned char *cp;
	int rr;
	int flag;
	
	flag = 0;
	if (size < 0)
	{
		size *= -1;
		flag = 1;
	}
	
	accum = 0;
	cp = (unsigned char *)buf;
	while ((rr = read (fd, cp, (size*sizeof(float))-accum)) > 0)
	{
		accum += rr;
		
		time (&last_io_time);
		
		if ((flag == 0) && (accum % sizeof(float)) == 0)
		{
			return (accum / sizeof(float));
		}
		
		cp += rr;
		if (accum >= (size*sizeof(float)))
		{
			break;
		}
	}
	if (rr <= 0)
	{
		fprintf (stderr, "Leaving from: %s\n", __FUNCTION__);
		receiver_leave(NULL, -9);
	}
	return (accum/sizeof(float));
}


int 
pulsar_plot_post(FL_OBJECT *ob, int ev,
               FL_Coord mx, FL_Coord my, int key, void *xev)
{
    if(ev == FL_PUSH || ev == FL_MOUSE)
    {
        float wx, wy;
        char buf[64];
		float ratio;
		float dfreq;
		float vs;
		
		fl_xyplot_s2w(ob, mx, my, &wx, &wy);	
        sprintf(buf,"%7.3fsec", wx);
        fl_show_oneliner(buf, ob->x + ob->form->x + 5, ob->y + ob->form->y);
        ob->wantkey = FL_KEY_ALL;
        ob->input = 1;
    }
    else if(ev == FL_RELEASE)
    {
        fl_hide_oneliner();
    }
    return 0;
}

int 
continuum_plot_post(FL_OBJECT *ob, int ev,
               FL_Coord mx, FL_Coord my, int key, void *xev)
{
    if(ev == FL_PUSH || ev == FL_MOUSE)
    {
        float wx, wy;
        char buf[64];
		float ratio;
		float dfreq;
		float vs;
		
		fl_xyplot_s2w(ob, mx, my, &wx, &wy);	
        sprintf(buf,"%7.3fs %8.2f", wx, wy);
        fl_show_oneliner(buf, ob->x + ob->form->x + 5, ob->y + ob->form->y);
        ob->wantkey = FL_KEY_ALL;
        ob->input = 1;
    }
    else if(ev == FL_RELEASE)
    {
        fl_hide_oneliner();
    }
    return 0;
}

int oneliner_up = 0;
int
spectral_plot_post(FL_OBJECT *ob, int ev,
               FL_Coord mx, FL_Coord my, int key, void *xev)
{
    if(ev == FL_PUSH || ev == FL_MOUSE)
    {
        float wx, wy;
        char buf[64];
		float ratio;
		float dfreq;
		float vs;

		fl_xyplot_s2w(ob, mx, my, &wx, &wy);

		dfreq = wx*1.0e6;
		ratio = sky_freq / dfreq;
        vs = 1.0 - ratio;
        vs *= 299792.0;
		
		if (key == 1)
		{
			sprintf(buf,"%10.5fMHz(%4.2fKm/sec) %5.2fdB", wx, vs, wy, key);
			fl_show_oneliner(buf, ob->x + ob->form->x + 5, ob->y + ob->form->y);
			ob->wantkey = FL_KEY_ALL;
			ob->input = 1;
			oneliner_up = 1;
		}
		else if (ev == FL_PUSH && key != 1)
		{
			ob->wantkey = FL_KEY_ALL;
			ob->input = 1;
			add_notch(wx*1.0e6, spec_fft_size, sky_freq);
		}
    }
    else if (ev == FL_RELEASE)
    {
		if (oneliner_up)
		{
        	fl_hide_oneliner();
			oneliner_up = 0;
		}
    }
    return 0;
}

int tp_out_cnt = 0;

log_data (direct, prefix, data)
char *direct;
char *prefix;
char *data;
{
	char filename[256];
	time_t now;
	struct tm *ltp;
	FILE *fp;
	
	time (&now);
	ltp = localtime (&now);
	sprintf (filename, "%s/%s-%04d%02d%02d%02d.dat", direct, prefix,
		ltp->tm_year+1900, ltp->tm_mon+1, ltp->tm_mday, ltp->tm_hour);
		
	fp = fopen (filename, "a");
	if (fp != NULL)
	{
		if ((strcmp (prefix, "tp") == 0) || (strcmp (prefix, "tp2") == 0))
		{
			if (tp_out_cnt >= 15)
			{
				fprintf (fp, "%02d:%02d:%02d %s Dn=%f,Ra=%f,Ti=%d,Fc=%f,Bw=%f,Ga=%d\n", current_lmst[0], current_lmst[1], current_lmst[2],
					data, declination, ra, tp_integration, sky_freq/1.0e6,(float)bandwidth/1.0e6, rf_gain);
					tp_out_cnt = 0;
			}
			else
			{
				fprintf (fp, "%02d:%02d:%02d %s\n", current_lmst[0], current_lmst[1], current_lmst[2], data);
			}
			tp_out_cnt++;
		}
		else if (strcmp (prefix, "spec") == 0)
		{
			fprintf (fp, "data:%02d:%02d:%02d Dn=%f,Ra=%f,Fc=%f,Bw=%f,Av=%d [ ",
				current_lmst[0], current_lmst[1], current_lmst[2],
				declination, ra, sky_freq/1.0e6, (float)bandwidth/1.0e6, spec_integration);
			fputs (data, fp);
			fprintf (fp, " ]\n");
		}
		else
		{
			fprintf (fp, "%02d:%02d:%02d %f %f %f %f %s\n", current_lmst[0], current_lmst[1], current_lmst[2],
				declination, ra, sky_freq/1.0e6, bandwidth/1.0e6, data);
		}
			
		fflush (fp);
		fclose (fp);
	}
}


int spec_flattened = 0;
float *spec_flatbuf = NULL;
int spec_flatten_cnt = 0;


spec_flatten (buf,len)
float *buf;
int len;
{
	int incr;
	int i, j;
	float mvavg;
	float a;
	float fst;
	float flatmax = 0;
	float flatmin = 99.9e9;
	int flatmaxloc = 0;
	int fcnt;

	/*
	 * Allocate buffer, if needed
	 */
	if (spec_flatbuf == NULL)
	{
		spec_flatbuf = (float *)malloc(len*sizeof(float));
	}
	
	/*
	 * If we haven't computed flatten function recently, do it now
	 */
	if (spec_flattened == 0)
	{
		mvavg = 0.0;
		fcnt = 0;
		
		/*
		 * Go through entire input buffer, checking for bogons, and computing
		 *   and average to stuff into littlebuf
		 */
		a = 0.3;
		mvavg = buf[0];
		for (i = 0; i < len; i++)
		{
			mvavg  = (a * buf[i]) + ((1.0 - a) * mvavg);
			spec_flatbuf[i] = mvavg;
		}
		
		for (i = 0; i < len; i++)
		{
			if (spec_flatbuf[i] == (1.0/0.0))
			{
				fprintf (stderr, "1: flatbuf %d contains %f\n", i, spec_flatbuf[i]);
				exit (1);
			}
			if (spec_flatbuf[i] > flatmax)
			{
				flatmax = spec_flatbuf[i];
			}
		}
		
		/*
		 * Invert the function
		 */
		for (i = 0; i < len; i++)
		{
			spec_flatbuf[i] = flatmax/spec_flatbuf[i];
			if (spec_flatbuf[i] == (1.0/0.0))
			{
				fprintf (stderr, "2: flatbuf %d contains %f\n", i, spec_flatbuf[i]);
				exit (1);
			}
		}
		
		/*
		 * Smooth it again
		 */
		a = 0.1;
		mvavg = spec_flatbuf[0];
		for (i = 0; i < len; i++)
		{
			mvavg = (a * spec_flatbuf[i]) + ((1.0 - a) * mvavg);
			spec_flatbuf[i] = mvavg;
			if (spec_flatbuf[i] == (1.0/0.0))
			{
				fprintf (stderr, "1:flatbuf %d contains %f\n", i, spec_flatbuf[i]);
				exit (1);
			}
		}
		spec_flattened = 1;

	}
	
	/*
	 * If time to force recompute of flat on next invocation?
	 */
	if (spec_flatten_cnt++ >= 10)
	{
		spec_flattened = 0;
		spec_flatten_cnt = 0;
	}
	
	/*
	 * Multiply buf by flatbuf
	 *
	 */
	for (i = 0; i < len; i++)
	{
		/*
		 * Flatten
		 */
		buf[i] *= spec_flatbuf[i];
		
		if (spec_flatbuf[i] == (1.0/0.0))
		{
			fprintf (stderr, "2:flatbuf %d contains %f\n", i, spec_flatbuf[i]);
			exit (1);
		}
	}
}

int
close_sub (FL_FORM *f, void (*p)())
{
	(*p)(NULL, 0);
	return (FL_IGNORE);
}

int
close_main (FL_FORM *f, void *p)
{
	fprintf (stderr, "Leaving from: %s\n", __FUNCTION__);
	receiver_leave (NULL, 0);
	return (FL_IGNORE);
}

add_notch (freq, fft_size, sky)
double freq;
int fft_size;
double sky;
{
	int i, j;
	double binwidth;
	double fst;
	double tmpnotches[NNOTCHES];
	int already = 0;
	int nsz;
	char *p;
	
	nsz = 64;
	if ((p = getenv("RCVR_NOTCH_SIZE")) != NULL)
	{
		nsz = atoi(p);
	}
	binwidth = bandwidth / nsz;
	fst = sky - (bandwidth/2);
	
	already = 0;
	
	for (i = 0; i < NNOTCHES; i++)
	{
		tmpnotches[i] = -1.0;
	}
	
	/*
	 * First, see if we already have this in the list, in which case, remove it
	 */
	memcpy (tmpnotches, notches, sizeof(tmpnotches));
	for (i = 0; i < NNOTCHES; i++)
	{
		if (fabs(tmpnotches[i]-freq) < binwidth)
		{
			tmpnotches[i] = -1.0;
			already = 1;
			break;
		}
	}
	if (!already)
	{
		for (i = 0; i < NNOTCHES; i++)
		{
			if (tmpnotches[i] < 0.0)
			{
				tmpnotches[i] = freq;
				break;
			}
		}
	}
	j = 0;
	for (i = 0; i < NNOTCHES; i++)
	{
		notches[i] = -1.0;
	}
	for (i = 0; i < NNOTCHES; i++)
	{
		if (tmpnotches[i] > 0)
		{
			notches[j] = tmpnotches[i];
			j++;
		}
	}
	notches[j] = -1.0;
	write_notches ();
}

int notch_ser = 0;

write_notches ()
{
	char sys_str[4096];
	char notch_str[4096], *notchp;
	int i;
	
	notchp = notch_str;
	
	for (i = 0 ; i < NNOTCHES; i++)
	{
		sprintf (notchp, "%f", notches[i]);
		notchp += strlen(notchp);
		if (notches[i] > 0.0)
		{
			sprintf (notchp, ",");
			notchp += strlen(notchp);
		}
		else
		{
			break;
		}
	}
	
	sprintf (sys_str, "python -c \"import xmlrpclib; xmlrpclib.Server('http://localhost:11420').set_notch_freqs([%s])\"",
		notch_str);
	
	system (sys_str);
	
	sprintf (sys_str, "python -c \"import xmlrpclib; xmlrpclib.Server('http://localhost:11420').set_notch_len(%d)\"",
		notch_length);
	
	system(sys_str);
}

void
write_pushed_variables()
{
	int i;
	
	for (i = 0; i < pushed_next; i++)
	{
		if (pushed_list[i].inuse)
		{
			write_rcvr_params(pushed_list[i].name, pushed_list[i].value);
		}
	}
}
