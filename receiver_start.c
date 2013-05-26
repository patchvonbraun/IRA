/* Form definition file generated by fdesign */

#include <stdlib.h>
#include "receiver_start.h"


/***************************************
 ***************************************/

FD_receiver_start *
create_form_receiver_start( void )
{
    FL_OBJECT *obj;
    FD_receiver_start *fdui = fl_malloc( sizeof *fdui );

    fdui->vdata = fdui->cdata = NULL;
    fdui->ldata = 0;

    fdui->receiver_start = fl_bgn_form( FL_NO_BOX, 518, 302 );

    fdui->receiver_starter = obj = fl_add_box( FL_UP_BOX, 0, 0, 518, 302, "Integrated RA Receiver Startup" );
    fl_set_object_color( obj, FL_INDIANRED, FL_COL1 );
    fl_set_object_lcolor( obj, FL_YELLOW );
    fl_set_object_lsize( obj, FL_LARGE_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM | FL_ALIGN_INSIDE );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );

    fdui->start_button = obj = fl_add_button( FL_NORMAL_BUTTON, 380, 10, 120, 40, "Start RA Receiver" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_CENTER );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, start_receiver, 0 );

    obj = fl_add_text( FL_NORMAL_TEXT, 25, 210, 450, 30, "Science Radio Laboratories  http://www.science-radio-labs.com" );
    fl_set_object_boxtype( obj, FL_ROUNDED_BOX );
    fl_set_object_color( obj, FL_PALEGREEN, FL_MCOL );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lstyle( obj, FL_BOLDITALIC_STYLE );

    fdui->license_input = obj = fl_add_input( FL_NORMAL_INPUT, 10, 140, 290, 30, "License Key" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, input_license_key, 0 );
    fl_set_object_return( obj, FL_RETURN_END_CHANGED );

    fdui->longitude_input = obj = fl_add_input( FL_NORMAL_INPUT, 240, 80, 110, 30, "Longitude" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, set_longitude, 0 );
    fl_set_object_return( obj, FL_RETURN_END_CHANGED );

    fdui->help_button = obj = fl_add_button( FL_NORMAL_BUTTON, 420, 255, 70, 30, "HELP" );
    fl_set_object_color( obj, FL_SLATEBLUE, FL_COL1 );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_CENTER );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, show_help, 0 );

    fdui->ra_choice = obj = fl_add_choice( FL_NORMAL_CHOICE2, 160, 10, 90, 30, "RA Mode" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, set_ra_mode, 0 );
    fl_addto_choice( obj, "manual" );
    fl_set_choice_item_mode( obj, 1, FL_PUP_NONE );
    fl_addto_choice( obj, "transit" );
    fl_set_choice_item_mode( obj, 2, FL_PUP_NONE );
    fl_addto_choice( obj, "external" );
    fl_set_choice_item_mode( obj, 3, FL_PUP_NONE );
    fl_set_choice( obj, 1 );

    fdui->dec_choice = obj = fl_add_choice( FL_NORMAL_CHOICE2, 260, 10, 90, 30, "DEC Mode" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_NORMAL_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, set_dec_mode, 0 );
    fl_addto_choice( obj, "manual" );
    fl_set_choice_item_mode( obj, 1, FL_PUP_NONE );
    fl_addto_choice( obj, "external" );
    fl_set_choice_item_mode( obj, 2, FL_PUP_NONE );
    fl_set_choice( obj, 1 );

    obj = fl_add_button( FL_NORMAL_BUTTON, 400, 120, 100, 40, "EXIT" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_MEDIUM_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_CENTER );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, exit_receiver, 0 );

    fdui->bw_choice = obj = fl_add_choice( FL_NORMAL_CHOICE2, 20, 10, 130, 30, "Bandwidth" );
    fl_set_object_boxtype( obj, FL_EMBOSSED_BOX );
    fl_set_object_lsize( obj, FL_MEDIUM_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, select_bandwidth, 0 );

    fdui->side_choice = obj = fl_add_choice( FL_NORMAL_CHOICE2, 20, 80, 80, 30, "Side" );
    fl_set_object_lsize( obj, FL_MEDIUM_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, set_side_choice, 0 );
    fl_addto_choice( obj, "A" );
    fl_addto_choice( obj, "B" );
    fl_set_choice( obj, 1 );

    fdui->rcvr_mode = obj = fl_add_choice( FL_NORMAL_CHOICE2, 110, 80, 120, 30, "Mode" );
    fl_set_object_lsize( obj, FL_MEDIUM_SIZE );
    fl_set_object_lalign( obj, FL_ALIGN_BOTTOM );
    fl_set_object_lstyle( obj, FL_BOLD_STYLE );
    fl_set_object_callback( obj, select_single, 0 );
    fl_addto_choice( obj, "single" );
    fl_addto_choice( obj, "interferometer" );
    fl_set_choice( obj, 1 );

    fl_end_form( );

    fdui->receiver_start->fdui = fdui;

    return fdui;
}
