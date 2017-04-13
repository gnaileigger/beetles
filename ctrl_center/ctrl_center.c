/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include <pthread.h>
#include <semaphore.h>

#include "../beetle.h"
#include "mosquitto.h"

sem_t sem_init_req;
sem_t sem_report;
struct mosquitto* g_mosq;


/* -------------------- The MAP --------------------
y pos:
y=200                  CC               (300,200)
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
y=101  C<<<<<<<<<<<<<<<CC<<<<<<<<<<<<<<<<<<C
y=100  C>>>>>>>>>>>>>>>CC>>>>>>>>>>>>>>>>>>C   ZS Road
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^ JF Road
(0,0)                  CC             
                   x=150,151             x=300                                         
                                        C=checkpoint
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

/* -------------------- The MAP --------------------
y pos:
y=200                  CC               (300,200)
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
y=101  C<<<<<<<<<<<<<<<CC<<<<<<<<<<<<<<<<<<C
y=100  C>>>>>>>>>>>>>>>CC>>>>>>>>>>>>>>>>>>C   ZS Road
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^
                       v^ JF Road
    (0,0)              CC             
      x=0          x=150,151             x=300                                         
                                        C=checkpoint
*/
checkpoint g_checkpoints[] = {
  //valid? pos  dir_cnt  dir0      dir1
    {0, {0, 100},   1, {{1, 0},   {0, 0}}},
    {0, {0, 101},   1, {{0, -1},  {0, 0}}},

    {0, {150, 0},   1, {{1, 0},  {0, 0}}},
    {0, {150, 100}, 2, {{1, 0},   {0, -1}}},
    {0, {150, 101}, 2, {{0, -1},  {-1, 0}}},
    {0, {150, 200}, 1, {{0, -1},  {0, 0}}},

    {0, {151, 0},   1, {{0, 1},  {0, 0}}},
    {0, {151, 100}, 2, {{0, 1},   {1, 0}}},
    {0, {151, 101}, 2, {{0, 1},  {-1, 0}}},
    {0, {151, 200}, 1, {{-1, 0},  {0, 0}}},

    {0, {300, 100}, 1, {{0, 1},   {0, 0}}},
    {0, {300, 101}, 1, {{-1, 0},  {0, 0}}},
  };


extern int mosquitto_lib_init();
struct mosquitto* client;


static void on_connect(struct mosquitto *mosq, void *obj, int result)
{
  if (mosquitto_subscribe(g_mosq, NULL, "beetle/+", 0) != 0) 
    printf("error mosquitto_subscribe\n");

  //mosquitto_subscribe(g_mosq, NULL, "beetle/report_0001", 0);
}

static void on_publish(struct mosquitto *mosq, void *userdata, int mid)
{
  //printf("[center] publish --->\n");
}

static void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
   //printf("4. on_subscribe\n");
}

/*
   beetle              ctrl_center
  =================================

   init_req_ID -->       
               <--     init_rsp_ID
     (do this for all beetles)

                       wait: n sec
               <--     sensor_update_ID
   report_ID   -->       
     (do this for all beetles)

                       map update
                       loop "wait"   


beetle
  sub: center/init_rsp_{id}, center/sensor_{id}
  pub: beetle/init_req_{id}, beetle/report_{id}

center
  sub: beetle/*
  pub: center/init_rsp_{all}, center/sensor_{all}
*/

#define NUM_OF_BEETLE 1
//#define MOSQ_ERR_SUCCESS 0


int cnt_init_req = 0;
int cnt_report = 0;

void on_message(struct mosquitto* mosq, void* obj, const struct mosquitto_message* msg)
{
  int i;
  int ret=1234;

  char* rx_topic = (char *)msg->topic;
  printf("[center] receive msg (%s)\n", rx_topic);

  //beetle init request
  if ((ret=strncmp(rx_topic, BEETLE_PUB_INIT_REQ_, sizeof(BEETLE_PUB_INIT_REQ_)-1)) == 0)
  {
    //in topic: BEETLE_PUB_INIT_REQ_nnnn ==> out topic: CENTER_PUB_INIT_RSP_nnnn
    printf("%s\n", rx_topic);
    int n_pos = sizeof(BEETLE_PUB_INIT_REQ_)-1;
    printf("%d\n", n_pos);
    char *nnnn = rx_topic + n_pos;
    printf("%s\n", nnnn);

    char topic[32];
    sprintf(topic, "%s%s", CENTER_PUB_INIT_RSP_, nnnn);
    //printf("topic to go: %s\n", topic);

    beetle b;
    b.id = atoi(nnnn);
    b.pos.x = 10;
    b.pos.y = 100;
    b.dir.x = 1;
    b.dir.y = 0;
    b.speed = 1;
    b.color = 0x00ff0000;
    b.sideward = 0;

    center_init_rsp rsp;
    rsp.beetle_data = b;

    mosquitto_publish(g_mosq, NULL, topic, sizeof(rsp), &rsp, 0, NULL);

    //if pub all rsp to all beetles, let main thread continue
    cnt_init_req++;
    if (cnt_init_req == NUM_OF_BEETLE)
    {
      sem_post(&sem_init_req);
      cnt_init_req = 0;
    }
  }
  else if ((ret=strncmp(rx_topic, BEETLE_PUB_REPORT_, 
    sizeof(BEETLE_PUB_REPORT_)-1)) == 0) //beetle report
  {
    position pos;
    pos = ((beetle_report *)(msg->payload))->pos;
    //printf("Got position (%d,%d)\n",pos.x, pos.y);

    cnt_report++;
    if (cnt_report == NUM_OF_BEETLE)
    {
      sem_post(&sem_report);
      cnt_report = 0;
    }
  }
  else
  {
    printf("ret=%d,%s,%s,%d\n",
      ret,rx_topic,BEETLE_PUB_INIT_REQ_,sizeof(BEETLE_PUB_INIT_REQ_)-1);
  }
}

void* mqtt_listener()
{

  mosquitto_lib_init();

  if(!(g_mosq = mosquitto_new("ctrl_center", 1, NULL))) {
    printf("[master]mosquitto_new failed!!!\n");
    goto ERROR;
  }

  mosquitto_connect_callback_set(g_mosq, on_connect);
  mosquitto_publish_callback_set(g_mosq, on_publish);
  mosquitto_message_callback_set(g_mosq, on_message);
  mosquitto_subscribe_callback_set(g_mosq, on_subscribe);

  if(mosquitto_connect(g_mosq, "localhost", 1886, 60) != MOSQ_ERR_SUCCESS) {
    printf("[master]Failed to connect to broker!!!\n");
    goto ERROR;
  }

  printf("[master] mosquitto_loop_forever...\n");
  mosquitto_loop_forever(g_mosq, -1, 1);

  printf("[master] mosquitto_disconnect...\n");
  mosquitto_disconnect(g_mosq);
  mosquitto_destroy(g_mosq);
  mosquitto_lib_cleanup();

  printf("[master] is exiting.\n");
  return NULL;

ERROR:
  printf("[master] Error occured, exiting...\n");
  if(g_mosq){
     mosquitto_disconnect(g_mosq);
     mosquitto_destroy(g_mosq);
     g_mosq = 0;
     mosquitto_lib_cleanup();
  }
  return NULL;
}

int main(int argc, void* argv)
{
  int bcnt = NUM_OF_BEETLE;
  int cnt = 0;
  char topic[32];
  int i;

  sem_init(&sem_init_req, 0, 0);
  sem_init(&sem_report, 0, 0);

  map_init();
  
  //thread to receive mqtt msg
  pthread_t mqtt_thread;
  pthread_create(&mqtt_thread, NULL, mqtt_listener, NULL);

  //this sem will be posted when all init_req been handled
  sem_wait(&sem_init_req); 

  while(1)
  {
    usleep(1000*1000); //1s

    // simulate sensor data and send to each beetle
    for(i = 1; i <= NUM_OF_BEETLE; i++)
    {
      sprintf(topic, "center/sensor_%04d", i);
      //printf("topic: %s\n", topic);
      center_sensor cs;
      cs.distance = 7;
      mosquitto_publish(g_mosq, NULL, topic, sizeof(center_sensor), &cs, 0, NULL);
    }

    //@beetle:
    //beetle rx
    //beetle action
    //beetle report

    //this sem will be posted when all reports been handled
    sem_wait(&sem_report);

    //update map, locations, etc
    //map_update();
    printf("update map...\n");
  }
}

/*
  notes

  - mosquittion seems has 2, standard mosquito and rsmb (really simple mqtt broker)
  - launch rsmb: ./broker_mqtts config.conf
        CWNAN0300I MQTT-S protocol starting, listening on port 1885
        CWNAN0014I MQTT protocol starting, listening on port 1886

        1885: beetles (RIOT) connect to
        1886: center (using standard mosquitto API) connect to

        These 2 ports can be talked each other! 
        see the borker_mqtt console logs, gertrud <-> MQTT-S, it is the reason?
      
*/