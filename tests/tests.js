exports.testInterface = function(test) {
    test.expect(7);
    var mysql = require("../build/Release/nodemysql");
    test.ok(mysql.connect, "connect exists");
    test.strictEqual(typeof mysql.connect, "function", "connect is a function");
    test.strictEqual(typeof mysql.connect({}), "undefined", "connect returns undefined");
    test.strictEqual(typeof mysql.Pool, "function", "Pool is a function");
    test.strictEqual(typeof mysql.Pool({}), "undefined", "Pool returns undefined when called as function");
    test.ok(new mysql.Pool({}) instanceof mysql.Pool, "Pool constructor works");
    test.strictEqual(typeof (new mysql.Pool({})).getConnection, "function", "Pool.getConnection is a function");
    test.done();
};

exports.testConnectSuccess = function(test) {
    var mysql, cfg;
    test.expect(3);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    mysql.connect(cfg.config, function(err, connection) {
        test.ok(!err, "Error in connection callback: " + err);
        test.ok(true, "Connected callback called");
        test.ok(connection.connected, "Connected is true");
        test.done();
    });
};

exports.testConnectSocketSuccess = function(test) {
    var mysql, cfg;
    test.expect(3);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    mysql.connect(cfg.configSocket, function(err, connection) {
        test.ok(!err, "Error in connection callback: " + err);
        test.ok(true, "Connected callback called");
        test.ok(connection.connected, "Connected is true");
        test.done();
    });
};

exports.testConnectError = function(test) {
    var mysql, cfg;
    test.expect(1);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    mysql.connect(cfg.configError, function(err) {
        test.ok(err instanceof Error, "Error object is passed to callback");
        test.done();
    });
};

exports.testConnectErrorDict = function(test) {
    var mysql;
    test.expect(2);
    mysql = require("../build/Release/nodemysql");
    try {
        mysql.connect(null, function() {
            test.ok(false, "Connected callback called");
            test.done();
        });
    } catch (e) {
        test.ok(true, "Error was thrown");
        test.ok(e instanceof TypeError, "TypeError was thrown");
        test.done();
    }
};

exports.testConnectionInterface = function(test) {
    var mysql, cfg;
    test.expect(12);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    mysql.connect(cfg.config, function(err, connection) {
        test.ok(!err, "Error in connection callback: " + err);
        test.strictEqual(typeof connection, "object", "Connection is an object");
        test.strictEqual(typeof connection.execute, "function", "Connection.execute is a function");
        test.strictEqual(typeof connection.fetchAll, "function", "Connection.fetchAll is a function");
        test.strictEqual(typeof connection.fetchRow, "function", "Connection.fetchRow is a function");
        test.strictEqual(typeof connection.beginTransaction, "function", "Connection.beginTransaction is a function");
        test.strictEqual(typeof connection.commit, "function", "Connection.commit is a function");
        test.strictEqual(typeof connection.rollback, "function", "Connection.rollback is a function");
        test.strictEqual(typeof connection.release, "function", "Connection.release is a function");
        test.strictEqual(typeof connection.connected, "boolean", "Connected is boolean type");
        test.ok(connection.connected, "Connected is true");
        connection.connected = false;
        test.ok(connection.connected, "Connected can't be changed by JS");
        test.done();
    });
};

exports.testConnectionIncorrectInstance = function(test) {
    var mysql, cfg;
    test.expect(4);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    mysql.connect(cfg.config, function(err, connection) {
        test.ok(!err, "Error in connection callback: " + err);
        test.throws(function() {
            connection.execute.call(this, "", null);
        }, TypeError, "Check call on incorrect instance");
        test.throws(function() {
            connection.fetchAll.call(this, "", null);
        }, TypeError, "Check call on incorrect instance");
        test.throws(function() {
            connection.fetchRow.call(this, "", null);
        }, TypeError, "Check call on incorrect instance");
        test.done();
    });
};

function onConnectSuccess(test, callback) {
    var mysql, cfg;
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    mysql.connect(cfg.config, function(err, connection) {
        test.ok(!err, "Error in connection callback: " + err);
        callback(connection);
    });
}

exports.testConnectionExecute = function(test) {
    function executeCallback(err, affected, lastInsertId) {
        test.ok(!err, "Error in execute callback: " + err);
        test.ok(true, "execute success called");
        test.strictEqual(typeof affected, "number", "Affected is a number");
        test.strictEqual(typeof lastInsertId, "undefined", "Affected is a number");
        test.done();
    }

    test.expect(6);
    onConnectSuccess(test, function(connection) {
        var ret;
        ret = connection.execute("SELECT 1+1", null, executeCallback);
        test.strictEqual(typeof ret, "undefined", "Execute returns undefined");
    });
};

exports.testConnectionExecuteParam = function(test) {
    function executeCallback(err, affected, lastInsertId) {
        test.ok(!err, "Error in execute callback: " + err);
        test.ok(true, "execute success called");
        test.strictEqual(typeof affected, "number", "Affected is a number");
        test.strictEqual(typeof lastInsertId, "undefined", "Affected is a number");
        test.done();
    }

    test.expect(6);
    onConnectSuccess(test, function(connection) {
        var ret;
        ret = connection.execute("SELECT 1+?", 1, executeCallback);
        test.strictEqual(typeof ret, "undefined", "Execute returns undefined");
    });
};

exports.testConnectionExecuteParams = function(test) {
    function executeCallback(err, affected, lastInsertId) {
        test.ok(!err, "Error in execute callback: " + err);
        test.ok(true, "execute success called");
        test.strictEqual(typeof affected, "number", "Affected is a number");
        test.strictEqual(typeof lastInsertId, "undefined", "Affected is a number");
        test.done();
    }

    test.expect(6);
    onConnectSuccess(test, function(connection) {
        var ret;
        ret = connection.execute("SELECT ?+?", [1,2], executeCallback);
        test.strictEqual(typeof ret, "undefined", "Execute returns undefined");
    });
};

function prepareDatabase(test, callback) {
    onConnectSuccess(test, function(connection) {
        //1. Create table1
        connection.execute("CREATE TABLE IF NOT EXISTS test.table1 (id INT NOT NULL AUTO_INCREMENT, name VARCHAR(50) NOT NULL, PRIMARY KEY(id)) ENGINE=InnoDB CHARACTER SET utf8 COLLATE utf8_general_ci",
            null, function(err) {
                test.ok(!err, "Error in execute callback: " + err);
                //2. Truncate table1
                connection.execute("TRUNCATE table1", null, function(err) {
                    test.ok(!err, "Error in execute truncate callback: " + err);
                    connection.execute("INSERT INTO table1 VALUES (1,'test1'), (2,'test2')", null, function(err) {
                        test.ok(!err, "Error in execute insert callback: " + err);
                        callback(connection);
                    });
                });
            });
    });
}

exports.testConnectionExecuteLastInsertId = function(test) {
    function executeCallback(err, affected, lastInsertId) {
        test.ok(!err, "Error in execute callback: " + err);
        test.ok(true, "execute success called");
        test.strictEqual(typeof affected, "number", "Affected is a number");
        test.strictEqual(typeof lastInsertId, "number", "lastInsertId is a number");
        test.strictEqual(lastInsertId, 3, "lastInsertId: " + lastInsertId);
        test.done();
    }

    test.expect(10);
    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.execute("INSERT INTO table1 (name) VALUES (?)", ["test"], executeCallback, true);
        test.strictEqual(typeof ret, "undefined", "Execute returns undefined");
    });
};

exports.testConnectionFetchRow = function(test) {
    test.expect(5);
    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchRow("SELECT * FROM table1 WHERE 1=2", null);
        test.strictEqual(typeof ret, "undefined", "fetchRow returns undefined");
        test.done();
    });
};

exports.testConnectionFetchRowEmptyResult = function(test) {
    test.expect(8);
    function fetchRowCallback(err, row) {
        test.ok(!err, "Error in fetchRow callback: " + err);
        test.ok(true, "fetchRow success called");
        test.strictEqual(row, null, "Row is null - empty result");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchRow("SELECT * FROM table1 WHERE 1=2", null, fetchRowCallback);
        test.strictEqual(typeof ret, "undefined", "fetchRow returns undefined");
    });
};

exports.testConnectionFetchRowSuccess = function(test) {
    test.expect(10);
    function fetchRowCallback(err, row) {
        test.ok(!err, "Error in fetchRow callback: " + err);
        test.ok(true, "fetchRow success called");
        test.strictEqual(typeof row, "object", "Row is an object");
        test.strictEqual(row.id, "1", "Check row field id");
        test.strictEqual(row.name, "test1", "Check row field id");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchRow("SELECT * FROM table1 WHERE id=1", null, fetchRowCallback);
        test.strictEqual(typeof ret, "undefined", "fetchRow returns undefined");
    });
};

exports.testConnectionFetchRowSuccessParam = function(test) {
    test.expect(10);
    function fetchRowCallback(err, row) {
        test.ok(!err, "Error in fetchRow callback: " + err);
        test.ok(true, "fetchRow success called");
        test.strictEqual(typeof row, "object", "Row is an object");
        test.strictEqual(row.id, "1", "Check row field id");
        test.strictEqual(row.name, "test1", "Check row field id");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchRow("SELECT * FROM table1 WHERE id=?", 1, fetchRowCallback);
        test.strictEqual(typeof ret, "undefined", "fetchRow returns undefined");
    });
};

exports.testConnectionFetchRowSuccessParams = function(test) {
    test.expect(10);
    function fetchRowCallback(err, row) {
        test.ok(!err, "Error in fetchRow callback: " + err);
        test.ok(true, "fetchRow success called");
        test.strictEqual(typeof row, "object", "Row is an object");
        test.strictEqual(row.id, "1", "Check row field id");
        test.strictEqual(row.name, "test1", "Check row field id");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchRow("SELECT * FROM table1 WHERE id=? AND name=?", [1, 'test1'], fetchRowCallback);
        test.strictEqual(typeof ret, "undefined", "fetchRow returns undefined");
    });
};

exports.testConnectionFetchRowError = function(test) {
    test.expect(8);
    function fetchRowCallback(err) {
        test.ok(true, "callback called");
        test.ok(err instanceof Error, "Given argument is an Error instance");
        test.ok(err.message.match(/You have an error in your SQL syntax/));
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchRow("Error in sql statement", null, fetchRowCallback);
        test.strictEqual(typeof ret, "undefined", "fetchRow returns undefined");
    });
};

exports.testConnectionFetchAll = function(test) {
    test.expect(5);
    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchAll("SELECT * FROM table1 WHERE 1=2", null);
        test.strictEqual(typeof ret, "undefined", "fetchAll returns undefined");
        test.done();
    });
};

exports.testConnectionFetchAllEmptyResult = function(test) {
    test.expect(9);
    function fetchAllCallback(err, rows) {
        test.ok(!err, "Error in fetchAll callback: " + err);
        test.ok(true, "fetchAll success called");
        test.ok(Array.isArray(rows), "Rows is an array");
        test.strictEqual(rows.length, 0, "Array is empty - empty result");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchAll("SELECT * FROM table1 WHERE 1=2", null, fetchAllCallback);
        test.strictEqual(typeof ret, "undefined", "fetchAll returns undefined");
    });
};

exports.testConnectionFetchAllSuccess = function(test) {
    test.expect(15);
    function fetchAllCallback(err, rows) {
        test.ok(!err, "Error in fetchAll callback: " + err);
        test.ok(true, "fetchAll success called");
        test.ok(Array.isArray(rows), "Rows is an array");
        test.strictEqual(rows.length, 2, "Result: 2 rows");
        test.strictEqual(typeof rows[0], "object", "1st row is an object");
        test.strictEqual(rows[0].id, "1", "Check row field id");
        test.strictEqual(rows[0].name, "test1", "Check row field id");
        test.strictEqual(typeof rows[1], "object", "2nd row is an object");
        test.strictEqual(rows[1].id, "2", "Check row field id");
        test.strictEqual(rows[1].name, "test2", "Check row field id");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchAll("SELECT * FROM table1 ORDER BY id", null, fetchAllCallback);
        test.strictEqual(typeof ret, "undefined", "fetchAll returns undefined");
    });
};

exports.testConnectionFetchAllSuccessParam = function(test) {
    test.expect(15);
    function fetchAllCallback(err, rows) {
        test.ok(!err, "Error in fetchAll callback: " + err);
        test.ok(true, "fetchAll success called");
        test.ok(Array.isArray(rows), "Rows is an array");
        test.strictEqual(rows.length, 2, "Result: 2 rows");
        test.strictEqual(typeof rows[0], "object", "1st row is an object");
        test.strictEqual(rows[0].id, "1", "Check row field id");
        test.strictEqual(rows[0].name, "test1", "Check row field id");
        test.strictEqual(typeof rows[1], "object", "2nd row is an object");
        test.strictEqual(rows[1].id, "2", "Check row field id");
        test.strictEqual(rows[1].name, "test2", "Check row field id");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchAll("SELECT * FROM table1 WHERE id=? OR id=2", 1, fetchAllCallback);
        test.strictEqual(typeof ret, "undefined", "fetchAll returns undefined");
    });
};

exports.testConnectionFetchAllSuccessParams = function(test) {
    test.expect(15);
    function fetchAllCallback(err, rows) {
        test.ok(!err, "Error in fetchAll callback: " + err);
        test.ok(true, "fetchAll success called");
        test.ok(Array.isArray(rows), "Rows is an array");
        test.strictEqual(rows.length, 2, "Result: 2 rows");
        test.strictEqual(typeof rows[0], "object", "1st row is an object");
        test.strictEqual(rows[0].id, "1", "Check row field id");
        test.strictEqual(rows[0].name, "test1", "Check row field id");
        test.strictEqual(typeof rows[1], "object", "2nd row is an object");
        test.strictEqual(rows[1].id, "2", "Check row field id");
        test.strictEqual(rows[1].name, "test2", "Check row field id");
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchAll("SELECT * FROM table1 WHERE id IN (?,?)", [1, 2], fetchAllCallback);
        test.strictEqual(typeof ret, "undefined", "fetchAll returns undefined");
    });
};

exports.testConnectionFetchAllError = function(test) {
    test.expect(8);
    function fetchAllCallback(err) {
        test.ok(true, "callback called");
        test.ok(err instanceof Error, "Given argument is an Error instance");
        test.ok(err.message.match(/You have an error in your SQL syntax/));
        test.done();
    }

    prepareDatabase(test, function(connection) {
        var ret;
        ret = connection.fetchAll("Error in sql statement", null, fetchAllCallback);
        test.strictEqual(typeof ret, "undefined", "fetchAll returns undefined");
    });
};

exports.testTransactionCommit = function(test) {
    test.expect(9);

    prepareDatabase(test, function(connection) {
        connection.beginTransaction(function(err) {
            test.ok(!err, "Error in beginTransaction callback: " + err);
            connection.execute("DELETE FROM table1", null, function(err) {
                test.ok(!err, "Error in delete callback: " + err);
                connection.commit(function(err) {
                    test.ok(!err, "Error in commit callback: " + err);
                    connection.fetchAll("SELECT * FROM table1", null, function(err, rows) {
                        test.ok(!err, "Error in fetchAll callback: " + err);
                        test.strictEqual(rows.length, 0, "Array is empty - empty result");
                        test.done();
                    });
                });
            });
        });
    });
};

exports.testTransactionRollback = function(test) {
    test.expect(9);

    prepareDatabase(test, function(connection) {
        connection.beginTransaction(function(err) {
            test.ok(!err, "Error in beginTransaction callback: " + err);
            connection.execute("DELETE FROM table1", null, function(err) {
                test.ok(!err, "Error in delete callback: " + err);
                connection.rollback(function(err) {
                    test.ok(!err, "Error in rollback callback: " + err);
                    connection.fetchAll("SELECT * FROM table1", null, function(err, rows) {
                        test.ok(!err, "Error in fetchAll callback: " + err);
                        test.notEqual(rows.length, 0, "Array is not empty:");
                        test.done();
                    });
                });
            });
        });
    });
};

exports.testPoolErrorDict = function(test) {
    var mysql;
    test.expect(2);
    mysql = require("../build/Release/nodemysql");
    try {
        new mysql.Pool(null);
    } catch (e) {
        test.ok(true, "Error was thrown");
        test.ok(e instanceof TypeError, "TypeError was thrown");
        test.done();
    }
};

exports.testPoolGetConnection = function(test) {
    var mysql, cfg;
    test.expect(3);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    var pool = new mysql.Pool(cfg.config);
    pool.getConnection(function(err, connection) {
        test.ok(!err, "Error in getConnection callback: " + err);
        test.ok(true, "Connected callback called");
        test.ok(connection.connected, "Connected is true");
        connection.release();
        test.done();
    });
};

/**
* Pool size = 1, so only 1 connection should be available.
* This test checks if 2nd getConnection is called after after 1st connection is released.
*/
exports.testPoolSize1 = function(test) {
    var called = false,
        mysql, cfg;
    test.expect(5);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    cfg.config.pool = 1;
    var pool = new mysql.Pool(cfg.config);
    pool.getConnection(function(err, connection) {
        test.ok(!err, "Error in getConnection callback: " + err);
        test.ok(true, "Connected callback called");
        called = true;
        connection.release();
    });
    pool.getConnection(function(err, connection) {
        test.ok(!err, "Error in getConnection callback: " + err);
        test.ok(true, "Connected callback called");
        test.ok(called, "This callback was called after Connection was returned to pool");
        connection.release();
        test.done();
    });
};

/**
* Pool size = 2, so two connections should be available at the same time.
*/
exports.testPoolSize2 = function(test) {
    var connection1, connection2, mysql, cfg;
    test.expect(5);
    mysql = require("../build/Release/nodemysql");
    cfg = require("./config");
    cfg.config.pool = 2;
    var pool = new mysql.Pool(cfg.config);
    pool.getConnection(function(err, connection) {
        test.ok(!err, "Error in getConnection callback: " + err);
        test.ok(true, "Connected callback called");
        connection1 = connection;
        if (connection1 && connection2) {
            test.ok(true, "Both connections were retrieved");
            connection1.release();
            connection2.release();
            test.done();
        }
    });
    pool.getConnection(function(err, connection) {
        test.ok(!err, "Error in getConnection callback: " + err);
        test.ok(true, "Connected callback called");
        connection2 = connection;
        if (connection1 && connection2) {
            test.ok(true, "Both connections were retrieved");
            connection1.release();
            connection2.release();
            test.done();
        }
    });
};