/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sema.h>

#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"
#include "random.h"

#include "beetle.h"
#include "utils.h"

//MQTT settings

#define EMCUTE_PORT         (1883U)
#define EMCUTE_ID           ("gertrud")
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)

#define TOPIC_TEST "test/test"
#define BEETLE_SUB_TOPIC "center/*"
#define BEETLE_PUB_TOPIC_INITREQ "beetle/initreq"

//var

static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];
static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

static sema_t sem_init;
static sema_t sem_sensor;

static beetle self_beetle;

static uint g_sensor_distance;

checkpoint g_checkpoints[] = {
  //valid? pos  dir_cnt  dir0      dir1
    {0, {0, ZS_ROAD_EAST_Y},   1, {DIR_RIGHT, DIR_NULL}},
    {0, {0, ZS_ROAD_WEST_Y},   1, {DIR_DOWN, DIR_NULL}},

    {0, {JF_ROAD_SOUTH_X, 0},   1, {DIR_RIGHT, DIR_NULL}},
    {0, {JF_ROAD_SOUTH_X, ZS_ROAD_EAST_Y}, 2, {DIR_RIGHT, DIR_DOWN}},
    {0, {JF_ROAD_SOUTH_X, ZS_ROAD_WEST_Y}, 2, {DIR_DOWN,DIR_LEFT}},
    {0, {JF_ROAD_SOUTH_X, JF_ROAD_LEN}, 1, {DIR_DOWN, DIR_NULL}},

    {0, {JF_ROAD_NORTH_X, 0},   1, {DIR_UP, DIR_NULL}},
    {0, {JF_ROAD_NORTH_X, ZS_ROAD_EAST_Y}, 2, {DIR_UP, DIR_RIGHT}},
    {0, {JF_ROAD_NORTH_X, ZS_ROAD_WEST_Y}, 2, {DIR_UP, DIR_LEFT}},
    {0, {JF_ROAD_NORTH_X, JF_ROAD_LEN}, 1, {DIR_LEFT, DIR_NULL}},

    {0, {ZS_ROAD_LEN, ZS_ROAD_EAST_Y}, 1, {DIR_UP, DIR_NULL}},
    {0, {ZS_ROAD_LEN, ZS_ROAD_WEST_Y}, 1, {DIR_LEFT, DIR_NULL}},
  };

/*
 * functions
 */

// MQTT APIs //

static void *emcute_thread(void *arg)
{
    (void)arg;
    emcute_run(EMCUTE_PORT, EMCUTE_ID);
    return NULL;    /* should never be reached */
}

//con fec0:affe::1 1885
static void mqtt_connect(void)
{
  char gw_ip[] = "fec0:affe::1";
  sock_udp_ep_t gw = { .family = AF_INET6, .port = EMCUTE_PORT };
  ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, gw_ip);
  //gw.port = (uint16_t)atoi("1885");
  gw.port = 1885;

  if (emcute_con(&gw, true, NULL, NULL, 0, 0) != EMCUTE_OK) {
      printf("error: unable to connect to [%s]:%i\n", gw_ip, (int)gw.port);
  }
  else
  {
    printf("Successfully connected to [%s]:%i\n", gw_ip, (int)gw.port);
  }
}

static void mqtt_on_message(const emcute_topic_t *topic, void *data, size_t len)
{
  int ret;
  //printf("[beetle] receive topic (%s)\n", topic->name);

  ((void)len);

  //receive init_rsp
  if ((ret=strncmp(topic->name, BEETLE_SUB_INIT_RSP_, 
    sizeof(BEETLE_SUB_INIT_RSP_)-1)) == 0)
  {
    self_beetle = ((center_init_rsp *)data)->beetle_data;
    sema_post(&sem_init);
  }
  else if ((ret=strncmp(topic->name, BEETLE_SUB_SENSOR_, 
    sizeof(BEETLE_SUB_SENSOR_)-1)) == 0)
  {
    g_sensor_distance = ((center_sensor *)data)->distance;
    sema_post(&sem_sensor);
  }
  else
  {
    puts("else!");
  }

}

//sub hello/world
static void mqtt_subscribe(char* topic)
{
  /* find empty subscription slot */
  unsigned i = 0;
  for (; (i < NUMOFSUBS) && (subscriptions[i].topic.id != 0); i++) {}
  if (i == NUMOFSUBS) {
      puts("error: no memory to store new subscriptions");
      return;
  }

  subscriptions[i].cb = mqtt_on_message;
  strcpy(topics[i], topic);
  subscriptions[i].topic.name = topics[i];
  if (emcute_sub(&subscriptions[i], EMCUTE_QOS_0) != EMCUTE_OK) {
      printf("error: unable to subscribe to %s\n", topic);
      return;
  }

  printf("Now subscribed to %s\n", topic);

}

//pub hello/world "it is good!"
void mqtt_publish(char* topic, void* data, uint len)
{
  emcute_topic_t t;
  t.name = topic;
  emcute_reg(&t);
  emcute_pub(&t, data, len, EMCUTE_QOS_0);
  //printf("Published %i bytes to topic '%s [%i]'\n", (int)len, t.name, t.id);
}
static int beetle_compare_position(position a, position b)
{
  if (a.x == b.x && a.y == b.y )
    return 0;
  else
    return 1;
}

// update the dir if it is in a checkpoint
static void beetle_dir_update(void)
{
  checkpoint cp;
  int cp_cnt = sizeof(g_checkpoints)/sizeof(checkpoint);
  int i = 0;
  //if position is a checkpoint, update the dir
  for (i = 0; i < cp_cnt; i++)
  {
    cp = g_checkpoints[i];
    if (beetle_compare_position(self_beetle.pos, cp.pos) == 0)
    {
      uint rn = random_uint32() % (cp.dir_cnt); //random select a dir in this checkpoint
      self_beetle.dir.x = cp.possible_dirs[rn].x;
      self_beetle.dir.y = cp.possible_dirs[rn].y;
      //printf("checkpoint! dir=(%d,%d)\n", self_beetle.dir.x, self_beetle.dir.y);
      break;
    }
  }
}

static void beetle_move(void)
{
  //printf("before move: (%d,%d), dir(%d,%d)\n", self_beetle.pos.x,self_beetle.pos.y,self_beetle.dir.x,self_beetle.dir.y);
  self_beetle.pos.x += self_beetle.dir.x;
  self_beetle.pos.y += self_beetle.dir.y;
  printf("[Beetle 0001] (%03d, %03d)\n", self_beetle.pos.x, self_beetle.pos.y);

  beetle_dir_update();
}


static uint beetle_sensor_update(void)
{
  sema_wait(&sem_sensor);
  return g_sensor_distance;
  
}

static void beetle_report_send(void)
{
  //position update to ctrl_center
  //mqtt send: pos(x,y), id
  char topic[32];
  sprintf(topic, "%s%s", BEETLE_PUB_REPORT_, "0001");
  beetle_report br;
  br.pos = self_beetle.pos;
  mqtt_publish(topic, &br, sizeof(beetle_report));
}

int beetle_start(int argc, char** argv)
{
  uint distance;

  ((void)argc);
  ((void)argv);

  /*
    how to connect to mqtt broker

    Broker: RSMB (Mosquitto Real Simple Message Broker)
            git clone https://github.com/eclipse/mosquitto.rsmb.git
            Detail please see examples/emute/README.md

    Client: emCute (provided by RIOT)

    TAP is required, to enable:
      ./RIOTDIR/dist/tools/tapsetup/tapsetup

    Set host IPv6:
      sudo ip a a fec0:affe::1/64 dev tapbr0

    Set client IP (in RIOT shell):
      ifconfig 6 add fec0:affe::99  //I dont know why '6', emCute.elf is '5'
    
  */

  /* the main thread needs a msg queue to be able to run `ping6`*/
  msg_init_queue(queue, (sizeof(queue) / sizeof(msg_t)));

  /* initialize our subscription buffers */
  memset(subscriptions, 0, (NUMOFSUBS * sizeof(emcute_sub_t)));

  /* start the emcute thread */
  thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0,
                emcute_thread, NULL, "emcute");

  /* semaphore */
  sema_create(&sem_init, 0);

  //mqtt
  mqtt_connect();
  mqtt_subscribe("center/init_rsp_0001");
  mqtt_subscribe("center/sensor_0001");

  beetle_init_req req;
  req.id = 1;
  mqtt_publish("beetle/init_req_0001", &req, sizeof(beetle_init_req));

  sema_wait(&sem_init);
  //puts("Init response received!");  

  //main loop
  while(1){
    distance = beetle_sensor_update();

    if (distance > SAFE_DISTANCE){
      beetle_move();
    }

    beetle_report_send();

  } //while(1)
  
  return 0; //not reach
}

int beetle_stop(int argc, char** argv)
{
  //clean up resources
  if(!argc || !argv)
    return -1;

  return 0;
}
