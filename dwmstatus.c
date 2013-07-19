#define _POSIX_SOURCE
#include <X11/Xlib.h>
#include <mntent.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <alsa/asoundlib.h>

#define BATT_FULL       "/sys/class/power_supply/BAT0/energy_full"
#define BATT_NOW        "/sys/class/power_supply/BAT0/energy_now"
#define BATT_STATUS     "/sys/class/power_supply/BAT0/status"
#define MAX_ERROR_MSG   0x1000
#define SOUNDCARD       "default"
#define VOL_CH          "Master"

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

static int
compile_regex (regex_t *r, const char *regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
	char error_message[MAX_ERROR_MSG];
	regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                 regex_text, error_message);
        return 1;
    }
    return 0;
}

static int
match_regex (regex_t *r, const char *to_match)
{
  const char *p = to_match;
  const int n_matches = 1;
  regmatch_t m[n_matches];
  int nomatch = regexec (r, p, n_matches, m, 0);
  if (nomatch) {
    return nomatch;
  } else {
    return 0;
  }
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
getvolume()
{
	long vol = 0, max = 0, min = 0;
	int mute = 0, realvol = 0;
	snd_mixer_t *handle;
	snd_mixer_elem_t *pcm_mixer, *mas_mixer;
	snd_mixer_selem_id_t *vol_info, *mute_info;
	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, SOUNDCARD);
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

char *
getbattery()
{
    long current, full = 0;
    char *status = malloc(sizeof(char)*12);
    char s = '?';
    FILE *fp = NULL;
    if ((fp = fopen(BATT_NOW, "r"))) {
        fscanf(fp, "%ld\n", &current);
        fclose(fp);
        fp = fopen(BATT_FULL, "r");
        fscanf(fp, "%ld\n", &full);
        fclose(fp);
        fp = fopen(BATT_STATUS, "r");
        fscanf(fp, "%s\n", status);
        fclose(fp);
        if (strcmp(status,"Charging") == 0)
            s = '+';
        if (strcmp(status,"Discharging") == 0)
            s = '-';
        if (strcmp(status,"Full") == 0)
            s = '=';
        return smprintf("%c%ld%%", s,(current/(full/100)));
    }
    else return smprintf("");
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
	bar[0] = '[';
	for (int i = 1; i < num+1; i++) {
	      bar[i] = '+';
	}
	for (int i = num+1; i < size-1; i++) {
	      bar[i] = ' ';
	}
	bar[size-1] = ']';
	bar[size] = '\0';
	return bar;
}

char *
usedram()
{
  struct sysinfo info;
  int cused = 0;
  const char unit[] = { 'k', 'M', 'G', 'T' };
  if(sysinfo(&info) != 0)
    perror("sysinfo");
  double used = (info.totalram - info.freeram);
  while(used > 1024) {
    used/=1024;
    cused++;
  }
  return(smprintf("%.2f%c", used, unit[cused-1]));
}

char *
freespace(char *mntpt)
{
  struct statvfs data;
  double total, used = 0;
  int ctotal, cused = 0;
  const char unit[] = { 'k', 'M', 'G', 'T' };
  if ( (statvfs(mntpt, &data)) < 0){
    fprintf(stderr, "can't get info on disk.\n");
    return("?");
  }
  total = (data.f_blocks * data.f_frsize);
  used = ((data.f_blocks - data.f_bfree) * data.f_frsize) ;
  while(total > 1024) {
    total/=1024;
    ctotal++;
  }
  while(used > 1024) {
    used/=1024;
    cused++;
  }
  return(smprintf("%.2f%c %.2f%c", total, unit[ctotal-1], used, unit[cused-1]));
}

char *
getmounted()
{
  struct mntent *ent = NULL;
  FILE *mtab = NULL;
  char *free;
  char buf[1024];
  regex_t r;
  const char * regex_text;
  const char * find_text;
  regex_text = "(/media/|/mnt/)";
  strcpy(buf,"");
  compile_regex(&r, regex_text);
  if ((mtab = setmntent("/etc/mtab", "r")) != NULL) {
    while ((ent = getmntent(mtab)) != NULL) {
      if ((ent->mnt_fsname  != NULL)) {
	find_text = ent->mnt_dir;
	if ((match_regex(&r, find_text)) == 0) {
	  strcat(buf," ");
	  strcat(buf,ent->mnt_dir);
	  strcat(buf," ");
	  strcat(buf,free = freespace(ent->mnt_dir));
	}
      }
    }
    regfree(&r);
    endmntent(mtab);
  }
  return smprintf ("%s",buf);
}

int
main(int argc, char *argv[])
{
	char *avgs;
	char *bat;
	char *mnt;
	char *status;
	char *tmmx;
	char *uram;
	char *volbar;
	int  vol;
	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}
	while(1) {
		avgs = loadavg();
		bat = getbattery();
		mnt = getmounted();
		tmmx = mktimes("%a %d %b %H:%M:%S", tzmexico);
		uram = usedram();
		vol = getvolume();
		volbar = mkprogressbar(20, vol);
		status = smprintf("%s | %s | %s | %s | %i%% %s %s",
				  tmmx, avgs, bat, uram, vol, volbar, mnt);
		setstatus(status);
		free(avgs);
		free(bat);
		free(mnt);
		free(status);
		free(tmmx);
		free(uram);
		free(volbar);
		sleep(1);
	}
	XCloseDisplay(dpy);
	return 0;
}
