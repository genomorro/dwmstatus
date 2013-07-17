#define _POSIX_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <mntent.h>

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

int is_mounted (char * dev_path) {
  FILE * mtab = NULL;
  struct mntent * part = NULL;
  int is_mounted = 0;

  if ( ( mtab = setmntent ("/etc/mtab", "r") ) != NULL) {
    while ( ( part = getmntent ( mtab) ) != NULL) {
      if ( ( part->mnt_fsname != NULL )
	   && ( strcmp ( part->mnt_fsname, dev_path ) ) == 0 ) {
	is_mounted = 1;
      }
    }
    endmntent ( mtab);
  }
  return is_mounted;
}



int main(int argc, char *argv[])
{
  if (is_mounted("/dev/sr0")) {
    printf("CDROM mounted!\n");
  }
  else {
    printf("CDROM not mounted!\n");
  }
  return 0;
}
