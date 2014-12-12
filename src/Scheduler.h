#ifndef NODE_MYSQL_SCHEDULER_H
#define NODE_MYSQL_SCHEDULER_H

#include <vector>
#include <uv.h>

namespace NodeMysql {

class Task;

/**
 * This class schedules tasks in uv_queue_work. It handles all neccessary calls
 * to uv library.
 * It also gives option to set maximum number of threads used by given instance.
 * UV library doesn't specify how many threads uv_queue_work will use.
 * Because mysql API is not fully thread safe, I needed to implement such class.
 * @see NodeMysql::Task
 */
class Scheduler {
public:
    /**
     * @param maxConcurrent indicates how many threads can be used - how many
     * concurrent tasks can be run
     */
    Scheduler(unsigned int maxConcurrent);
    /**
     * Add Task interface to queue. Task::asyncWork() will be called
     * asynchronously in unspecified thread. After it finishes
     * Task::afterAsyncWork will be called in main thread.
     * @param task
     */
    void addTask(Task* task);
    virtual ~Scheduler();
private:
    unsigned int m_maxConcurrent;
    //current number of tasks in progress
    unsigned int m_concurrent;
    std::vector<Task*> m_tasks;
    uv_mutex_t m_tasksLock;
    //this will be called as asynchronous task
    void static workCallback(uv_work_t* req);
    //this will be called after workCallback is finished in main thread
    void static afterWorkCallback(uv_work_t* req, int status);
    void runNextTask();
    /**
     * Queue task in uv_queue_work
     * It doesn't lock m_tasksLock, you need to do this, because this
     * method touches m_concurrent.
     * @param task
     */
    void runTask(Task* task);
};

}//NodeMysql

#endif //NODE_MYSQL_SCHEDULER_H

