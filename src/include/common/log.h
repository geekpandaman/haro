#pragma once

#include<string>
#include<stdint.h>
#include<memory>
#include<list>
#include<sstream>
#include<fstream>
#include<algorithm>
#include<iostream>
#include<vector>

namespace haro {

class Logger;
class LoggerManager;

class LogLevel{
public:
  enum Level{
    DEBUG = 1,
    INFO = 2,
    WARN =3,
    ERROR = 4,
    FATAL = 5
  };
  static const char* ToString(LogLevel::Level level);
};


//日志事件
class LogEvent{
public:
  typedef std::shared_ptr<LogEvent> ptr;
  LogEvent();
  const char* getFile() const {return m_file_;};
  uint32_t getLine() const {return m_line_;};
  uint32_t getThreadID() const {return m_threadID_;};
  uint32_t getFiberID() const {return m_fiberID_;};
  uint64_t getTime() const {return m_time_;};
  const std::string& getContent() const {return m_content_;};
  LogLevel::Level getLevel() const {return m_level_;};
  uint32_t getElapse() const {return m_elapse;};
  std::shared_ptr<Logger> getLogger() const {return m_logger;};
private:
  //文件名
  const char* m_file_ = nullptr;
  //行号
  uint32_t m_line_ = 0;
  //线程号
  uint32_t m_threadID_ = 0;
  //协程号
  uint32_t m_fiberID_ = 0;
  //时间戳
  uint64_t m_time_ = 0;
  //日志内容
  std::string m_content_;
  //日志等级
  LogLevel::Level m_level_;
  // 程序启动开始到现在的毫秒数
  uint32_t m_elapse = 0;
  // 线程名称
  std::string m_threadName;
  // 日志内容流
  std::stringstream m_ss;
  // 日志器
  std::shared_ptr<Logger> m_logger;
};

/*
LogFormatter负责对事件的各个元素进行格式化输出，
因此为每个元素定义一个FormatItem
*/
class LogFormatter{
public:
  typedef std::shared_ptr<LogFormatter> ptr;
  LogFormatter(const std::string& pattern) : m_pattern_(pattern){};
  std::string format(LogEvent::ptr event);
  //解析m_pattern，对m_items进行初始化
  void init();
public:
  class FormatItem{
    public:
      typedef std::shared_ptr<FormatItem> ptr;
      virtual ~FormatItem();
      virtual void format(std::ostream& os,LogEvent::ptr event) = 0;
  };
private:
  std::string m_pattern_;
  std::vector<FormatItem::ptr> m_items_;
  bool m_error_ = false;
};

//日志输出目的地
class LogAppender{
public:
  typedef std::shared_ptr<LogAppender> ptr;
  virtual ~LogAppender();
  virtual void log(LogLevel::Level level,LogEvent::ptr event) = 0;
  void setFormatter(LogFormatter::ptr f){m_formatter_ = f;};
  LogFormatter::ptr getFormatter(){return m_formatter_;};
protected:
  LogLevel::Level m_level_;
  LogFormatter::ptr m_formatter_;
};

//标准输出
class StdoutAppender : public LogAppender{
public:
  typedef std::shared_ptr<StdoutAppender> ptr;
  void log(LogLevel::Level level,LogEvent::ptr event) override;
private:
};
//文件输出
class FileAppender : public LogAppender{
public:
  typedef std::shared_ptr<FileAppender> ptr;
  FileAppender(const std::string& file_name);
  void log(LogLevel::Level level,LogEvent::ptr event) override;
  bool reopen();
private:
  std::string m_filename_;
  std::ofstream m_filestream_;
};

//日志器
//只有满足日志级别的日志会被输出
class Logger{
public:
  typedef std::shared_ptr<Logger> ptr;
  Logger():m_name_("root"),m_level_(LogLevel::Level::ERROR){};
  void log(LogLevel::Level level,LogEvent::ptr event);

  inline void debug(LogEvent::ptr event){
    log(LogLevel::Level::DEBUG,event);
  };
  inline void info(LogEvent::ptr event){
    log(LogLevel::Level::INFO,event);
  };
  inline void warn(LogEvent::ptr event){
    log(LogLevel::Level::WARN,event);
  };
  inline void error(LogEvent::ptr event){
    log(LogLevel::Level::ERROR,event);
  };

  void addAppender(LogAppender::ptr);
  void removeAppender(LogAppender::ptr);
  LogLevel::Level getLevel(){return m_level_;};
  void setLevel(LogLevel::Level level){m_level_ = level;}
  std::string getName() const {return m_name_;};
private:
  //日志器名称
  std::string m_name_;
  LogLevel::Level m_level_;
  //目的地集合
  std::list<LogAppender::ptr> m_appenders_;
};



}
