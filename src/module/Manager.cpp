
#include "Manager.h"

int Manager::start() {
    isRunning = true;
    canStoped = false;
    return 0;
}

int Manager::stop() {
    isRunning = false;
    return 0;
}
