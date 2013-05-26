#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Integrated Radio Astronomy Receiver
# Author: Marcus Leech
# Description: gnuradio flow graph
# Generated: Fri Apr  3 14:16:44 2009
##################################################

from gnuradio import gr
from gnuradio import window
from gnuradio.gr import firdes
from grc_gnuradio import usrp as grc_usrp
from optparse import OptionParser
import numpy
import os
import signal
import sys
import time

class integrated_ra_receiver_split(gr.top_block):

	def __init__(self, freq=1420.4058e6, gain=55, setisize=4096, ratepsr=8000, ratetp=100, decim=16, g_cor_a=1.0, g_cor_b=1.0):
		gr.top_block.__init__(self, "Integrated Radio Astronomy Receiver")

		##################################################
		# Parameters
		##################################################
		self.freq = freq
		self.gain = gain
		self.setisize = setisize
		self.ratepsr = ratepsr
		self.ratetp = ratetp
		self.decim = decim
		self.g_cor_a = g_cor_a
		self.g_cor_b = g_cor_b

		##################################################
		# Variables
		##################################################
		self.input_rate = input_rate = int(64e6/decim)

		##################################################
		# Blocks
		##################################################
		self.gr_complex_to_mag_0 = gr.complex_to_mag(setisize)
		self.gr_complex_to_mag_squared_0_4_0_1 = gr.complex_to_mag_squared(1)
		self.gr_complex_to_mag_squared_0_4_0_1_0 = gr.complex_to_mag_squared(1)
		self.gr_fft_vxx_0 = gr.fft_vcc(setisize, True, (window.blackmanharris(setisize)), False)
		self.gr_file_sink_0_0 = gr.file_sink(gr.sizeof_float*setisize, "ra_seti_fifo")
		self.gr_file_sink_2 = gr.file_sink(gr.sizeof_float*1, "ra_psr_fifo")
		self.gr_file_sink_2_0 = gr.file_sink(gr.sizeof_float*1, "ra_validation_fifo")
		self.gr_keep_one_in_n_1 = gr.keep_one_in_n(gr.sizeof_float*1, int((input_rate)/ratepsr))
		self.gr_keep_one_in_n_1_0 = gr.keep_one_in_n(gr.sizeof_float*1, int((input_rate)/ratepsr))
		self.gr_multiply_const_vxx_0 = gr.multiply_const_vcc((complex(g_cor_a,g_cor_a), ))
		self.gr_multiply_const_vxx_0_0 = gr.multiply_const_vcc((complex(g_cor_b,g_cor_b), ))
		self.gr_single_pole_iir_filter_xx_1 = gr.single_pole_iir_filter_ff(1.0/((input_rate)/(ratepsr/2)), 1)
		self.gr_single_pole_iir_filter_xx_1_0 = gr.single_pole_iir_filter_ff(1.0/((input_rate)/(ratepsr/2)), 1)
		self.gr_stream_to_vector_0 = gr.stream_to_vector(gr.sizeof_gr_complex*1, setisize)
		self.notch_filter_a = gr.fft_filter_ccc(1, (complex(1,1), ))
		self.notch_filter_b = gr.fft_filter_ccc(1, (complex(1,1), ))
		self.usrp_dual_input = grc_usrp.dual_source_c(which=0, rx_ant_a='RXA', rx_ant_b='RXA')
		self.usrp_dual_input.set_format(width=8, shift=0)
		self.usrp_dual_input.set_decim_rate(decim)
		self.usrp_dual_input.set_frequency_a(freq, verbose=True)
		self.usrp_dual_input.set_frequency_b(freq, verbose=True)
		self.usrp_dual_input.set_gain_a(gain)
		self.usrp_dual_input.set_gain_b(gain)

		##################################################
		# Connections
		##################################################
		self.connect((self.gr_stream_to_vector_0, 0), (self.gr_fft_vxx_0, 0))
		self.connect((self.gr_fft_vxx_0, 0), (self.gr_complex_to_mag_0, 0))
		self.connect((self.gr_complex_to_mag_0, 0), (self.gr_file_sink_0_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1, 0), (self.gr_keep_one_in_n_1, 0))
		self.connect((self.gr_keep_one_in_n_1, 0), (self.gr_file_sink_2, 0))
		self.connect((self.usrp_dual_input, 0), (self.notch_filter_a, 0))
		self.connect((self.notch_filter_a, 0), (self.gr_multiply_const_vxx_0, 0))
		self.connect((self.usrp_dual_input, 1), (self.notch_filter_b, 0))
		self.connect((self.notch_filter_b, 0), (self.gr_multiply_const_vxx_0_0, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.gr_complex_to_mag_squared_0_4_0_1, 0))
		self.connect((self.gr_multiply_const_vxx_0_0, 0), (self.gr_complex_to_mag_squared_0_4_0_1_0, 0))
		self.connect((self.gr_complex_to_mag_squared_0_4_0_1, 0), (self.gr_single_pole_iir_filter_xx_1, 0))
		self.connect((self.gr_complex_to_mag_squared_0_4_0_1_0, 0), (self.gr_single_pole_iir_filter_xx_1_0, 0))
		self.connect((self.gr_single_pole_iir_filter_xx_1_0, 0), (self.gr_keep_one_in_n_1_0, 0))
		self.connect((self.gr_keep_one_in_n_1_0, 0), (self.gr_file_sink_2_0, 0))
		self.connect((self.gr_multiply_const_vxx_0, 0), (self.gr_stream_to_vector_0, 0))

	def set_freq(self, freq):
		self.freq = freq
		self.usrp_dual_input.set_frequency_a(self.freq)
		self.usrp_dual_input.set_frequency_b(self.freq)

	def set_gain(self, gain):
		self.gain = gain
		self.usrp_dual_input.set_gain_a(self.gain)
		self.usrp_dual_input.set_gain_b(self.gain)

	def set_setisize(self, setisize):
		self.setisize = setisize

	def set_ratepsr(self, ratepsr):
		self.ratepsr = ratepsr
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/((self.input_rate)/(self.ratepsr/2)))
		self.gr_single_pole_iir_filter_xx_1_0.set_taps(1.0/((self.input_rate)/(self.ratepsr/2)))
		self.gr_keep_one_in_n_1.set_n(int((self.input_rate)/self.ratepsr))
		self.gr_keep_one_in_n_1_0.set_n(int((self.input_rate)/self.ratepsr))

	def set_ratetp(self, ratetp):
		self.ratetp = ratetp

	def set_decim(self, decim):
		self.decim = decim
		self.set_input_rate(int(64e6/self.decim))
		self.usrp_dual_input.set_decim_rate(self.decim)

	def set_g_cor_a(self, g_cor_a):
		self.g_cor_a = g_cor_a
		self.gr_multiply_const_vxx_0.set_k((complex(self.g_cor_a,self.g_cor_a), ))

	def set_g_cor_b(self, g_cor_b):
		self.g_cor_b = g_cor_b
		self.gr_multiply_const_vxx_0_0.set_k((complex(self.g_cor_b,self.g_cor_b), ))

	def set_input_rate(self, input_rate):
		self.input_rate = input_rate
		self.gr_single_pole_iir_filter_xx_1.set_taps(1.0/((self.input_rate)/(self.ratepsr/2)))
		self.gr_single_pole_iir_filter_xx_1_0.set_taps(1.0/((self.input_rate)/(self.ratepsr/2)))
		self.gr_keep_one_in_n_1.set_n(int((self.input_rate)/self.ratepsr))
		self.gr_keep_one_in_n_1_0.set_n(int((self.input_rate)/self.ratepsr))
	def external_gain_and_freq(self, gain, freq):
		if (abs(self.freq - freq) > 10.0):
			print >>sys.stderr, "Changing freq from ", self.freq, "to ", freq
			self.freq = freq
			self.set_freq(self.freq)
			
		if (int(self.gain) != int(gain)):
			print >>sys.stderr, "Changing gain from ", self.gain, "to ", gain
			self.gain = gain
			self.set_gain(self.gain)

	def compute_notch_taps(self,notchlist,bw,freq,flen):
		tmptaps = numpy.zeros(flen, dtype=numpy.complex)
		binwidth = bw / flen
		for i in range(0,flen):
			tmptaps[i] = complex(1.0,0.0)

		for i in notchlist:
			diff = i - freq
			
			if int(i) == 0:
				break

			if ((i < (freq - bw/2)) or (i > (freq + bw/2))):
				continue
				
			if (diff > 0):
				idx = diff / binwidth
				idx = round(idx)
				idx = int(idx)
				
				if (idx < 0 or idx > (flen/2)):
					break
					
				tmptaps[idx] = complex(0.0, 0.0)

			if (diff < 0):
				idx = -diff / binwidth
				idx = round(idx)
				idx = (flen/2) - idx
				idx = int(idx+(flen/2))
				
				if (idx < 0 or idx >= (flen)):
					break
					
				tmptaps[idx] = complex(0.0, 0.0)

		return (numpy.fft.ifft(tmptaps))

	def set_notch_taps(self,taps):
		 self.notch_filter_a.set_taps(taps)
		 self.notch_filter_b.set_taps(taps)



if __name__ == '__main__':
	parser = OptionParser()
	parser.add_option("--freq", dest="freq", type="float", default=1420.4058e6)
	parser.add_option("--gain", dest="gain", type="int", default=55)
	parser.add_option("--setisize", dest="setisize", type="int", default=4096)
	parser.add_option("--ratepsr", dest="ratepsr", type="int", default=8000)
	parser.add_option("--ratetp", dest="ratetp", type="int", default=100)
	parser.add_option("--decim", dest="decim", type="int", default=16)
	parser.add_option("--g_cor_a", dest="g_cor_a", type="float", default=1.0)
	parser.add_option("--g_cor_b", dest="g_cor_b", type="float", default=1.0)
	(options, args) = parser.parse_args()
	tb = integrated_ra_receiver_split(freq=options.freq, gain=options.gain, setisize=options.setisize, ratepsr=options.ratepsr, ratetp=options.ratetp, decim=options.decim, g_cor_a=options.g_cor_a, g_cor_b=options.g_cor_b)
	tb.start()
	NOTCH_SIZE=64
	nsz = os.getenv ("RCVR_NOTCH_SIZE")
	if (nsz != None):
		NOTCH_SIZE=int(nsz)
	pid = os.getpid()
	pidfile = open("receiver.pid","w")
	pidfile.write (str(pid) + "\n")
	pidfile.close()
	oldtaps = complex(0,0)
	ots = "99:99:99"
	n = os.getenv ("RCVR_NOTCHES")
	if n != None:
		notches = n.split(",")
		for i in range(0,len(notches)):
			notches[i] = float(notches[i])
		taps = tb.compute_notch_taps (notches, tb.setisize, tb.freq, NOTCH_SIZE)
		tb.set_notch_taps(taps)
	
	gc_a = 1.0
	gc_b = 1.0
	while True:
		time.sleep(0.25)
		try:
			file = open("rcvr_control_file", "r")
		except:
			continue
			
		str = file.read()
		if (len(str) < 5):
			file.close()
			continue
		file.close()
		t = str.split()
		if (len(t) < 4):
			continue
		
		if (len(t) > 4):
			if (gc_a != float(t[4])):
				tb.set_g_cor_a(float(t[4]))
				print >>sys.stderr, "Changing GC_A from ", gc_a, "to ", float(t[4])
				gc_a = float(t[4])
				
			if (gc_b != float(t[5])):
				tb.set_g_cor_b(float(t[5]))
				print >>sys.stderr, "Changing GC_B from ", gc_b, "to ", float(t[5])
				gc_b = float(t[5])
			
		myfreq = float(t[0])
		mygain = int(t[1])
		runrun = int(t[2])
		mydm = float(t[3])
		
		if (runrun <= 0):
			break
		tb.external_gain_and_freq(mygain, myfreq)
		
		try:
			file = open ("rcvr_notches_file", "r")
		except:
			continue
			
		str = file.read()
		file.close()
		
		parts = str.split(" ")
		if len(parts) != 3:
			continue
			
		ts = parts[0]
		ns = parts[1]
		NOTCH_SIZE = int(parts[2])
		
		if (ts != ots):
			ots = ts
			print >>sys.stderr, "Changing notches to ", ns
			notches = ns.split(",")
			
			if (len(notches) < 1):
				continue
				
			for i in range(0,len(notches)):
				notches[i] = float(notches[i])
				
			taps = tb.compute_notch_taps(notches, tb.setisize, tb.freq, NOTCH_SIZE)
			tb.set_notch_taps (taps)
	tb.stop()

