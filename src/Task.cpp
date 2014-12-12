#include "Task.h"
#include "TaskConnection.h"

namespace NodeMysql {

Task::Task():
        m_scheduler(NULL) {
}

void Task::setCallback(v8::Persistent<v8::Function> success) {
    m_callback = success;
}

void Task::setScheduler(Scheduler* scheduler) {
    m_scheduler = scheduler;
}

Scheduler* Task::getScheduler() const {
    return m_scheduler;
}

Task::~Task() {
    //it's not clear to me if I need to create weak callbacks
    //also I don't know if destructor calls Dispose() or not
    //http://create.tpsitulsa.com/wiki/V8/Persistent_Handles
    if (!m_callback.IsEmpty()) {
        m_callback.Dispose();
        m_callback.Clear();
    }
}

}//NodeMysql
