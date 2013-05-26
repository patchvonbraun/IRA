#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated Radio Astronomy Receiver
# Author: Marcus Leech
# Description: gnuradio flow graph
# Generated: Sun Jun 12 12:30:47 2011
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
import math
import sys
import threading

class uhd_test_receiver(gr.top_block):

	def __init__(self, gain=55, ratepsr=8000, devaddr="addr=192.168.10.2", adrate=int(64.0e6), dfiltsize=65536, sky=250.0e6, dm=0.01, freq=1420.4058e6, binwidth=10, bandwidth=10e6, setisize=262144):
		gr.top_block.__init__(self, "Integrated Radio Astronomy Receiver")

		##################################################
		# Parameters
		##################################################
		self.gain = gain
		self.ratepsr = ratepsr
		self.devaddr = devaddr
		self.adrate = adrate
		self.dfiltsize = dfiltsize
		self.sky = sky
		self.dm = dm
		self.freq = freq
		self.binwidth = binwidth
		self.bandwidth = bandwidth
		self.setisize = setisize

		##################################################
		# Variables
		##################################################
		self.max_bus_bw = max_bus_bw = 25.0e6
		self.skyfreq = skyfreq = freq
		self.notch_len = notch_len = 128
		self.notch_freqs = notch_freqs = [0]
		self.ifreq = ifreq = freq
		self.idm = idm = dm
		self.ibandwidth = ibandwidth = bandwidth if bandwidth <= max_bus_bw else max_bus_bw
		self.cdoppler = cdoppler = 0.0
		self.seti_rate = seti_rate = 5
		self.pid = pid = iraconfig.writepid()
		self.inotches = inotches = iraconfig.compute_notches(notch_freqs,int(notch_len),bandwidth,ifreq,idm)
		self.igain = igain = gain
		self.dedisperse_size = dedisperse_size = len(iraconfig.compute_dispfilter(idm,1.0,ibandwidth,skyfreq))
		self.VARDUMP = VARDUMP = iraconfig.writevars(["skyfreq","cdoppler","combo"], [skyfreq,cdoppler,ifreq+(skyfreq*cdoppler)])

		##################################################
		# Blocks
		##################################################
		self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(("localhost", 11420), allow_none=True)
		self.xmlrpc_server_0.register_instance(self)
		threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()
		self.uhd_usrp_source_0 = uhd.usrp_source(
			device_addr=devaddr,
			io_type=uhd.io_type.COMPLEX_FLOAT32,
			num_channels=1,
		)
		self.uhd_usrp_source_0.set_samp_rate(ibandwidth)
		self.uhd_usrp_source_0.set_center_freq(ifreq+(skyfreq*cdoppler), 0)
		self.uhd_usrp_source_0.set_gain(igain, 0)
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, int(setisize))
		self.gr_single_pole_iir_filter_xx_1 = gr.single_pole_iir_filter_ff(1.0/binwidth, int(setisize))
		self.gr_single_pole_iir_filter_xx_0 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/(ratepsr/2.0)), 1)
		self.gr_multiply_const_vxx_0 = gr.multiply_const_vcc((3.0e4, ))
		self.gr_keep_one_in_n_1 = gr.keep_one_in_n(gr.sizeof_float*int(setisize), int(bandwidth/setisize)/seti_rate)
		self.gr_keep_one_in_n_0 = gr.keep_one_in_n(gr.sizeof_float*1, int(ibandwidth/ratepsr))
		self.gr_file_sink_1 = gr.file_sink(gr.sizeof_float*1, "/dev/null")
		self.gr_file_sink_1.set_unbuffered(True)
		self.gr_file_sink_0_0 = gr.file_sink(gr.sizeof_float*int(setisize), "/dev/null")
		self.gr_file_sink_0_0.set_unbuffered(True)
		self.gr_fft_vxx_0 = gr.fft_vcc(int(setisize), True, (window.blackmanharris(int(bandwidth))), False)
		self.gr_fft_filter_xxx_1 = gr.fft_filter_ccc(1, (inotches))
		self.gr_fft_filter_xxx_0 = gr.fft_filter_ccc(1, (iraconfig.compute_dispfilter(idm,(1.0-cdoppler),ibandwidth,skyfreq)))
		self.gr_complex_to_mag_squared_0 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_0 = gr.complex_to_mag(int(setisize))

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_stream_to_vector_0, 0), (self.gr_fft_vxx_0, 0))
		self.connect((self.gr_fft_vxx_0, 0), (self.gr_complex_to_mag_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_0, 0), (self.gr_keep_one_in_n_0, 0))
		self.connect((self.gr_complex_to_mag_squared_0, 0), (self.gr_single_pole_iir_filter_xx_0, 0))
		self.connect((self.gr_keep_one_in_n_0, 0), (self.gr_file_sink_1, 0))
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_single_pole_iir_filter_xx_1, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.gr_fft_filter_xxx_1, 0))
		self.connect((self.gr_fft_filter_xxx_1, 0), (self.gr_stream_to_vector_0, 0))
		self.connect((self.uhd_usrp_source_0, 0), (self.gr_multiply_const_vxx_0, 0))
		self.connect((self.gr_fft_filter_xxx_1, 0), (self.gr_fft_filter_xxx_0, 0))
		self.connect((self.gr_fft_filter_xxx_0, 0), (self.gr_complex_to_mag_squared_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1, 0), (self.gr_keep_one_in_n_1, 0))
		self.connect((self.gr_keep_one_in_n_1, 0), (self.gr_file_sink_0_0, 0))

	def get_gain(self):
		return self.gain

	def set_gain(self, gain):
		self.gain = gain
		self.set_igain(self.gain)

	def get_ratepsr(self):
		return self.ratepsr

	def set_ratepsr(self, ratepsr):
		self.ratepsr = ratepsr
		self.gr_keep_one_in_n_0.set_n(int(self.ibandwidth/self.ratepsr))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/(self.ratepsr/2.0)))

	def get_devaddr(self):
		return self.devaddr

	def set_devaddr(self, devaddr):
		self.devaddr = devaddr

	def get_adrate(self):
		return self.adrate

	def set_adrate(self, adrate):
		self.adrate = adrate

	def get_dfiltsize(self):
		return self.dfiltsize

	def set_dfiltsize(self, dfiltsize):
		self.dfiltsize = dfiltsize

	def get_sky(self):
		return self.sky

	def set_sky(self, sky):
		self.sky = sky

	def get_dm(self):
		return self.dm

	def set_dm(self, dm):
		self.dm = dm
		self.set_idm(self.dm)

	def get_freq(self):
		return self.freq

	def set_freq(self, freq):
		self.freq = freq
		self.set_ifreq(self.freq)
		self.set_skyfreq(self.freq)

	def get_binwidth(self):
		return self.binwidth

	def set_binwidth(self, binwidth):
		self.binwidth = binwidth
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/self.binwidth)

	def get_bandwidth(self):
		return self.bandwidth

	def set_bandwidth(self, bandwidth):
		self.bandwidth = bandwidth
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/(self.ratepsr/2.0)))
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.set_ibandwidth(self.bandwidth if self.bandwidth <= self.max_bus_bw else self.max_bus_bw)
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)

	def get_setisize(self):
		return self.setisize

	def set_setisize(self, setisize):
		self.setisize = setisize
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)

	def get_max_bus_bw(self):
		return self.max_bus_bw

	def set_max_bus_bw(self, max_bus_bw):
		self.max_bus_bw = max_bus_bw
		self.set_ibandwidth(self.bandwidth if self.bandwidth <= self.max_bus_bw else self.max_bus_bw)

	def get_skyfreq(self):
		return self.skyfreq

	def set_skyfreq(self, skyfreq):
		self.skyfreq = skyfreq
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.skyfreq*self.cdoppler), 0)
		self.set_VARDUMP(iraconfig.writevars(["skyfreq","cdoppler","combo"], [self.skyfreq,self.cdoppler,self.ifreq+(self.skyfreq*self.cdoppler)]))
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.ibandwidth,self.skyfreq)))
		self.set_dedisperse_size(len(iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def get_notch_len(self):
		return self.notch_len

	def set_notch_len(self, notch_len):
		self.notch_len = notch_len
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def get_notch_freqs(self):
		return self.notch_freqs

	def set_notch_freqs(self, notch_freqs):
		self.notch_freqs = notch_freqs
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))

	def get_ifreq(self):
		return self.ifreq

	def set_ifreq(self, ifreq):
		self.ifreq = ifreq
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.skyfreq*self.cdoppler), 0)
		self.set_VARDUMP(iraconfig.writevars(["skyfreq","cdoppler","combo"], [self.skyfreq,self.cdoppler,self.ifreq+(self.skyfreq*self.cdoppler)]))

	def get_idm(self):
		return self.idm

	def set_idm(self, idm):
		self.idm = idm
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.ibandwidth,self.skyfreq)))
		self.set_dedisperse_size(len(iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def get_ibandwidth(self):
		return self.ibandwidth

	def set_ibandwidth(self, ibandwidth):
		self.ibandwidth = ibandwidth
		self.gr_keep_one_in_n_0.set_n(int(self.ibandwidth/self.ratepsr))
		self.uhd_usrp_source_0.set_samp_rate(self.ibandwidth)
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.ibandwidth,self.skyfreq)))
		self.set_dedisperse_size(len(iraconfig.compute_dispfilter(self.idm,1.0,self.ibandwidth,self.skyfreq)))

	def get_cdoppler(self):
		return self.cdoppler

	def set_cdoppler(self, cdoppler):
		self.cdoppler = cdoppler
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.skyfreq*self.cdoppler), 0)
		self.set_VARDUMP(iraconfig.writevars(["skyfreq","cdoppler","combo"], [self.skyfreq,self.cdoppler,self.ifreq+(self.skyfreq*self.cdoppler)]))
		self.gr_fft_filter_xxx_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.ibandwidth,self.skyfreq)))

	def get_seti_rate(self):
		return self.seti_rate

	def set_seti_rate(self, seti_rate):
		self.seti_rate = seti_rate
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)

	def get_pid(self):
		return self.pid

	def set_pid(self, pid):
		self.pid = pid

	def get_inotches(self):
		return self.inotches

	def set_inotches(self, inotches):
		self.inotches = inotches
		self.gr_fft_filter_xxx_1.set_taps((self.inotches))

	def get_igain(self):
		return self.igain

	def set_igain(self, igain):
		self.igain = igain
		self.uhd_usrp_source_0.set_gain(self.igain, 0)

	def get_dedisperse_size(self):
		return self.dedisperse_size

	def set_dedisperse_size(self, dedisperse_size):
		self.dedisperse_size = dedisperse_size

	def get_VARDUMP(self):
		return self.VARDUMP

	def set_VARDUMP(self, VARDUMP):
		self.VARDUMP = VARDUMP

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	parser.add_option("", "--gain", dest="gain", type="eng_float", default=eng_notation.num_to_str(55),
		help="Set gain [default=%default]")
	parser.add_option("", "--ratepsr", dest="ratepsr", type="eng_float", default=eng_notation.num_to_str(8000),
		help="Set ratepsr [default=%default]")
	parser.add_option("", "--devaddr", dest="devaddr", type="string", default="addr=192.168.10.2",
		help="Set devaddr [default=%default]")
	parser.add_option("", "--adrate", dest="adrate", type="intx", default=int(64.0e6),
		help="Set None [default=%default]")
	parser.add_option("", "--dfiltsize", dest="dfiltsize", type="intx", default=65536,
		help="Set None [default=%default]")
	parser.add_option("", "--sky", dest="sky", type="eng_float", default=eng_notation.num_to_str(250.0e6),
		help="Set None [default=%default]")
	parser.add_option("", "--dm", dest="dm", type="eng_float", default=eng_notation.num_to_str(0.01),
		help="Set dm [default=%default]")
	parser.add_option("", "--freq", dest="freq", type="eng_float", default=eng_notation.num_to_str(1420.4058e6),
		help="Set freq [default=%default]")
	parser.add_option("", "--binwidth", dest="binwidth", type="intx", default=10,
		help="Set None [default=%default]")
	parser.add_option("", "--bandwidth", dest="bandwidth", type="eng_float", default=eng_notation.num_to_str(10e6),
		help="Set None [default=%default]")
	parser.add_option("", "--setisize", dest="setisize", type="intx", default=262144,
		help="Set setisize [default=%default]")
	(options, args) = parser.parse_args()
	tb = uhd_test_receiver(gain=options.gain, ratepsr=options.ratepsr, devaddr=options.devaddr, adrate=options.adrate, dfiltsize=options.dfiltsize, sky=options.sky, dm=options.dm, freq=options.freq, binwidth=options.binwidth, bandwidth=options.bandwidth, setisize=options.setisize)
	tb.run()

