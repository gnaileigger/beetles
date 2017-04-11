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
#define NUM_CHKPTS 20
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
#define BEETLE_PUB_REPORT_ "eetle/report_"

#define CENTER_SUB "beetle/*"
#define CENTER_PUB_INIT_RSP "center/init_rsp_"
#define CENTER_PUB_SENSOR "center/sensor_"

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
  speed spd;
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
  int id;
}center_init_rsp;


//externs

extern int beetle_start(int argc, char** argv);
extern int beetle_stop(int argc, char** argv);

#endif //BEETLE_H