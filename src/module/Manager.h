#ifndef _MODULE_MANAGER_H_
#define _MODULE_MANAGER_H_

class Manager {
public:
    bool isRunning;
    Manager() {isRunning = false; canStoped = false;}
    virtual ~Manager() {};
    virtual int start();
    virtual int stop();
    virtual void retrieve() = 0;
    virtual void dump() = 0;
    bool running() {return isRunning;}
    bool canStoped;
    bool canstoped() {return canStoped;}
    void stoped() {canStoped = true;}
    void started() {canStoped = false;}
};


#endif
