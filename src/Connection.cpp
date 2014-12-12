#include "Connection.h"
#include "PoolWrapper.h"
#include <mysql_driver.h>
#include <cppconn/prepared_statement.h>
#include <memory>

namespace NodeMysql {

//a deleter class for driverPtr, it will make sure that threadEnd will
//be always called
struct DriverDeleter {
    void operator()(sql::mysql::MySQL_Driver* driver) {
        driver->threadEnd();
    }
};

//this should be only used when threadInit() is called
typedef std::unique_ptr<sql::mysql::MySQL_Driver, DriverDeleter> driverPtr;

Connection::Connection(ConnectionOptions* options):
        m_connection(NULL),
        m_options(NULL),
//Use one thread per connection: allow scheduler to run only 1 task concurrently
//This means that all queries will be called only from one thread one right
//after another
        m_scheduler(1),
        m_poolId(0) {
    m_options = options;
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    //this method is not called in main thread, so according to mysqlcppconnector
    //docs, it should call threadInit and threadEnd
    driver->threadInit();
    sql::ConnectOptionsMap sqlOptions;
    //http://dev.mysql.com/doc/refman/5.0/en/mysql-options.html
    //@todo other options
    //https://github.com/Beirdo/mysql-connector-cpp/blob/master/driver/mysql_connection.cpp#L277
    /*
    We support :
    - hostName
    - userName
    - password
    - port
    - socket
    - pipe
    - characterSetResults
    - schema
    - sslKey
    - sslCert
    - sslCA
    - sslCAPath
    - sslCipher
    - defaultStatementResultType
    - defaultPreparedStatementResultType
    - CLIENT_COMPRESS
    - CLIENT_FOUND_ROWS
    - CLIENT_IGNORE_SIGPIPE
    - CLIENT_IGNORE_SPACE
    - CLIENT_INTERACTIVE
    - CLIENT_LOCAL_FILES
    - CLIENT_MULTI_RESULTS
    - CLIENT_MULTI_STATEMENTS
    - CLIENT_NO_SCHEMA
    - CLIENT_COMPRESS
    - OPT_CONNECT_TIMEOUT
    - OPT_NAMED_PIPE
    - OPT_READ_TIMEOUT
    - OPT_WRITE_TIMEOUT
    - OPT_RECONNECT
    - OPT_CHARSET_NAME
    - OPT_REPORT_DATA_TRUNCATION
  */
    if (m_options->isSocketSet()) {
        sqlOptions["socket"] = sql::SQLString(m_options->getSocket());
    } else {
        //when socket is set, host and port are ignored
        sqlOptions["hostName"] = sql::SQLString(m_options->getHost());
        sqlOptions["port"] = m_options->getPort();
    }
    sqlOptions["userName"] = sql::SQLString(m_options->getUser());
    sqlOptions["password"] = sql::SQLString(m_options->getPassword());
    if (m_options->isDbSet()) {
        sqlOptions["schema"] = sql::SQLString(m_options->getDb());
    }
    if (m_options->isConnectTimeoutSet()) {
        //connection timeout is in seconds, from JS we retrieve in ms
        sqlOptions["OPT_CONNECT_TIMEOUT"] = m_options->getConnectTimeout() / 1000;
    }
    sqlOptions["OPT_RECONNECT"] = m_options->isAutoReconnect();
    sqlOptions["OPT_CHARSET_NAME"] = sql::SQLString(m_options->getCharset());
    m_connection = driver->connect(sqlOptions);
}

void Connection::addTask(Task* task) {
    m_scheduler.addTask(task);
}

/**
 * Binds vector of string parameters to sql statement.
 * Number of parameters should be equal to "?" in query.
 * @param stmt not NULL
 * @param parameters if NULL or empty, it will do nothing
 */
void bindParams(sql::PreparedStatement *stmt, std::vector<std::string>* parameters) {
    if (NULL == parameters || parameters->empty()) {
        return;
    }
    auto it = parameters->begin();
    unsigned int i = 1;
    for (; it != parameters->end(); ++it, ++i) {
        stmt->setString(i, sql::SQLString(*it));
    }
}

int Connection::execute(const std::string& query, std::vector<std::string>* parameters) {
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    //this method is not called in main thread, so according to mysqlcppconnector
    //docs, it should call threadInit and threadEnd
    driver->threadInit();
    std::unique_ptr<sql::PreparedStatement> stmt(
            m_connection->prepareStatement(sql::SQLString(query)));
    bindParams(stmt.get(), parameters);
    return stmt->executeUpdate();
}

/**
 * Saves mysql result to Row object (a map with string key and value)
 * @param result
 * @param meta is needed to retrieve column names
 * @return new Row - you have to delete it
 */
Row* createRow(sql::ResultSet* result, sql::ResultSetMetaData* meta) {
    Row* row = new Row();
    try {
        for (unsigned int i = 1; i <= meta->getColumnCount(); ++i) {
            row->insert(std::pair<std::string, std::string>(
                    meta->getColumnName(i).asStdString(),
                    result->getString(i).asStdString()));
        }
    } catch(...) {
        delete row;
        throw;
    }
    return row;
}

RowsList* Connection::fetchAll(const std::string& query, std::vector<std::string>* parameters) {
    auto deleter = [](RowsList* rows) {
        for (Row* row: *rows) {
            delete row;
        }
        delete rows;
    };
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    //this method is not called in main thread, so according to mysqlcppconnector
    //docs, it should call threadInit and threadEnd
    driver->threadInit();
    std::unique_ptr<sql::PreparedStatement> stmt(
            m_connection->prepareStatement(sql::SQLString(query)));
    bindParams(stmt.get(), parameters);
    std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
    //fetch meta to get column names
    sql::ResultSetMetaData* meta = result->getMetaData();
    std::unique_ptr<RowsList, decltype(deleter)> rows(new RowsList(), deleter);
    while (result->next()) {
        rows->push_back(createRow(result.get(), meta));
    }
    return rows.release();
}

Row* Connection::fetchRow(const std::string& query, std::vector<std::string>* parameters) {
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    //this method is not called in main thread, so according to mysqlcppconnector
    //docs, it should call threadInit and threadEnd
    driver->threadInit();
    std::unique_ptr<sql::PreparedStatement> stmt(
            m_connection->prepareStatement(sql::SQLString(query)));
    bindParams(stmt.get(), parameters);
    std::unique_ptr<sql::ResultSet> result(stmt->executeQuery());
    sql::ResultSetMetaData* meta = result->getMetaData();
    if (result->next()) {
        return createRow(result.get(), meta);
    }
    return NULL;
}

bool Connection::isConnected() {
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    driver->threadInit();
    return !m_connection->isClosed();
}

void Connection::beginTransaction() {
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    driver->threadInit();
    m_connection->setAutoCommit(false);
}

void Connection::commit() {
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    driver->threadInit();
    m_connection->commit();
    m_connection->setAutoCommit(true);
}

void Connection::rollback() {
    driverPtr driver(sql::mysql::get_mysql_driver_instance());
    driver->threadInit();
    m_connection->rollback();
    m_connection->setAutoCommit(true);
}

void Connection::setPoolId(unsigned int id) {
    m_poolId = id;
}

unsigned int Connection::getPoolId() const {
    return m_poolId;
}

Connection::~Connection() {
    delete m_connection;
    delete m_options;
}

}//NodeMysql
