//------------------------------------------------------------------------------
/// \file      debug.h
/// \author    Serge Aleynikov
/// \copyright (c) 2018, Serge Aleynikov
//------------------------------------------------------------------------------
/// \brief     Primitives for tracing debug messages in the DebugView++ app.
//------------------------------------------------------------------------------
#pragma once

#ifndef __NO_CPP
# include <assert.h>
# include <stdio.h>
#else
# include <cassert>
# include <cstdio>
#endif
#include <windows.h>

#ifdef   _DEBUG
# define ASSERT(cond) \
  do { \
    if (!(cond)) { \
      DEBUG_OUTPUT(1, "%s: Assertion failed: %s\n", __FUNCTION__, #cond); \
      assert(cond); \
    } \
  } while(0)
#else
# define ASSERT(cond)
#endif

inline const char* to_string_dbg(const char* file, int filesz, int line, char* out, int out_sz, const char* fmt, ...)
{
  va_list(args);
  va_start(args, fmt);
  int   n = vsnprintf(out, out_sz, fmt, args);
  va_end(args);

  if (file) {
    const char* p = file + filesz;
    for (; p > file; --p) if (*p == '/' || *p == '\\') { ++p; break; }
    if (n > 0 && n < out_sz) {
      // Strip trailing '\r\n'
      if (out[n - 1] == '\n') {
        --n;
        if (n && out[n - 1] == '\r') --n;
      }
      snprintf(out + n, out_sz - n, " [%s:%d]\r\n", p, line);
    }
  }
  return out;
}

#define DEBUG_OUTPUT(cond, fmt, ...) DEBUG_OUTPUT__(__FILE__, sizeof(__FILE__)-1, __LINE__, (cond), fmt, __VA_ARGS__)

#ifndef __NO_CPP
# define DEBUG_OUTPUT__(f, fsz, l, cond, fmt, ...) do { \
      if (!!(cond)) { \
        char s[512]; \
        OutputDebugStringA(to_string_dbg(f, (fsz), l, s, sizeof(s), fmt, __VA_ARGS__)); \
      } \
    } while(0)
#else
# define DEBUG_OUTPUT__(f, fsz, l, cond, fmt, ...) do { \
      if (!!(cond)) \
        OutputDebugStringA(to_string_dbg(f, fsz, l, fmt, __VA_ARGS__).c_str()); \
    } while(0)

struct Tracer {
  template <int N>
  Tracer(bool dbg, const char* fun, const char(&file)[N], int line)
    : m_dbg(dbg), m_fun(fun)
  {
    DEBUG_OUTPUT__(file, N, line, m_dbg, "%*s-> Entering %s", level(true), " ", m_fun);
  }

  ~Tracer() { DEBUG_OUTPUT__(nullptr, 0, 0, m_dbg, "%*s <- Exiting %s", level(false), " ", m_fun); }
private:
  bool        m_dbg;
  const char* m_fun;

  static int level(bool inc) {
    static int s_level;
    return inc ? s_level++ : --s_level;
  }
};

#endif

#ifdef _TRACE
# define TRACE_FUNCTION(dbg) Tracer __tr##__LINE__(dbg, __FUNCTION__, __FILE__, __LINE__)
#else
# define TRACE_FUNCTION(dbg) (void)0
#endif
/*
#ifdef DEBUG_MEMORY
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
# define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
# define DEBUG_CLIENTBLOCK
#endif // _DEBUG
*/