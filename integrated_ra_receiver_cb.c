#include <forms.h>
#include "integrated_ra_receiver.h"
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include "rcvr_globals.h"
#include <ctype.h>

/*** callbacks and freeobj handles for form seti_fft ***/
void adjust_frequency(FL_OBJECT *ob, long data)
{
  char tbuf[50];
  char *p;
  double lo, hi;
  
  lo = 50.0e6;
  hi = 870.0e6;
  
  if (getenv ("F_LOW") != NULL)
  {
	  lo = atof(getenv("F_LOW"));
	  hi = atof(getenv("F_HIGH"));
  }
  
  strcpy (tbuf, fl_get_input(ob));
  p = strchr(tbuf, '\n');
  if (p != NULL)
  {
	  *p = '\0';
  }
  if (check_float(tbuf) < 0)
  {
	  fl_set_input (ob, "Format Error");
  }
  else if (atof(tbuf) < lo || atof(tbuf) > hi)
  {
	  fl_set_input (ob, "Out of range");
  }
  else
  {
	set_rf_freq (atof(tbuf));
	}
}

void adjust_continuum_int(FL_OBJECT *ob, long data)
{
  tp_integration = fl_get_slider_value(ob);
}

void adjust_spec_int(FL_OBJECT *ob, long data)
{
	spec_integration = fl_get_slider_value(ob);
}

void adjust_dc_gain(FL_OBJECT *ob, long data)
{
	dc_gain = fl_get_slider_value(ob);
	detector_peak = 0.0;
}

void adjust_dc_offset(FL_OBJECT *ob, long data)
{
	dc_offset = fl_get_slider_value(ob);
	detector_peak = 0.0;
}

void adjust_declination(FL_OBJECT *ob, long data)
{
	char tbuf[50];
    char *p;
  
	strcpy (tbuf, fl_get_input(ob));
	p = strchr(tbuf, '\n');
	if (p != NULL)
	{
		*p = '\0';
	}
	if (check_float (tbuf) < 0)
	{
		fl_set_input (ob, "Format Error");
	}
	else
	{
		declination = atof(tbuf);
	}
}


void adjust_ra(FL_OBJECT *ob, long data)
{
	char tbuf[50];
    char *p;
  
	strcpy (tbuf, fl_get_input(ob));
	p = strchr(tbuf, '\n');
	if (p != NULL)
	{
		*p = '\0';
	}
	if (check_float (tbuf) < 0)
	{
		fl_set_input (ob, "Format Error");
	}
	else
	{
		ra = atof(tbuf);
	}
}

void adjust_pulsar_rate(FL_OBJECT *ob, long data)
{
	char tbuf[50];
  char *p;
  double real_rate;
  int int_rate;
  double err;
  char str[64];
  
  strcpy (tbuf, fl_get_input(ob));
  p = strchr(tbuf, '\n');
  if (p != NULL)
  {
	  *p = '\0';
  }
  if ((p = strchr(tbuf, '\r')) != NULL)
  {
	  *p = '\0';
	}
	if (check_float (tbuf) < 0)
	{
		fl_set_input (ob, "Format Error");
	}
	else
	{
		pulsar_rate = (double)atof(tbuf);
		
		real_rate = (1.0/pulsar_rate)*(double)psr_rate;
		int_rate = (int)(real_rate);
		err = fabsf(real_rate - int_rate);
		err = err / real_rate;
		sprintf (str, "%8.2f", err*1.0e6);
		fl_set_object_label (fd_receiver_main->phase_display, str);
	}

}

void adjust_waterfall_segment(FL_OBJECT *ob, long data)
{
	waterfall_segment = (int) fl_get_slider_value (ob);
}
void adjust_fine_segment (FL_OBJECT *ob, long data)
{
	waterfall_fine = (int) fl_get_slider_value (ob);
}
void adjust_tp_maxval (FL_OBJECT *ob, long data)
{
	tp_maxval = fl_get_slider_value(ob);
	tp_minval = tp_maxval - tp_span;
	fl_set_xyplot_ybounds (fd_receiver_continuum->tp_chart, tp_minval, tp_maxval);
}

void adjust_tp_span (FL_OBJECT *ob, long data)
{
	tp_span = fl_get_slider_value (ob);
	tp_minval = tp_maxval - tp_span;
	fl_set_xyplot_ybounds (fd_receiver_continuum->tp_chart, tp_minval, tp_maxval);
}
void adjust_waterfall_int(FL_OBJECT *ob, long data)
{
	waterfall_integration = (int)fl_get_slider_value (ob);
}

void
adjust_sigma_k (FL_OBJECT *ob, long data)
{
	sigma_k = fl_get_slider_value (ob);
}

void
adjust_dm (FL_OBJECT *ob, long data)
{
	double f1, f2;
	char buf[32];
	double K;
	
	K = 1.0/(2.410331 * 10e-4);
	
	f1 = frequency-(bandwidth/2);
	f2 = frequency+(bandwidth/2);
	f1 /= 1.0e6;
	f2 /= 1.0e6;
	pulsar_dm = fl_get_slider_value (ob);
	pulsar_delay = (1.0/(f1*f1)) - (1.0/(f2*f2));
	pulsar_delay *= pulsar_dm ;
	pulsar_delay *= K;
	sprintf (buf, "%9.4f", pulsar_delay * 1000);
	fl_set_object_label (fd_receiver_main->pulsar_delay_display, buf);
	write_rcvr_params ("idm", pulsar_dm);
}

int pulsar_displayed = 0;
void
pulsar_display (FL_OBJECT *ob, long data)
{
	if (pulsar_displayed == 0)
	{
		fl_show_form(fd_receiver_pulsar->receiver_pulsar,FL_PLACE_CENTER,FL_FULLBORDER,"IRA Pulsar");
		pulsar_displayed = 1;
	}
	else
	{
		fl_raise_form(fd_receiver_pulsar->receiver_pulsar);
	}
}

void
dismiss_psr (FL_OBJECT *ob, long data)
{
	if (pulsar_displayed != 0)
	{
		fl_hide_form (fd_receiver_pulsar->receiver_pulsar);
		pulsar_displayed = 0;
	}
}

void
adjust_rf_gain (FL_OBJECT *ob, long data)
{
	double val;
	
	val = fl_get_slider_value (ob);
	if ((val - (int) val)  >= 0.5)
	{
		val += 1.0;
	}
	rf_gain = (int)val;
	write_rcvr_params("igain", rf_gain);
	detector_peak = 0.0;
}

int spectrum_displayed = 0;

void
spectrum_display (FL_OBJECT *ob, long data)
{
	if (spectrum_displayed == 0)
	{
		fl_show_form(fd_receiver_spectrum->receiver_spectrum, FL_PLACE_CENTER,FL_FULLBORDER,"IRA Spectrum");
		spectrum_displayed = 1;
	}
	else
	{
		fl_raise_form(fd_receiver_spectrum->receiver_spectrum);
	}
}

void
spectrum_hide (FL_OBJECT *ob, long data)
{
	if (spectrum_displayed == 1)
	{
		fl_hide_form(fd_receiver_spectrum->receiver_spectrum);
		spectrum_displayed = 0;
	}
}

void
adjust_spectral_max (FL_OBJECT *ob, long data)
{
	current_smax = (int)fl_get_slider_value(ob);
	fl_set_xyplot_ybounds(fd_receiver_spectrum->spectral_plot, (double)current_smax-current_span, (double)current_smax);
}

void
adjust_spectral_span(FL_OBJECT *ob, long data)
{
	current_span = (int)fl_get_slider_value(ob);
	fl_set_xyplot_ybounds(fd_receiver_spectrum->spectral_plot, (double)current_smax-current_span, (double)current_smax);
}

void
adjust_tp_scale (FL_OBJECT *ob, long data)
{
	tp_time_scale = (int)fl_get_choice (ob);
}

int before = 0;
void
receiver_leave (FL_OBJECT *ob, long data)
{
	FILE *fp;
	int pid;
	

	rcvr_runrun = 0;
	if (data > -1  && (fp = fopen ("receiver.pid", "r")) != NULL)
	{
		fl_hide_form (fd_receiver_main->receiver_main);
		hide_info(NULL, 0);
		hide_interferometer (NULL, 0);
		hide_waterfall (NULL, 0);
		tp_hide (NULL, 0);
		spectrum_hide (NULL, 0);
		dismiss_psr (NULL, 0);
		
		fl_show_form (fd_receiver_shutdown->receiver_shutdown, FL_PLACE_CENTER, FL_NOBORDER, "IRA Shutdown");
		fl_check_forms ();
		
		fscanf (fp, "%d", &pid);
		kill (pid, SIGINT);
		sleep (1);
		kill (pid, SIGHUP);
		sleep (1);
		kill (pid, SIGKILL);
		fclose (fp);
	}
	if (data < -1)
	{
		char string[129];
		
		if (before)
		{
			fprintf (stderr, "Leaving from: %s\n", __FUNCTION__);
			receiver_leave (ob, 0);
		}
		before++;
		
		fp = fopen ("receiver_py.err", "r");
		fl_set_browser_fontsize(fd_receiver_error->error_browser, 14);
		fl_set_browser_fontstyle(fd_receiver_error->error_browser, FL_BOLD_STYLE);
		if (fp != NULL)
		{
			
			while (fgets (string, 128, fp) != NULL)
			{
				string[strlen(string)-1] = '\0'; 
				fl_add_browser_line (fd_receiver_error->error_browser, string);
			}
			fclose (fp);
			if ((fp = fopen ("receiver_py.log", "r"))!= NULL)
			{
				while (fgets (string, 128, fp) != NULL)
				{
					string[strlen(string)-1] = '\0';
					fl_add_browser_line (fd_receiver_error->error_browser, string);
				}
			}
		}
		else
		{
			fl_add_browser_line (fd_receiver_error->error_browser, "No Error Information Is Available");
		}
		if (data == -10)
		{
			fl_add_browser_line (fd_receiver_error->error_browser, 
			  "I/O Timeout from Gnu Radio--no data for >30 seconds");
			fprintf (stderr, "I/O timeout--exiting");
		}
		fl_show_form (fd_receiver_error->receiver_error, FL_PLACE_CENTER, FL_FULLBORDER, "IRA Errors");
		fl_check_forms ();
		no_more_input = 1;
		return;
	}
	exit (0);
}

int tp_displayed = 0;

void
tp_display (FL_OBJECT *ob, long data)
{
	if (tp_displayed == 0)
	{
		fl_show_form (fd_receiver_continuum->receiver_continuum, FL_PLACE_CENTER, FL_FULLBORDER, "IRA Continuum");
		tp_displayed = 1;
	}
	else
	{
		fl_raise_form(fd_receiver_continuum->receiver_continuum);
	}
}


void
tp_hide (FL_OBJECT *ob, long data)
{
	if (tp_displayed == 1)
	{
		fl_hide_form (fd_receiver_continuum->receiver_continuum);
		tp_displayed = 0;
	}
}

void
adjust_refmult (FL_OBJECT *ob, long data)
{
	refmult = fl_get_slider_value (ob);
}

void
adjust_seti_integ (FL_OBJECT *ob, long data)
{
	seti_integ = (int)fl_get_slider_value (ob);
}

int inter_displayed = 0;

void
hide_interferometer (FL_OBJECT *ob, long data)
{
	if (inter_displayed == 1)
	{
		fl_hide_form (fd_receiver_interferometer->receiver_interferometer);
		inter_displayed = 0;
	}
}

void
interferometer_display (FL_OBJECT *ob, long data)
{
	if (inter_displayed == 0)
	{
		fl_show_form (fd_receiver_interferometer->receiver_interferometer, 
			FL_PLACE_CENTER, FL_FULLBORDER, "IRA Interferometer");
		inter_displayed = 1;
	}
	else
	{
		fl_raise_form(fd_receiver_interferometer->receiver_interferometer);
	}
}

int wfall_displayed = 0;

void
hide_waterfall (FL_OBJECT *ob, long data)
{
	if (wfall_displayed == 1)
	{
		fl_hide_form (fd_receiver_waterfall->receiver_waterfall);
		wfall_displayed = 0;
	}
}

void
display_waterfall (FL_OBJECT *ob, long data)
{
	if (wfall_displayed == 0)
	{
		fl_show_form(fd_receiver_waterfall->receiver_waterfall, FL_PLACE_CENTER,FL_FULLBORDER,"IRA WaterFall");
		wfall_displayed = 1;
	}
	else
	{
		fl_raise_form(fd_receiver_waterfall->receiver_waterfall);
	}
}

void
waterfall_adjust_segment (FL_OBJECT *ob, long data)
{	
	waterfall_segment = (int)fl_get_slider_value(ob);
}

void
adjust_interferometer_gain (FL_OBJECT *ob, long data)
{
	interferometer_gain = fl_get_slider_value (ob);
}

void
adjust_interferometer_phase (FL_OBJECT *ob, long data)
{
	interferometer_phase = fl_get_slider_value (ob);
	write_rcvr_params ("phcorr", interferometer_phase);
}

void
adjust_interferometer_delay (FL_OBJECT *ob, long data)
{
	interferometer_delay = fl_get_slider_value (ob);
	write_rcvr_params ("delay", interferometer_delay);
}

void
adjust_interferometer_span (FL_OBJECT *ob, long data)
{
	interferometer_span = fl_get_slider_value (ob);
	fl_set_xyplot_ybounds (fd_receiver_interferometer->interferometer_chart, -1*interferometer_span, interferometer_span);
}

int info_displayed = 0;

void
info_display (FL_OBJECT *ob, long data)
{
	if (info_displayed == 0)
	{
		fl_show_form (fd_receiver_info->receiver_info, FL_PLACE_CENTER, FL_FULLBORDER, "IRA Info");
		info_displayed = 1;
	}
	else
	{
		fl_raise_form(fd_receiver_info->receiver_info);
	}
}

void
hide_info (FL_OBJECT *ob, long data)
{
	if (info_displayed == 1)
	{
		fl_hide_form (fd_receiver_info->receiver_info);
		info_displayed = 0;
	}
}

void
lproblem_leave (FL_OBJECT *ob, long data)
{
	fprintf (stderr, "Leaving from: %s\n", __FUNCTION__);
	receiver_leave(NULL, 0);
}

void
create_spec_postscript (FL_OBJECT *ob, long data)
{
	fl_object_ps_dump(fd_receiver_spectrum->spectral_plot, NULL);
}

void
create_tp_postscript (FL_OBJECT *ob, long data)
{
	fl_object_ps_dump(fd_receiver_continuum->tp_chart, NULL);
}

void
create_pulsar_postscript (FL_OBJECT *ob, long data)
{
	fl_object_ps_dump(fd_receiver_pulsar->pulsar_plot, NULL);
}

void
create_inter_postscript (FL_OBJECT *ob, long data)
{
	fl_object_ps_dump(fd_receiver_interferometer->interferometer_chart, NULL);
}

void popup_browser (FL_OBJECT *ob, long data)
{
	system ("firefox -remote \"openURL(http://www.science-radio-labs.com,new-window)\"");
}

void
do_help (char *str)
{
	char hbuf[512];
	char *prefix = "/usr/local";
	char *p;
	
	if ((p = getenv ("RCVR_HELP_PREFIX")) != NULL)
	{
		prefix = p;
	}
	
	sprintf (hbuf, "firefox -remote \"openURL(file:%s/share/doc/ira/%s)\"",
		prefix, str);
		
	system (hbuf);
}

void
show_help (FL_OBJECT *ob, long data)
{
	do_help ("ira_control.html");
}

void
show_continuum_help (FL_OBJECT *ob, long data)
{
	do_help ("ira_continuum.html");
}

void
show_waterfall_help (FL_OBJECT *ob, long data)
{
	do_help ("ira_waterfall.html");
}

void
show_pulsar_help (FL_OBJECT *ob, long data)
{
	do_help ("ira_pulsar.html");
}

void
show_spectrum_help (FL_OBJECT *ob, long data)
{
	do_help ("ira_spectrum.html");
}

void
show_inter_help (FL_OBJECT *ob, long data)
{
	do_help ("ira_interferometer.html");
}

void
adjust_spec_method (FL_OBJECT *ob, long data)
{
	spec_method = fl_get_choice (ob);
}

void export_waterfall (FL_OBJECT *ob, long data)
{
	char *filename;
	FILE *fp;
	
	filename = (char *)fl_show_fselector ("Select Output",".", "*.xpm","waterfall.xpm");
	if (filename != NULL)
	{
		fp = fopen (filename, "w");
		if (fp != NULL)
		{
			wfall_export (fp);
		
		}
	}
}
	
void
change_flatten (FL_OBJECT *ob, long data)
{
	spec_flat_on = fl_get_button (ob);
}

void
change_rfi_marking (FL_OBJECT *ob, long data)
{
	mark_rfi = fl_get_button (ob);
}

void
adjust_brightness (FL_OBJECT *ob, long data)
{
	const char *str;
	
	str = fl_get_choice_text (ob);
	if (strcmp (str, "100") == 0)
	{
		waterfall_brightness = 1.0;
	}
	if (strcmp (str, "75") == 0)
	{
		waterfall_brightness = 0.75;
	}
	if (strcmp (str, "66") == 0)
	{
		waterfall_brightness = 0.66;
	}
	if (strcmp (str, "50") == 0)
	{
		waterfall_brightness = 0.50;
	}
	update_wfall_colormap ();
}

void
set_pulsar_folding (FL_OBJECT *ob, long data)
{
	pulsar_folding = fl_get_choice (ob);
	if (pulsar_folding < 1)
	{
		pulsar_folding = 1;
	}
	pulsar_folding = pulsar_folding * 5;
}

void
adjust_sky_frequency (FL_OBJECT *ob, long data)
{
	char *p;
	char tbuf[128];
	
	p = (char *)fl_get_input (ob);
	strcpy (tbuf, p);
	if ((p = strchr (tbuf, '\n')) != NULL)
	{
		*p = '\0';
	}
	if (strcmp (tbuf, "lock") == 0 ||
		strcmp (tbuf, "locked") == 0 ||
		strcmp (tbuf, "tuner") == 0 ||
		strncmp (tbuf, "----", 4) == 0)
	{
		char d[128];
		sky_locked = 1;
		sky_freq = frequency;
		sprintf (d, "%f", sky_freq);
		fl_set_input (ob, d);
		fl_set_button (fd_receiver_main->sky_lock_button, 1);
	}
	else
	{
		if (check_float (tbuf) < 0)
		{
			fl_set_input (ob, "Format Error");
		}
		else
		{
			sky_freq = atof(tbuf);
			sky_locked = 0;
		}
		write_rcvr_params ("skyfreq", sky_freq);
	}
}

void
set_sky_locked (FL_OBJECT *ob, long data)
{
	sky_locked = fl_get_button (ob);
	if (sky_locked != 0)
	{
		fl_deactivate_object (fd_receiver_main->sky_freq_input);
		fl_set_input (fd_receiver_main->sky_freq_input, "--------");
		write_rcvr_params ("skyfreq", frequency);
	}
	else
	{
		fl_activate_object (fd_receiver_main->sky_freq_input);
	}
}

void
clear_notches (FL_OBJECT *ob, long data)
{
	int i;
	
	for (i = 0; i < NNOTCHES; i++)
	{
		notches[i] = -1.0;
	}
	notches[0] = 1.0;
	write_notches ();
	dump_params ();
}

void
set_notch_length (FL_OBJECT *ob, long data)
{
	notch_length = (int)fl_get_slider_value (ob);
	write_notches ();
}

void
set_transient_threshold (FL_OBJECT *ob, long data)
{
	transient_threshold = fl_get_slider_value (ob);
}

void
set_transient_duration (FL_OBJECT *ob, long data)
{
	double round();
	transient_duration = round(fl_get_slider_value (ob));
}

int
check_float (s)
char *s;
{
	int good;
	
	good = 0;
	while (*s)
	{
		if (isdigit(*s) || *s == '.' || *s == 'e' || *s == '-')
		{
			s++;
			continue;
		}
		s++;
		good = -1;
	}
	return (good);
}

void
set_mult_choice (FL_OBJECT *ob, long data)
{
	const char *p;
	
	p = fl_get_choice_text (ob);
	dc_mult = atoi (p+1);
	detector_peak = 0.0;
}

void
set_gc_a (FL_OBJECT *ob, long data)
{
	char tbuf[128];
	char *p;
	
	p = (char *)fl_get_input (ob);
	strcpy (tbuf, p);
	if ((p = strchr (tbuf, '\n')) != NULL)
	{
		*p = '\0';
	}
	if (check_float(tbuf) < 0)
	{
	  fl_set_input (ob, "Format Error");
	}
  	else
	{
		gc_a = atof(tbuf);
		write_rcvr_params ("cora", gc_a);
	}
}

void
set_gc_b (FL_OBJECT *ob, long data)
{
	char tbuf[128];
	char *p;
	
	p = (char *)fl_get_input (ob);
	strcpy (tbuf, p);
	if ((p = strchr (tbuf, '\n')) != NULL)
	{
		*p = '\0';
	}
	if (check_float(tbuf) < 0)
	{
		fl_set_input (ob, "Format Error");
	}
	else
	{
		gc_b = atof(tbuf);
		write_rcvr_params ("corb", gc_b);
	}
}

extern double met;

void
pulsar_jog (FL_OBJECT *ob, long data)
{
	double pulsar_period;

	pulsar_period = 1.0/(double)pulsar_rate;
	pulsar_period /= 20.0;
	pulsar_period *= data;
	
	met += (double) pulsar_period;
}

void
set_squash_mode (FL_OBJECT *ob, long data)
{
	squash_mode = fl_get_button (ob);
}

