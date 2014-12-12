#ifndef NODE_MYSQL_TASK_QUERY_FETCH_ALL_H
#define NODE_MYSQL_TASK_QUERY_FETCH_ALL_H

#include "TaskQuery.h"
#include <string>
#include <vector>
#include <map>

namespace NodeMysql {

class TaskQueryFetchAll: public TaskQuery {
public:
    TaskQueryFetchAll();
    virtual ~TaskQueryFetchAll();
protected:
    //this will be called as asynchronous task
    virtual void asyncWork();
    //this will be called after asyncWork is finished in main thread
    virtual void afterAsyncWork();
    std::vector<std::map<std::string, std::string>*>* m_rows;
};

}//NodeMysql

#endif //NODE_MYSQL_TASK_QUERY_FETCH_ALL_H

