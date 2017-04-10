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


//check points settings
#define NUM_CHKPTS 20
#define NUM_MAX_POSSIBLE_DIR 2

typedef struct _checkpoint {
  int valid;
  position pos;
  uint dir_cnt; //how many possible dirs this checkpoint has
  direction possible_dirs[NUM_MAX_POSSIBLE_DIR];  //what are they
}checkpoint;

extern int beetle_start(int argc, char** argv);
extern int beetle_stop(int argc, char** argv);

#endif //BEETLE_H