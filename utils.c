#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


//generate a random number (freq: at least one second)
unsigned int gen_rand()
{
  unsigned int rn;

  srand((unsigned)time(NULL));
  rn = (unsigned int)rand();

  //printf("rn=%d\n",rn);
  return rn;
}