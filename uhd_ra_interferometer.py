#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated RA Receiver
# Author: Marcus Leech
# Description: An RA receiver
# Generated: Mon Jul 30 17:16:58 2012
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
import threading
import time

class uhd_ra_interferometer(gr.top_block):

	def __init__(self, sky=1420.4058e6, dm=1500.0, setisize=262144, freq=1420.4058e6, bandwidth=5.0e6, gain=55, devaddr="addr=192.168.10.2", binwidth=20, ratepsr=50000, rateinter=20, subdev="A:A B:A", cora=1.0, corb=1.0, phcorr=0.0, pdelay=0):
		gr.top_block.__init__(self, "Integrated RA Receiver")

		##################################################
		# Parameters
		##################################################
		self.sky = sky
		self.dm = dm
		self.setisize = setisize
		self.freq = freq
		self.bandwidth = bandwidth
		self.gain = gain
		self.devaddr = devaddr
		self.binwidth = binwidth
		self.ratepsr = ratepsr
		self.rateinter = rateinter
		self.subdev = subdev
		self.cora = cora
		self.corb = corb
		self.phcorr = phcorr
		self.pdelay = pdelay

		##################################################
		# Variables
		##################################################
		self.channel_ratio = channel_ratio = 1.0
		self.notch_len = notch_len = 64
		self.notch_freqs = notch_freqs = [0]
		self.ifreq = ifreq = freq
		self.idm = idm = dm
		self.cr = cr = 1.0 if (channel_ratio > 20.0 or channel_ratio < 0.05 )else channel_ratio
		self.skyfreq = skyfreq = sky
		self.seti_rate = seti_rate = 5
		self.pid = pid = iraconfig.writepid()
		self.phi = phi = phcorr*(3.14159/180.0)
		self.inotches = inotches = iraconfig.compute_notches(notch_freqs,int(notch_len),bandwidth,ifreq,idm)
		self.igain = igain = gain
		self.delay = delay = int(pdelay)
		self.cdoppler = cdoppler = 0.0
		self.autob = autob = 1.0 if cr < 1.0 else math.sqrt(cr)
		self.autoa = autoa = 1.0 if cr > 1.0 else math.sqrt(cr)

		##################################################
		# Blocks
		##################################################
		self.ratio_probe = gr.probe_signal_f()
		self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(("localhost", 11420), allow_none=True)
		self.xmlrpc_server_0.register_instance(self)
		threading.Thread(target=self.xmlrpc_server_0.serve_forever).start()
		self.uhd_usrp_source_0 = uhd.usrp_source(
			device_addr=devaddr,
			stream_args=uhd.stream_args(
				cpu_format="fc32",
				channels=range(2),
			),
		)
		self.uhd_usrp_source_0.set_subdev_spec(subdev, 0)
		self.uhd_usrp_source_0.set_samp_rate(bandwidth)
		self.uhd_usrp_source_0.set_center_freq(ifreq+(cdoppler*skyfreq), 0)
		self.uhd_usrp_source_0.set_gain(igain, 0)
		self.uhd_usrp_source_0.set_center_freq(ifreq+(cdoppler*skyfreq), 1)
		self.uhd_usrp_source_0.set_gain(igain, 1)
		self.uhd_usrp_source_0.set_bandwidth(bandwidth, 1)
		self.gr_vector_to_stream_0 = gr.vector_to_stream(gr.sizeof_gr_complex*1, setisize)
		self.gr_stream_to_vector_1 = gr.stream_to_vector(gr.sizeof_gr_complex*1, setisize)
		self.gr_stream_to_vector_0_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, setisize)
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, setisize)
		self.gr_single_pole_iir_filter_xx_2_0 = gr.single_pole_iir_filter_ff(1.0/(bandwidth*4), 1)
		self.gr_single_pole_iir_filter_xx_2 = gr.single_pole_iir_filter_ff(1.0/(bandwidth*4), 1)
		self.gr_single_pole_iir_filter_xx_1 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/setisize), setisize)
		self.gr_single_pole_iir_filter_xx_0_0 = gr.single_pole_iir_filter_cc(1.0/(bandwidth/rateinter), 1)
		self.gr_single_pole_iir_filter_xx_0 = gr.single_pole_iir_filter_ff(1.0/(bandwidth/ratepsr), 1)
		self.gr_multiply_xx_1 = gr.multiply_vcc(setisize)
		self.gr_multiply_xx_0 = gr.multiply_vcc(1)
		self.gr_multiply_const_vxx_1 = gr.multiply_const_vcc((complex(math.cos(phi),math.sin(phi)), ))
		self.gr_multiply_const_vxx_0_0 = gr.multiply_const_vcc((30e3*(corb)*autob, ))
		self.gr_multiply_const_vxx_0 = gr.multiply_const_vcc((30e3*(cora)*autoa, ))
		self.gr_keep_one_in_n_2 = gr.keep_one_in_n(gr.sizeof_float*1, int(bandwidth/2))
		self.gr_keep_one_in_n_1 = gr.keep_one_in_n(gr.sizeof_float*setisize, int(bandwidth/setisize)/seti_rate)
		self.gr_keep_one_in_n_0_0 = gr.keep_one_in_n(gr.sizeof_gr_complex*1, int(bandwidth/rateinter))
		self.gr_keep_one_in_n_0 = gr.keep_one_in_n(gr.sizeof_float*1, int(bandwidth/ratepsr))
		self.gr_file_sink_2 = gr.file_sink(gr.sizeof_gr_complex*1, "ra_inter_fifo")
		self.gr_file_sink_2.set_unbuffered(True)
		self.gr_file_sink_1 = gr.file_sink(gr.sizeof_float*setisize, "ra_seti_fifo")
		self.gr_file_sink_1.set_unbuffered(True)
		self.gr_file_sink_0 = gr.file_sink(gr.sizeof_float*1, "ra_psr_fifo")
		self.gr_file_sink_0.set_unbuffered(True)
		self.gr_fft_vxx_0_0 = gr.fft_vcc(setisize, True, (window.blackmanharris(1024)), False, 2)
		self.gr_fft_vxx_0 = gr.fft_vcc(setisize, True, (window.blackmanharris(1024)), False, 2)
		self.gr_fft_filter_xxx_1_0 = gr.fft_filter_ccc(1, (inotches), 1)
		self.gr_fft_filter_xxx_1 = gr.fft_filter_ccc(1, (inotches), 1)
		self.gr_fft_filter_xxx_0_0_0 = gr.fft_filter_ccc(1, (iraconfig.compute_dispfilter(idm,(1.0-cdoppler),bandwidth,skyfreq)), 2)
		self.gr_fft_filter_xxx_0_0 = gr.fft_filter_ccc(1, (iraconfig.compute_dispfilter(idm,(1.0-cdoppler),bandwidth,skyfreq)), 2)
		self.gr_divide_xx_0 = gr.divide_ff(1)
		self.gr_delay_0 = gr.delay(gr.sizeof_gr_complex*1, delay)
		self.gr_conjugate_cc_1 = gr.conjugate_cc()
		self.gr_conjugate_cc_0 = gr.conjugate_cc()
		self.gr_complex_to_mag_squared_1_0 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_squared_1 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_squared_0_0 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_squared_0 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_0 = gr.complex_to_mag(setisize)
		self.gr_add_xx_0 = gr.add_vff(1)
		self.gr_add_const_vxx_0_0 = gr.add_const_vff((1.0e-9, ))
		self.gr_add_const_vxx_0 = gr.add_const_vff((1.0e-9, ))
		def _channel_ratio_probe():
			while True:
				val = self.ratio_probe.level()
				try: self.set_channel_ratio(val)
				except AttributeError, e: pass
				time.sleep(1.0/(2))
		_channel_ratio_thread = threading.Thread(target=_channel_ratio_probe)
		_channel_ratio_thread.daemon = True
		_channel_ratio_thread.start()

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_single_pole_iir_filter_xx_0, 0), (self.gr_keep_one_in_n_0, 0))
		self.connect((self.gr_keep_one_in_n_0, 0), (self.gr_file_sink_0, 0))
		self.connect((self.gr_stream_to_vector_0, 0), (self.gr_fft_vxx_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1, 0), (self.gr_keep_one_in_n_1, 0))
		self.connect((self.gr_keep_one_in_n_1, 0), (self.gr_file_sink_1, 0))
		self.connect((self.gr_fft_filter_xxx_1, 0), (self.gr_fft_filter_xxx_0_0, 0))
		self.connect((self.gr_fft_filter_xxx_0_0, 0), (self.gr_complex_to_mag_squared_0, 0))
		self.connect((self.uhd_usrp_source_0, 0), (self.gr_multiply_const_vxx_0, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.gr_fft_filter_xxx_1, 0))
		self.connect((self.uhd_usrp_source_0, 1), (self.gr_multiply_const_vxx_0_0, 0))
		self.connect((self.gr_multiply_const_vxx_0_0, 0), (self.gr_multiply_const_vxx_1, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.gr_conjugate_cc_0, 0))
		self.connect((self.gr_conjugate_cc_0, 0), (self.gr_multiply_xx_0, 1))
		self.connect((self.gr_multiply_xx_0, 0), (self.gr_single_pole_iir_filter_xx_0_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_0_0, 0), (self.gr_keep_one_in_n_0_0, 0))
		self.connect((self.gr_keep_one_in_n_0_0, 0), (self.gr_file_sink_2, 0))
		self.connect((self.gr_multiply_const_vxx_0_0, 0), (self.gr_fft_filter_xxx_1_0, 0))
		self.connect((self.gr_fft_filter_xxx_1_0, 0), (self.gr_fft_filter_xxx_0_0_0, 0))
		self.connect((self.gr_complex_to_mag_squared_0, 0), (self.gr_add_xx_0, 0))
		self.connect((self.gr_add_xx_0, 0), (self.gr_single_pole_iir_filter_xx_0, 0))
		self.connect((self.gr_complex_to_mag_squared_0_0, 0), (self.gr_add_xx_0, 1))
		self.connect((self.gr_fft_filter_xxx_0_0_0, 0), (self.gr_complex_to_mag_squared_0_0, 0))
		self.connect((self.gr_fft_filter_xxx_1, 0), (self.gr_stream_to_vector_0, 0))
		self.connect((self.gr_stream_to_vector_0_0, 0), (self.gr_fft_vxx_0_0, 0))
		self.connect((self.gr_fft_filter_xxx_1_0, 0), (self.gr_stream_to_vector_0_0, 0))
		self.connect((self.gr_fft_vxx_0, 0), (self.gr_multiply_xx_1, 0))
		self.connect((self.gr_fft_vxx_0_0, 0), (self.gr_vector_to_stream_0, 0))
		self.connect((self.gr_vector_to_stream_0, 0), (self.gr_conjugate_cc_1, 0))
		self.connect((self.gr_conjugate_cc_1, 0), (self.gr_stream_to_vector_1, 0))
		self.connect((self.gr_stream_to_vector_1, 0), (self.gr_multiply_xx_1, 1))
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_single_pole_iir_filter_xx_1, 0))
		self.connect((self.gr_multiply_xx_1, 0), (self.gr_complex_to_mag_0, 0))
		self.connect((self.uhd_usrp_source_0, 1), (self.gr_complex_to_mag_squared_1, 0))
		self.connect((self.uhd_usrp_source_0, 0), (self.gr_complex_to_mag_squared_1_0, 0))
		self.connect((self.gr_complex_to_mag_squared_1_0, 0), (self.gr_single_pole_iir_filter_xx_2_0, 0))
		self.connect((self.gr_complex_to_mag_squared_1, 0), (self.gr_single_pole_iir_filter_xx_2, 0))
		self.connect((self.gr_divide_xx_0, 0), (self.gr_keep_one_in_n_2, 0))
		self.connect((self.gr_keep_one_in_n_2, 0), (self.ratio_probe, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_2, 0), (self.gr_add_const_vxx_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_2_0, 0), (self.gr_add_const_vxx_0_0, 0))
		self.connect((self.gr_add_const_vxx_0, 0), (self.gr_divide_xx_0, 0))
		self.connect((self.gr_add_const_vxx_0_0, 0), (self.gr_divide_xx_0, 1))
		self.connect((self.gr_multiply_const_vxx_1, 0), (self.gr_delay_0, 0))
		self.connect((self.gr_delay_0, 0), (self.gr_multiply_xx_0, 0))

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
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_keep_one_in_n_0.set_n(int(self.bandwidth/self.ratepsr))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/self.ratepsr))
		self.gr_keep_one_in_n_0_0.set_n(int(self.bandwidth/self.rateinter))
		self.gr_single_pole_iir_filter_xx_0_0.set_taps(1.0/(self.bandwidth/self.rateinter))
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/(self.bandwidth/self.setisize))
		self.uhd_usrp_source_0.set_samp_rate(self.bandwidth)
		self.uhd_usrp_source_0.set_bandwidth(self.bandwidth, 1)
		self.gr_keep_one_in_n_2.set_n(int(self.bandwidth/2))
		self.gr_single_pole_iir_filter_xx_2.set_taps(1.0/(self.bandwidth*4))
		self.gr_single_pole_iir_filter_xx_2_0.set_taps(1.0/(self.bandwidth*4))
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))
		self.gr_fft_filter_xxx_0_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

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

	def get_ratepsr(self):
		return self.ratepsr

	def set_ratepsr(self, ratepsr):
		self.ratepsr = ratepsr
		self.gr_keep_one_in_n_0.set_n(int(self.bandwidth/self.ratepsr))
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.bandwidth/self.ratepsr))

	def get_rateinter(self):
		return self.rateinter

	def set_rateinter(self, rateinter):
		self.rateinter = rateinter
		self.gr_keep_one_in_n_0_0.set_n(int(self.bandwidth/self.rateinter))
		self.gr_single_pole_iir_filter_xx_0_0.set_taps(1.0/(self.bandwidth/self.rateinter))

	def get_subdev(self):
		return self.subdev

	def set_subdev(self, subdev):
		self.subdev = subdev

	def get_cora(self):
		return self.cora

	def set_cora(self, cora):
		self.cora = cora
		self.gr_multiply_const_vxx_0.set_k((30e3*(self.cora)*self.autoa, ))

	def get_corb(self):
		return self.corb

	def set_corb(self, corb):
		self.corb = corb
		self.gr_multiply_const_vxx_0_0.set_k((30e3*(self.corb)*self.autob, ))

	def get_phcorr(self):
		return self.phcorr

	def set_phcorr(self, phcorr):
		self.phcorr = phcorr
		self.set_phi(self.phcorr*(3.14159/180.0))

	def get_pdelay(self):
		return self.pdelay

	def set_pdelay(self, pdelay):
		self.pdelay = pdelay
		self.set_delay(int(self.pdelay))

	def get_channel_ratio(self):
		return self.channel_ratio

	def set_channel_ratio(self, channel_ratio):
		self.channel_ratio = channel_ratio
		self.set_cr(1.0 if (self.channel_ratio > 20.0 or self.channel_ratio < 0.05 )else self.channel_ratio)

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
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 1)

	def get_idm(self):
		return self.idm

	def set_idm(self, idm):
		self.idm = idm
		self.set_inotches(iraconfig.compute_notches(self.notch_freqs,int(self.notch_len),self.bandwidth,self.ifreq,self.idm))
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))
		self.gr_fft_filter_xxx_0_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

	def get_cr(self):
		return self.cr

	def set_cr(self, cr):
		self.cr = cr
		self.set_autoa(1.0 if self.cr > 1.0 else math.sqrt(self.cr))
		self.set_autob(1.0 if self.cr < 1.0 else math.sqrt(self.cr))

	def get_skyfreq(self):
		return self.skyfreq

	def set_skyfreq(self, skyfreq):
		self.skyfreq = skyfreq
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 0)
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 1)
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))
		self.gr_fft_filter_xxx_0_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

	def get_seti_rate(self):
		return self.seti_rate

	def set_seti_rate(self, seti_rate):
		self.seti_rate = seti_rate
		self.gr_keep_one_in_n_1.set_n(int(self.bandwidth/self.setisize)/self.seti_rate)

	def get_pid(self):
		return self.pid

	def set_pid(self, pid):
		self.pid = pid

	def get_phi(self):
		return self.phi

	def set_phi(self, phi):
		self.phi = phi
		self.gr_multiply_const_vxx_1.set_k((complex(math.cos(self.phi),math.sin(self.phi)), ))

	def get_inotches(self):
		return self.inotches

	def set_inotches(self, inotches):
		self.inotches = inotches
		self.gr_fft_filter_xxx_1.set_taps((self.inotches))
		self.gr_fft_filter_xxx_1_0.set_taps((self.inotches))

	def get_igain(self):
		return self.igain

	def set_igain(self, igain):
		self.igain = igain
		self.uhd_usrp_source_0.set_gain(self.igain, 0)
		self.uhd_usrp_source_0.set_gain(self.igain, 1)

	def get_delay(self):
		return self.delay

	def set_delay(self, delay):
		self.delay = delay
		self.gr_delay_0.set_delay(self.delay)

	def get_cdoppler(self):
		return self.cdoppler

	def set_cdoppler(self, cdoppler):
		self.cdoppler = cdoppler
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 0)
		self.uhd_usrp_source_0.set_center_freq(self.ifreq+(self.cdoppler*self.skyfreq), 1)
		self.gr_fft_filter_xxx_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))
		self.gr_fft_filter_xxx_0_0_0.set_taps((iraconfig.compute_dispfilter(self.idm,(1.0-self.cdoppler),self.bandwidth,self.skyfreq)))

	def get_autob(self):
		return self.autob

	def set_autob(self, autob):
		self.autob = autob
		self.gr_multiply_const_vxx_0_0.set_k((30e3*(self.corb)*self.autob, ))

	def get_autoa(self):
		return self.autoa

	def set_autoa(self, autoa):
		self.autoa = autoa
		self.gr_multiply_const_vxx_0.set_k((30e3*(self.cora)*self.autoa, ))

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
	parser.add_option("", "--gain", dest="gain", type="eng_float", default=eng_notation.num_to_str(55),
		help="Set gain [default=%default]")
	parser.add_option("", "--devaddr", dest="devaddr", type="string", default="addr=192.168.10.2",
		help="Set devaddr [default=%default]")
	parser.add_option("", "--binwidth", dest="binwidth", type="eng_float", default=eng_notation.num_to_str(20),
		help="Set binwidth [default=%default]")
	parser.add_option("", "--ratepsr", dest="ratepsr", type="eng_float", default=eng_notation.num_to_str(50000),
		help="Set ratepsr [default=%default]")
	parser.add_option("", "--rateinter", dest="rateinter", type="eng_float", default=eng_notation.num_to_str(20),
		help="Set rateinter [default=%default]")
	parser.add_option("", "--subdev", dest="subdev", type="string", default="A:A B:A",
		help="Set A:A B:A [default=%default]")
	parser.add_option("", "--cora", dest="cora", type="eng_float", default=eng_notation.num_to_str(1.0),
		help="Set cora [default=%default]")
	parser.add_option("", "--corb", dest="corb", type="eng_float", default=eng_notation.num_to_str(1.0),
		help="Set corb [default=%default]")
	parser.add_option("", "--phcorr", dest="phcorr", type="eng_float", default=eng_notation.num_to_str(0.0),
		help="Set phcorr [default=%default]")
	parser.add_option("", "--pdelay", dest="pdelay", type="eng_float", default=eng_notation.num_to_str(0),
		help="Set pdelay [default=%default]")
	(options, args) = parser.parse_args()
	tb = uhd_ra_interferometer(sky=options.sky, dm=options.dm, setisize=options.setisize, freq=options.freq, bandwidth=options.bandwidth, gain=options.gain, devaddr=options.devaddr, binwidth=options.binwidth, ratepsr=options.ratepsr, rateinter=options.rateinter, subdev=options.subdev, cora=options.cora, corb=options.corb, phcorr=options.phcorr, pdelay=options.pdelay)
	tb.run()

