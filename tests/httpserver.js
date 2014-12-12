var http = require('http'),
    mysql = require("../build/Release/nodemysql"),
    cfg = require("./config"),
    pool;

cfg.config.pool = 10;
pool = new mysql.Pool(cfg.config);

http.createServer(function(req, res) {
    console.log(req.url);
    res.writeHead(200, {'Content-Type': 'text/plain'});
    pool.getConnection(function(err, connection) {
        if (err) {
            console.log('Failed to getConnection: ' + err);
            res.end('Failed to getConnection: ' + err);
            return;
        }
        connection.fetchAll("SELECT * FROM table1 ORDER BY id", null, function(err, rows) {
            if (err) {
                console.log('Failed to fetch rows: ' + err);
                res.end('Failed to fetch rows: ' + err);
                return;
            }
            console.log('Fetched rows: ' + rows.length);
            res.end('Fetched rows: ' + rows.length);
            connection.release();
        });
    });
}).listen(1337, '127.0.0.1');

console.log('Server running at http://127.0.0.1:1337/');