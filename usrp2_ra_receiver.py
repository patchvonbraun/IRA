#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated Radio Astronomy Receiver
# Author: Marcus Leech
# Description: gnuradio flow graph
# Generated: Mon Nov 29 20:47:26 2010
##################################################

from gnuradio import eng_notation
from gnuradio import gr
from gnuradio import uhd
from gnuradio import window
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from optparse import OptionParser
import SimpleXMLRPCServer
import iraconfig
import sys
import threading

class usrp2_ra_receiver(gr.top_block):

	def __init__(self, freq=1420.4058e6, gain=55, ratepsr=8000, bandwidth=100.0e3, dm=0.1, devaddr="addr=192.168.10.2", adrate=int(64.0e6)):
		gr.top_block.__init__(self, "Integrated Radio Astronomy Receiver")

		##################################################
		# Parameters
		##################################################
		self.freq = freq
		self.gain = gain
		self.ratepsr = ratepsr
		self.bandwidth = bandwidth
		self.dm = dm
		self.devaddr = devaddr
		self.adrate = adrate

		##################################################
		# Variables
		##################################################
		self.notch_len = notch_len = 128
		self.notch_freqs = notch_freqs = [0]
		self.max_bus_bw = max_bus_bw = 25.0e6
		self.ifreq = ifreq = freq
		self.idm = idm = dm
		self.bin_width = bin_width = 10
		self.skyfreq = skyfreq = ifreq
		self.setisize = setisize = int(bandwidth/bin_width)
		self.seti_rate = seti_rate = 5
		self.pid = pid = iraconfig.writepid()
		self.inotches = inotches = iraconfig.compute_notches(notch_freqs,int(notch_len),bandwidth,ifreq,idm)
		self.igain = igain = gain
		self.ibandwidth = ibandwidth = bandwidth if bandwidth <= max_bus_bw else max_bus_bw

		##################################################
		# Blocks
		##################################################
		self.gr_complex_to_mag_0 = gr.complex_to_mag(int(setisize))
		self.gr_complex_to_mag_squared_0 = gr.complex_to_mag_squared(1)
		self.gr_fft_filter_xxx_0 = gr.fft_filter_ccc(1, (iraconfig.compute_dispfilter(idm,1.0,ibandwidth,skyfreq)))
		self.gr_fft_vxx_0 = gr.fft_vcc(int(setisize), True, (window.blackmanharris(int(bandwidth))), False)
		self.gr_file_sink_0_0 = gr.file_sink(gr.sizeof_float*int(setisize), "ra_seti_fifo")
		self.gr_file_sink_0_0.set_unbuffered(True)
		self.gr_file_sink_1 = gr.file_sink(gr.sizeof_float*1, "ra_psr_fifo")
		self.gr_file_sink_1.set_unbuffered(True)
		self.gr_keep_one_in_n_0 = gr.keep_one_in_n(gr.sizeof_float*1, int((ibandwidth)/ratepsr))
		self.gr_keep_one_in_n_1 = gr.keep_one_in_n(gr.sizeof_float*int(setisize), (bin_width/seti_rate))
		self.gr_multiply_const_vxx_0 = gr.multiply_const_vcc((3.0e4, ))
		self.gr_single_pole_iir_filter_xx_0 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/(ratepsr/2.0)), 1)
		self.gr_single_pole_iir_filter_xx_1 = gr.single_pole_iir_filter_ff(1.0/bin_width, int(setisize))
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, int(setisize))
		self.notch_filter_a = gr.fft_filter_ccc(1, (inotches))
		self.uhd_single_usrp_source_0 = uhd.single_usrp_source(
			device_addr=devaddr,
			io_type=uhd.io_type_t.COMPLEX_FLOAT32,
			num_channels=1,
		)
		self.uhd_single_usrp_source_0.set_samp_rate(ibandwidth)
		self.uhd_single_usrp_source_0.set_center_freq(ifreq, 0)
		self.uhd_single_usrp_source_0.set_gain(igain, 0)
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
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_single_pole_iir_filter_xx_1, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1, 0), (self.gr_keep_one_in_n_1, 0))
		self.connect((self.gr_keep_one_in_n_1, 0), (self.gr_file_sink_0_0, 0))
		self.connect((self.uhd_single_usrp_source_0, 0), (self.gr_multiply_const_vxx_0, 0))

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

	def set_bandwidth(self, bandwidth):
		self.bandwidth = bandwidth
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/(self.ratepsr/2.0)))
		self.set_setisize(int(self.bandwidth/self.bin_width))
		self.set_ibandwidth(self.bandwidth if self.bandwidth <= self.max_bus_bw else self.max_bus_bw)

	def set_dm(self, dm):
		self.dm = dm
		self.set_idm(self.dm)

	def set_devaddr(self, devaddr):
		self.devaddr = devaddr

	def set_adrate(self, adrate):
		self.adrate = adrate

	def set_notch_len(self, notch_len):
		self.notch_len = notch_len
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def set_notch_freqs(self, notch_freqs):
		self.notch_freqs = notch_freqs
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def set_max_bus_bw(self, max_bus_bw):
		self.max_bus_bw = max_bus_bw
		self.set_ibandwidth(self.bandwidth if self.bandwidth <= self.max_bus_bw else self.max_bus_bw)

	def set_ifreq(self, ifreq):
		self.ifreq = ifreq
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.uhd_single_usrp_source_0.set_center_freq(self.ifreq, 0)
		self.set_skyfreq(self.ifreq)

	def set_idm(self, idm):
		self.idm = idm
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def set_bin_width(self, bin_width):
		self.bin_width = bin_width
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/self.bin_width)
		self.set_setisize(int(self.bandwidth/self.bin_width))
		self.gr_keep_one_in_n_1.set_n((self.bin_width/self.seti_rate))

	def set_skyfreq(self, skyfreq):
		self.skyfreq = skyfreq
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def set_setisize(self, setisize):
		self.setisize = setisize

	def set_seti_rate(self, seti_rate):
		self.seti_rate = seti_rate
		self.gr_keep_one_in_n_1.set_n((self.bin_width/self.seti_rate))

	def set_pid(self, pid):
		self.pid = pid

	def set_inotches(self, inotches):
		self.inotches = inotches
		self.notch_filter_a.set_taps((self.inotches))

	def set_igain(self, igain):
		self.igain = igain
		self.uhd_single_usrp_source_0.set_gain(self.igain, 0)

	def set_ibandwidth(self, ibandwidth):
		self.ibandwidth = ibandwidth
		self.gr_keep_one_in_n_0.set_n(int((self.ibandwidth)/self.ratepsr))
		self.uhd_single_usrp_source_0.set_samp_rate(self.ibandwidth)
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	parser.add_option("", "--freq", dest="freq", type="eng_float", default=eng_notation.num_to_str(1420.4058e6),
		help="Set freq [default=%default]")
	parser.add_option("", "--gain", dest="gain", type="eng_float", default=eng_notation.num_to_str(55),
		help="Set gain [default=%default]")
	parser.add_option("", "--ratepsr", dest="ratepsr", type="eng_float", default=eng_notation.num_to_str(8000),
		help="Set ratepsr [default=%default]")
	parser.add_option("", "--bandwidth", dest="bandwidth", type="eng_float", default=eng_notation.num_to_str(100.0e3),
		help="Set bandwidth [default=%default]")
	parser.add_option("", "--dm", dest="dm", type="eng_float", default=eng_notation.num_to_str(0.1),
		help="Set dm [default=%default]")
	parser.add_option("", "--devaddr", dest="devaddr", type="string", default="addr=192.168.10.2",
		help="Set devaddr [default=%default]")
	parser.add_option("", "--adrate", dest="adrate", type="intx", default=int(64.0e6),
		help="Set adrate [default=%default]")
	(options, args) = parser.parse_args()
	tb = usrp2_ra_receiver(freq=options.freq, gain=options.gain, ratepsr=options.ratepsr, bandwidth=options.bandwidth, dm=options.dm, devaddr=options.devaddr, adrate=options.adrate)
	tb.run()

