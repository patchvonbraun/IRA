/* Header file generated by fdesign on Mon Aug  8 21:03:45 2011 */

#ifndef FD_receiver_start_h_
#define FD_receiver_start_h_

#include <forms.h>

/* Callbacks, globals and object handlers */

extern void start_receiver( FL_OBJECT *, long );
extern void input_license_key( FL_OBJECT *, long );
extern void set_longitude( FL_OBJECT *, long );
extern void show_help( FL_OBJECT *, long );
extern void set_ra_mode( FL_OBJECT *, long );
extern void set_dec_mode( FL_OBJECT *, long );
extern void exit_receiver( FL_OBJECT *, long );
extern void select_bandwidth( FL_OBJECT *, long );
extern void set_side_choice( FL_OBJECT *, long );
extern void select_single( FL_OBJECT *, long );


/* Forms and Objects */

typedef struct {
    FL_FORM   * receiver_start;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * receiver_starter;
    FL_OBJECT * start_button;
    FL_OBJECT * license_input;
    FL_OBJECT * longitude_input;
    FL_OBJECT * help_button;
    FL_OBJECT * ra_choice;
    FL_OBJECT * dec_choice;
    FL_OBJECT * bw_choice;
    FL_OBJECT * side_choice;
    FL_OBJECT * rcvr_mode;
} FD_receiver_start;

extern FD_receiver_start * create_form_receiver_start( void );

#endif /* FD_receiver_start_h_ */
