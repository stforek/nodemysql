#ifndef NODE_MYSQL_CONNECTION_OPTIONS_H
#define NODE_MYSQL_CONNECTION_OPTIONS_H

#include <v8.h>
#include <string>

namespace NodeMysql {

/**
 * Simple class to store connection options passed from v8.
 * It was created mainly because V8 methods can't be used in other threads
 * than main.
 */
class ConnectionOptions {
public:
    ConnectionOptions(v8::Local<v8::Object> options);
    ConnectionOptions(const ConnectionOptions &options);
    std::string getHost() const;
    std::string getUser() const;
    std::string getPassword() const;
    std::string getDb() const;
    bool isDbSet() const;
    int getPort() const;
    bool isAutoReconnect() const;
    bool isSocketSet() const;
    std::string getSocket() const;
    std::string getCharset() const;
    bool isConnectTimeoutSet() const;
    int getConnectTimeout() const;
    unsigned int getPoolSize() const;
    virtual ~ConnectionOptions();
protected:
    std::string m_socket;
    bool m_isSetSocket;
    std::string m_host;
    int m_port;
    std::string m_user;
    std::string m_password;
    std::string m_db;
    bool m_isDbSet;
    bool m_autoReconnect;
    std::string m_charset;
    int m_connectTimeout;
    unsigned int m_poolSize;
};

}

#endif //NODE_MYSQL_CONNECTION_OPTIONS_H

