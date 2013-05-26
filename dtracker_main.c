#include "dtracker.h"
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

extern double longitude;
extern double latitude;
double decln;
double ra;
int tracking = 0;

FD_dtracker *fd_dtracker;

void
write_rcvr_params (char *pname, double parm);

int
main( int    argc,
      char * argv[ ] )
{

    void doppler_update (FL_OBJECT *, long);
    char *p;

    fl_initialize( &argc, argv, 0, 0, 0 );
    fd_dtracker = create_form_dtracker( );

	if ((p = getenv("RCVR_LONGITUDE")) != NULL)
	{
		longitude = atof(p);
	}
	else
	{
		exit(0);
	}
	
    /* Fill-in form initialization code */

    /* Show the first form */
	fl_add_timeout (1000.0, (FL_TIMEOUT_CALLBACK)doppler_update, 0);
    fl_show_form( fd_dtracker->dtracker, FL_PLACE_CENTERFREE, FL_FULLBORDER,
          "Doppler Tracker" );
	fl_set_button (fd_dtracker->track_set, 0);

    fl_do_forms( );

    if ( fl_form_is_visible( fd_dtracker->dtracker ) )
        fl_hide_form( fd_dtracker->dtracker );
    fl_free( fd_dtracker );
    fl_finish( );

    return 0;
}
double current_cdoppler  = 0.0;

#define TORADIANS 3.14159265358/180.0

int dp_counter = 0;
void doppler_update (FL_OBJECT *ob, long data)
{
	unsigned char sid[8];
	double sidseconds;
	double raseconds;
	double tdiff;
	time_t now;
	struct tm *ltp;
	double x;
	double cdoppler;
	char lmst_buf[80];
	void get_sid_time (unsigned char sid[], 
		double lon, struct tm *ltp);
	
	x = 2.0 *(3.14159265358 / 86400.0);
	
	fl_add_timeout (1000.0, (FL_TIMEOUT_CALLBACK)doppler_update, 0);
	
	/*
	 * Determine current LMST
	 */
	time (&now);
	ltp = gmtime (&now);
	get_sid_time (sid, longitude, ltp);

	sidseconds = (double)(sid[0] * 3600.0);
	sidseconds += (double)(sid[1] * 60.0);
	sidseconds += (double)(sid[2]);
	sprintf (lmst_buf, "%02d:%02d:%02d", (int)sid[0], (int)sid[1], 
		(int)sid[2]);
	fl_set_object_label(fd_dtracker->current_lmst, lmst_buf);
	
	raseconds = ra * 3600.0;
	
	/*
	 * Calculate different between meridian-transit time and
	 *   RA of object
	 */
	tdiff = sidseconds - raseconds;
	
	if (tracking)
	{
		/*
		 * Now use that to calculate the current absolute
		 *   correction coefficient, based on appropriate
		 *   tangential velocity and the time difference
		 */
		cdoppler = 1.54e-6 * cos(TORADIANS*latitude) * cos(TORADIANS*decln);
		cdoppler *= sin (x * tdiff);
	}
	else
	{
		cdoppler = 0.0;
	}
		
	sprintf (lmst_buf, "%le (%lf)", cdoppler, tdiff);
	fl_set_object_label(fd_dtracker->current_doppler,  lmst_buf );
	
	/*
	 * We only apply the correction to the flow-graph every
	 *   10 seconds
	 */
	dp_counter++;
	if ((dp_counter % 10) == 0)
	{
		if (current_cdoppler != cdoppler)
		{
			if (tracking)
			{
				write_rcvr_params ("cdoppler", (float)cdoppler);
				current_cdoppler = cdoppler;
			}
			else
			{
				write_rcvr_params ("cdoppler", 0.0);
				current_cdoppler = cdoppler;
			}
		}
	}
}

void
write_rcvr_params (char *pname, double parm)
{
	char sys_str[256];
	
	sprintf (sys_str, "python -c \"import xmlrpclib;xmlrpclib.Server('http://localhost:11420').set_%s(%11.8e)\"", pname, parm);
	system (sys_str);
}
	
