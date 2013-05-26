#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated Radio Astronomy Receiver
# Author: Marcus Leech
# Description: gnuradio flow graph
# Generated: Mon May  4 22:02:55 2009
##################################################

from gnuradio import gr
from gnuradio import window
from gnuradio.gr import firdes
from grc_gnuradio import usrp as grc_usrp
from optparse import OptionParser
import math
import numpy
import os
import signal
import sys
import time

class integrated_ra_receiver_nohb(gr.top_block):

	def __init__(self, freq=1420.4058e6, gain=55, setisize=4096, ratetp=100, nopsr=0, noseti=0, decim=16, side="A", ratepsr=8000):
		gr.top_block.__init__(self, "Integrated Radio Astronomy Receiver")

		##################################################
		# Parameters
		##################################################
		self.freq = freq
		self.gain = gain
		self.setisize = setisize
		self.ratetp = ratetp
		self.nopsr = nopsr
		self.noseti = noseti
		self.decim = decim
		self.side = side
		self.ratepsr = ratepsr

		##################################################
		# Variables
		##################################################
		self.taps = taps = gr.firdes.low_pass(1.0, 1.0, (1.0/8.0)*0.45, (1.0/8.0)*0.1, gr.firdes.WIN_HANN)
		self.input_rate = input_rate = int(64e6/decim)

		##################################################
		# Blocks
		##################################################
		self.gr_complex_to_mag_0 = gr.complex_to_mag(setisize)
		self.gr_complex_to_mag_squared_0 = gr.complex_to_mag_squared(1)
		self.gr_fft_vxx_0 = gr.fft_vcc(setisize, True, (window.blackmanharris(setisize)), False)
		self.gr_file_sink_0_0 = gr.file_sink(gr.sizeof_float*setisize, "ra_seti_fifo")
		self.gr_file_sink_1 = gr.file_sink(gr.sizeof_float*1, "ra_psr_fifo")
		self.gr_keep_one_in_n_0 = gr.keep_one_in_n(gr.sizeof_float*1, int(input_rate/ratepsr))
		self.gr_single_pole_iir_filter_xx_0 = gr.single_pole_iir_filter_ff(1.0/(input_rate/(ratepsr/2.0)), 1)
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, setisize)
		self.notch_filter_a = gr.fft_filter_ccc(1, (complex(1.0,1.0), ))
		self.usrp_single_input = grc_usrp.simple_source_c(which=0, side='A', rx_ant='RXA', no_hb=True)
		self.usrp_single_input.set_format(width=8, shift=8)
		self.usrp_single_input.set_decim_rate(decim)
		self.usrp_single_input.set_frequency(freq, verbose=True)
		self.usrp_single_input.set_gain(gain)

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_stream_to_vector_0, 0), (self.gr_fft_vxx_0, 0))
		self.connect((self.gr_fft_vxx_0, 0), (self.gr_complex_to_mag_0, 0))
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_file_sink_0_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_0, 0), (self.gr_keep_one_in_n_0, 0))
		self.connect((self.gr_complex_to_mag_squared_0, 0), (self.gr_single_pole_iir_filter_xx_0, 0))
		self.connect((self.gr_keep_one_in_n_0, 0), (self.gr_file_sink_1, 0))
		self.connect((self.usrp_single_input, 0), (self.notch_filter_a, 0))
		self.connect((self.notch_filter_a, 0), (self.gr_complex_to_mag_squared_0, 0))
		self.connect((self.notch_filter_a, 0), (self.gr_stream_to_vector_0, 0))

	def set_freq(self, freq):
		self.freq = freq
		self.usrp_single_input.set_frequency(self.freq)

	def set_gain(self, gain):
		self.gain = gain
		self.usrp_single_input.set_gain(self.gain)

	def set_setisize(self, setisize):
		self.setisize = setisize

	def set_ratetp(self, ratetp):
		self.ratetp = ratetp

	def set_nopsr(self, nopsr):
		self.nopsr = nopsr

	def set_noseti(self, noseti):
		self.noseti = noseti

	def set_decim(self, decim):
		self.decim = decim
		self.set_input_rate(int(64e6/self.decim))
		self.usrp_single_input.set_decim_rate(self.decim)

	def set_side(self, side):
		self.side = side

	def set_ratepsr(self, ratepsr):
		self.ratepsr = ratepsr
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.input_rate/(self.ratepsr/2.0)))
		self.gr_keep_one_in_n_0.set_n(int(self.input_rate/self.ratepsr))

	def set_taps(self, taps):
		self.taps = taps

	def set_input_rate(self, input_rate):
		self.input_rate = input_rate
		self.gr_single_pole_iir_filter_xx_0.set_taps(1.0/(self.input_rate/(self.ratepsr/2.0)))
		self.gr_keep_one_in_n_0.set_n(int(self.input_rate/self.ratepsr))

if __name__ == '__main__':
	parser = OptionParser()
	parser.add_option("--freq", dest="freq", type="float", default=1420.4058e6)
	parser.add_option("--gain", dest="gain", type="int", default=55)
	parser.add_option("--setisize", dest="setisize", type="int", default=4096)
	parser.add_option("--ratetp", dest="ratetp", type="int", default=100)
	parser.add_option("--nopsr", dest="nopsr", type="int", default=0)
	parser.add_option("--noseti", dest="noseti", type="int", default=0)
	parser.add_option("--decim", dest="decim", type="int", default=16)
	parser.add_option("--side", dest="side", type="string", default="A")
	parser.add_option("--ratepsr", dest="ratepsr", type="int", default=8000)
	(options, args) = parser.parse_args()
	tb = integrated_ra_receiver_nohb(freq=options.freq, gain=options.gain, setisize=options.setisize, ratetp=options.ratetp, nopsr=options.nopsr, noseti=options.noseti, decim=options.decim, side=options.side, ratepsr=options.ratepsr)
	tb.start()
	raw_input('Press Enter to quit: ')
	tb.stop()

