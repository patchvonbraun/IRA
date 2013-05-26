#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated Radio Astronomy Receiver
# Author: Marcus Leech
# Description: gnuradio flow graph
# Generated: Sat Aug 21 20:02:24 2010
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import window
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from grc_gnuradio import usrp as grc_usrp
from optparse import OptionParser
import SimpleXMLRPCServer
import iraconfig
import sys
import threading

class integrated_ra_receiver(gr.top_block):

	def __init__(self, freq=1420.4058e6, gain=55, ratepsr=8000, dm=0.1, side='A', bandwidth=100.0e3):
		gr.top_block.__init__(self, "Integrated Radio Astronomy Receiver")

		##################################################
		# Parameters
		##################################################
		self.freq = freq
		self.gain = gain
		self.ratepsr = ratepsr
		self.dm = dm
		self.side = side
		self.bandwidth = bandwidth

		##################################################
		# Variables
		##################################################
		self.max_bus_bw = max_bus_bw = int(16.0e6)
		self.notch_len = notch_len = 128
		self.notch_freqs = notch_freqs = [0]
		self.istop = istop = 0.0
		self.ifreq = ifreq = freq
		self.idm = idm = dm
		self.ibandwidth = ibandwidth = bandwidth if bandwidth <= max_bus_bw else max_bus_bw
		self.bin_width = bin_width = 10
		self.ad_rate = ad_rate = int(64.0e6)
		self.tragedy = tragedy = 1 if istop <= 0.0 else sys.exit()
		self.test_probe = test_probe = 1.0
		self.skyfreq = skyfreq = ifreq
		self.setisize = setisize = int(bandwidth)/bin_width
		self.seti_rate = seti_rate = 5
		self.pid = pid = iraconfig.writepid()
		self.inotches = inotches = iraconfig.compute_notches(notch_freqs,int(notch_len),bandwidth,ifreq,idm)
		self.igain = igain = gain
		self.decim = decim = int(ad_rate/ibandwidth)

		##################################################
		# Blocks
		##################################################
		self.gr_complex_to_mag_0 = gr.complex_to_mag(int(setisize))
		self.gr_complex_to_mag_squared_0 = gr.complex_to_mag_squared(1)
		self.gr_fft_filter_xxx_0 = gr.fft_filter_ccc(1, (iraconfig.compute_dispfilter(idm,1.0,ibandwidth,skyfreq)))
		self.gr_fft_vxx_0 = gr.fft_vcc(int(setisize), True, (window.blackmanharris(int(bandwidth))), False)
		self.gr_file_sink_0_0 = gr.file_sink(gr.sizeof_float*int(setisize), "ra_seti_fifo")
		self.gr_file_sink_0_0.set_unbuffered(False)
		self.gr_file_sink_1 = gr.file_sink(gr.sizeof_float*1, "ra_psr_fifo")
		self.gr_file_sink_1.set_unbuffered(False)
		self.gr_keep_one_in_n_0 = gr.keep_one_in_n(gr.sizeof_float*1, int((ibandwidth)/ratepsr))
		self.gr_keep_one_in_n_1 = gr.keep_one_in_n(gr.sizeof_float*int(setisize), int(bin_width/seti_rate))
		self.gr_multiply_const_vxx_0 = gr.multiply_const_vcc((1.0, ))
		self.gr_single_pole_iir_filter_xx_0 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/(ratepsr/2.0)), 1)
		self.gr_single_pole_iir_filter_xx_1 = gr.single_pole_iir_filter_ff(1.0/bin_width, int(setisize))
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, int(setisize))
		self.notch_filter_a = gr.fft_filter_ccc(1, (inotches))
		self.usrp_simple_source_x_0 = grc_usrp.simple_source_c(which=0, side=side, rx_ant="RXA")
		self.usrp_simple_source_x_0.set_format(width=8, shift=8)
		self.usrp_simple_source_x_0.set_decim_rate(decim)
		self.usrp_simple_source_x_0.set_frequency(ifreq, verbose=True)
		self.usrp_simple_source_x_0.set_gain(igain)
		self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(("localhost", 11420), allow_none=True)
		self.xmlrpc_server_0.register_instance(self)
		threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_stream_to_vector_0, 0), (self.gr_fft_vxx_0, 0))
		self.connect((self.gr_fft_vxx_0, 0), (self.gr_complex_to_mag_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_0, 0), (self.gr_keep_one_in_n_0, 0))
		self.connect((self.gr_complex_to_mag_squared_0, 0), (self.gr_single_pole_iir_filter_xx_0, 0))
		self.connect((self.gr_keep_one_in_n_0, 0), (self.gr_file_sink_1, 0))
		self.connect((self.notch_filter_a, 0), (self.gr_stream_to_vector_0, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.notch_filter_a, 0))
		self.connect((self.notch_filter_a, 0), (self.gr_fft_filter_xxx_0, 0))
		self.connect((self.gr_fft_filter_xxx_0, 0), (self.gr_complex_to_mag_squared_0, 0))
		self.connect((self.usrp_simple_source_x_0, 0), (self.gr_multiply_const_vxx_0, 0))
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_single_pole_iir_filter_xx_1, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1, 0), (self.gr_keep_one_in_n_1, 0))
		self.connect((self.gr_keep_one_in_n_1, 0), (self.gr_file_sink_0_0, 0))

	def set_freq(self, freq):
		self.freq = freq
		self.set_ifreq(self.freq)

	def set_gain(self, gain):
		self.gain = gain
		self.set_igain(self.gain)

	def set_ratepsr(self, ratepsr):
		self.ratepsr = ratepsr
		self.gr_keep_one_in_n_0.set_n(int((self.ibandwidth)/self.ratepsr))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/(self.ratepsr/2.0)))

	def set_dm(self, dm):
		self.dm = dm
		self.set_idm(self.dm)

	def set_side(self, side):
		self.side = side

	def set_bandwidth(self, bandwidth):
		self.bandwidth = bandwidth
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/(self.ratepsr/2.0)))
		self.set_ibandwidth(self.bandwidth if self.bandwidth <= self.max_bus_bw else self.max_bus_bw)
		self.set_setisize(int(self.bandwidth)/self.bin_width)
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def set_max_bus_bw(self, max_bus_bw):
		self.max_bus_bw = max_bus_bw
		self.set_ibandwidth(self.bandwidth if self.bandwidth <= self.max_bus_bw else self.max_bus_bw)

	def set_notch_len(self, notch_len):
		self.notch_len = notch_len
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def set_notch_freqs(self, notch_freqs):
		self.notch_freqs = notch_freqs
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def set_istop(self, istop):
		self.istop = istop
		self.set_tragedy(1 if self.istop <= 0.0 else sys.exit())

	def set_ifreq(self, ifreq):
		self.ifreq = ifreq
		self.usrp_simple_source_x_0.set_frequency(self.ifreq)
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.set_skyfreq(self.ifreq)

	def set_idm(self, idm):
		self.idm = idm
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def set_ibandwidth(self, ibandwidth):
		self.ibandwidth = ibandwidth
		self.set_decim(int(self.ad_rate/self.ibandwidth))
		self.gr_keep_one_in_n_0.set_n(int((self.ibandwidth)/self.ratepsr))
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def set_bin_width(self, bin_width):
		self.bin_width = bin_width
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/self.bin_width)
		self.set_setisize(int(self.bandwidth)/self.bin_width)
		self.gr_keep_one_in_n_1.set_n(int(self.bin_width/self.seti_rate))

	def set_ad_rate(self, ad_rate):
		self.ad_rate = ad_rate
		self.set_decim(int(self.ad_rate/self.ibandwidth))

	def set_tragedy(self, tragedy):
		self.tragedy = tragedy

	def set_test_probe(self, test_probe):
		self.test_probe = test_probe

	def set_skyfreq(self, skyfreq):
		self.skyfreq = skyfreq
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def set_setisize(self, setisize):
		self.setisize = setisize

	def set_seti_rate(self, seti_rate):
		self.seti_rate = seti_rate
		self.gr_keep_one_in_n_1.set_n(int(self.bin_width/self.seti_rate))

	def set_pid(self, pid):
		self.pid = pid

	def set_inotches(self, inotches):
		self.inotches = inotches
		self.notch_filter_a.set_taps((self.inotches))

	def set_igain(self, igain):
		self.igain = igain
		self.usrp_simple_source_x_0.set_gain(self.igain)

	def set_decim(self, decim):
		self.decim = decim
		self.usrp_simple_source_x_0.set_decim_rate(self.decim)

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	parser.add_option("", "--freq", dest="freq", type="eng_float", default=eng_notation.num_to_str(1420.4058e6),
		help="Set freq [default=%default]")
	parser.add_option("", "--gain", dest="gain", type="eng_float", default=eng_notation.num_to_str(55),
		help="Set gain [default=%default]")
	parser.add_option("", "--ratepsr", dest="ratepsr", type="eng_float", default=eng_notation.num_to_str(8000),
		help="Set ratepsr [default=%default]")
	parser.add_option("", "--dm", dest="dm", type="eng_float", default=eng_notation.num_to_str(0.1),
		help="Set dm [default=%default]")
	parser.add_option("", "--side", dest="side", type="string", default='A',
		help="Set side [default=%default]")
	parser.add_option("", "--bandwidth", dest="bandwidth", type="eng_float", default=eng_notation.num_to_str(100.0e3),
		help="Set bandwidth [default=%default]")
	(options, args) = parser.parse_args()
	if gr.enable_realtime_scheduling() != gr.RT_OK:
		print "Error: failed to enable realtime scheduling."
	tb = integrated_ra_receiver(freq=options.freq, gain=options.gain, ratepsr=options.ratepsr, dm=options.dm, side=options.side, bandwidth=options.bandwidth)
	tb.run()

