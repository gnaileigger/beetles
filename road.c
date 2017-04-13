/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include "beetle.h"
#include "utils.h"

/*

6 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx (left side limit)
5
4
3      o->
2
1
0 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx (right side limit)
  0                                                               300
*/
#define LANE_LEN 100
#define LANE_WIDTH 6
#define LANE_LEFT_LIMIT 6
#define LANE_RIGHT_LIMIT 0
#define LANE_SAFE_INTERVAL 1
#define LANE_SAFE_DISTANCE 2

#if 0

/* Random a swing: left, right or none
    Prevent hitting the sides: if the interval to sides are 1 (LANE_SAFE_INTERVAL), 
      stop swinging to that side
*/
void beetle_get_swing(beetle * c)
{
  unsigned int rn = gen_rand();
  //unsigned int rn=0;

  if ((c->pos.y - LANE_RIGHT_LIMIT -1) <= LANE_SAFE_INTERVAL) //close to right
  {
    //move leftwards 
    c->sideward = 1;
  }
  else if ((LANE_LEFT_LIMIT - c->pos.y -1) <= LANE_SAFE_INTERVAL) //close to left
  {
    //move rightwards 
    c->sideward = -1 ; //-1, 0
  }
  else
  {
    c->sideward = rn%3 - 1; //-1, 0, 1
  }
  
  printf("sideward %d\n",c->sideward); //-1 left(up); 1 right(down)
}

/*
  move: update a new position
*/
void beetle_move(beetle * c, unsigned int left, unsigned int right)
{
  //move one forward in every call
  c->pos.x++;
  // loop back when reach the end
  c->pos.x %= LANE_LEN;

  // swing protection
  if (left <= LANE_SAFE_DISTANCE)
    c->pos.y-- ; //move right
  else if (right <= LANE_SAFE_DISTANCE)
    c->pos.y++ ; //move left
}

void beetle_report(beetle * c)
{
  printf("%3d,%3d\n", c->pos.x, c->pos.y);
}


/*
  assume we have distance sensors
*/
unsigned int beetle_left_sensor_update(beetle * c)
{
  usleep (500);

  return (LANE_LEFT_LIMIT - c->pos.y);
}

unsigned int beetle_right_sensor_update(beetle * c)
{
  usleep (500);

  return (c->pos.y - LANE_RIGHT_LIMIT);
}
/*
int road_init()
{
  beetle c;
  unsigned int left, right;

  printf("pai = %d\n", sizeof(pai));

  //initialize
  memset(&c, 0, sizeof(c));
  c.pos.x = 0;
  c.pos.y = 3;
  c.sideward = 0;

  //main loop
  while(1){
    left = beetle_left_sensor_update(&c);
    right = beetle_right_sensor_update(&c);
    beetle_move(&c, left, right);
    beetle_report(&c);
  } //while(1)
  
   return 0;

}
*/

#endif
