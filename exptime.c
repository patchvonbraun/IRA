#include <stdlib.h>
#include <stdio.h>
#include <time.h>

main ()
{
	time_t now;
	struct tm *ltp;
	FILE *fp;
	
	time (&now);
	now += 60*86400;
	
	ltp = localtime (&now);
	fp = fopen ("exptime.h", "w");
	if (fp != NULL)
	{
		fprintf (fp, "#define EXPTIME \"%04d/%02d/%02d-00:00:00\"\n",
			ltp->tm_year+1900, ltp->tm_mon+1, ltp->tm_mday);
		fclose (fp);
	}
}
