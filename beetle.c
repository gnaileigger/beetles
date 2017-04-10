/*
 * 
 * @author      r.liang <xxx@xxx.xx>
 *
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"

#include "beetle.h"
#include "utils.h"

//MQTT settings

#define EMCUTE_PORT         (1883U)
#define EMCUTE_ID           ("gertrud")
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)

#define BEETLE_SUB_TOPIC "center/*"
#define BEETLE_PUB_TOPIC "beetle/*"

static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];

static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

//CHK PTS
static checkpoint checkpoints[NUM_CHKPTS];

static uint beetle_init_request(beetle* b)
{
  //pos
  //dir
  //speed
  //checkpoints
  ((void)b);
  //mqtt request

  //mqtt response
  return 0;

}

static int beetle_compare_position(position a, position b)
{
  if (a.x == b.x && a.y == b.y )
    return 0;
  else
    return 1;
}

// update the dir if it is in a checkpoint
static void beetle_dir_update(beetle* b)
{
  checkpoint cp;
  int i = 0;
  //if position is a checkpoint, update the dir
  for (i = 0; i < NUM_CHKPTS; i++)
  {
    cp = checkpoints[i];
    if (beetle_compare_position(b->pos, cp.pos) == 0)
    {
      uint rn = gen_rand() % (cp.dir_cnt); //random select a dir in this checkpoint
      b->pos.x = cp.possible_dirs[rn].x;
      b->pos.y = cp.possible_dirs[rn].y;
      break;
    }
  }
}

static void beetle_move(beetle* b)
{
  b->pos.x += b->dir.x;
  b->pos.y += b->dir.y;

  beetle_dir_update(b);
}

static uint g_sensor_distance;

static uint beetle_sensor_update(void)
{
  //semTake(sem_mqtt);
  return g_sensor_distance;
  
}

static void beetle_report(beetle b)
{
  //position update to ctrl_center
  //mqtt send: pos(x,y), id
  (void)b;
}

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
    char *in = (char *)data;

    printf("### got publication for topic '%s' [%i] ###\n",
           topic->name, (int)topic->id);
    for (size_t i = 0; i < len; i++) {
        printf("%c", in[i]);
    }
    puts("");
}


//sub hello/world
static void mqtt_subscribe(void)
{
  /* find empty subscription slot */
  unsigned i = 0;
  for (; (i < NUMOFSUBS) && (subscriptions[i].topic.id != 0); i++) {}
  if (i == NUMOFSUBS) {
      puts("error: no memory to store new subscriptions");
      return;
  }

  subscriptions[i].cb = mqtt_on_message;
  strcpy(topics[i], SUB_TOPIC);
  subscriptions[i].topic.name = topics[i];
  if (emcute_sub(&subscriptions[i], EMCUTE_QOS_0) != EMCUTE_OK) {
      printf("error: unable to subscribe to %s\n", SUB_TOPIC);
      return;
  }

  printf("Now subscribed to %s\n", SUB_TOPIC);

}

int beetle_start(int argc, char** argv)
{
  beetle b;
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

  //mqtt
  mqtt_connect();
  mqtt_subscribe();

  if(beetle_init_request(&b) != 0)
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
  
  return 0; //not reach
}

int beetle_stop(int argc, char** argv)
{
  //clean up resources
  if(!argc || !argv)
    return -1;

  return 0;
}
