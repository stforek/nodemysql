#include "Scheduler.h"
#include "Task.h"

namespace NodeMysql {

Scheduler::Scheduler(unsigned int maxConcurrent):
        m_maxConcurrent(maxConcurrent),
        m_concurrent(0) {
    uv_mutex_init(&m_tasksLock);
}

void Scheduler::addTask(Task* task) {
    uv_mutex_lock(&m_tasksLock);
    //if max number of concurrent tasks is set and it is lower than
    //currently processing number of threads, add it to queue
    if (m_maxConcurrent > 0 && m_concurrent >= m_maxConcurrent) {
        m_tasks.push_back(task);
    } else {
        //we have free space in queue, schedule task in uv
        runTask(task);
    }
    uv_mutex_unlock(&m_tasksLock);
}

void Scheduler::workCallback(uv_work_t* req) {
    Task* task = static_cast<Task*>(req->data);
    task->asyncWork();
}

void Scheduler::afterWorkCallback(uv_work_t* req, int status) {
    Task* task = static_cast<Task*>(req->data);
    task->afterAsyncWork();
    //decrease number of current tasks in progress
    Scheduler* scheduler = task->getScheduler();
    delete task;
    delete req;
    uv_mutex_lock(&scheduler->m_tasksLock);
    scheduler->m_concurrent--;
    uv_mutex_unlock(&scheduler->m_tasksLock);
    //check if there is another task to be run
    scheduler->runNextTask();
}

void Scheduler::runNextTask() {
    uv_mutex_lock(&m_tasksLock);
    //run next task only if there is available task and if
    //there is no limit in concurrent number of tasks
    //or limit is lower than current number of tasks
    if (!m_tasks.empty() && (m_maxConcurrent == 0 || m_concurrent < m_maxConcurrent)) {
        auto it = m_tasks.begin();
        runTask(*it);
        m_tasks.erase(it);
    }
    uv_mutex_unlock(&m_tasksLock);
}

void Scheduler::runTask(Task* task) {
    ++m_concurrent;
    uv_loop_t* loop = uv_default_loop();
    uv_work_t* request = new uv_work_t();
    request->data = task;
    task->setScheduler(this);
    uv_queue_work(loop, request, workCallback, afterWorkCallback);
}


Scheduler::~Scheduler() {
    uv_mutex_destroy(&m_tasksLock);
}

}//NodeMysql
