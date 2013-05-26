#include "dtracker.h"
#include <stdlib.h>

extern double longitude;
extern double latitude;
extern double decln;
extern double ra;
extern int tracking;

/* Callbacks and freeobj handles for form dtracker */

/***************************************
 ***************************************/

void set_tracking( FL_OBJECT * ob,
         long        data )
{
    tracking = fl_get_button (ob);
}


/***************************************
 ***************************************/

void set_latitude( FL_OBJECT * ob,
         long        data )
{
    latitude = atof(fl_get_input(ob));
}


/***************************************
 ***************************************/

void set_tracking_ra( FL_OBJECT * ob,
         long        data )
{
    ra = atof(fl_get_input(ob));
}


/***************************************
 ***************************************/

void set_tracking_dec( FL_OBJECT * ob,
         long        data )
{
   decln = atof(fl_get_input(ob));
}

void do_help (FL_OBJECT *ob, long data)
{
	char helpbuf[512];
	char *prefix = "/usr/local";
	char *p;
	if ((p = getenv ("RCVR_HELP_PREFIX")) != NULL)
	{
		prefix = p;
	}
	sprintf (helpbuf,
		"firefox -remote \"openURL(file:%s/share/doc/ira/ira_dtracker.html)\"", prefix);
	system (helpbuf);
}



