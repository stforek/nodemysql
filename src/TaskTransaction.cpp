#include "TaskTransaction.h"
#include "Connection.h"
#include <stdexcept>
#include <cppconn/exception.h>
#include <v8.h>

using namespace v8;

namespace NodeMysql {

TaskTransaction::TaskTransaction(TransactionType type):
        m_type(type),
        m_connection(NULL) {
}

void TaskTransaction::setConnection(Connection* connection) {
    m_connection = connection;
}

Connection* TaskTransaction::getConnection() {
    if (!m_connection) {
        throw new std::runtime_error("Connection is null");
    }
    return m_connection;
}

void TaskTransaction::asyncWork() {
    try {
        switch (m_type) {
            case TRANSACTION_BEGIN:
                getConnection()->beginTransaction();
                break;
            case TRANSACTION_COMMIT:
                getConnection()->commit();
                break;
            case TRANSACTION_ROLLBACK:
                getConnection()->rollback();
                break;
            default:
                throw new std::runtime_error("Unsupported transaction type");
        }
    } catch (sql::SQLException& e) {
        m_exception = e.what();
    } catch (...) {
        m_exception = "Unknown error";
    }
}

void TaskTransaction::afterAsyncWork() {
    if (!m_callback.IsEmpty() && m_callback->IsCallable()) {
        if (m_exception.empty()) {
            Handle<Value> args[1];
            args[0] = Null();
            m_callback->Call(Context::GetCurrent()->Global(), 1, args);
        } else {
            Handle<Value> args[1];
            args[0] = Exception::Error(String::New(m_exception.c_str()));
            m_callback->Call(Context::GetCurrent()->Global(), 1, args);
        }
    }
}

TaskTransaction::~TaskTransaction() {
}

}//NodeMysql
