#include <cmath>

#include "ConnectionOptions.h"

namespace NodeMysql {

using namespace v8;

ConnectionOptions::ConnectionOptions(Local<Object> options) {
    //we need to copy all settings from V8 object, because
    //we will use them in other threads

    Local<String> param = String::New("socketPath");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_socket = *(v8::String::Utf8Value(value->ToString()));
        m_isSetSocket = true;
    } else {
        m_isSetSocket = false;
    }

    param = String::New("host");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_host = *(v8::String::Utf8Value(value->ToString()));
    } else {
        m_host = "localhost";
    }

    param = String::New("port");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_port = value->ToUint32()->Value();
        if (std::isnan(m_port)) {
            m_port = 0;
        }
    } else {
        m_port = 3306;
    }

    param = String::New("user");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_user = *(v8::String::Utf8Value(value->ToString()));
    } else {
        m_user = "root";
    }

    param = String::New("password");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_password = *(v8::String::Utf8Value(value->ToString()));
    } else {
        m_password = "";
    }

    param = String::New("database");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_db = *(v8::String::Utf8Value(value->ToString()));
        m_isDbSet = true;
    } else {
        m_isDbSet = false;
    }

    param = String::New("reconnect");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_autoReconnect = value->BooleanValue();
    } else {
        m_autoReconnect = false;
    }

    param = String::New("charset");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_charset = *(v8::String::Utf8Value(value->ToString()));
    } else {
        m_charset = "utf8";
    }

    param = String::New("connectTimeout");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_connectTimeout = value->ToUint32()->Value();
        if (std::isnan(m_connectTimeout)) {
            m_connectTimeout = -1;
        }
    } else {
        m_connectTimeout = -1;
    }

    param = String::New("pool");
    if (options->Has(param)) {
        Local<Value> value = options->Get(param);
        m_poolSize = value->ToUint32()->Value();
        if (std::isnan(m_poolSize) || m_poolSize < 1) {
            m_poolSize = 1;
        }
    } else {
        m_poolSize = 1;
    }
}

ConnectionOptions::ConnectionOptions(const ConnectionOptions &options):
        m_socket(options.m_socket),
        m_isSetSocket(options.m_isSetSocket),
    m_host(options.m_host),
    m_port(options.m_port),
    m_user(options.m_user),
    m_password(options.m_password),
    m_db(options.m_db),
    m_isDbSet(options.m_isDbSet),
    m_autoReconnect(options.m_autoReconnect),
    m_charset(options.m_charset),
    m_connectTimeout(options.m_connectTimeout) {
    
}

std::string ConnectionOptions::getHost() const {
    return m_host;
}

std::string ConnectionOptions::getUser() const {
    return m_user;
}

std::string ConnectionOptions::getPassword() const {
    return m_password;
}

std::string ConnectionOptions::getDb() const {
    return m_db;
}

bool ConnectionOptions::isDbSet() const {
    return m_isDbSet;
}

int ConnectionOptions::getPort() const {
    return m_port;
}

bool ConnectionOptions::isAutoReconnect() const {
    return m_autoReconnect;
}

bool ConnectionOptions::isSocketSet() const {
    return m_isSetSocket;
}

std::string ConnectionOptions::getSocket() const {
    return m_socket;
}

std::string ConnectionOptions::getCharset() const {
    return m_charset;
}

bool ConnectionOptions::isConnectTimeoutSet() const {
    return m_connectTimeout != -1;
}

int ConnectionOptions::getConnectTimeout() const {
    return m_connectTimeout;
}

unsigned int ConnectionOptions::getPoolSize() const {
    return m_poolSize;
}

ConnectionOptions::~ConnectionOptions() {
}

}
