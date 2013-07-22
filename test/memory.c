#define _POSIX_SOURCE
#include <stdio.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


/* int main() */
/* { */
     
/*   long numPages, pageSize, freePages, totalSize, totalUsed, totalFree; */
     
/*   numPages = sysconf(_SC_PHYS_PAGES)/1024; */
/*   pageSize = sysconf(_SC_PAGESIZE)/1024; */
/*   freePages = sysconf( _SC_AVPHYS_PAGES)/1024; */
/*   totalSize = numPages * pageSize; */
/*   totalFree = pageSize * freePages; */
/*   totalUsed = totalSize - totalFree; */

/*   long pa = numPages - totalUsed / numPages * 100.0; */
     
/*   printf("total memory == %u MB\n",totalSize); */
/*   printf("total free memory == %u MB\n",totalFree); */
/*   printf("total used memory == %u MB\n",totalUsed); */
/*   printf("Percent Available == %u MB\n",pa); */


/*   //double percentAvail = double(totalPages - freePages) / double(totalPages) * 100.0;      */
/*   return(0); */
     
/* } */
     
/* int main(int argc, char *argv[]) */
/* { */
/*   char *cmd="awk '{{Active} {print $2}}' /proc/meminfo"; */
/*   FILE *cmdfile=popen(cmd,"r"); */
/*   char result[256]={0x0}; */
/*   while(fgets(result,sizeof(result),cmdfile)!=NULL) */
/*     { */
/*       printf("%s\n",result); */
/*     } */
/*   pclose(cmdfile); */
/*   return 0; */
/* } */

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

int
meminfo(char *param)
{
  int mem;
  char line[256];
  FILE *meminfo = fopen("/proc/meminfo", "r");
  printf ("%s\n",param);
  while(fgets(line, sizeof(line), meminfo))
    {
      if(sscanf(line, param, &mem) == 1)
        {
	  fclose(meminfo);
	  return mem;
        }
    }
  fclose(meminfo);
  return -1;
}

char *
usedram()
{
  struct sysinfo info;
  int cused = 0;
  const char unit[] = { 'k', 'M', 'G', 'T' };
  if(sysinfo(&info) != 0)
    perror("sysinfo");
  printf ("%d\n",meminfo("Cached: %d kB"));
  double used = ((info.totalram - info.freeram - info.bufferram)/1024)-meminfo("Cached: %d kB");
  printf ("used: %.2f\n",used);

  while(used > 1024) {
    used/=1024;
    cused++;
  }
  return(smprintf("%.2f%c", used, unit[cused]));
}


int main(int argc, char *argv[])
{
  char *mem;
  mem = usedram();
  char *status = smprintf("%s", mem);
  printf ("%s\n",status);
  return 0;
}
