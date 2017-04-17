/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include "../beetle.h"

#define MAP_SIZE (ZS_ROAD_LEN*JF_ROAD_LEN)
#define MAP_WIDTH ZS_ROAD_LEN
#define MAP_HEIGH JF_ROAD_LEN

/*

++++1+++++2++

+ road
1~9 beetle#
NULL blocks (unavailable area)

*/
typedef struct _map_elem{
  //position pos;
  //'+'-road; '#n'-beetle#; '(space)'-block
  char type; 
}map_elem;

static map_elem g_map_view[MAP_SIZE];

static position idx2pos(uint n)
{
  int width = MAP_WIDTH;
  position pos;
  
  pos.x = n/width;
  pos.y = n%width;

  return pos;
}

static uint pos2idx(position pos)
{
  int width = MAP_WIDTH;
  int ret;

  ret = ((MAP_HEIGH - pos.y) * width + pos.x);
  //printf("pos2idx: %d,%d -> %d\n",pos.x, pos.y, ret);

  return ret;
}

static void block_init()
{
  int i;

  for (i = 0; i < MAP_SIZE; i++)
  {
    g_map_view[i].type = ' ';
  }
}

static void road_init()
{
  int i;
  position pos;
  int idx;
  int width = MAP_WIDTH;

  //(0,ZS_ROAD_EAST_Y) -> (ZS_ROAD_LEN, ZS_ROAD_EAST_Y)
  pos.x = 0;
  pos.y = ZS_ROAD_EAST_Y;
  idx = pos2idx(pos);
  i = 0;
  while(i < width)
  {
  	g_map_view[idx++].type = '>';
  	i++;
  }
  //(0,ZS_ROAD_WEST_Y) -> (ZS_ROAD_LEN, ZS_ROAD_WEST_Y)
  pos.x = 0;
  pos.y = ZS_ROAD_WEST_Y;
  idx = pos2idx(pos);
  i = 0;
  while(i < width)
  {
  	g_map_view[idx++].type = '<';
  	i++;
  }
  //(JF_ROAD_SOUTH_X,0) -> (JF_ROAD_SOUTH_X, JF_ROAD_LEN)
  for (i = 0; i < MAP_SIZE; i++)
  {
    if (i%width == JF_ROAD_SOUTH_X) 
    {
  	  g_map_view[i].type = 'v';
    }
  }
  //(JF_ROAD_NORTH_X,0) -> (JF_ROAD_NORTH_X, JF_ROAD_LEN)
  for (i = 0; i < MAP_SIZE; i++)
  {
    if (i%width == JF_ROAD_NORTH_X) 
    {
  	  g_map_view[i].type = '^';
    }
  }
}

void view_update(position pos, uint type)
{
  int idx;

  idx = pos2idx(pos);
  if (idx < 0)
  {
  	printf("ERRRRROR position (%d,%d) not found!\n", pos.x, pos.y);
  	return;
  }
  g_map_view[idx].type = type;   
}

void view_display()
{
  int i,j;
  int width = MAP_WIDTH;

  //g_map_view[3000].type = 'K';
  //g_map_view[2900].type = 'K';

  printf("\n==================== MAP ==============================\n");
  printf("  Figure: \n");
  printf("    +   Road\n");
  printf("    n  Beetle #N\n");
  printf("==================================================\n");
  for (i = 0; i < MAP_SIZE; i++)
  {
  	if (i%width == 0) printf("\n");
  	printf("%c", g_map_view[i].type);
  }
}

void view_init()
{
  block_init();
  road_init();
}

void view_reset()
{
  view_init();
}

static void main()
{
  view_init();
  view_display();
}
