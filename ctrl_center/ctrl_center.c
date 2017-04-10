/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include "../beetle.h"



/* -------------------- The MAP --------------------

                                         y pos:
y=200                                     CC                                    (300,200)
                                               v^
                                               v^
                                               v^
                                               v^
                                               v^
y=101  C<<<<<<<<<<<<<<<CC<<<<<<<<<<<<<<<<<<C
y=100  C>>>>>>>>>>>>>>>CC>>>>>>>>>>>>>>>>>>C   ZS Road
                                               v^
                                               v^
                                               v^ JF Road
                                               v^
                                               v^
                                               v^
         (0,0)                               CC               C=checkpoint
                                       x=150,151                                      x=300                                         
*/

//ZS road
#define ZS_ROAD_LEN 300
#define ZS_ROAD_EAST_Y 100
#define ZS_ROAD_WEST_Y 101
//JF road
#define JF_ROAD_LEN 200
#define JF_ROAD_SOUTH_X 150
#define JF_ROAD_NORTH_X 151

#define NUM_MAX_POS ((ZS_ROAD_LEN + JF_ROAD_LEN) * 2)

typedef struct _map_position{
  position p;
  uint chkpt;
  uint used; //for init, whether this pos can be assigned
}map_position;

map_position g_maps[NUM_MAX_POS];

void map_init(void)
{
  int x,y;
  int idx = 0;
  
  memset(g_maps, 0, sizeof(g_maps));

  position p;
  //add zs road postions
  for (x = 0; x < ZS_ROAD_LEN; x++)
  {
    //zs road east
    p.x = x;
    p.y = ZS_ROAD_EAST_Y;
    g_maps[idx].p = p;
    if (x == 0 || x == JF_ROAD_SOUTH_X || x == JF_ROAD_NORTH_X || x == ZS_ROAD_LEN)
      g_maps[idx].chkpt = 1;
    idx++;
    
    //zs road west
    p.x = x;
    p.y = ZS_ROAD_WEST_Y;
    g_maps[idx].p = p;
    if (x == 0 || x == JF_ROAD_SOUTH_X || x == JF_ROAD_NORTH_X || x == ZS_ROAD_LEN)
      g_maps[idx].chkpt = 1;
    idx++;
  }

  //add jf road postions
  for (y = 0; y < JF_ROAD_LEN; y++)
  {
    //no duplicate
    if (y == ZS_ROAD_EAST_Y || y ==ZS_ROAD_WEST_Y)
      continue; //these are cross pts covered by zs road
      
    //jf road north
    p.x = JF_ROAD_SOUTH_X;
    p.y = y;
    g_maps[idx].p = p;
    if (y == 0 || y == JF_ROAD_LEN)
      g_maps[idx].chkpt = 1;
    idx++;
    
    //jf road south
    p.x = JF_ROAD_NORTH_X;
    p.y = y;
    g_maps[idx].p = p;
    if (y == 0 || y == JF_ROAD_LEN)
      g_maps[idx].chkpt = 1;
    idx++;
  }

  if (idx >= NUM_MAX_POS)
    printf("ERRRRRRRRRRRROR! g_maps overflow %d\n", idx);
}



checkpoint cc_checkpoints[NUM_CHKPTS];
/*
                                         y pos:
y=200                                     CC                                    (300,200)
                                               v^
                                               v^
                                               v^
                                               v^
                                               v^
y=101  C<<<<<<<<<<<<<<<CC<<<<<<<<<<<<<<<<<<C
y=100  C>>>>>>>>>>>>>>>CC>>>>>>>>>>>>>>>>>>C   ZS Road
                                               v^
                                               v^
                                               v^ JF Road
                                               v^
                                               v^
                                               v^
         (0,0)                               CC               C=checkpoint
                                       x=150,151                                      x=300                                         
*/
void chkpt_init(void)
{

/*
  cc_checkpoints[NUM_CHKPTS] = {
  //v     pos    cnt   d0    d1
    (0, (0, 100), 1, ((1, 0), (0, 0))),
    (0, (0, 101), 1, ((0, -1), (0, 0))),
    (0, (150, 100), 2, ((1, 0), (0, -1))),
    (0, (150, 101), 2, ((0, -1), (-1, 0))),
  };
*/
}



int main(int argc, void* argv)
{
  map_init();
  chkpt_init();

  printf("ctrl_center running with %d beeltes...\n", n);

  //waiting for beelte init request
  //reply with: pos, dir, id, color, speed, etc.
  //reply with: checkpoints - let beetles control their ways

  //kick off...
  //  unblock all beetle "sensor"

  //waiting for beelte pos update
  //reply with: 
  
  return 0;

}
