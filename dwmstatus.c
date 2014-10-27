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
#include <mpd/client.h>

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
	char status[12];
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
			s = "<span color=\""YELLOW"\">"BAT_CHARGING_GLYPH"</span>";
		if (strcmp(status,"Discharging") == 0) {
			if (batpercent >= 75) {
				s = "<span color=\""BLUE"\">"BAT_FULL_GLYPH"</span>";
			} else if (batpercent >= 50) {
				s = "<span color=\""BLUE"\">"BAT_70_PERCENT_GLYPH"</span>";
			} else if (batpercent >= 15) {
				s = "<span color=\""MAGENTA"\">"BAT_30_PERCENT_GLYPH"</span>";
			} else {
				s = "<span color=\""RED"\">"BAT_EMPTY_GLYPH"</span>";
			}
		}
		if (strcmp(status,"Full") == 0)
			s = "<span color=\""BLUE"\">"BAT_FULL_GLYPH"</span>";
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
get_time(char *fmt, char *tzname)
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

	return smprintf("%s %s", CLOCK_GLYPH, buf);
}

char *
get_signal_strength()
{
	static int bufsize = 255;
	char buf[bufsize];
	char *glyph;
	char *datastart;
	FILE *devfd;
	int strength = 0;

	if ((devfd = fopen("/proc/net/wireless", "r")) == NULL) {
		perror("parse_wireless");
	}

	// Ignore the first two lines of the file
	fgets(buf, bufsize, devfd);
	fgets(buf, bufsize, devfd);
	fgets(buf, bufsize, devfd);
	if ((datastart = strstr(buf, "wlp1s0:")) != NULL) {
		datastart = strstr(buf, ":");
		sscanf(datastart + 1, " %*d   %d  %*d  %*d        %*d      %*d      %*d      %*d      %*d        %*d",
		       &strength);
		glyph = "<span foreground=\""BLUE"\">"WIFI_GLYPH"</span>";
	} else {
		glyph = "<span foreground=\""BLACK"\">"WIFI_GLYPH"</span>";
	}

	fclose(devfd);

	return smprintf("%s %d", glyph, strength);
}

char *
get_mpd_stat() {
	struct mpd_song * song = NULL;
	const char * title = NULL;
	const char * artist = NULL;
	char * retstr = NULL;
	struct mpd_connection * conn ;
	if (!(conn = mpd_connection_new("localhost", 0, 30000)) ||
	    mpd_connection_get_error(conn)){
		return smprintf("");
	}

	mpd_command_list_begin(conn, true);
	mpd_send_status(conn);
	mpd_send_current_song(conn);
	mpd_command_list_end(conn);

	struct mpd_status* theStatus = mpd_recv_status(conn);
	if ((theStatus) && (mpd_status_get_state(theStatus) == MPD_STATE_PLAY)) {
		mpd_response_next(conn);
		song = mpd_recv_song(conn);
		title = smprintf("%s",mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
		artist = smprintf("%s",mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));

		mpd_song_free(song);
		retstr = smprintf("%s %s - %s",
		                  "<span color=\""MAGENTA"\">"MUSIC_GLYPH"</span>",
		                  artist, title);
		free((char*)title);
		free((char*)artist);
	}
	else retstr = smprintf(MUSIC_GLYPH" ");
	mpd_response_finish(conn);
	mpd_connection_free(conn);
	return retstr;
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

void term(int signum)
{
	term_request = True;
}

int
main(void)
{

	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = term;
	if (sigaction(SIGTERM, &sa, 0) == -1) {
		perror(0);
		exit(1);
	}

	char *status;
	char *tcan;
	char *strength;
	char *bat;
	char *mpdstring;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (; !term_request; sleep(3)) {
		tcan= get_time("%H:%M", tcanada);
		strength = get_signal_strength();
		bat = getbattery();
		mpdstring = get_mpd_stat();
		status = smprintf(" %s %s %s %s %s %s %s %s ",
		                  POWERLINE_SOFT_RIGHT, mpdstring,
		                  POWERLINE_SOFT_RIGHT, strength,
		                  POWERLINE_SOFT_RIGHT, bat,
		                  POWERLINE_SOFT_RIGHT, tcan);
		setstatus(status);
		free(mpdstring);
		free(bat);
		free(strength);
		free(tcan);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
