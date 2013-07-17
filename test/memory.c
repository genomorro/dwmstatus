#define _POSIX_SOURCE
#include <stdio.h>
#include <unistd.h>
     
int main()
{
     
  long numPages, pageSize, freePages, totalSize, totalUsed, totalFree;
     
  numPages = sysconf(_SC_PHYS_PAGES)/1024;
  pageSize = sysconf(_SC_PAGESIZE)/1024;
  freePages = sysconf( _SC_AVPHYS_PAGES)/1024;
  totalSize = numPages * pageSize;
  totalFree = pageSize * freePages;
  totalUsed = totalSize - totalFree;

  long pa = numPages - totalUsed / numPages * 100.0;
     
  printf("total memory == %u MB\n",totalSize);
  printf("total free memory == %u MB\n",totalFree);
  printf("total used memory == %u MB\n",totalUsed);
  printf("Percent Available == %u MB\n",pa);


  //double percentAvail = double(totalPages - freePages) / double(totalPages) * 100.0;     
  return(0);
     
}
     
     


