#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>

// Redis request parser
struct Request {
  std::string command;
  std::vector<std::string> args;
};

struct DB_Entry {
  std::string value;
  uint64_t date;
  uint64_t expiry;
};

typedef std::map<std::string, DB_Entry> database;

enum class Role {
    Master,
    Slave
};

struct DB_Config {
  std::string dir;
  std::string db_filename;
  std::string file;
  int port;
  database db;
  database in_memory_db;
  Role role;
  bool isMaster;
  std::string masterHost;
  uint64_t masterPort;
};
