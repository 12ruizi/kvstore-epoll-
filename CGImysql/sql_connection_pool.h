#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H

// *!* This project is divided into two parts: Connection Pool and Sign/Register
// function
#include "../log/Log.h"
#include <condition_variable>
#include <errno.h>
#include <iostream>
#include <list>
#include <mutex>
#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
#include <string>

class Connection_Pool {
public:
  MYSQL *GetConnection();                    // Get the connection
  bool ReleaseConnection(MYSQL *connection); // Release the connection
  int GetFreeConnections();                  // Get the free connection
  void DestroyPool();                        // Destroy the connection pool

  // Single model
  static Connection_Pool *get_Instance(); // Get the instance

  // Initialize the connection pool
  void init(std::string url, std::string user, std::string password,
            std::string DataBaseName, int port, int maxConnections,
            int close_log);

private:
  Connection_Pool();
  ~Connection_Pool();

  int m_MaxConnections;               // Max number of connections
  int m_CurrentConnections;           // Current number of connections
  int m_FreeConnections;              // Number of free connections
  std::mutex _mutex;                  // Mutex for the lock
  std::list<MYSQL *> connection_list; // List of connections
  std::condition_variable
      _condition_variable; // Condition variable for the lock

public:
  std::string m_url;          // URL of the connection pool
  std::string m_user;         // Username of the connection pool
  int m_port;                 // Port of the connection pool
  std::string m_DataBaseName; // Data base name
  std::string m_password;     // Password of the connection pool
  int m_close_log;            // Close connection flag
};

class Connection_RAII {
public:
  Connection_RAII(MYSQL **SQL, Connection_Pool *connection_pool);
  ~Connection_RAII();

private:
  MYSQL *con_RAII;
  Connection_Pool *pool_RAII;
};

#endif /* SQL_CONNECTION_POOL_H */
