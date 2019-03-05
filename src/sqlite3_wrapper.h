#pragma once

#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINVER 0x0501
//#define _WIN32_WINNT 0x0501
//#include <windef.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>

// Error code
#define INIT_SUCCESS 0
#define ERROR_INVALID_TERM_DATA_DIR 0x01
#define ERROR_BUF_TOO_SMALL 99999

// GC parameters
#define MAX_GC_ITEM_COUNT 100
#define GC_EXEC_LIMIT 90

#define APIEXPORT __declspec(dllexport) 

extern int RegisterExtensionFunctions(sqlite3 *db);

APIEXPORT int            WINAPI sqlite_initialize   (const wchar_t* term_data_path);
APIEXPORT BOOL           WINAPI sqlite_get_fname    (const wchar_t* db_filename, wchar_t* path, int pathlen);
APIEXPORT sqlite3*       WINAPI sqlite_open         (const wchar_t* db_filename);
APIEXPORT void           WINAPI sqlite_close        (sqlite3* db);
APIEXPORT const wchar_t* WINAPI sqlite_errmsg       (sqlite3* db);
APIEXPORT int            WINAPI sqlite_errcode      (sqlite3* db);
APIEXPORT int            WINAPI sqlite_changes      (sqlite3* db);
APIEXPORT int            WINAPI sqlite_exec         (sqlite3* db, const wchar_t* sql);
APIEXPORT int            WINAPI sqlite_exec2        (const wchar_t* db_filename, const wchar_t* sql);
APIEXPORT int            WINAPI sqlite_table_exists (sqlite3* db, const wchar_t* table_name);
APIEXPORT int            WINAPI sqlite_table_exists2(const wchar_t* db_filename, const wchar_t* table_name);
APIEXPORT int            WINAPI sqlite_query        (sqlite3* db, const wchar_t* sql, int* cols);
APIEXPORT int            WINAPI sqlite_query2       (const wchar_t* db_filename, const wchar_t* sql, int* cols);
APIEXPORT BOOL           WINAPI sqlite_reset        (int handle);
APIEXPORT BOOL           WINAPI sqlite_bind_int     (int handle, int col, int bind_value);
APIEXPORT BOOL           WINAPI sqlite_bind_int64   (int handle, int col, __int64 bind_value);
APIEXPORT BOOL           WINAPI sqlite_bind_double  (int handle, int col, double bind_value);
APIEXPORT BOOL           WINAPI sqlite_bind_text    (int handle, int col, const wchar_t* bind_value);
APIEXPORT BOOL           WINAPI sqlite_bind_null    (int handle, int col);
APIEXPORT int            WINAPI sqlite_bind_param_count(int handle);
APIEXPORT int            WINAPI sqlite_bind_param_index(int handle, const wchar_t* param_name);
APIEXPORT int            WINAPI sqlite_step         (int handle);
APIEXPORT const wchar_t* WINAPI sqlite_get_col      (int handle, int col);
APIEXPORT int            WINAPI sqlite_get_col_int  (int handle, int col);
APIEXPORT __int64        WINAPI sqlite_get_col_int64(int handle, int col);
APIEXPORT double         WINAPI sqlite_get_col_double(int handle, int col);
APIEXPORT int            WINAPI sqlite_free_query   (int handle);
APIEXPORT void           WINAPI sqlite_set_busy_timeout(int ms);
APIEXPORT int            WINAPI sqlite_set_journal_mode(const wchar_t* mode);

#ifdef DEBUG_TRACE
#define DEBUG_OUTPUT(fmt, ...) \
  do { char buf[256]; snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); OutputDebugStringA(buf); } \
  while(0)
#define DEBUG_OUTPUT_IF(cond, fmt, ...) \
  do { if (cond) { char buf[256]; snprintf(buf, sizeof(buf), fmt, __VA_ARGS__); OutputDebugStringA(buf); } } \
  while(0)
#else
#define DEBUG_OUTPUT(fmt, ...)
#define DEBUG_OUTPUT_IF(cond, fmt, ...)
#endif
