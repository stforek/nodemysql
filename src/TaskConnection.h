#ifndef NODE_MYSQL_TASK_CONNECTION_H
#define NODE_MYSQL_TASK_CONNECTION_H

#include "ConnectionOptions.h"
#include "Connection.h"
#include "Task.h"

namespace NodeMysql {

class TaskConnection: public Task {
public:
    TaskConnection();
    void setConnectionOptions(ConnectionOptions* con);
    void setPoolId(unsigned int id);
    virtual ~TaskConnection();
protected:
    //this will be called as asynchronous task
    virtual void asyncWork();
    //this will be called after asyncWork is finished in main thread
    virtual void afterAsyncWork();
    ConnectionOptions* m_options;
    Connection *m_connection;
    unsigned int m_poolId;
};

}

#endif //NODE_MYSQL_TASK_CONNECTION_H

