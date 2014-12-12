#include "TaskConnection.h"
#include "ConnectionWrapper.h"
#include "PoolWrapper.h"
#include <cppconn/exception.h>

using namespace v8;

namespace NodeMysql {

TaskConnection::TaskConnection():
        m_options(NULL),
        m_connection(NULL),
        m_poolId(0) {
}

void TaskConnection::setConnectionOptions(ConnectionOptions* con) {
    m_options = con;
}

void TaskConnection::asyncWork() {
    try {
        m_connection = new Connection(m_options);
        m_connection->setPoolId(m_poolId);
        m_options = NULL; //now Connection owns pointer
    } catch (sql::SQLException& e) {
        m_exception = e.what();
    } catch (...) {
        m_exception = "Unknown error";
    }
}

void TaskConnection::afterAsyncWork() {
    if (!m_callback.IsEmpty() && m_callback->IsCallable()) {
        if (m_exception.empty()) {
            //Tell about successfully created connection.
            //If connection doesn't use pool, this call will be ignored.
            //Called here to avoid mutex (we are in main thread)
            PoolWrapper::connectionCreatedCallback(m_poolId);
                Handle<Value> args[2];
                args[0] = Null();
                args[1] = ConnectionWrapper::Create(m_connection);
                m_callback->Call(Context::GetCurrent()->Global(), 2, args);
                m_connection = NULL;
        } else {
            Handle<Value> args[1];
            args[0] = Exception::Error(String::New(m_exception.c_str()));
            m_callback->Call(Context::GetCurrent()->Global(), 1, args);
        }
    }
}

void TaskConnection::setPoolId(unsigned int id) {
    m_poolId = id;
}

TaskConnection::~TaskConnection() {
    delete m_options;
    //if success callback is set, this should be NULL
    //in other cases, it means that there was an error or success callback
    //wasn't defined
    delete m_connection;
}

}
