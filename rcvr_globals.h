extern int spec_fd, handle_spec_io();
int seti_fd, handle_seti_io();
int pulsar_fd, handle_pulsar_io();
int tp_fd, handle_tp_io();
int dicke_fd, handle_dicke_io();

void write_rcvr_params (char *, double);

extern int spec_integration;
extern int spec_method;
extern int spec_flat_on;
extern int spec_fft_size;
extern int current_smax;
extern int current_span;
extern int tp_integration;
extern double detector_peak;
extern double refmult;
extern double waterfall_integration;
extern int seti_integ;
extern int mark_rfi;
extern int squash_mode;
extern double pulsar_rate;
extern int pulsar_folding;
extern double frequency;
extern double sky_freq;
extern int sky_locked;
extern double bandwidth;
extern int rf_gain;
extern double gc_a;
extern double gc_b;
extern double dc_gain;
extern double dc_offset;
extern double dc_mult;
extern double declination;
extern double ra;
extern int seti_size;
extern int bin_width;
extern int waterfall_segment;
extern int waterfall_fine;
extern int waterfall_fincr;
extern double waterfall_brightness;
extern double sigma_k;
extern double pulsar_dm;
extern double pulsar_delay;
extern int tp_rate;
extern int tp_time_scale;
extern int psr_rate;
extern double tp_maxval;
extern double tp_minval;
extern double tp_span;
extern char datadir[];

extern int rcvr_runrun;
extern int no_more_input;
extern double interferometer_gain;
extern double interferometer_span;
extern double interferometer_phase;
extern double interferometer_delay;
extern double transient_threshold;
extern int transient_duration;
extern int split_mode;


extern double notches[];
extern int notch_length;

extern double longitude;

extern unsigned char current_lmst[];

extern FD_receiver_main *fd_receiver_main;
extern FD_receiver_pulsar *fd_receiver_pulsar;
extern FD_receiver_spectrum *fd_receiver_spectrum;
extern FD_receiver_continuum *fd_receiver_continuum;
extern FD_receiver_waterfall *fd_receiver_waterfall;
extern FD_receiver_interferometer *fd_receiver_interferometer;
extern FD_receiver_info *fd_receiver_info;
extern FD_receiver_error *fd_receiver_error;
extern FD_receiver_shutdown *fd_receiver_shutdown;
extern int gr_fd;

#define NNOTCHES 64
