#ifndef NODE_MYSQL_CONNECTION_WRAPPER_H
#define NODE_MYSQL_CONNECTION_WRAPPER_H

#include <node.h>
#include "Connection.h"
#include "TaskQuery.h"
#include "TaskTransaction.h"

namespace NodeMysql {

/**
 * A class that creates connection object in JS and wraps internally
 * Connection class.
 */
class ConnectionWrapper: public node::ObjectWrap {
public:
    /**
     * Create a JS instance and wrap Connection internally.
     * @param connection can't be NULL
     * @return v8 object, a Connection instance
     */
    static v8::Handle<v8::Value> Create(Connection* connection);
private:
    ConnectionWrapper();
    virtual ~ConnectionWrapper();
    /**
     * Common method which inserts to Task object from arguments: a query, parameters, success
     * callback and error callback. Next it adds task to scheduler.
     * @param task TaskQuery base class.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> parseAndRun(TaskQuery* task,
            const v8::Arguments& args);
    /**
     * Common method to run one of transaction methods: begin, commit and rollback.
     * It inserts to TaskTransaction object success and error callback and then
     * adds task to scheduler.
     * @param type a type of transaction method
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> runTransaction(TransactionType type, const v8::Arguments& args);
    /**
     * Tries to unwrapp ConnectionWrapper object from v8::Object handle.
     * If it fails it returns NULL and schedule exception in v8.
     * @param thisObject
     * @return NULL if fail
     */
    static ConnectionWrapper* getWrappedObject(v8::Local<v8::Object> thisObject);
    /**
     * Exposed method in JS instance - executes query.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> execute(const v8::Arguments& args);
    /**
     * Exposed method in JS instance - fetches all rows for given query.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> fetchAll(const v8::Arguments& args);
    /**
     * Exposed method in JS instance - fetches one row for given query.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> fetchRow(const v8::Arguments& args);
    /**
     * Exposed method in JS instance - checks connection status.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> isConnected(v8::Local<v8::String> property,
            const v8::AccessorInfo& info);
    /**
     * Exposed method in JS instance - begins transaction.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> beginTransaction(const v8::Arguments& args);
    /**
     * Exposed method in JS instance - roll backs transaction.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> rollback(const v8::Arguments& args);
    /**
     * Exposed method in JS instance - commits transaction.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> commit(const v8::Arguments& args);
    /**
     * Exposed method in JS instance - releases connection to Pool.
     * If there is no Pool connected to this connection, it does nothing.
     * @param args
     * @return v8::Undefined()
     */
    static v8::Handle<v8::Value> release(const v8::Arguments& args);
    Connection* m_connection;
};

}//NodeMysql

#endif //NODE_MYSQL_CONNECTION_WRAPPER_H

