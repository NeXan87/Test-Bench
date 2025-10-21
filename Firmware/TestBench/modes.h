#ifndef MODES_H
#define MODES_H

void modes_init();
void modes_run();
void modes_reset();
unsigned long modes_getCycleElapsedTime();
const char* modes_getStatus();
bool modes_isWorking();
bool modes_isPaused();
bool modes_isFinished();
bool modes_isReady();

#endif