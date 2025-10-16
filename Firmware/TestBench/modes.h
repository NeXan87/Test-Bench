#ifndef MODES_H
#define MODES_H

void modes_init();
void modes_run();
void modes_reset();
unsigned long modes_getCycleElapsedTime();
bool modes_isWaitingForUserAction();
void modes_clearWaitingState();
void modes_resetCycleData();
void modes_forceIdle();
const char* modes_getStatus();
bool modes_isWorking();
bool modes_isFinished();

#endif