#include <stdexcept>

#include "TaskQuery.h"

namespace NodeMysql {

TaskQuery::TaskQuery():
        m_query(""),
        m_parameters(NULL),
        m_connection(NULL) {
}

void TaskQuery::setConnection(Connection* connection) {
    //TaskQuery never owns Connection object, so it won't be deleted in this class
    m_connection = connection;
}

Connection* TaskQuery::getConnection() {
    if (!m_connection) {
        throw new std::runtime_error("Connection is null");
    }
    return m_connection;
}

void TaskQuery::setQuery(const std::string& query, std::vector<std::string>* parameters) {
    m_query = query;
    m_parameters = parameters;
}

TaskQuery::~TaskQuery() {
    delete m_parameters;
}

}
