#include "TaskQueryFetchRow.h"
#include "Connection.h"
#include <cppconn/exception.h>
#include <v8.h>

using namespace v8;

namespace NodeMysql {

TaskQueryFetchRow::TaskQueryFetchRow():
        m_row(NULL) {
}

TaskQueryFetchRow::~TaskQueryFetchRow() {
    delete m_row;
}

void TaskQueryFetchRow::asyncWork() {
    try {
        m_row = getConnection()->fetchRow(m_query, m_parameters);
    } catch (sql::SQLException& e) {
        m_exception = e.what();
    } catch (...) {
        m_exception = "Unknown error";
    }
}

void TaskQueryFetchRow::afterAsyncWork() {
    if (!m_callback.IsEmpty() && m_callback->IsCallable()) {
        if (m_exception.empty()) {
            Handle<Value> args[2];
            args[0] = Null();
            if (m_row) {
                Local<Object> row = Object::New();
                auto it = m_row->begin();
                for (; it != m_row->end(); ++it) {
                    row->Set(String::NewSymbol(it->first.c_str()),
                            String::New(it->second.c_str()));
                }
                args[1] = row;
            } else {
                args[1] = Null();
            }
            m_callback->Call(Context::GetCurrent()->Global(), 2, args);
        } else {
            Handle<Value> args[1];
            args[0] = Exception::Error(String::New(m_exception.c_str()));
            m_callback->Call(Context::GetCurrent()->Global(), 1, args);
        }
    }
}



}//NodeMysql
