#include "forms.h"
#include "receiver_start.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

extern int bandwidth;
extern int single;
extern int psr_srate;
extern int continuum_srate;
extern char license_key[];
extern double longitude;
extern char side[];
extern char ra_mode[];
extern char dec_mode[];
extern double rxa_cor;
extern double rxb_cor;

/*** callbacks and freeobj handles for form receiver_start ***/
void select_bandwidth(FL_OBJECT *ob, long data)
{
	const char *str;
	double fbw;
	
	str = fl_get_choice_text (ob);
	
	fbw = atof (str);
	bandwidth = (int)fbw;
}

void select_single(FL_OBJECT *ob, long data)
{
	const char *str;
	str = fl_get_choice_text (ob);
	if (strcmp (str, "dual") == 0)
	{
		single = 0;
	}
	if (strcmp (str, "single") == 0)
	{
		single = 1;
	}
	if (strcmp (str, "interferometer") == 0)
	{
		single = 2;
	}
	if (strcmp (str, "split") == 0)
	{
		single = 3;
	}
	if (strcmp (str, "uhd") == 0)
	{
		single = 4;
	}
}

extern char *psr_rate_strs[];

#define MILLION 1000*1000
void
start_receiver(FL_OBJECT *ob, long data)
{
	FILE *fp;
	char *modestrs[] = {"INVALID", "single", "interferometer", "INVALID", "INVALID", "INVALID", "INVALID"};
	int decim;
	int ad_rate;
	int max_bus_bw;
	int i;
	int last;
	
	for (i = 0; psr_rate_strs[i] != NULL; i++)
	{
		last = i;
	}
	
	/*
	 * Find the highest rate that is a proper divisor of the bandwidth
	 */
	for (i = last; i >= 0; i--)
	{
		if ((bandwidth % atoi(psr_rate_strs[i])) == 0)
		{
			psr_srate = atoi (psr_rate_strs[i]);
			break;
		}
	}
	
	fp = fopen ("rcvr_start_params", "w");
	
	fprintf (fp, "export RCVR_BANDWIDTH=%d\n", bandwidth);
	fprintf (fp, "export RCVR_MODE=%s\n", modestrs[single]);
	fprintf (fp, "export RCVR_PSR_RATE=%d\n", psr_srate);
	if (longitude > (float)(-900))
	{
		fprintf (fp, "export RCVR_LONGITUDE=%f\n", longitude);
	}
	if (strlen (license_key) != 0)
	{
		fprintf (fp, "export RCVR_LICENSE_KEY=\"%s\"\n", license_key);
	}

	if (strcmp (modestrs[single], "interferometer") == 0)
	{
		fprintf (fp, "export RCVR_SUBDEV=\"A:0 B:0\"\n");
	}
	else
	{
		fprintf (fp, "export RCVR_SUBDEV=%s\n", side);
	}
	fprintf (fp, "export RCVR_RA_MODE=\"%s\"\n", ra_mode);
	fprintf (fp, "export RCVR_DEC_MODE=\"%s\"\n", dec_mode);
	fclose (fp);
	exit (0);
}

void
select_psr_srate (FL_OBJECT *ob, long data)
{
	char *str;
	char numbuf[128], *np, *p;
	
	str = (char *)fl_get_choice_text (ob);
	p = str;
	np = numbuf;
	while (isdigit(*p))
	{
		*np = *p;
		np++;
		*np = '\0';
		p++;
	}
	psr_srate = atoi(numbuf) * 1000;
}

void
input_license_key (FL_OBJECT *ob, long data)
{
	strcpy (license_key, fl_get_input (ob));
}

void
set_longitude (FL_OBJECT  *ob, long data)
{
	longitude = atof (fl_get_input (ob));
}

void
set_side_choice (FL_OBJECT *ob, long data)
{
	const char *str;
	
	if ((str = fl_get_choice_text (ob)) != NULL)
	{
		if (strcmp (str, "A") == 0)
		{
			strcpy (side, "A:0");
		}
		else if (strcmp (str, "B") == 0)
		{
			strcpy (side, "B:0");
		}
	}
	else
	{
		strcpy (side, "A:0");
	}
}

void
show_help (FL_OBJECT *ob, long data)
{
	char helpbuf[512];
	char *prefix = "/usr/local";
	char *p;
	if ((p = getenv ("RCVR_HELP_PREFIX")) != NULL)
	{
		prefix = p;
	}
	sprintf (helpbuf,
		"firefox -remote \"openURL(file:%s/share/doc/ira/ira_start.html)\"", prefix);
	system (helpbuf);
}

void
set_ra_mode (FL_OBJECT *ob, long data)
{	
	strcpy (ra_mode, fl_get_choice_text (ob));
	if (strcmp (ra_mode, "external") == 0)
	{
		sprintf (ra_mode, "external %s",
			(char *)fl_show_fselector ("Select RA Control File",".", "*.*","ra_control_file"));
	}
}

void
set_dec_mode (FL_OBJECT *ob, long data)
{
	strcpy (dec_mode, fl_get_choice_text (ob));
	if (strcmp (dec_mode, "external") == 0)
	{
		sprintf (dec_mode, "external %s",
			(char *)fl_show_fselector ("Select DEC Control File",".", "*.*","dec_control_file"));
	}
}

void
exit_receiver (FL_OBJECT *ob, long data)
{
	FILE *fp;
	
	fp = fopen ("receiver_no_start", "w");
	fprintf (fp, "No Start\n");
	fclose (fp);
	
	start_receiver (NULL, 0);
}

void
set_rxa_cor (FL_OBJECT *ob, long data)
{
	const char *p;
	
	p = fl_get_input (ob);
	if (p != NULL && *p != '\0')
	{
		sscanf (p, "%lf", &rxa_cor);
	}
}

void
set_rxb_cor (FL_OBJECT *ob, long data)
{
	const char *p;
	
	p = fl_get_input (ob);
	if (p != NULL && *p != '\0')
	{
		sscanf (p, "%lf", &rxb_cor);
	}
}
