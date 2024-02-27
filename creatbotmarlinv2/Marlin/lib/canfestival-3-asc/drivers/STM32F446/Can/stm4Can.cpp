#include <config.h>

#ifdef DRIVER_USE_CAN

  #include "../../../inc/data.h"
  #include "../../../inc/timer.h"
  #include "canfestival.h"
  #include "stm4Can.h"

static CO_Data      *co_data = NULL;
static STM32_CAN     stm32_can(CANx, CANp);
static CAN_message_t can;
static TIMER_HANDLE  runTimer = TIMER_NONE;

static void canLoop(CO_Data *d, UNS32 id);

void canClose(void) {}

UNS8 canInit(CO_Data *d, uint32_t bitrate) {
  co_data = d;
  stm32_can.begin();
  stm32_can.setBaudRate(bitrate);
  initTimer();

  DelAlarm(runTimer);
  runTimer = SetAlarm(co_data, 0, &canLoop, MS_TO_TIMEVAL(500), US_TO_TIMEVAL(50));

  return 0;
}

UNS8 canSend(CAN_PORT notused, Message *m) {
  CAN_message_t msg;

  msg.id             = m->cob_id;
  msg.flags.extended = 0;
  msg.flags.remote   = m->rtr;
  msg.len            = m->len;
  memcpy(msg.buf, m->data, 8);

  return !stm32_can.write(msg);
}

static void canLoop(CO_Data *d, UNS32 id) {
  CAN_message_t msg;
  Message       m;
  while (stm32_can.read(msg)) {
    if (msg.flags.extended == 1) continue;

    m.cob_id = msg.id;
    m.rtr    = msg.flags.remote;
    m.len    = msg.len;
    memcpy(m.data, msg.buf, 8);

    canDispatch(co_data, &m);
  }
}

#endif  // DRIVER_USE_CAN
