#include <stdio.h>
#include <sys/sysinfo.h>

#define MEGABYTE 1048576

int main( int argc, char **argv )
{
  struct sysinfo info;
  sysinfo(&info);

  printf ("Total ram: %d\n",(long)info.totalram/MEGABYTE );
  printf ("Free ram: %d\n", (long)info.freeram/MEGABYTE );
  printf ("Mem unit %d\n",(long)info.mem_unit );
  printf ("Shared: %d\n", (long)info.sharedram/MEGABYTE );
  printf ("Buffered: %d\n",(long)info.bufferram/MEGABYTE );
  printf ("Total High: %d\n",(long)info.totalhigh/MEGABYTE );
  printf ("Free high: %d\n",(long)info.freehigh/MEGABYTE );
     
  return 0;
}

/* unsigned long loads[3];  /\* 1, 5, and 15 minute load averages *\/ */
/* unsigned long totalram;  /\* Total usable main memory size *\/ */
/* unsigned long freeram;   /\* Available memory size *\/ */
/* unsigned long sharedram; /\* Amount of shared memory *\/ */
/* unsigned long bufferram; /\* Memory used by buffers *\/ */
/* unsigned long totalswap; /\* Total swap space size *\/ */
/* unsigned long freeswap;  /\* swap space still available *\/ */
/* unsigned short procs;    /\* Number of current processes *\/ */
/* unsigned long totalhigh; /\* Total high memory size *\/ */
/* unsigned long freehigh;  /\* Available high memory size *\/ */
