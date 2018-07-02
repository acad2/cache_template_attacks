#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include "../cacheutils.h"

size_t array[5*1024];

size_t hit_histogram[80];
size_t miss_histogram[80];

size_t onlyreload(void* addr)
{
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  return delta;
}

size_t flushandreload(void* addr)
{
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  flush(addr);
  return delta;
}

int main(int argc, char** argv)
{
  memset(array,-1,5*1024*sizeof(size_t));
  maccess(array + 2*1024);
  sched_yield();
  for (int i = 0; i < 4*1024*1024; ++i)
  {
    size_t d = onlyreload(array+2*1024);
    hit_histogram[MIN(79,d/5)]++;
    sched_yield();
  }
  flush(array+1024);
  for (int i = 0; i < 4*1024*1024; ++i)
  {
    size_t d = flushandreload(array+2*1024);
    miss_histogram[MIN(79,d/5)]++;
    sched_yield();
  }
  printf(".\n");
  size_t hit_max = 0;
  size_t hit_max_i = 0;
  size_t miss_min_i = 0;
  for (int i = 0; i < 80; ++i)
  {
    printf("%3d: %10zu %10zu\n",i*5,hit_histogram[i],miss_histogram[i]);
    if (hit_max < hit_histogram[i])
    {
      hit_max = hit_histogram[i];
      hit_max_i = i;
    }
    if (miss_histogram[i] > 3 && miss_min_i == 0)
      miss_min_i = i;
  }

  return 0;
}
