/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include <stdio.h>

extern unsigned int random_uint32(void);

//generate a random number (freq: at least one second)
unsigned int gen_rand(void)
{
#if 0
  unsigned int rn;

  srand((unsigned)time(NULL));
  rn = (unsigned int)rand();

  //printf("rn=%d\n",rn);
  return rn;
#endif

  return random_uint32();
}