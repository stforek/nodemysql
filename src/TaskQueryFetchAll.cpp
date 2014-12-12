#include "TaskQueryFetchAll.h"
#include "Connection.h"
#include <cppconn/exception.h>
#include <v8.h>

using namespace v8;

namespace NodeMysql {

TaskQueryFetchAll::TaskQueryFetchAll():
        m_rows(NULL) {
}

TaskQueryFetchAll::~TaskQueryFetchAll() {
    if (m_rows) {
        auto it = m_rows->begin();
        it = m_rows->begin();
        for (; it != m_rows->end(); ++it) {
            delete *it;
        }
        delete m_rows;
    }
}

void TaskQueryFetchAll::asyncWork() {
    try {
        m_rows = getConnection()->fetchAll(m_query, m_parameters);
    } catch (sql::SQLException& e) {
        m_exception = e.what();
    } catch (...) {
        m_exception = "Unknown error";
    }
}

void TaskQueryFetchAll::afterAsyncWork() {
    if (!m_callback.IsEmpty() && m_callback->IsCallable()) {
        if (m_exception.empty()) {
            Handle<Value> args[2];
            Local<Array> arr = Array::New(m_rows->size());
            auto it = m_rows->begin();
            uint32_t i = 0;
            for(; it != m_rows->end(); ++it, ++i) {
                Local<Object> row = Object::New();
                auto itr = (*it)->begin();
                for (; itr != (*it)->end(); ++itr) {
                    row->Set(String::NewSymbol(itr->first.c_str()),
                            String::New(itr->second.c_str()));
                }
                arr->Set(i, row);
            }
            args[0] = Null();
            args[1] = arr;
            m_callback->Call(Context::GetCurrent()->Global(), 2, args);
        } else {
            Handle<Value> args[1];
            args[0] = Exception::Error(String::New(m_exception.c_str()));
            m_callback->Call(Context::GetCurrent()->Global(), 1, args);
        }
    }
}



}//NodeMysql
