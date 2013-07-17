#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <mntent.h>
#include <regex.h>
#include <sys/statvfs.h>

#define MAX_ERROR_MSG 0x1000

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
  /* "P" is a pointer into the string which points to the end of the
     previous match. */
  const char *p = to_match;
  /* "N_matches" is the maximum number of matches allowed. */
  const int n_matches = 1;
  /* "M" contains the matches found. */
  regmatch_t m[n_matches];

  int nomatch = regexec (r, p, n_matches, m, 0);
  if (nomatch) {
    return nomatch;
  } else {
    return 0;
  }
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
  return(smprintf("%.2f%c %.1f%c", total, unit[ctotal-1], used, unit[cused-1]));
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

  regex_text = "(/media|/mnt)";

  strcpy(buf," ::");
  
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
    endmntent(mtab);
  }
  return smprintf ("%s",buf);
}

int
main(int argc, char *argv[])
{
  char *mounted;
  mounted = getmounted();
  char *status = smprintf("%s",mounted);
  printf ("%s\n",status);
  return 0;
}
