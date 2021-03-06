#ifndef LOG_H
#define LOG_H

#include "settings.h"
#include <Syslog.h>

class Logger : public Syslog {
  private:

  public:
    using Syslog::Syslog;

    void init();
    void send(const char *, bool=false);
    void send(const String &, bool=false);
};

extern Logger sLog;

#endif // LOG_H