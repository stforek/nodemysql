{
  "targets": [
    {
      "target_name": "nodemysql",
      "sources": [
          "nodemysql.cpp",
          "src/Connection.cpp",
          "src/ConnectionWrapper.cpp",
          "src/ConnectionOptions.cpp",
          "src/PoolWrapper.cpp",
          "src/Task.cpp",
          "src/TaskConnection.cpp",
          "src/TaskQuery.cpp",
          "src/TaskQueryExecute.cpp",
          "src/TaskQueryFetchAll.cpp",
          "src/TaskQueryFetchRow.cpp",
          "src/TaskTransaction.cpp",
          "src/Scheduler.cpp"
      ],
      "libraries": [
          '-lmysqlcppconn'
      ],
      "cflags!": [ '-fno-exceptions' ],
      "cflags_cc!": [ '-fno-exceptions' ],
      "cflags_cc" : ['-std=c++11']
    }
  ]
}
