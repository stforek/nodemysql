#ifndef NODE_MYSQL_POOL_WRAPPER_H
#define NODE_MYSQL_POOL_WRAPPER_H

#include <node.h>
#include <v8.h>
#include "Scheduler.h"
#include <map>
#include <vector>

namespace NodeMysql {

class ConnectionOptions;
class Connection;

class PoolWrapper: public node::ObjectWrap {
public:
    PoolWrapper(ConnectionOptions* options);
    virtual ~PoolWrapper();
    /**
     * Returns constructor for Pool class. Constructor is not cached,
     * so every call to this method creates new one.
     */
    static v8::Persistent<v8::Function> getConstructor();
    /**
     * Returns Connection object to Pool.
     * If Pool was already destroyed by GC or Connection didn't have owner,
     * it would return false.
     */
    static bool releaseConnection(unsigned int poolId, Connection* con);
    /**
     * Tell PoolWrapper about new connection successfully created.
     * NOT thread safe.
     */
    static void connectionCreatedCallback(unsigned int poolId);
    /**
     * Scheduler instance for creating new Connection objects.
     */
    static Scheduler async;
private:
    ConnectionOptions* m_options;
    unsigned int m_createdConnections;
    std::vector<Connection*> m_availableConnections;
    std::vector< v8::Persistent<v8::Function> > m_waitingCallbacks;
    unsigned int m_id;
    static unsigned int nextPoolId;
    static std::map<unsigned int, PoolWrapper*> pools;
    static v8::Handle<v8::Value> newInstance(const v8::Arguments& args);
    static v8::Handle<v8::Value> getConnection(const v8::Arguments& args);
    static PoolWrapper* getWrappedObject(v8::Local<v8::Object> thisObject);
    void checkWaitingCallbacks();
};

}//NodeMysql

#endif //NODE_MYSQL_POOL_WRAPPER_H

