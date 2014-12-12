#include "PoolWrapper.h"
#include "ConnectionOptions.h"
#include "TaskConnection.h"
#include "ConnectionWrapper.h"
#include <uv.h>

using namespace v8;

namespace NodeMysql {

namespace {
Persistent<FunctionTemplate> templateFunction;
}

//it turns out that sql::mysql::MySQL_Driver::connect is not thread safe
//so we can't run connection process in parallel
Scheduler PoolWrapper::async = Scheduler(1);
unsigned int PoolWrapper::nextPoolId = 1;
std::map<unsigned int, PoolWrapper*> PoolWrapper::pools = std::map<unsigned int, PoolWrapper*>();

PoolWrapper::PoolWrapper(ConnectionOptions* options):
        m_options(options),
        m_createdConnections(0) {
    m_id = nextPoolId;
    //We need to save pointers to Pool in static map with unique id.
    //If Pool is destroyed by GC, it will be removed from this map.
    //If Connection will try release to destroyed Pool, nothing will happen
    pools.insert(std::pair<unsigned int, PoolWrapper*>(m_id, this));
    nextPoolId++;
}

PoolWrapper::~PoolWrapper() {
    pools.erase(m_id);
    //release all Connection pointers
    for (auto connection: m_availableConnections) {
        delete connection;
    }
    delete m_options;
}

bool PoolWrapper::releaseConnection(unsigned int poolId, Connection* con) {
    auto it = pools.find(poolId);
    if (it != pools.end()) {
        it->second->m_availableConnections.push_back(con);
        it->second->checkWaitingCallbacks();
        return true;
    }
    return false;
}

void PoolWrapper::connectionCreatedCallback(unsigned int poolId) {
    auto it = pools.find(poolId);
    if (it != pools.end()) {
        it->second->m_createdConnections++;
    }
}

Persistent<Function> PoolWrapper::getConstructor() {
    //create new template function an link it with c++ method "PoolWrapper::newInstance"
    Local<FunctionTemplate> t = FunctionTemplate::New(newInstance);
    templateFunction = Persistent<FunctionTemplate>::New(t);
    //set class name
    templateFunction->SetClassName(String::NewSymbol("Pool"));
    templateFunction->PrototypeTemplate()->Set(
                String::NewSymbol("getConnection"),
                FunctionTemplate::New(getConnection)->GetFunction());
    //mark that function will have object in internal storage
    templateFunction->InstanceTemplate()->SetInternalFieldCount(1);
    //create function based on template
    return Persistent<Function>::New(templateFunction->GetFunction());

}

struct CallbackData {
    Connection* connection;
    Persistent<Function> success;
};

void getConnectionCallback(uv_idle_t* handle, int status) {
    CallbackData* data = static_cast<CallbackData*>(handle->data);
    if (data) {
        if (!data->success.IsEmpty() && data->success->IsCallable()) {
            Handle<Value> args[2];
            args[0] = Null();
            args[1] = ConnectionWrapper::Create(data->connection);
            data->success->Call(Context::GetCurrent()->Global(), 2, args);
        }
    }
    uv_idle_stop(handle);
    //https://groups.google.com/forum/#!msg/libuv/9QCC3K_kUIQ/StVEFFth0B4J
    //Free the handle in your close callback, never before
    uv_close((uv_handle_t*)handle, (uv_close_cb)free);
    delete data;
}

void addIdle(Connection* connection, Persistent<Function> success) {
    CallbackData* data = new CallbackData();
    data->connection = connection;
    data->success = success;
    //malloc used to simplify free() operation, see getConnectionCallback
    uv_idle_t* handle = (uv_idle_t*)malloc(sizeof(uv_idle_t));
    uv_idle_init(uv_default_loop(), handle);
    handle->data = data;
    uv_idle_start(handle, getConnectionCallback);
}

Handle<Value> PoolWrapper::getConnection(const Arguments& args) {
    HandleScope scope;
    PoolWrapper* wrapper = getWrappedObject(args.This());
    if (!wrapper) {
        return scope.Close(Undefined());
    }
    //actually without success callback, there is no way to get connection
    //object, so maybe it should not be optional argument?
    Persistent<Function> callback;
    if (args.Length() > 0) {
        if (args[0]->IsFunction()) {
            callback = Persistent<Function>::New(
                    Local<Function>::Cast(args[0]));
        } else {
            ThrowException(Exception::TypeError(String::New(
                "Expected a function in argument 1")));
        }
    }


    if (wrapper->m_availableConnections.empty()) {
        if (wrapper->m_createdConnections < wrapper->m_options->getPoolSize()) {
            //if there is no available Connection and created connections number
            //is lower than pool number, create new connection:
            TaskConnection* task = new TaskConnection();
            ConnectionOptions* optionsCopy = new ConnectionOptions(*(wrapper->m_options));
            task->setConnectionOptions(optionsCopy);
            //This pool is owner of this connection, so when MysqlConnection.release
            //is called, connection will be returned to this Pool object
            task->setPoolId(wrapper->m_id);
            task->setCallback(callback);
            //add connection task to scheduler, it will be run in other thread
            PoolWrapper::async.addTask(task);
        } else {
            //no available connections and max connection size reached
            //wait for connection
            wrapper->m_waitingCallbacks.push_back(callback);
        }
    } else {
        addIdle(wrapper->m_availableConnections.back(), callback);
        wrapper->m_availableConnections.pop_back();
    }
    return scope.Close(Undefined());
}

void PoolWrapper::checkWaitingCallbacks() {
    if (!m_availableConnections.empty() && !m_waitingCallbacks.empty()) {
        //@todo: m_waitingCallbacks.back() - maybe it should keep order of requests?
        addIdle(m_availableConnections.back(), m_waitingCallbacks.back());
        m_availableConnections.pop_back();
        m_waitingCallbacks.pop_back();
    }
}

Handle<Value> PoolWrapper::newInstance(const Arguments& args) {
    HandleScope scope;
    //check if function was called with "new"
    if (args.IsConstructCall()) {
        if (args.Length() < 1) {
            ThrowException(Exception::TypeError(String::New(
                    "Expected at least 1 argument")));
            return scope.Close(Undefined());
        }

        //first argument should be a dictionary with connection settings
        Local<Object> dict = args[0]->ToObject();
        if (dict.IsEmpty()) {
            ThrowException(Exception::TypeError(String::New(
                    "Expected an object in argument 1")));
            return scope.Close(Undefined());
        }

        //parse dictionary params inside ConnectionOptions object
        ConnectionOptions* options = new ConnectionOptions(dict);
        //create internal object
        PoolWrapper* priv = new PoolWrapper(options);
        //connect JS object with C++ object
        priv->Wrap(args.This());
        //return new instance
        return args.This();
    }
    //if this wasn't called as constructor, return undefined
    return scope.Close(Undefined());
}

PoolWrapper* PoolWrapper::getWrappedObject(Local<Object> thisObject) {
    //check if given JS object is instance of Pool class
    //check also if it has wrapped object inside
    //this should protect us from calling Pool methods on other
    //object using Function.apply or Function.call
    if (!templateFunction->HasInstance(thisObject) || thisObject->InternalFieldCount() == 0) {
        ThrowException(Exception::TypeError(String::New("Called on incorrect instance")));
        return NULL;
    }
    return node::ObjectWrap::Unwrap<PoolWrapper>(thisObject);
}

}