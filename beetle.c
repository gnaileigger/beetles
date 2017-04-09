/*
 * 
 * author: r.liang
 * email: xxxx@xxx.com 
 *
*/

#include "beetle.h"

uint beetle_init(beetle* b)
{
  //pos
  //dir
  //speed

  //mqtt request

  //mqtt response
}


extern uint gen_rand();

int beetle_compare_position(position a, position b)
{
  if (a.x == b.x && a.y == b.y )
    return 0;
  else
    return 1;
}

// update the dir if it is in a checkpoint
void beetle_dir_update(beetle* b)
{
  checkpoint cp;
  int i = 0;
  //if position is a checkpoint, update the dir
  for (i = 0; i < NUM_CHKPTS; i++)
  {
    cp = g_checkpoints[i];
    if (beetle_compare_position(b->pos, cp.pos) == 0)
    {
      uint rn = gen_rand() % (cp.dir_cnt); //random select a dir in this checkpoint
      b->pos.x = cp.possible_dirs[rn].x;
      b->pos.y = cp.possible_dirs[rn].y;
      break;
    }
  }
}

void beetle_move(beetle* b)
{
  b->pos.x += b->dir.x;
  b->pos.y += b->dir.y;

  beetle_dir_update(b);
}

uint g_sensor_distance;

uint beetle_sensor_update()
{
  //semTake(sem_mqtt);
  return g_sensor_distance;
  
}

void beetle_report()
{
  //mqtt send: pos(x,y), id
}

int main()
{
  beetle b;
  uint distance;
  
  if(beetle_init(&b) != 0)
  {
    printf("init error\n");
    return -1;
  }

  //main loop
  while(1){
    distance = beetle_sensor_update();

    if (distance > SAFE_DISTANCE){
      beetle_move(&b);
    }

    beetle_report(b);

  } //while(1)


}

