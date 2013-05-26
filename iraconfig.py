#
# Configuration management for IRA
#
# Marcus Leech, Science Radio Laboratories, Inc.
#
import os
import numpy
import math
import sys

def compute_notches(notchlist,flen,bw,freq,dm):
	tmptaps = numpy.zeros(flen, dtype=numpy.complex)
	binwidth = bw / flen
	for i in range(0,flen):
		tmptaps[i] = complex(1.0,0.0)
	
	added=0
		
	#
	# Compute a multi-bin notch filter (a comb filter)
	#   based on the input notch list
	#
	for i in notchlist:
		diff = i - freq
		
		if int(i) <= 0:
			break

		if ((i < (freq - bw/2)) or (i > (freq + bw/2))):
			continue
		
		idx = diff/binwidth
		idx = round(idx)
		
		if (idx < 0):
			idx = -1 * idx
			idx = ((flen)-1) - idx
		
		while (idx < 0):
			idx = idx + 1
			
		tmptaps[int(idx)] = complex(0.0, 0.0)
		added = added + 1
		
	if (added <= 0):
		tmptaps = [complex(1.0,0.0),complex(1.0,0.0),complex(1.0,0.0)]
		
	return (numpy.fft.ifft(tmptaps))


#
# Compute a de-dispersion filter
#  From Hankins, et al, 1975
#
# This code translated from dedisp_filter.c from Swinburne
#   pulsar software repository
#
def compute_dispfilter(dm,doppler,bw,centerfreq):
	npts = compute_disp_ntaps(dm,bw,centerfreq)
	tmp = numpy.zeros(npts, dtype=numpy.complex)
	M_PI = 3.14159265358
	DM = dm/2.41e-10

	#
	# Because astronomers are a crazy bunch, the "standard" calculation
	#   is in Mhz, rather than Hz
	#
	centerfreq = centerfreq / 1.0e6
	bw = bw / 1.0e6
	
	isign = int(bw / abs (bw))
	
	# Center frequency may be doppler shifted
	cfreq     = centerfreq / doppler

	# As well as the bandwidth..
	bandwidth = bw / doppler

	# Bandwidth divided among bins
	binwidth  = bandwidth / npts

	# Delay is an "extra" parameter, in usecs, and largely
	#  untested in the Swinburne code.
	delay = 0.0
	
	# This determines the coefficient of the frequency response curve
	# Linear in DM, but quadratic in center frequency
	coeff = isign * 2.0*M_PI * DM / (cfreq*cfreq)
	
	# DC to nyquist/2
	n = 0
	for i in range(0,int(npts/2)):
		freq = (n + 0.5) * binwidth
		phi = coeff*freq*freq/(cfreq+freq) + (2.0*M_PI*freq*delay)
		tmp[i] = complex(math.cos(phi), math.sin(phi))
		n += 1

	# -nyquist/2 to DC
	n = int(npts/2)
	n *= -1
	for i in range(int(npts/2),npts):
		freq = (n + 0.5) * binwidth
		phi = coeff*freq*freq/(cfreq+freq) + (2.0*M_PI*freq*delay)
		tmp[i] = complex(math.cos(phi), math.sin(phi))
		n += 1
	
	
	return(numpy.fft.ifft(tmp))

#
# Compute minimum number of taps required in de-dispersion FFT filter
#
def compute_disp_ntaps(dm,bw,freq):
	NTLIMIT=65536*2
	#
	# Dt calculations are in Mhz, rather than Hz
	#    crazy astronomers....
	mbw = bw/1.0e6
	mfreq = freq/1.0e6

	f_lower = mfreq-(mbw/2)
	f_upper = mfreq+(mbw/2)

	# Compute smear time
	Dt = dm/2.41e-4 * (1.0/(f_lower*f_lower)-1.0/(f_upper*f_upper))

	# ntaps is now bandwidth*smeartime
	ntaps = bw*Dt
	if (ntaps < 32):
		ntaps = 32
	# special "flag" from command-line invoker to get around a bug
	#   in Gnu Radio involving the FFT filter implementation
	#   we can *never* increase the size of an FFT filter at runtime
	#   but can decrease it.  So there's a special "startup" flag (dm=1500.0)
	#   that causes us to return the NTLIMIT number of taps
	#
	if (dm >= 1500.0):
		ntaps = NTLIMIT
	if (ntaps > NTLIMIT):
		ntaps = NTLIMIT
	ntaps = int(math.log(ntaps) / math.log(2))
	ntaps = int(math.pow(2,ntaps+1))
	return(int(ntaps))

def writepid():
	f=open("receiver.pid","w")
	pid=os.getpid()
	f.write(str(pid)+"\n")
	f.close()
	return(pid)

def writevars(varnames,vars):
	f=open("variables.dump","w")
	for i in range(0,len(varnames)):
		f.write(varnames[i]+"="+str(float(vars[i]))+"\n")
	f.close()
	return 1
		
	
