#ifndef NODE_MYSQL_TASK_TRANSACTION_H
#define NODE_MYSQL_TASK_TRANSACTION_H

#include "Task.h"

namespace NodeMysql {

class Connection;

enum TransactionType {
    TRANSACTION_BEGIN,
    TRANSACTION_COMMIT,
    TRANSACTION_ROLLBACK
};

class TaskTransaction: public Task {
public:
    TaskTransaction(TransactionType type);
    void setConnection(Connection* connection);
    Connection* getConnection();
    virtual ~TaskTransaction();
protected:
    //this will be called as asynchronous task
    virtual void asyncWork();
    //this will be called after asyncWork is finished in main thread
    virtual void afterAsyncWork();
private:
    TransactionType m_type;
    Connection* m_connection;

};

}//NodeMysql

#endif //NODE_MYSQL_TASK_TRANSACTION_H

