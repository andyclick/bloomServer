#include "LogStatisticManager.h"

LogStatisticManager::LogStatisticManager(LogGlobalCtrl * pLogGlobalCtrl) {
    m_pLogGlobalCtrl = pLogGlobalCtrl;

}

LogStatisticManager::~LogStatisticManager() {

}

int LogStatisticManager::start() {
    Manager::start();
    return 0;

}

int LogStatisticManager::stop() {
    Manager::stop();
}

void LogStatisticManager::dump() {}
void LogStatisticManager::retrieve() {}
