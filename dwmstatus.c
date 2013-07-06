#define _POSIX_SOURCE
#include <unistd.h>
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

#include <alsa/asoundlib.h>

#define VOL_CH   "Master"

char *tzmexico = "Mexico/General";

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

int getvolume()
{
	long vol = 0, max = 0, min = 0;
	int mute = 0, realvol = 0;
	
	snd_mixer_t *handle;
	snd_mixer_elem_t *pcm_mixer, *mas_mixer;
	snd_mixer_selem_id_t *vol_info, *mute_info;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, "default");
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);
	snd_mixer_selem_id_malloc(&vol_info);
	snd_mixer_selem_id_malloc(&mute_info);
	snd_mixer_selem_id_set_name(vol_info, VOL_CH);
	snd_mixer_selem_id_set_name(mute_info, VOL_CH);
	pcm_mixer = snd_mixer_find_selem(handle, vol_info);
	mas_mixer = snd_mixer_find_selem(handle, mute_info);
	snd_mixer_selem_get_playback_volume_range((snd_mixer_elem_t *)pcm_mixer, &min, &max);
	snd_mixer_selem_get_playback_volume((snd_mixer_elem_t *)pcm_mixer, SND_MIXER_SCHN_MONO, &vol);
	snd_mixer_selem_get_playback_switch(mas_mixer, SND_MIXER_SCHN_MONO, &mute);
	if (!mute)
           	realvol = 0;
	else {
		realvol = (vol * 100) / max;
	}
	if (vol_info)
		snd_mixer_selem_id_free(vol_info);
	if (mute_info)
		snd_mixer_selem_id_free(mute_info);
	if (handle)
		snd_mixer_close(handle);
	return realvol;
}

char*
mkprogressbar(unsigned int size, unsigned int percent)
{
	unsigned int num = ((size-2)*percent)/100;
	char *bar = malloc(size+1);
	if (bar == NULL) {
		perror("malloc");
		exit(1);
	}
	bar[0] = '|';
	for (int i = 1; i < num+1; i++) {
	      bar[i] = '+';
	}
	for (int i = num+1; i < size-1; i++) {
	      bar[i] = '_';
	}
	bar[size-1] = '|';
	bar[size] = '\0';
	return bar;
}

int
main(void)
{
	char *status;
	char *avgs;
	char *tmmx;
	int  vol;
	char *volbar;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	while(1) {
		avgs = loadavg();
		tmmx = mktimes("WN %W %a %d %b %H:%M:%S %Z", tzmexico);
		vol = getvolume();
		volbar = mkprogressbar(20, vol);

		status = smprintf("%s :: %s :: %i%% %s",
				  tmmx, avgs, vol, volbar);
		setstatus(status);
		free(avgs);
		free(tmmx);
		free(volbar);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
