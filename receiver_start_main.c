#include "forms.h"
#include "receiver_start.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int bandwidth = 2000000;
int single = 1;
int psr_srate = 4000;
int continuum_srate = 50;
char license_key[128];
double longitude = -900.0;
char side[128];
char ra_mode[128];
char dec_mode[128];
double rxa_cor=1.0;
double rxb_cor=1.0;
char *psr_rate_strs[] =
{
	"2000", "4000", "8000", "10000", "15000", "16000",
	"20000", "25000", "32000", "40000", "50000", "75000", "80000",
	"100000", "120000", "125000", "150000", "160000", "200000", NULL
};

char *usrp1_bandwidths[] =
{
	"500e3", "1.0e6", "2.0e6", "3.2e6", "4.0e6", "6.4e6", "8.0e6", NULL
};

char *usrp2_bandwidths[] =
{
	"500e3", "1.0e6", "2.0e6", "2.5e6", "5.0e6", "10.0e6", "12.5e6", "16.666667e6", "25.0e6", NULL
};

int main(int argc, char *argv[])
{
   FD_receiver_start *fd_receiver_start;
   char *s;
   char *p;
   int i;
   char *utype;
   char **bwlistp;
   int bwp;
   char *clock;
   int iclock;
   float fclock;

   fl_initialize(&argc, argv, 0, 0, 0);
   fd_receiver_start = create_form_receiver_start();

   strcpy (side, "A:0");
   strcpy (ra_mode, "manual");
   strcpy (dec_mode, "manual");

   /* fill-in form initialization code */

   /* show the first form */   

   s = getenv("RCVR_PSR_RATE");
   if (s == NULL)
   {
	   s = "2000";
   }
   psr_srate = atoi(s);
#ifdef NOT_NOW
	for (i = 0; psr_rate_strs[i] != NULL; i++)
	{
		if (strcmp (s, psr_rate_strs[i]) == 0)
		{
			fl_set_choice (fd_receiver_start->psr_sample_choice, i+1);
			break;
		}
	}
#endif

   s = getenv ("RCVR_MODE");
   single = 1;
   if (s == NULL)
   {
	   s = "single";
   }
   if (strcmp (s, "single") == 0)
   {
	   single = 1;
	   fl_set_choice (fd_receiver_start->rcvr_mode, 1);
   }
   if (strcmp (s, "interferometer") == 0)
   {
	   single = 2;
 		fl_set_choice (fd_receiver_start->rcvr_mode, 2);
	}

	license_key[0] = 0;
	if ((p = getenv ("RCVR_LICENSE_KEY")) != NULL)
	{
		strcpy (license_key, p);
		fl_set_input (fd_receiver_start->license_input, license_key);
	}
	else
	{
		strcpy (license_key, "none");
		fl_set_input (fd_receiver_start->license_input, license_key);
	}
	if ((p = getenv ("RCVR_LONGITUDE")) != NULL)
	{
		fl_set_input (fd_receiver_start->longitude_input, p);
		longitude = atof(p);
	}
	else
	{
		fl_set_input (fd_receiver_start->longitude_input, "-900");
		longitude = -900.0;
	}
	if ((p = getenv ("RCVR_SUBDEV")) != NULL)
	{
		if (strcmp (p, "A:0") == 0)
		{
			fl_set_choice (fd_receiver_start->side_choice, 1);
			strcpy (side, p);
		}
		if (strcmp (p, "B:0") == 0)
		{
			fl_set_choice (fd_receiver_start->side_choice, 2);
			strcpy (side, p);
		}
	}
	
	if ((p = getenv ("RCVR_HW_TYPE")) != NULL)
	{
		if (strcmp (p, "USRP1"))
		{
			fl_delete_object  (fd_receiver_start->rcvr_mode);
			single = 1;
		}
	}
	
	if ((p = getenv ("RCVR_RA_MODE")) != NULL)
	{
		if (strncmp (p, "manual", 6) == 0)
		{
			fl_set_choice (fd_receiver_start->ra_choice, 1);
		}
		if (strncmp (p, "transit", 7) == 0)
		{
			fl_set_choice (fd_receiver_start->ra_choice, 2);
		}
		if (strncmp (p, "external", 8) == 0)
		{
			fl_set_choice (fd_receiver_start->ra_choice, 3);
		}
		strcpy (ra_mode, p);
	}
	if ((p = getenv ("RCVR_DEC_MODE")) != NULL)
	{
		strcpy (dec_mode, p);
		if (strncmp (p, "manual", 6) == 0)
		{
			fl_set_choice (fd_receiver_start->dec_choice, 1);
		}
		if (strncmp (p, "external", 8) == 0)
		{
			fl_set_choice (fd_receiver_start->dec_choice, 2);
		}
	}
	
	if ((clock = getenv ("RCVR_HW_CLOCK")) != NULL)
	{
		fclock = atof (clock);
		iclock = (int)fclock;
		
		switch (iclock)
		{
		case (int)64e6:
			bwlistp = usrp1_bandwidths;
			break;
		case (int)100e6:
			bwlistp = usrp2_bandwidths;
			break;
		}
	}
	else
	{
		utype = getenv ("RCVR_HW_TYPE");
		bwlistp = usrp1_bandwidths;
		bwp = 1;
		if (utype == NULL || (strcmp (utype, "USRP1") == 0) || (strcmp (utype, "B100") == 0))
		{
			bwlistp = usrp1_bandwidths;
		}
		else
		{
			bwlistp = usrp2_bandwidths;
		}
	}
	
	s = getenv ("RCVR_BANDWIDTH");
	if (s == NULL)
	{
		s = "2000000";
	}
   
     bandwidth = atoi(s);
     
	for (i = 0; bwlistp[i] != NULL; i++)
	{
		fl_addto_choice (fd_receiver_start->bw_choice, bwlistp[i]);
		if (atoi(s) == (int)atof(bwlistp[i]))
		{
			bwp = i+1;
		}
	}
	fl_set_choice (fd_receiver_start->bw_choice, bwp);	
   fl_show_form(fd_receiver_start->receiver_start,FL_PLACE_CENTERFREE,FL_FULLBORDER,"IRA Starter");
   while (fl_do_forms())
   	;
   return 0;
}
