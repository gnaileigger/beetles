/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#ifndef BEETLE_H
#define BEETLE_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SAFE_DISTANCE 3

#define SPEED_LOW 1
#define SPEED_MEDIUM 2

//check points settings
//#define NUM_CHKPTS 20
#define NUM_MAX_POSSIBLE_DIR 2

/*
   MQTT Topic define

    beetle
      sub: center/init_rsp_{id}, center/sensor_{id}
      pub: beetle/init_req_{id}, beetle/report_{id}

    center
      sub: beetle/\*
      pub: center/init_rsp_{all}, center/sensor_{all}
*/
#define BEETLE_SUB_INIT_RSP_ "center/init_rsp_"
#define BEETLE_SUB_SENSOR_ "center/sensor_"
#define BEETLE_PUB_INIT_REQ_ "beetle/init_req_"
#define BEETLE_PUB_REPORT_ "beetle/report_"

#define CENTER_SUB "beetle/*"
#define CENTER_PUB_INIT_RSP_ "center/init_rsp_"
#define CENTER_PUB_SENSOR_ "center/sensor_"

// structures

typedef unsigned int uint;

typedef struct _pos {
  int x;
  int y;
} position;

typedef struct _dir {
  int x;
  int y;
} direction;

/*
enum speed {
  SLOW = SPEED_LOW,
  MEDIUM = SPEED_MEDIUM,
  FAST = SPEED_MEDIUM,   //FAST: future
};
*/
typedef uint speed;

typedef struct _beetle {
  uint id;
  position pos;
  direction dir;
  //extensions
  uint speed;
  uint color;
  int sideward;
} beetle;

typedef struct _checkpoint {
  int valid;
  position pos;
  uint dir_cnt; //how many possible dirs this checkpoint has
  direction possible_dirs[NUM_MAX_POSSIBLE_DIR];  //what are they
}checkpoint;

typedef struct _beetle_init_req {
  int id;
}beetle_init_req;

typedef struct _center_init_rsp {
  beetle beetle_data;
  //checkpoint checkpoints[NUM_CHKPTS];
}center_init_rsp;

typedef struct _center_sensor {
  uint distance;
}center_sensor;

typedef struct _beetle_report {
  position pos;
}beetle_report;

//externs

extern int beetle_start(int argc, char** argv);
extern int beetle_stop(int argc, char** argv);
extern int atoi(const char *nptr);

/*
   Maps and Checkpoints

*/

/* -------------------- The MAP --------------------
y pos:
y=Ymax                 CC               (Xmax,Ymax)
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
Ymax                   v^
/2+1   C<<<<<<<<<<<<<<<CC<<<<<<<<<<<<<<<<<<C             Y:ZS_ROAD_WEST_Y
Ymax/2 C>>>>>>>>>>>>>>>CC>>>>>>>>>>>>>>>>>>C   ZS Road   Y:ZS_ROAD_EAST_Y
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^ JF Road
y=0                    CC             
    x=0                                  x=Xmas 
               x=Xmas:JF_ROAD_SOUTH_X
                        Xmas/2+1:JF_ROAD_NORTH_X  
                                                                                      
                  C=checkpoint
*/

//ZS road
#define ZS_ROAD_LEN 100
#define ZS_ROAD_EAST_Y (JF_ROAD_LEN/2)
#define ZS_ROAD_WEST_Y (JF_ROAD_LEN/2 + 1)
//JF road
#define JF_ROAD_LEN 60
#define JF_ROAD_SOUTH_X (ZS_ROAD_LEN/2)
#define JF_ROAD_NORTH_X (ZS_ROAD_LEN/2 + 1)

#define NUM_MAX_POS ((ZS_ROAD_LEN + JF_ROAD_LEN) * 2)

typedef struct _map_position{
  position p;
  uint chkpt;
  uint used; //for init, whether this pos can be assigned
}map_position;

#define DIR_UP {0,1}
#define DIR_DOWN {0,-1}
#define DIR_LEFT {-1,0}
#define DIR_RIGHT {1,0}
#define DIR_NULL {0,0}



#endif //BEETLE_H