#ifndef RELAYS_H
#define RELAYS_H

#include <Arduino.h>
#include "config.h"

enum RelayGroup { GROUP_A, GROUP_B };

void relays_init();
void relays_setGroup(RelayGroup group);
void relays_activateFirst(bool blockOther = true);
void relays_activateSecond(bool blockOther = true);
void relays_deactivateAll();
bool relays_isIdle();

#endif