#ifndef NODE_MYSQL_CONNECTION_H
#define NODE_MYSQL_CONNECTION_H

#include "ConnectionOptions.h"
#include "Scheduler.h"
#include <string>
#include <vector>
#include <map>

namespace sql {
class Connection;
}

namespace NodeMysql {

typedef std::map<std::string, std::string> Row;
typedef std::vector<Row*> RowsList;

class Connection {
public:
    Connection(ConnectionOptions* options);
    void addTask(Task* task);
    /**
     * Executes given query with parameters
     * @param query
     * @param parameters can be NULL
     * @return number of affected rows
     */
    int execute(const std::string &query, std::vector<std::string>* parameters);
    /**
     * Fetches all rows from query.
     * @param query
     * @param parameters can be NULL
     * @return vector or Row*. Can be empty, never NULL
     */
    std::vector<Row*>* fetchAll(const std::string &query, std::vector<std::string>* parameters);
    /**
     * Fetches only one row from query, even if query returns more rows
     * @param query
     * @param parameters can be NULL
     * @return Row*, can be NULL if query returns no rows
     */
    Row* fetchRow(const std::string &query, std::vector<std::string>* parameters);
    /**
     * Checks if we have still connection.
     * @return
     */
    bool isConnected();
    /**
     * Begins mysql transaction. Actually it calls setAutoCommit(false)
     */
    void beginTransaction();
    /**
     * Commits mysql transaction. After this calls setAutoCommit(true), so you
     * need to call beginTransaction() again if you want new transaction.
     */
    void commit();
    /**
     * Rolls back mysql transaction. After this calls setAutoCommit(true), so you
     * need to call beginTransaction() again if you want new transaction.
     */
    void rollback();
    /**
     * Set id of pool which is owner of this connection
     */
    void setPoolId(unsigned int id);
    unsigned int getPoolId() const;
    virtual ~Connection();
private:
    sql::Connection *m_connection;
    ConnectionOptions* m_options;
    Scheduler m_scheduler;
    unsigned int m_poolId;
};

}//NodeMysql

#endif //NODE_MYSQL_CONNECTION_H

