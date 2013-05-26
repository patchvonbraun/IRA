/* Header file generated by fdesign on Tue Sep 18 21:33:13 2012 */

#ifndef FD_receiver_main_h_
#define FD_receiver_main_h_

#include <forms.h>

/* Callbacks, globals and object handlers */

extern void info_display( FL_OBJECT *, long );
extern void adjust_frequency( FL_OBJECT *, long );
extern void adjust_continuum_int( FL_OBJECT *, long );
extern void adjust_spec_int( FL_OBJECT *, long );
extern void adjust_dc_gain( FL_OBJECT *, long );
extern void adjust_dc_offset( FL_OBJECT *, long );
extern void adjust_declination( FL_OBJECT *, long );
extern void adjust_pulsar_rate( FL_OBJECT *, long );
extern void adjust_sigma_k( FL_OBJECT *, long );
extern void adjust_dm( FL_OBJECT *, long );
extern void pulsar_display( FL_OBJECT *, long );
extern void adjust_rf_gain( FL_OBJECT *, long );
extern void spectrum_display( FL_OBJECT *, long );
extern void receiver_leave( FL_OBJECT *, long );
extern void tp_display( FL_OBJECT *, long );
extern void adjust_refmult( FL_OBJECT *, long );
extern void adjust_ra( FL_OBJECT *, long );
extern void adjust_seti_integ( FL_OBJECT *, long );
extern void display_waterfall( FL_OBJECT *, long );
extern void interferometer_display( FL_OBJECT *, long );
extern void popup_browser( FL_OBJECT *, long );
extern void show_help( FL_OBJECT *, long );
extern void set_pulsar_folding( FL_OBJECT *, long );
extern void adjust_sky_frequency( FL_OBJECT *, long );
extern void set_sky_locked( FL_OBJECT *, long );
extern void set_transient_threshold( FL_OBJECT *, long );
extern void set_transient_duration( FL_OBJECT *, long );
extern void set_mult_choice( FL_OBJECT *, long );
extern void set_gc_a( FL_OBJECT *, long );
extern void set_gc_b( FL_OBJECT *, long );

extern void dismiss_psr( FL_OBJECT *, long );
extern void create_pulsar_postscript( FL_OBJECT *, long );
extern void show_pulsar_help( FL_OBJECT *, long );
extern void pulsar_jog( FL_OBJECT *, long );

extern void spectrum_hide( FL_OBJECT *, long );
extern void adjust_spectral_max( FL_OBJECT *, long );
extern void adjust_spectral_span( FL_OBJECT *, long );
extern void create_spec_postscript( FL_OBJECT *, long );
extern void adjust_spec_method( FL_OBJECT *, long );
extern void change_flatten( FL_OBJECT *, long );
extern void show_spectrum_help( FL_OBJECT *, long );
extern void clear_notches( FL_OBJECT *, long );
extern void set_notch_length( FL_OBJECT *, long );

extern void tp_hide( FL_OBJECT *, long );
extern void adjust_tp_maxval( FL_OBJECT *, long );
extern void adjust_tp_span( FL_OBJECT *, long );
extern void create_tp_postscript( FL_OBJECT *, long );
extern void show_continuum_help( FL_OBJECT *, long );
extern void adjust_tp_scale( FL_OBJECT *, long );

extern void hide_waterfall( FL_OBJECT *, long );
extern void waterfall_adjust_segment( FL_OBJECT *, long );
extern void adjust_fine_segment( FL_OBJECT *, long );
extern void export_waterfall( FL_OBJECT *, long );
extern void change_rfi_marking( FL_OBJECT *, long );
extern void adjust_brightness( FL_OBJECT *, long );
extern void show_waterfall_help( FL_OBJECT *, long );
extern void set_squash_mode( FL_OBJECT *, long );

extern void hide_interferometer( FL_OBJECT *, long );
extern void adjust_interferometer_gain( FL_OBJECT *, long );
extern void adjust_interferometer_span( FL_OBJECT *, long );
extern void create_inter_postscript( FL_OBJECT *, long );
extern void show_inter_help( FL_OBJECT *, long );
extern void adjust_interferometer_phase( FL_OBJECT *, long );
extern void adjust_interferometer_delay( FL_OBJECT *, long );

extern void hide_info( FL_OBJECT *, long );

extern void lproblem_leave( FL_OBJECT *, long );

extern void receiver_leave( FL_OBJECT *, long );



/* Forms and Objects */

typedef struct {
    FL_FORM   * receiver_main;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * frequency_input;
    FL_OBJECT * continuum_int;
    FL_OBJECT * spec_int_slider;
    FL_OBJECT * dc_gain_control;
    FL_OBJECT * dc_offset_control;
    FL_OBJECT * declination_input;
    FL_OBJECT * pulsar_rate_input;
    FL_OBJECT * seti_hit_browser;
    FL_OBJECT * lmst_display;
    FL_OBJECT * tp_current_display;
    FL_OBJECT * sigma_k_slider;
    FL_OBJECT * dm_input;
    FL_OBJECT * psr_display_ctl;
    FL_OBJECT * rf_gain_slider;
    FL_OBJECT * spectrum_control;
    FL_OBJECT * exit_button;
    FL_OBJECT * pulsar_delay_display;
    FL_OBJECT * startup_text;
    FL_OBJECT * phase_display;
    FL_OBJECT * show_tp;
    FL_OBJECT * refmult_slider;
    FL_OBJECT * ra_input;
    FL_OBJECT * seti_integ_slider;
    FL_OBJECT * show_waterfall_button;
    FL_OBJECT * interferometer_button;
    FL_OBJECT * io_status;
    FL_OBJECT * ira_xpm_button;
    FL_OBJECT * help_button;
    FL_OBJECT * transients_browser;
    FL_OBJECT * pulsar_choice;
    FL_OBJECT * sky_freq_input;
    FL_OBJECT * sky_lock_button;
    FL_OBJECT * runtime_text;
    FL_OBJECT * trans_thr_slider;
    FL_OBJECT * trans_dur_slider;
    FL_OBJECT * mult_choice;
    FL_OBJECT * detector_peak;
    FL_OBJECT * dicke_mode;
    FL_OBJECT * gc_a;
    FL_OBJECT * gc_b;
} FD_receiver_main;

extern FD_receiver_main * create_form_receiver_main( void );
typedef struct {
    FL_FORM   * receiver_pulsar;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * pulsar_plot;
    FL_OBJECT * psr_dismiss_button;
    FL_OBJECT * pulsar_help;
    FL_OBJECT * psr_jog_left;
    FL_OBJECT * psr_jog_right;
} FD_receiver_pulsar;

extern FD_receiver_pulsar * create_form_receiver_pulsar( void );
typedef struct {
    FL_FORM   * receiver_spectrum;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * dismiss;
    FL_OBJECT * spectral_plot;
    FL_OBJECT * spec_max_slider;
    FL_OBJECT * spec_span_slider;
    FL_OBJECT * spec_method_choice;
    FL_OBJECT * flaten_button;
    FL_OBJECT * spectral_help;
    FL_OBJECT * notch_clear_button;
    FL_OBJECT * notch_slider;
} FD_receiver_spectrum;

extern FD_receiver_spectrum * create_form_receiver_spectrum( void );
typedef struct {
    FL_FORM   * receiver_continuum;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * tp_dismiss;
    FL_OBJECT * chart_lmst_low;
    FL_OBJECT * chart_lmst_high;
    FL_OBJECT * chart_lmst_mid;
    FL_OBJECT * tp_current_display;
    FL_OBJECT * tp_max_slider;
    FL_OBJECT * tp_span_slider;
    FL_OBJECT * tp_chart;
    FL_OBJECT * continuum_help_button;
    FL_OBJECT * tp_time_mult;
} FD_receiver_continuum;

extern FD_receiver_continuum * create_form_receiver_continuum( void );
typedef struct {
    FL_FORM   * receiver_waterfall;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * RcvrWaterFall;
    FL_OBJECT * wfall_seg_slider;
    FL_OBJECT * waterfall_display;
    FL_OBJECT * freq_low;
    FL_OBJECT * freq_high;
    FL_OBJECT * fine_segment;
    FL_OBJECT * freq_mid;
    FL_OBJECT * waterfall_export;
    FL_OBJECT * mark_rfi_button;
    FL_OBJECT * waterfall_show_min;
    FL_OBJECT * waterfall_show_max;
    FL_OBJECT * waterfall_show_avg;
    FL_OBJECT * colormap_pixmap;
    FL_OBJECT * high_db;
    FL_OBJECT * wfall_brightness;
    FL_OBJECT * wfall_help_button;
    FL_OBJECT * squashbutton;
} FD_receiver_waterfall;

extern FD_receiver_waterfall * create_form_receiver_waterfall( void );
typedef struct {
    FL_FORM   * receiver_interferometer;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * RcvrInterferometer;
    FL_OBJECT * dismiss_button;
    FL_OBJECT * int_gain_slider;
    FL_OBJECT * int_span_slider;
    FL_OBJECT * lmst_low;
    FL_OBJECT * lmst_high;
    FL_OBJECT * interferometer_chart;
    FL_OBJECT * phase_adjust;
    FL_OBJECT * delay_adjust;
} FD_receiver_interferometer;

extern FD_receiver_interferometer * create_form_receiver_interferometer( void );
typedef struct {
    FL_FORM   * receiver_info;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * info_browser;
} FD_receiver_info;

extern FD_receiver_info * create_form_receiver_info( void );
typedef struct {
    FL_FORM   * receiver_lproblem;
    void      * vdata;
    char      * cdata;
    long        ldata;
} FD_receiver_lproblem;

extern FD_receiver_lproblem * create_form_receiver_lproblem( void );
typedef struct {
    FL_FORM   * receiver_error;
    void      * vdata;
    char      * cdata;
    long        ldata;
    FL_OBJECT * error_browser;
    FL_OBJECT * error_button;
} FD_receiver_error;

extern FD_receiver_error * create_form_receiver_error( void );
typedef struct {
    FL_FORM   * receiver_shutdown;
    void      * vdata;
    char      * cdata;
    long        ldata;
} FD_receiver_shutdown;

extern FD_receiver_shutdown * create_form_receiver_shutdown( void );

#endif /* FD_receiver_main_h_ */
