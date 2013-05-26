
/*  stime.c
    Sidereal time formulae
*/

/*  sclock
 *  Dr. Simon J. Melhuish 1995 - 2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

double sin(), cos(), sqrt();
double atan2(), asin();


/*
#include "components.h"
#include "gadgut.h"
*/

#define F 57.29577951
#define temp_string_len 256
#define IGREG (15+31L*(10+12L*1582))

/* Variables to export to GUI portion */

double longitude = 0.0, latitude = 0.0 ;

/* Functions internal to stime.c */

static long julday(int mm, int id, int iyyy)
{
/*   void nrerror(char error_text[]); */
  long jul;
  int ja, jy=iyyy, jm;

  if (jy == 0) /*werr(0,"julday: there is no year zero.");*/
  if (jy < 0) ++jy;
  if (mm > 2)
  {
    jm = mm + 1;
  }
  else
  {
    --jy;
    jm = mm + 13;
  }
  jul = (long) (floor(365.25*jy)+floor(30.6001*jm)+id+1720995);
  if (id+31L*(mm+12L*iyyy) >= IGREG)
  {
    ja = (int) (0.01*jy);
    jul += (long) (2-ja+(int) (0.25*(double)ja));
  }
  return jul ;
}

/*--------------------------------------------------------*/

#define COEF1 24110.54841
#define COEF2 8640184.812866
#define COEF3 0.093104
#define COEF4 -6.2e-6
#define COEF5 1.00273790934

#define SQR(x) (x)*(x)
#define CUBE(x) (x)*SQR(x)

/** Misc. functions **/

static double sidereal(double jul)

/*
  Returns the GMST (in hours) of the julian day (including fraction) given
  as the parameter - adapted from The Astronomical Almanac 1990 - B6.
*/

    {
    double gmst,
           rem_jul;
    rem_jul=jul;
    jul=floor(jul)+0.5;
    rem_jul-=jul;
    jul=(jul-2451545.0)/36525.0;
    gmst=(COEF1+COEF2*jul+COEF3*SQR(jul)+COEF4*CUBE(jul))/86400.0;
    gmst+=COEF5*rem_jul;
    gmst-= floor(gmst);
    gmst*=24;
    return(gmst);
    }

#undef CUBE
#undef SQR
#undef COEF5
#undef COEF4
#undef COEF3
#undef COEF2
#undef COEF1

/*-------------------------------------------------------------------------*/

static double r_ascension(double jultime)


/*
  Returns the right ascension for the zenith at the time given.
*/

    {
    double ra;
    ra=sidereal(jultime);
    ra-=24.0*floor(ra/24.0);
    return(ra/24.0); /* Return ra day number */
    }

static double sind(double ang)
{
  return sin(ang/F) ;
}

static double cosd(double ang)
{
  return cos(ang/F) ;
}

static void geo2topo(double *ra, double *dec, double ha, double par, double lat, double lon)
{
  /* ra and dec are geocentric - convert them to topocentric */
  double topra, topdec, rho, gclat, g ;

  /* Correct for flattening of Earth */

  gclat = lat - 0.1924 * sind(2*lat) ;
  rho = 0.99883 + 0.00167 * cosd(2*lat) ;

  /* Auxiliary angle */

  g = F * atan2(tan(gclat), cos(ha)) ;

  topra = *ra - par * rho * cosd(gclat) * sind(ha) / cosd(*dec) ;
  topdec = *dec - par * rho * sind(gclat) * sind(g - *dec) / sind(g) ;

  *ra = topra ;
  *dec = topdec ;
}

static void sun_coords(double jd, double *ra, double *dec)
{
/*
  n ... time argument
  L ... mean longitude of sun
  g ... mean anomaly
  epsilon ... obliquity of ecliptic (in radians)
  lambda ... ecliptic longitude
*/

  double n, L, g, epsilon, lambda, t ;

  n = jd - 2451545.0 ;
  L = 280.460 + 0.9856474 * n ;
  g = 357.528 + 0.9856003 * n ;
  while (L < 0) L += 360.0 ;
  while (L > 360.0) L -= 360.0 ;
  while (g < 0) g += 360.0 ;
  while (g > 360.0) g -= 360.0 ;
  lambda = L + 1.915 * sin(g/F) + 0.020 * sin(2*g/F) ;
  epsilon = 23.439 - 0.0000004 * n ;
  t = tan(epsilon/(2.0*F)) ;
  t *= t ;
  *ra = lambda - F * t * sin(2.0 * lambda/F) + F*sin(4.0 * lambda/F)
        * t * t / 2.0 ;
  *dec = F*asin(sin(epsilon/F) * sin(lambda/F)) ;

  return ;
}

static void moon_coords(double jd, double *ra, double *dec)
{
  double T, lambda, beta, pi, theta, l, m, n, r ;
  double x, y, z, xt, yt, zt, rt, jt ;

  T = (jd - 2451545.0) / 36525.0 ;

  lambda =  218.32 + 481267.883 * T
            +6.29*sind(134.9+477198.85*T)-1.27*sind(259.2-413335.38*T)
            +0.66*sind(235.7+890534.23*T)+0.21*sind(269.9+954397.70*T)
            -0.19*sind(357.5+35999.05*T)-0.11*sind(186.6+966404.05*T);

  beta = 5.13*sind(93.3+483202.03*T)+0.28*sind(228.2+960400.87*T)
        -0.28*sind(318.3+6003.18*T)+0.17*sind(217.6-407332.20*T);

  pi = 0.9508
        +0.0518*cosd(134.9+477198.85*T)+0.0095*cosd(259.2-413335.38*T)
        +0.0078*cosd(235.7+890534.23*T)+0.0028*cosd(269.9+954397.70*T) ;

  l = cosd(beta) * cosd(lambda) ;
  m = 0.9175*cosd(beta)*sind(lambda)-0.3978*sind(beta);
  n = 0.3978*cosd(beta)*sind(lambda)+0.9175*sind(beta);
  r = 1.0 / sind(pi) ;

  *ra = F * atan2(m,l) ;
  *dec = F * asin(n) ;

  x = r * cosd(*dec) * cosd(*ra) ;
  y = r * cosd(*dec) * sind(*ra) ;
  z = r * n ;

  jt = jd + 0.5 ;
  jt -= floor(jt) ;

  theta = 100.46 + 36000.77 * T + longitude + 360.0 * jt ;

  xt = x - cosd(latitude) * cosd(theta) ;
  yt = y - cosd(latitude) * sind(theta) ;
  zt = z - sind(latitude) ;

  rt = sqrt(xt*xt+yt*yt+zt*zt) ;

  *ra = F * atan2(yt, xt) ;
  *dec = F * asin(zt / rt) ;

  return ;
}

get_sid_time (sid, longit, ltp)
unsigned char sid[];
double longit;
struct tm *ltp;
{
  int hour, min, sec ;
  double ra;
  double jul;
  double tm_to_ra ();


  longitude = longit;
  ra = tm_to_ra (ltp, &jul);
  ra = ra+longitude/360;

  while (ra < 0) ++ra ;
  while (ra > 1) --ra ;

  ra *= 24.0 ;
  hour = (int) floor(ra) ;
  ra -= (double) hour ;
  ra *= 60.0 ;
  min = (int) floor(ra) ;
  ra -= (double) min ;
  sec = (int) floor(ra * 60.0) ;

  sid[0] = hour;
  sid[1] = min;
  sid[2] = sec;

  return ;
}

static void conv_time(char *string, double ra)
{
/* RA is given in days. */

  int hour, min, sec ;

  while (ra < 0) ++ra ;
  while (ra > 1) --ra ;

  ra *= 24.0 ;
  hour = (int) floor(ra) ;
  ra -= (double) hour ;
  ra *= 60.0 ;
  min = (int) floor(ra) ;
  ra -= (double) min ;
  sec = (int) floor(ra * 60.0) ;

  sprintf(string, "%02i:%02i:%02i", hour, min, sec) ;
  return ;
}

static void conv_ang(char *string, double dec)
{
/* dec is in degrees */

  int deg, min, sec ;
  int negative ;

  negative = (dec < 0.0) ;
  if (negative) dec = -dec ;

  deg = (int) floor(dec) ;
  dec -= (double) deg ;
  dec *= 60.0 ;
  min = (int) floor(dec) ;
  dec -= (double) min ;
  sec = (int) floor(dec * 60.0) ;

  if (negative)
    sprintf(string, "-%i°%i'%i\"", deg, min, sec) ;
  else
    sprintf(string, "%i°%i'%i\"", deg, min, sec) ;
  /*printf("%s\n",string);*/
  return ;
}

/*-------------------------------------------------------------------------*/

/* Exported function to convert times */

double tm_to_ra(struct tm *tm_time, double *jultime)
{
  double dayfrac ;

  *jultime = julday((tm_time->tm_mon + 1), tm_time->tm_mday,
                   (tm_time->tm_year + 1900)) ;
  dayfrac = (double) tm_time->tm_sec / 86400.0 +
            (double) tm_time->tm_min / 1440.0 +
            (double) tm_time->tm_hour / 24.0 ;
  *jultime += dayfrac - 0.5 ;
  return r_ascension(*jultime) ;
}
