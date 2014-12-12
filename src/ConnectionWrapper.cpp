#include "ConnectionWrapper.h"
#include "TaskQueryExecute.h"
#include "TaskQueryFetchAll.h"
#include "TaskQueryFetchRow.h"
#include "TaskTransaction.h"
#include "PoolWrapper.h"
#include <cppconn/exception.h>
#include <vector>

using namespace v8;

namespace NodeMysql {

namespace {
Persistent<FunctionTemplate> templateFunction;
}

Handle<Value> ConnectionWrapper::Create(Connection* connection) {
    ConnectionWrapper* obj = new ConnectionWrapper();
    obj->m_connection = connection;
    if (templateFunction.IsEmpty()) {
        //if MysqlConnection class is not initialized in V8, create it
        Local<FunctionTemplate> t = FunctionTemplate::New();
        templateFunction = Persistent<FunctionTemplate>::New(t);
        templateFunction->SetClassName(String::NewSymbol("MysqlConnection"));
        //add methods to prototype and join them with ConnectionWrapper methods
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("execute"),
                FunctionTemplate::New(execute)->GetFunction());
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("fetchAll"),
                FunctionTemplate::New(fetchAll)->GetFunction());
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("fetchRow"),
                FunctionTemplate::New(fetchRow)->GetFunction());
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("beginTransaction"),
                FunctionTemplate::New(beginTransaction)->GetFunction());
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("commit"),
                FunctionTemplate::New(commit)->GetFunction());
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("rollback"),
                FunctionTemplate::New(rollback)->GetFunction());
        templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("release"),
                FunctionTemplate::New(release)->GetFunction());
        //add readonly field connected
        templateFunction->PrototypeTemplate()->SetAccessor(
                String::NewSymbol("connected"),
                isConnected,
                0, //no setter
                Handle<Value>(),//default
                DEFAULT,//default
                ReadOnly,
                AccessorSignature::New(templateFunction)
        );
    }
    //create template which can instance new object based on function template
    //todo move it to !initialized, it should be run only once
    Local<ObjectTemplate> templateObject = templateFunction->InstanceTemplate();
    templateObject->SetInternalFieldCount(1);
    //instance new MysqlConnection object in JS based on template
    Local<Object> jsObject = templateObject->NewInstance();
    //wrap ConnectionWrapper into JS object
    obj->Wrap(jsObject);
    return jsObject;
}

ConnectionWrapper* ConnectionWrapper::getWrappedObject(Local<Object> thisObject) {
    //check if given JS object is instance of MysqlConnection class
    //check also if it has wrapped object inside
    //this should protect us from calling MysqlConnection methods on other
    //object using Function.apply or Function.call
    if (!templateFunction->HasInstance(thisObject) || thisObject->InternalFieldCount() == 0) {
        ThrowException(Exception::TypeError(String::New("Called on incorrect instance")));
        return NULL;
    }
    return node::ObjectWrap::Unwrap<ConnectionWrapper>(thisObject);
}

v8::Handle<v8::Value> ConnectionWrapper::parseAndRun(TaskQuery* task, const v8::Arguments& args) {
    HandleScope scope;
    ConnectionWrapper* wrapper = getWrappedObject(args.This());
    if (!wrapper) {
        delete task;
        return scope.Close(Undefined());
    }
    std::vector<std::string>* parameters = NULL;
    try {
        if (args.Length() < 1) {
            ThrowException(Exception::TypeError(String::New(
                    "Expected at least 1 argument")));
            return scope.Close(Undefined());
        }

        if (!args[0]->IsString()) {
            ThrowException(Exception::TypeError(String::New(
                    "Expected a string in argument 1")));
            return scope.Close(Undefined());
        }

        parameters = new std::vector<std::string>();
        if (args.Length() > 1) {
            //if it is an array, try cast to string all elements
            if (args[1]->IsArray()) {
                 Handle<Array> binds = Handle<Array>::Cast(args[1]);
                 for (uint32_t i = 0; i < binds->Length(); ++i) {
                     parameters->push_back(
                             *(v8::String::Utf8Value(binds->Get(i)->ToString())));
                 }
            } else if (!args[1]->IsUndefined() && !args[1]->IsNull()) {
                //in other case cast this argument to string
                Local<String> param = args[1]->ToString();
                if (!param.IsEmpty()) {
                    parameters->push_back(
                             *(v8::String::Utf8Value(param)));
                }
            }
        }

        Persistent<Function> callback;
        if (args.Length() > 2) {
            if (args[2]->IsFunction()) {
                callback = Persistent<Function>::New(
                        Local<Function>::Cast(args[2]));
            } else {
                ThrowException(Exception::TypeError(String::New(
                    "Expected a function in argument 3")));
                delete parameters;
                return scope.Close(Undefined());
            }
        }

        task->setConnection(wrapper->m_connection);
        task->setQuery(*(v8::String::Utf8Value(args[0]->ToString())),
                parameters);
        //if we crash right after this parameters will be released twice
        //make TaskQuery responsible for releasing memory
        parameters = NULL;
        task->setCallback(callback);
        wrapper->m_connection->addTask(task);
    } catch (...) {
        delete task;
        delete parameters;
        ThrowException(Exception::Error(String::New("Unknown exception")));
    }
    return scope.Close(Undefined());
}

Handle<Value> ConnectionWrapper::execute(const Arguments& args) {
    HandleScope scope;
    ConnectionWrapper* wrapper = getWrappedObject(args.This());
    if (!wrapper) {
        return scope.Close(Undefined());
    }
    TaskQueryExecute* task = NULL;
    std::vector<std::string>* parameters = NULL;
    try {
        if (args.Length() < 1) {
            ThrowException(Exception::TypeError(String::New(
                    "Expected at least 1 argument")));
            return scope.Close(Undefined());
        }

        if (!args[0]->IsString()) {
            ThrowException(Exception::TypeError(String::New(
                    "Expected a string in argument 1")));
            return scope.Close(Undefined());
        }

        parameters = new std::vector<std::string>();
        if (args.Length() > 1) {
            //if it is an array, try cast to string all elements
            if (args[1]->IsArray()) {
                 Handle<Array> binds = Handle<Array>::Cast(args[1]);
                 for (uint32_t i = 0; i < binds->Length(); ++i) {
                     parameters->push_back(
                             *(v8::String::Utf8Value(binds->Get(i)->ToString())));
                 }
            } else if (!args[1]->IsUndefined() && !args[1]->IsNull()) {
                //in other case cast this argument to string
                Local<String> param = args[1]->ToString();
                if (!param.IsEmpty()) {
                    parameters->push_back(
                             *(v8::String::Utf8Value(param)));
                }
            }
        }

        Persistent<Function> callback;
        if (args.Length() > 2) {
            if (args[2]->IsFunction()) {
                callback = Persistent<Function>::New(
                        Local<Function>::Cast(args[2]));
            } else {
                ThrowException(Exception::TypeError(String::New(
                    "Expected a function in argument 3")));
                delete parameters;
                return scope.Close(Undefined());
            }
        }

        task = new TaskQueryExecute();

        if (args.Length() > 3 && args[3]->BooleanValue()) {
            task->fetchLastInsertId(true);
        }

        task->setConnection(wrapper->m_connection);
        task->setQuery(*(v8::String::Utf8Value(args[0]->ToString())),
                parameters);
        //if we crash right after this parameters will be released twice
        //make TaskQuery responsible for releasing memory
        parameters = NULL;
        task->setCallback(callback);
        wrapper->m_connection->addTask(task);
    } catch (...) {
        delete task;
        delete parameters;
        ThrowException(Exception::Error(String::New("Unknown exception")));
    }
    return scope.Close(Undefined());
}

v8::Handle<v8::Value> ConnectionWrapper::fetchAll(const v8::Arguments& args) {
    return parseAndRun(new TaskQueryFetchAll(), args);
}

v8::Handle<v8::Value> ConnectionWrapper::fetchRow(const v8::Arguments& args) {
    return parseAndRun(new TaskQueryFetchRow(), args);
}

v8::Handle<v8::Value> ConnectionWrapper::beginTransaction(const v8::Arguments& args) {
    return runTransaction(TRANSACTION_BEGIN, args);
}

v8::Handle<v8::Value> ConnectionWrapper::commit(const v8::Arguments& args) {
    return runTransaction(TRANSACTION_COMMIT, args);
}

v8::Handle<v8::Value> ConnectionWrapper::rollback(const v8::Arguments& args) {
    return runTransaction(TRANSACTION_ROLLBACK, args);
}

v8::Handle<v8::Value> ConnectionWrapper::release(const v8::Arguments& args) {
    HandleScope scope;
    ConnectionWrapper* wrapper = getWrappedObject(args.This());
    if (!wrapper) {
        return scope.Close(Undefined());
    }
    if (wrapper->m_connection->getPoolId() > 0) {
        bool released = PoolWrapper::releaseConnection(wrapper->m_connection->getPoolId(),
            wrapper->m_connection);
        if (released) {
            //if pool still exists, it will become an owner of this Connection*
            //set NULL in handle, it will make this connection an invalid object
            wrapper->handle_->SetPointerInInternalField(0, NULL);
        }
    }
    return scope.Close(Undefined());
}

v8::Handle<v8::Value> ConnectionWrapper::runTransaction(TransactionType type, const v8::Arguments& args) {
    HandleScope scope;
    ConnectionWrapper* wrapper = getWrappedObject(args.This());
    if (!wrapper) {
        return scope.Close(Undefined());
    }
    TaskTransaction* task = NULL;
    try {
        Persistent<Function> callback;
        if (args.Length() > 0) {
            if (args[0]->IsFunction()) {
                callback = Persistent<Function>::New(
                        Local<Function>::Cast(args[0]));
            } else {
                ThrowException(Exception::TypeError(String::New(
                    "Expected a function in argument 1")));
                return scope.Close(Undefined());
            }
        }

        task = new TaskTransaction(type);
        task->setConnection(wrapper->m_connection);
        task->setCallback(callback);
        wrapper->m_connection->addTask(task);
    } catch (...) {
        delete task;
        ThrowException(Exception::Error(String::New("Unknown exception")));
    }
    return scope.Close(Undefined());
}

Handle<Value> ConnectionWrapper::isConnected(Local<String> property,
            const AccessorInfo& info) {
    HandleScope scope;
    //see SetAccessor call - it will make sure that "this" is a MysqlConnection
    //so we don't need to check it here
    //but Accessor doesn't check if wrapper is not null - see release method
    ConnectionWrapper* wrapper = getWrappedObject(info.This());
    if (!wrapper) {
        return scope.Close(Undefined());
    }
    return scope.Close(Boolean::New(wrapper->m_connection->isConnected()));
}

ConnectionWrapper::ConnectionWrapper() {
}

ConnectionWrapper::~ConnectionWrapper() {
    delete m_connection;
}

}//NodeMysql
