#ifndef NODE_MYSQL_TASK_QUERY_H
#define NODE_MYSQL_TASK_QUERY_H

#include "Task.h"
#include <string>
#include <vector>

namespace NodeMysql {

class Connection;

class TaskQuery: public Task {
public:
    TaskQuery();
    void setConnection(Connection* connection);
    Connection* getConnection();
    void setQuery(const std::string& query,
            std::vector<std::string>* parameters);
    
    virtual ~TaskQuery();
protected:
    std::string m_query;
    std::vector<std::string>* m_parameters;
private:
    Connection* m_connection;
};

}//NodeMysql

#endif //NODE_MYSQL_TASK_QUERY_H

