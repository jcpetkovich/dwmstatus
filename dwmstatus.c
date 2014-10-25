#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

#include "dwmstatus.h"

char *tcanada = "Canada/Eastern";
volatile sig_atomic_t term_request = False;

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

char *
getbattery(){
	long lnum1, lnum2 = 0;
	char *status = malloc(sizeof(char)*12);
	char *s = "?";
	FILE *fp = NULL;
	long batpercent;
	if ((fp = fopen(BATT_NOW, "r"))) {
		fscanf(fp, "%ld\n", &lnum1);
		fclose(fp);
		fp = fopen(BATT_FULL, "r");
		fscanf(fp, "%ld\n", &lnum2);
		fclose(fp);
		fp = fopen(BATT_STATUS, "r");
		fscanf(fp, "%s\n", status);
		fclose(fp);
		batpercent = (lnum1/(lnum2/100));
		if (strcmp(status,"Charging") == 0)
			s = BAT_CHARGING_GLYPH;
		if (strcmp(status,"Discharging") == 0) {
			if (batpercent >= 70) {
				s = "<span color=\""BLUE"\">"BAT_70_PERCENT_GLYPH"</span>";
			} else if (batpercent >= 30) {
				s = "<span color=\""MAGENTA"\">"BAT_30_PERCENT_GLYPH"</span>";
			} else {
				s = "<span color=\""RED"\">"BAT_EMPTY_GLYPH"</span>";
			}
		}
		if (strcmp(status,"Full") == 0) s = BAT_FULL_GLYPH;
		return smprintf("%s %ld%%", s, batpercent);
	}
	else return smprintf("");
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

int
main(void)
{
	char *status;
	char *avgs;
	char *tmar;
	char *tmutc;
	char *tmbln;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(90)) {
		avgs = loadavg();
		tmar = mktimes("%H:%M", tzargentina);
		tmutc = mktimes("%H:%M", tzutc);
		tmbln = mktimes("KW %W %a %d %b %H:%M %Z %Y", tzberlin);

		status = smprintf("L:%s A:%s U:%s %s",
				avgs, tmar, tmutc, tmbln);
		setstatus(status);
		free(avgs);
		free(tmar);
		free(tmutc);
		free(tmbln);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}

