#ifndef NODE_MYSQL_TASK_QUERY_EXECUTE_H
#define NODE_MYSQL_TASK_QUERY_EXECUTE_H

#include "TaskQuery.h"
#include <string>
#include <vector>

namespace NodeMysql {

class TaskQueryExecute: public TaskQuery {
public:
    TaskQueryExecute();
    void fetchLastInsertId(bool flag);
    virtual ~TaskQueryExecute();
protected:
    //this will be called as asynchronous task
    virtual void asyncWork();
    //this will be called after asyncWork is finished in main thread
    virtual void afterAsyncWork();
    int m_affected;
    bool m_fetchLastInsertId;
    long long int m_lastInsertId;
};

}//NodeMysql

#endif //NODE_MYSQL_TASK_QUERY_EXECUTE_H

