#ifndef NODE_MYSQL_TASK_H
#define NODE_MYSQL_TASK_H

#include <v8.h>
#include <string>

namespace NodeMysql {

class Scheduler;

/**
 * Interface to handle asynchronous tasks. Allows to store JS callbacks.
 * Task::asyncWork() will be called asynchronously in 
 * unspecified thread. After it finishes Task::afterAsyncWork will be called
 * in main thread.
 * @see NodeMysql::Scheduler
 */
class Task {
public:
    Task();
    virtual void setCallback(v8::Persistent<v8::Function> success);
    void setScheduler(Scheduler* scheduler);
    Scheduler* getScheduler() const;
    //this will be called as asynchronous task
    virtual void asyncWork() = 0;
    //this will be called after asyncWork is finished in main thread
    virtual void afterAsyncWork() = 0;
    virtual ~Task();
protected:
    Scheduler* m_scheduler;
    std::string m_exception;
    v8::Persistent<v8::Function> m_callback;
};

}//NodeMysql

#endif //NODE_MYSQL_TASK_H

