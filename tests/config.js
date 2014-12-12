exports.config = {
    host: "localhost",
    port: 3306,
    user: "root",
    password: "",
    database: "test",
};

exports.configSocket = {
    socketPath: "/var/run/mysqld/mysqld.sock",
    user: "root",
    password: "",
    database: "test",
};

exports.configError = {
    host: "localhost",
    port: 3306,
    user: "root2",
    password: "incorrect"
};
