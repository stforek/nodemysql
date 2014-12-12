#include <node.h>
#include <v8.h>
#include <mysql_driver.h>
#include "src/ConnectionOptions.h"
#include "src/TaskConnection.h"
#include "src/PoolWrapper.h"

using namespace v8;
using namespace NodeMysql;

Handle<Value> ConnectMethod(const Arguments& args) {
    HandleScope scope;
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
    ConnectionOptions* con = new ConnectionOptions(dict);

    //actually without success callback, there is no way to get connection
    //object, so maybe it should not be optional argument?
    Persistent<Function> callback;
    if (args.Length() > 1) {
        if (args[1]->IsFunction()) {
            callback = Persistent<Function>::New(
                    Local<Function>::Cast(args[1]));
        } else {
            ThrowException(Exception::TypeError(String::New(
                "Expected a function in argument 2")));
        }
    }

    TaskConnection* task = new TaskConnection();
    task->setConnectionOptions(con);
    task->setCallback(callback);
    //add connection task to scheduler, it will be run in other thread
    PoolWrapper::async.addTask(task);
    return scope.Close(Undefined());
}

void init(Handle<Object> exports) {
    //expose connect method
    exports->Set(String::NewSymbol("connect"),
            FunctionTemplate::New(ConnectMethod)->GetFunction());
    //expose Pool class
    exports->Set(String::NewSymbol("Pool"), PoolWrapper::getConstructor());
}

NODE_MODULE(nodemysql, init)