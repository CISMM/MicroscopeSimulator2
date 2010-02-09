#ifndef _DIRTY_LISTENER_H_
#define _DIRTY_LISTENER_H_

#include <string>


#define SetGetMacro(name, type) \
  virtual void Set##name(type arg) { \
    if (m_##name != arg) { \
      m_##name = arg; \
    } \
    Sully(); \
  } \
  \
  virtual type Get##name() { \
    return m_##name; \
  }


class DirtyListener {

public:
  virtual ~DirtyListener() {};

  virtual void Sully() = 0;

  virtual void SetStatusMessage(const std::string& status) = 0;

}; // class DirtyListener

#endif // _DIRTY_LISTENER_H_
