# nodemysql

nodemysql is a [node.js] addon written in C++ using [mysqlcppconn] library. It was made for fun just to check how addons for nodejs are made.

  - [nodeunit] for API tests
  - pool of connections
  - escaping data using prepared statements
  - queries are run in uv_queue_work()

### Building
[node-gyp] is used to build addon. You need to have installed [node.js] and [mysqlcppconn] library. I'm also using some features from C++11 standard (check your compiler).

Configure a project:
``` bash
$ node-gyp configure
```
And build:
``` bash
$ node-gyp build
```

I tested it only on Linux, but building on other systems should be easy to achieve.

### Testing

You can run unit tests using nodeunit. First edit tests/config.js to configure database access.

``` bash
$ nodeunit tests/tests.js
```

### API

#### Loading library
```javascript
var mysql = require('./build/Release/nodemysql');
```
#### Obtaining connection
It is an asynchronous method.
```javascript
mysql.connect({
    host: "localhost",
    port: 3306,
    user: "root",
    password: "",
    database: "test"
}, function(err, connection) {
    if (err) throw err;
    //do operations on connection object
});
```

Connection options:
```javascript
mysql.connect({
    host: "localhost",//host of mysql server
    port: 3306,//port of mysql server
    socketPath: "/var/run/mysqld/mysqld.sock",//if given, hostname and port options will be ignored
    user: "root",//mysql username
    password: "",//mysql password
    database: "test",//select database after connection, without this you need manually select database or make queries with database.table names
    reconnect: false,//if true, in case of lost connection, it wil reconnect. In other case, queries will fail with error that connection was lost
    charset: "utf8",//connection charset
    connectTimeout: -1,//connection timeout in miliseconds
});
```

#### Fetching single row
connection.fetchRow(query, binds, function);
```javascript
connection.fetchRow("SELECT * FROM table1 LIMIT 1", null, function(err, row) {
    if (err) throw err;
    console.log(row);
});
```
Binding one param:
```javascript
connection.fetchRow("SELECT * FROM table1 WHERE name=? LIMIT 1", "test1", function(err, row) {
    if (err) throw err;
    console.log(row);
});
```
Binding multiple params:
```javascript
connection.fetchRow("SELECT * FROM table1 WHERE name=? LIMIT ?", ["test1", 1], function(err, row) {
    if (err) throw err;
    console.log(row);
});
```
If `binds` argument is not an array, it will be converted to string as one parameter. If it is an array, it will be converter to multiple parameters. Addon won't check if correct number of parameters is given, it will be passed to mysql and there will be an error in callback:
- Uncaught Error: Value not set for all parameters
- Uncaught Error: MySQL_Prepared_Statement::setString: invalid 'parameterIndex'

#### Fetching multiple rows
connection.fetchAll(query, binds, function);
```javascript
connection.fetchAll("SELECT * FROM table1", null, function(err, rows) {
    if (err) throw err;
    console.log(rows);
});
```
If argument `err` is null, `rows` will be an array, it may be empty.

Binding params works the same way:
```javascript
connection.fetchAll("SELECT * FROM table1 WHERE name=?", ["test1"], function(err, rows) {
    if (err) throw err;
    console.log(rows);
});
```

#### Executing query (insert, update, delete etc)
connection.execute(query, binds, function, fetchLastInsertId = false);

```javascript
connection.execute("DELETE FROM table1 WHERE name=?", "test", function(err, affected) {
    if (err) throw err;
    console.log(affected);
});
```
Argument `fetchLastInsertId` allows to retrieve last insert id. It requires addon to run another query right after query given by developer. By default this is disabled (performance). When you set `fetchLastInsertId` to true, 3rd argument in callback function will be a Number of last inserted id:
```javascript
connection.execute("INSERT INTO table1 (name) VALUES (?)", "test", function(err, affected, lastId) {
    if (err) throw err;
    console.log(affected);
    console.log(lastId);
}, true);
```
#### Transactions
You can manually run SQL queries to manage transactions, but there are also methods which will make it easier.
connection.beginTransaction(function)
connection.commit(function)
connection.rollback(function)

```javascript
connection.beginTransaction(function(err) {
    if (err) throw err;
    connection.execute("DELETE FROM table1", null, function(err) {
        if (err) throw err;
        connection.commit(function(err) {
            if (err) throw err;
        });
    });
});
```
All queries made after beginTransaction can be rolled back (remember that not all methods can by rolled back, for example TRUNCATE).
```javascript
connection.beginTransaction(function(err) {
    if (err) throw err;
    connection.execute("DELETE FROM table1", null, function(err) {
        if (err) throw err;
        connection.rollback(function(err) {
            if (err) throw err;
        });
    });
});
```

#### Pool
In most simple scripts you will need only one connection object. However in server implementations it will create a huge bottleneck. Mysql servers allow more than one connection, number of available connections depends on server configuration. Pool object allows to manage multiple connections.

```javascript
var pool = new mysql.Pool({
    host: "localhost",
    port: 3306,
    user: "root",
    password: "",
    database: "test",
    pool: 10//max 10 connections
});
//obtain connection
pool.getConnection(function(err, connection) {
    if (err) throw err;
    connection.fetchAll("SELECT * FROM table1", null, function(err, rows) {
        if (err) throw err;
        console.log(rows);
        connection.release();//release connection back to pool
    });
});
```

### Development details
In this project I tested two main things: nodejs and mysqlcppconn. Both of them are not well documented, you need to google a lot in order to find details about methods implementations. Sometimes looking in code of library was a very big surprise, like this file (forked mysqlcppconn svn) - full documentation of available connection options:
https://github.com/Beirdo/mysql-connector-cpp/blob/master/driver/mysql_connection.cpp#L277

Mysqlcppconn is thread safe, so I could use uv_work_queue. However, only stress tests I found out that creating new connections is only available from 1 thread concurrently. Running queries seems to work from multiple threads concurrently.

In nodejs it was hard to find a proper solution of retrieving internal objects from Javascript instances. It was easy to crash node or corrupt memory if someone uses call() or apply() functions on methods from addon objects:

```javascript
connection.execute.call({}, "");
```

On cpp side `ConnectionWrapper::execute(const Arguments& args)` will be called, but args.This() is not a connection instance. If we just call `node::ObjectWrap::Unwrap<ConnectionWrapper>(args.This())` assert will be thrown: assert(handle->InternalFieldCount() > 0). 

In more complex example
```javascript
connection.execute.call(poolInstance, "");
```
assert won't be called (), we will cast PoolWrapper instance to ConnectionWrapper, because internal field count is equal 1. After some testing and googling I have found a solution which seems to be correct. You can check it in code in methods `getWrappedObject()`.

I tested addon with valgrind. In order to do this I needed to build Debug packages (node-gyp build --debug) and node_g (debug version of node). It seems that my addon doesn't leak, problems are inside node and mysqlcppconn.

License
----

MIT


[mysqlcppconn]:http://dev.mysql.com/downloads/connector/cpp/
[node.js]:http://nodejs.org
[nodeunit]:https://github.com/caolan/nodeunit
[node-gyp]:https://github.com/TooTallNate/node-gyp