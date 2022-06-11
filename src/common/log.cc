#include"common/log.h"
#include<unordered_map>
#include <functional>

namespace haro{

/*Level类*/
const char* LogLevel::ToString(LogLevel::Level level){
  switch (level)
  {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
  default:
    return "UNKNOWN";
    break;
  }
}

/*Logger类*/
void Logger::log(LogLevel::Level level,LogEvent::ptr event){
  if(level>=m_level_){
    for(auto i : m_appenders_){
      i->log(level,event);
    }
  }
}
void Logger::addAppender(LogAppender::ptr appender){
  //需要考虑线程安全
  if(std::find(m_appenders_.begin(),m_appenders_.end(),appender)!=m_appenders_.end())
    m_appenders_.push_back(appender);
}
void Logger::removeAppender(LogAppender::ptr appender){
  m_appenders_.remove(appender);
}

/*Appender类*/
void StdoutAppender::log(LogLevel::Level level,LogEvent::ptr event){
  if(level>m_level_)
    std::cout<<m_formatter_->format(event);    
}
FileAppender::FileAppender(const std::string& file_name) : m_filename_(file_name){
  //不要在构造函数中open
  // m_filestream_.open(file_name);
}
bool FileAppender::reopen(){
  if(m_filestream_)
    m_filestream_.close();
  m_filestream_.open(m_filename_);
  return !!m_filestream_;
}
void FileAppender::log(LogLevel::Level level,LogEvent::ptr event){  
  if(level>m_level_)
    m_filestream_<<m_formatter_->format(event);
}

/*FormatterItem子类*/

class MessageFormatItem : public LogFormatter::FormatItem{
public:
  MessageFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << event->getContent();
  };
};
class LevelFormatItem : public LogFormatter::FormatItem{
public:
  LevelFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << LogLevel::ToString(event->getLevel());
  };
};
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
  ElapseFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << event->getElapse();
  };
};
class NameFormatItem : public LogFormatter::FormatItem{
//logger名称
public:
  NameFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << event->getLogger()->getName();
  };
};
class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
  ThreadIdFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << event->getThreadID();
  };
};
class FiberIdFormatItem : public LogFormatter::FormatItem{
public:
  FiberIdFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << event->getFiberID();
  };
};
class NewLineFormatItem : public LogFormatter::FormatItem{
public:
  NewLineFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os << std::endl;
  };
};
class DateTimeFormatItem : public LogFormatter::FormatItem{
public:
  DateTimeFormatItem(const std::string& str = "%Y:%m:%d %H:%M%S");
  void format(std::ostream& os,LogEvent::ptr event){
    os << event->getTime();
  };
};
class FilenameFormatItem : public LogFormatter::FormatItem{
public:
  FilenameFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os<<event->getFile();
  };
};
class LineFormatItem : public LogFormatter::FormatItem{
public:
  LineFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    os<<event->getLine();
  };
};
class TabFormatItem : public LogFormatter::FormatItem{
public:
  TabFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    
  };
};
class ThreadNameFormatItem : public LogFormatter::FormatItem{
public:
  ThreadNameFormatItem(const std::string& str = "");
  void format(std::ostream& os,LogEvent::ptr event){
    
  };
};

class StringFormatItem : public LogFormatter::FormatItem{
public:
  StringFormatItem(const std::string& str);
  void format(std::ostream& os,LogEvent::ptr event){
    os<<str;
  };
private:
  const std::string& str;
};

/*Formatter类*/
std::string LogFormatter::format(LogEvent::ptr event){
  std::stringstream ss;
  for(auto i :m_items_){
    i->format(ss,event);
  }
  return ss.str();
}
//TODO
// std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
//     for(auto& i : m_items_) {
//         i->format(ofs, logger, level, event);
//     }
//     return ofs;
// }

//%xxx %xxx{xxx} %%
void LogFormatter::init() {
    //str, format, type
    std::vector<std::tuple<std::string, std::string, int> > vec;
    //外层的非格式字符串
    std::string nstr;
    for(size_t i = 0; i < m_pattern_.size(); ++i) {
        if(m_pattern_[i] != '%') {
            nstr.append(1, m_pattern_[i]);
            continue;
        }
        //判断是否是%%转义%的情况
        if((i + 1) < m_pattern_.size()) {
            if(m_pattern_[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        //{}括号内format字符串
        std::string fmt;
        while(n < m_pattern_.size()) {
            if(!fmt_status && (!isalpha(m_pattern_[n]) && m_pattern_[n] != '{'
                    && m_pattern_[n] != '}')) {
                str = m_pattern_.substr(i + 1, n - i - 1);
                break;
            }
            if(fmt_status == 0) {
                if(m_pattern_[n] == '{') {
                    str = m_pattern_.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1; //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {
                if(m_pattern_[n] == '}') {
                    fmt = m_pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern_.size()) {
                if(str.empty()) {
                    str = m_pattern_.substr(i + 1);
                }
            }
        }

        if(fmt_status == 0) {
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if(fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern_ << " - " << m_pattern_.substr(i) << std::endl;
            m_error_ = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
    static std::unordered_map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    for(auto& i : vec) {
        if(std::get<2>(i) == 0) {
            m_items_.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()) {
                m_items_.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error_ = true;
            } else {
                m_items_.push_back(it->second(std::get<1>(i)));
            }
        }

        //std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
    //std::cout << m_items.size() << std::endl;
}




}


