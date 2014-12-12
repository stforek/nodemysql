#ifndef NODE_MYSQL_TASK_QUERY_FETCH_ROW_H
#define NODE_MYSQL_TASK_QUERY_FETCH_ROW_H

#include "TaskQuery.h"
#include <string>
#include <vector>
#include <map>

namespace NodeMysql {

class TaskQueryFetchRow: public TaskQuery {
public:
    TaskQueryFetchRow();
    virtual ~TaskQueryFetchRow();
protected:
    //this will be called as asynchronous task
    virtual void asyncWork();
    //this will be called after asyncWork is finished in main thread
    virtual void afterAsyncWork();
    std::map<std::string, std::string>* m_row;
};

}//NodeMysql

#endif //NODE_MYSQL_TASK_QUERY_FETCH_ROW_H

