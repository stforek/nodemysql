#include "TaskQueryExecute.h"
#include "Connection.h"
#include <cppconn/exception.h>
#include <v8.h>

using namespace v8;

namespace NodeMysql {

TaskQueryExecute::TaskQueryExecute():
        m_affected(0),
        m_fetchLastInsertId(false),
        m_lastInsertId(0) {
}

void TaskQueryExecute::asyncWork() {
    try {
        m_affected = getConnection()->execute(m_query, m_parameters);
        if (m_fetchLastInsertId) {
            Row* row = getConnection()->fetchRow("SELECT LAST_INSERT_ID()", NULL);
            if (row) {
                m_lastInsertId = atoll(row->at("LAST_INSERT_ID()").c_str());
            } else {
                throw std::string("Empty last insert id row");
            }
            delete row;
        }
    } catch (sql::SQLException& e) {
        m_exception = e.what();
    } catch (std::string &s) {
        m_exception = s;
    } catch (...) {
        m_exception = "Unknown error";
    }
}

void TaskQueryExecute::fetchLastInsertId(bool flag) {
    m_fetchLastInsertId = flag;
}

void TaskQueryExecute::afterAsyncWork() {
    if (!m_callback.IsEmpty() && m_callback->IsCallable()) {
        if (m_exception.empty()) {
            Handle<Value> args[m_fetchLastInsertId ? 3 : 2];
            args[0] = Null();
            args[1] = Number::New(m_affected);
            if (m_fetchLastInsertId) {
                args[2] = Number::New(m_lastInsertId);
            }
            m_callback->Call(Context::GetCurrent()->Global(),
                    m_fetchLastInsertId ? 3 : 2,
                    args);
        } else {
            Handle<Value> args[1];
            args[0] = Exception::Error(String::New(m_exception.c_str()));
            m_callback->Call(Context::GetCurrent()->Global(), 1, args);
        }
    }
}

TaskQueryExecute::~TaskQueryExecute() {
}

}//NodeMysql
