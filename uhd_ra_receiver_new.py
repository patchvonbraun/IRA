#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated RA Receiver
# Author: Marcus Leech
# Description: An RA receiver
# Generated: Mon Feb  6 19:16:25 2012
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
import threading

class uhd_ra_receiver_new(gr.top_block):

	def __init__(self, sky=1420.4058e6, dm=1500.0, setisize=262144, freq=1420.4058e6, bandwidth=5.0e6, ratepsr=50000, gain=55, devaddr="addr=192.168.10.2", binwidth=20, subdev="A:A"):
		gr.top_block.__init__(self, "Integrated RA Receiver")

		##################################################
		# Parameters
		##################################################
		self.sky = sky
		self.dm = dm
		self.setisize = setisize
		self.freq = freq
		self.bandwidth = bandwidth
		self.ratepsr = ratepsr
		self.gain = gain
		self.devaddr = devaddr
		self.binwidth = binwidth
		self.subdev = subdev

		##################################################
		# Variables
		##################################################
		self.notch_len = notch_len = 64
		self.notch_freqs = notch_freqs = [0]
		self.ifreq = ifreq = freq
		self.idm = idm = dm
		self.skyfreq = skyfreq = sky
		self.seti_rate = seti_rate = 5
		self.pid = pid = iraconfig.writepid()
		self.inotches = inotches = iraconfig.compute_notches(notch_freqs,int(notch_len),bandwidth,ifreq,idm)
		self.igain = igain = gain
		self.cdoppler = cdoppler = 0.0

		##################################################
		# Blocks
		##################################################
		self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(("localhost", 11420), allow_none=True)
		self.xmlrpc_server_0.register_instance(self)
		threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()
		self.uhd_usrp_source_0 = uhd.usrp_source(
			device_addr=devaddr,
			stream_args=uhd.stream_args(
				cpu_format="fc32",
				channels=range(1),
			),
		)
		self.uhd_usrp_source_0.set_subdev_spec(subdev, 0)
		self.uhd_usrp_source_0.set_samp_rate(bandwidth)
		self.uhd_usrp_source_0.set_center_freq(ifreq+(cdoppler*skyfreq), 0)
		self.uhd_usrp_source_0.set_gain(igain, 0)
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, setisize)
		self.gr_single_pole_iir_filter_xx_1 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/setisize), setisize)
		self.gr_single_pole_iir_filter_xx_0 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/ratepsr), 1)
		self.gr_multiply_const_vxx_0 = gr.multiply_const_vcc((30e3, ))
		self.gr_keep_one_in_n_1 = gr.keep_one_in_n(gr.sizeof_float*setisize, int(bandwidth/setisize)/seti_rate)
		self.gr_keep_one_in_n_0 = gr.keep_one_in_n(gr.sizeof_float*1, int(bandwidth/ratepsr))
		self.gr_file_sink_1 = gr.file_sink(gr.sizeof_float*setisize, "ra_seti_fifo")
		self.gr_file_sink_1.set_unbuffered(True)
		self.gr_file_sink_0 = gr.file_sink(gr.sizeof_float*1, "ra_psr_fifo")
		self.gr_file_sink_0.set_unbuffered(True)
		self.gr_fft_vxx_0 = gr.fft_vcc(setisize, True, (window.blackmanharris(1024)), False, 3)
		self.gr_fft_filter_xxx_1 = gr.fft_filter_ccc(1, (inotches), 1)
		self.gr_fft_filter_xxx_0_0 = gr.fft_filter_ccc(1, (iraconfig.compute_dispfilter(idm,(1.0-cdoppler),bandwidth,skyfreq)), 3)
		self.gr_complex_to_mag_squared_0 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_0 = gr.complex_to_mag(setisize)

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_complex_to_mag_squared_0, 0), (self.gr_single_pole_iir_filter_xx_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_0, 0), (self.gr_keep_one_in_n_0, 0))
		self.connect((self.gr_keep_one_in_n_0, 0), (self.gr_file_sink_0, 0))
		self.connect((self.gr_stream_to_vector_0, 0), (self.gr_fft_vxx_0, 0))
		self.connect((self.gr_fft_vxx_0, 0), (self.gr_complex_to_mag_0, 0))
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_single_pole_iir_filter_xx_1, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1, 0), (self.gr_keep_one_in_n_1, 0))
		self.connect((self.gr_keep_one_in_n_1, 0), (self.gr_file_sink_1, 0))
		self.connect((self.gr_fft_filter_xxx_1, 0), (self.gr_stream_to_vector_0, 0))
		self.connect((self.gr_fft_filter_xxx_1, 0), (self.gr_fft_filter_xxx_0_0, 0))
		self.connect((self.gr_fft_filter_xxx_0_0, 0), (self.gr_complex_to_mag_squared_0, 0))
		self.connect((self.uhd_usrp_source_0, 0), (self.gr_multiply_const_vxx_0, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.gr_fft_filter_xxx_1, 0))

	def get_sky(self):
		return self.sky

	def set_sky(self, sky):
		self.sky = sky
		self.set_skyfreq(self.sky)

	def get_dm(self):
		return self.dm

	def set_dm(self, dm):
		self.dm = dm
		self.set_idm(self.dm)

	def get_setisize(self):
		return self.setisize

	def set_setisize(self, setisize):
		self.setisize = setisize
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/(self.bandwidth/self.setisize))
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)

	def get_freq(self):
		return self.freq

	def set_freq(self, freq):
		self.freq = freq
		self.set_ifreq(self.freq)

	def get_bandwidth(self):
		return self.bandwidth

	def set_bandwidth(self, bandwidth):
		self.bandwidth = bandwidth
		self.gr_keep_one_in_n_0.set_n(int(self.bandwidth/self.ratepsr))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/self.ratepsr))
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/(self.bandwidth/self.setisize))
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)
		self.uhd_usrp_source_0.set_samp_rate(self.bandwidth)
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

	def get_ratepsr(self):
		return self.ratepsr

	def set_ratepsr(self, ratepsr):
		self.ratepsr = ratepsr
		self.gr_keep_one_in_n_0.set_n(int(self.bandwidth/self.ratepsr))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/self.ratepsr))

	def get_gain(self):
		return self.gain

	def set_gain(self, gain):
		self.gain = gain
		self.set_igain(self.gain)

	def get_devaddr(self):
		return self.devaddr

	def set_devaddr(self, devaddr):
		self.devaddr = devaddr

	def get_binwidth(self):
		return self.binwidth

	def set_binwidth(self, binwidth):
		self.binwidth = binwidth

	def get_subdev(self):
		return self.subdev

	def set_subdev(self, subdev):
		self.subdev = subdev

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
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 0)

	def get_idm(self):
		return self.idm

	def set_idm(self, idm):
		self.idm = idm
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

	def get_skyfreq(self):
		return self.skyfreq

	def set_skyfreq(self, skyfreq):
		self.skyfreq = skyfreq
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 0)
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

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

	def get_cdoppler(self):
		return self.cdoppler

	def set_cdoppler(self, cdoppler):
		self.cdoppler = cdoppler
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 0)
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	parser.add_option("", "--sky", dest="sky", type="eng_float", default=eng_notation.num_to_str(1420.4058e6),
		help="Set sky [default=%default]")
	parser.add_option("", "--dm", dest="dm", type="eng_float", default=eng_notation.num_to_str(1500.0),
		help="Set dm [default=%default]")
	parser.add_option("", "--setisize", dest="setisize", type="intx", default=262144,
		help="Set setisize [default=%default]")
	parser.add_option("", "--freq", dest="freq", type="eng_float", default=eng_notation.num_to_str(1420.4058e6),
		help="Set freq [default=%default]")
	parser.add_option("", "--bandwidth", dest="bandwidth", type="eng_float", default=eng_notation.num_to_str(5.0e6),
		help="Set bandwidth [default=%default]")
	parser.add_option("", "--ratepsr", dest="ratepsr", type="eng_float", default=eng_notation.num_to_str(50000),
		help="Set ratepsr [default=%default]")
	parser.add_option("", "--gain", dest="gain", type="eng_float", default=eng_notation.num_to_str(55),
		help="Set gain [default=%default]")
	parser.add_option("", "--devaddr", dest="devaddr", type="string", default="addr=192.168.10.2",
		help="Set devaddr [default=%default]")
	parser.add_option("", "--binwidth", dest="binwidth", type="eng_float", default=eng_notation.num_to_str(20),
		help="Set binwidth [default=%default]")
	parser.add_option("", "--subdev", dest="subdev", type="string", default="A:A",
		help="Set A:A [default=%default]")
	(options, args) = parser.parse_args()
	tb = uhd_ra_receiver_new(sky=options.sky, dm=options.dm, setisize=options.setisize, freq=options.freq, bandwidth=options.bandwidth, ratepsr=options.ratepsr, gain=options.gain, devaddr=options.devaddr, binwidth=options.binwidth, subdev=options.subdev)
	tb.run()

