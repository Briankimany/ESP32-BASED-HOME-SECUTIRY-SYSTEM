#include "states.h"
#include "sensors.h"

// Idle state.
void idle(){
    turnOffAlarm();
    displayHomeScreen();
}