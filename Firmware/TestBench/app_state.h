#ifndef APP_STATE_H
#define APP_STATE_H

#include <Arduino.h>

enum Mode { MODE_MANUAL_BLOCKING = 1,
            MODE_MANUAL_INDEPENDENT = 2,
            MODE_SYNC_AUTO = 3,
            MODE_ASYNC_AUTO = 4
};

void app_state_init();
void app_state_update();
bool app_state_readSwitches();

Mode app_state_getMode();
bool app_state_getGroupA();
unsigned long app_state_getRelay1Time();
unsigned long app_state_getDelay1Time();
unsigned long app_state_getRelay2Time();
unsigned long app_state_getDelay2Time();
int app_state_getCycleLimit();
bool app_state_getInfiniteCycles();
bool app_state_isIdle();
void app_state_setCurrentCycle(int cycle);
int app_state_getCurrentCycle();

#endif