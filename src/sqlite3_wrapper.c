#include "debug.h"
#include "sqlite3_wrapper.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

// MetaTrader4 TERMINAL_DATA_PATH
static wchar_t terminal_data_path[2048];

// how long wait when DB is busy
static int busy_timeout = 1000;

// pragma for journal mode
static char journal_statement[256];

static int debug;

struct query_result {
    sqlite3 *s;
    sqlite3_stmt *stmt;
};

static void *my_alloc(size_t size)
{
    return HeapAlloc (GetProcessHeap (), 0, size);
}

static void *my_realloc(void *ptr, size_t size)
{
    return HeapReAlloc (GetProcessHeap (), 0, ptr, size);
}

static BOOL my_free(void *ptr)
{
    return HeapFree (GetProcessHeap (), 0, ptr);
}

static BOOL directory_exists (const wchar_t* path)
{
    DWORD   attr =  GetFileAttributesW(path);
    return (attr != INVALID_FILE_ATTRIBUTES &&
           (attr &  FILE_ATTRIBUTE_DIRECTORY));
}

static const char* unicode_to_ansi_string (const wchar_t* unicode, char* buf, int bufsz, BOOL chk_size)
{
    int ansi_bytes = bufsz;

    if (chk_size) {
      ansi_bytes = WideCharToMultiByte(
          CP_ACP,
          WC_COMPOSITECHECK | WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR,
          unicode, -1, NULL, 0, NULL, NULL);

      if (ansi_bytes == 0 || ansi_bytes > bufsz)
          return NULL;
    }

    const int converted_bytes = WideCharToMultiByte(
        CP_ACP,
        WC_COMPOSITECHECK | WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR,
        unicode, -1, buf, ansi_bytes, NULL, NULL);

    return converted_bytes ? buf : NULL;
}

static const char* unicode_to_ansi_str(const wchar_t* unicode, char* buf, int bufsz)
{
    return unicode_to_ansi_string(unicode, buf, bufsz, TRUE);
}

static const char* unicode_to_ansi_alloc(const wchar_t* unicode)
{
    const int ansi_bytes = WideCharToMultiByte(
        CP_ACP,
        WC_COMPOSITECHECK | WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR,
        unicode, -1, NULL, 0, NULL, NULL);

    if (ansi_bytes == 0)
        return NULL;

    char*  ansi_buf = (char*) my_alloc(ansi_bytes);
    const char* res = unicode_to_ansi_string(unicode, ansi_buf, ansi_bytes, FALSE);

    if (!res)
        my_free (ansi_buf);

    return res;
}

static const wchar_t* ansi_to_unicode_string(const char *ansi, wchar_t* buf, int bufsz, BOOL chk_size)
{
    int unicode_bytes = bufsz;

    if (chk_size) {
        unicode_bytes = MultiByteToWideChar(
          CP_ACP,
          MB_COMPOSITE,
          ansi, -1, NULL, 0);

        if (unicode_bytes == 0 || unicode_bytes > bufsz)
          return NULL;
    }

    const int converted_bytes = MultiByteToWideChar(
        CP_ACP,
        MB_COMPOSITE,
        ansi, -1, buf, unicode_bytes);

    return converted_bytes ? buf : NULL;
}

static const wchar_t* ansi_to_unicode_str(const char *ansi, wchar_t* buf, int bufsz)
{
  return ansi_to_unicode_string(ansi, buf, bufsz, TRUE);
}

static const wchar_t* ansi_to_unicode_alloc(const char *ansi)
{
    const int unicode_bytes = MultiByteToWideChar(
        CP_ACP,
        MB_COMPOSITE,
        ansi, -1, NULL, 0);

    if (unicode_bytes == 0)
        return NULL;

    wchar_t*       buf = (wchar_t*) my_alloc (unicode_bytes);
    const wchar_t* res = ansi_to_unicode_string(ansi, buf, unicode_bytes, FALSE);

    if (!res)
        my_free (buf);

    return buf;
}

static wchar_t* my_wcscat (wchar_t** dst, const wchar_t* src)
{
    long dst_buf_size = 0;

    if (*dst == NULL) {
        dst_buf_size = wcslen (src) + 1;
        *dst = (wchar_t*) my_alloc (sizeof (wchar_t) * dst_buf_size);
        *dst[0] = L'\0';
    }
    else {
        dst_buf_size = wcslen (*dst) + wcslen (src) + 1;
        *dst = (wchar_t*) my_realloc (*dst, sizeof (wchar_t) * dst_buf_size);
    }

    return wcsncat (*dst, src, wcslen (src));
}

/* We assume that given file name is relative to MT Terminal Data Path. */
static wchar_t* build_db_path (const wchar_t* db_filename, wchar_t* path, int pathlen)
{
    // if path is absolute, just return it, assuming it holds full db path
    if (!PathIsRelativeW (db_filename) || (db_filename && db_filename[0] == L'/')) {
        wcscpy_s(path, pathlen, db_filename);
        return path;
    }

    int n = _scwprintf(path, L"%s/MQL4/Files", terminal_data_path);
    if (n < 0 || n > pathlen)
        return NULL;
    
    if (!directory_exists (path))
        CreateDirectoryW (path, NULL);

    int len = _scwprintf(path+n, L"/%s", db_filename);
    for (wchar_t* p=path+n; p; ++p)
      if (*p == L'\\') *p = L'/';

    return len < 0 || n + len > pathlen ? NULL : path;
}

static void tune_db_handler (sqlite3* s)
{
    sqlite3_busy_timeout (s, busy_timeout);

    if (journal_statement[0] != '\0')
        sqlite3_exec (s, journal_statement, NULL, NULL, NULL);

    RegisterExtensionFunctions (s);
}

static BOOL set_terminal_data_path(const wchar_t* path)
{
    if (!directory_exists (path))
        return FALSE;

    terminal_data_path[0] = L'\0';
    auto res = wcscpy_s(terminal_data_path, sizeof(terminal_data_path)/sizeof(wchar_t), path) == 0;
    
    for (wchar_t* p = terminal_data_path; *p; ++p)
      if (*p == L'\\') *p = L'/';

    return TRUE;
}

void error_log_callback(void* arg, int err_code, const char* err_msg) {
  char buf[256];
  snprintf(buf, sizeof(buf), "Error in SQLite: (%d) %s", err_code, err_msg);
  OutputDebugStringA(buf);
}

APIEXPORT int WINAPI sqlite_initialize(const wchar_t* term_data_path)
{
    return set_terminal_data_path (term_data_path) ? INIT_SUCCESS : ERROR_INVALID_TERM_DATA_DIR;
}

APIEXPORT BOOL WINAPI sqlite_get_fname (const wchar_t* db_filename, wchar_t* path, int pathlen)
{
    return build_db_path (db_filename, path, pathlen) != NULL;
}

APIEXPORT sqlite3* WINAPI sqlite_open (const wchar_t* db_filename)
{
  wchar_t path[1024];
  const wchar_t* db_path = build_db_path(db_filename, path, sizeof(path) / sizeof(wchar_t));

  if (!db_path)
    return NULL;

  sqlite3* s;
  int res = sqlite3_open16(db_path, &s);

  if (res != SQLITE_OK)
    return NULL;

  tune_db_handler(s);

  sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, NULL);
  return s;
}

APIEXPORT sqlite3* WINAPI sqlite_open_v2(const wchar_t* db_filename, unsigned flags)
{
  wchar_t path[1024];
  const wchar_t* db_path = build_db_path(db_filename, path, sizeof(path) / sizeof(wchar_t));

  if (!db_path)
    return NULL;

  const char* paths = unicode_to_ansi_alloc(path);
  if (!paths)
    return NULL;

  if (flags == 0)
    flags = SQLITE_OPEN_READONLY;

  sqlite3* s;
  int res = sqlite3_open_v2(paths, &s, flags, NULL);

  my_free((void*)paths);

  if (res != SQLITE_OK)
    return NULL;

  tune_db_handler(s);

  sqlite3_config(SQLITE_CONFIG_LOG, error_log_callback, NULL);
  return s;
}

APIEXPORT void WINAPI sqlite_close(sqlite3* db) { if (db) sqlite3_close(db); }

APIEXPORT int  WINAPI sqlite_errcode(sqlite3* db) { return sqlite3_errcode(db); }

APIEXPORT int  WINAPI sqlite_changes(sqlite3* db) { return sqlite3_changes(db); }

APIEXPORT const wchar_t* WINAPI sqlite_errmsg (sqlite3* db)
{
  return (const wchar_t* )sqlite3_errmsg16(db);
}

APIEXPORT int WINAPI sqlite_exec(sqlite3* db, const wchar_t* sql)
{
    if (db == NULL) return SQLITE_ERROR;

    int   res;
    char  sqlbuf[4096];
    const char* sql_ansi = unicode_to_ansi_str(sql, sqlbuf, sizeof(sqlbuf));
    if (sql_ansi)
        res = sqlite3_exec(db, sql_ansi, NULL, NULL, NULL);
    else {
        const char* sql_ansi = unicode_to_ansi_alloc(sql);
        res = sqlite3_exec(db, sql_ansi, NULL, NULL, NULL);
        my_free((void *)sql_ansi);
    }
    return res;
}

APIEXPORT int  WINAPI sqlite_exec2(const wchar_t* db_filename, const wchar_t* sql)
{
    sqlite3* s = sqlite_open(db_filename);

    if (!s)
        return -1;

    int    res = sqlite_exec(s, sql);
    sqlite3_close (s);
    return res;
}

/*
 * return 1 if table exists in database, 0 oterwise. -ERROR returned on error.
 */
APIEXPORT int WINAPI sqlite_table_exists(sqlite3* db, const wchar_t* table_name)
{
    if (db == NULL) return -SQLITE_ERROR;

    sqlite3_stmt *stmt;

    wchar_t sql[256];
    _snwprintf_s(sql, 256, _TRUNCATE, L"select count(*) from sqlite_master where type='table' and name='%s'", table_name);

    int res = sqlite3_prepare16_v2(db, sql, sizeof(sql), &stmt, NULL);

    if (res != SQLITE_OK)
        return -res;

    res = sqlite3_step (stmt);
    int exists = res == SQLITE_ROW ? sqlite3_column_int(stmt, 0) : 0;
    sqlite3_finalize (stmt);

    return exists > 0;
}

APIEXPORT int WINAPI sqlite_table_exists2(const wchar_t* db_filename, const wchar_t* table_name)
{
  sqlite3* s = sqlite_open(db_filename);
  int    res = sqlite_table_exists(s, table_name);
  sqlite_close(s);
  return res;
}

APIEXPORT long WINAPI sqlite_query(sqlite3* db, const wchar_t* sql, int* cols)
{
    assert(cols);

    *cols = 0;
    if (db == NULL)
      return -SQLITE_ERROR;

    sqlite3_stmt* stmt;
    int res = sqlite3_prepare16(db, sql, (int)(wcslen(sql) * sizeof(wchar_t)), &stmt, NULL);
    if (res != SQLITE_OK)
      return -res;

    struct query_result* result = (struct query_result*)malloc (sizeof (struct query_result));
    if (!result)
      return -SQLITE_NOMEM;
    result->s    = NULL;
    result->stmt = stmt;
    *cols = sqlite3_column_count(stmt);
    return (long)result;
}

/*
* Perform query and pack results in internal structure. Routine returns amount of data fetched and
* integer handle which can be used to sqlite_get_data. On error, return -SQLITE_ERROR.
*/
APIEXPORT long WINAPI sqlite_query2(const wchar_t* db_filename, const wchar_t* sql, int* cols)
{
  sqlite3* s = sqlite_open(db_filename);
  if (s == NULL)
      return 0;

  DEBUG_OUTPUT(debug, "sqlite_query2: Database open -> %p", (long)s);
  long res = sqlite_query(s, sql, cols);

  DEBUG_OUTPUT(debug, "sqlite_query2: sqlite_query -> %d (cols=%d)", res, *cols);

  if (res < 0) {
      sqlite3_close(s);
      return res;
  }

  struct query_result* result = (struct query_result*)res;
  result->s = s;

  return res;
}

APIEXPORT BOOL WINAPI sqlite_reset (int handle)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_reset (res->stmt);

    return ret == SQLITE_OK;
}

APIEXPORT BOOL WINAPI sqlite_bind_int (int handle, int col, int bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_int (res->stmt, col, bind_value);

    return ret == SQLITE_OK;
}

APIEXPORT BOOL WINAPI sqlite_bind_int64 (int handle, int col, __int64 bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_int64 (res->stmt, col, bind_value);

    return ret == SQLITE_OK;
}

APIEXPORT BOOL WINAPI sqlite_bind_double (int handle, int col, double bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_double (res->stmt, col, bind_value);

    return ret == SQLITE_OK;
}

APIEXPORT BOOL WINAPI sqlite_bind_text(int handle, int col, const char* bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_text(res->stmt, col, bind_value, -1, SQLITE_STATIC);

    return ret == SQLITE_OK;
}

APIEXPORT BOOL WINAPI sqlite_bind_text16(int handle, int col, const wchar_t* bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_text16 (res->stmt, col, bind_value, -1, SQLITE_STATIC);

    return ret == SQLITE_OK;
}

APIEXPORT BOOL WINAPI sqlite_bind_null (int handle, int col)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_null (res->stmt, col);

    return ret == SQLITE_OK;
}

APIEXPORT int WINAPI sqlite_bind_param_count(int handle)
{
  struct query_result *res = (struct query_result*)handle;

  if (!res)
    return 0;

  return sqlite3_bind_parameter_count(res->stmt);
}

APIEXPORT int WINAPI sqlite_bind_param_index(int handle, const wchar_t* param_name)
{
  struct query_result *res = (struct query_result*)handle;

  if (!res)
    return 0;

  char parambuf[128];
  const char* param = unicode_to_ansi_str(param_name, parambuf, sizeof(parambuf));

  return !param ? -ERROR_BUF_TOO_SMALL : sqlite3_bind_parameter_index(res->stmt, param);
}

/*
 * Return 1 if next row fetched, 0 if end of resultset reached
 */
APIEXPORT int WINAPI sqlite_step(int handle)
{
    struct query_result *res = (struct query_result*)handle;

    if (!res)
        return 0;

    int ret = sqlite3_step (res->stmt);

    DEBUG_OUTPUT((ret != SQLITE_DONE && ret != SQLITE_ROW), "SQLite step returned: %d", ret);

    return ret;
}


APIEXPORT int WINAPI sqlite_get_col_type(int handle, int col)
{
  struct query_result* data = (struct query_result*)handle;

  if (!data)
    return -1;

  return sqlite3_column_type(data->stmt, col);
}

APIEXPORT const wchar_t* WINAPI sqlite_get_col (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return NULL;

    // In sqlite3.h we have the following note:
    // ** The pointers returned are valid until a type conversion occurs as
    // ** described above, or until [sqlite3_step()] or [sqlite3_reset()] or
    // ** [sqlite3_finalize()] is called.  ^The memory space used to hold strings
    // ** and BLOBs is freed automatically.  Do <b>not</b> pass the pointers returned
    // ** from [sqlite3_column_blob()], [sqlite3_column_text()], etc. into
    // ** [sqlite3_free()].

    // So, it's safe to just return pointer, as mql will copy string's content into it's own buffer
    // (according to docs).

    return (const wchar_t*)sqlite3_column_text16 (data->stmt, col);
}

APIEXPORT int WINAPI sqlite_get_col_int (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    return sqlite3_column_int (data->stmt, col);
}

APIEXPORT __int64 WINAPI sqlite_get_col_int64 (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    return sqlite3_column_int64 (data->stmt, col);
}

APIEXPORT double WINAPI sqlite_get_col_double (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    return sqlite3_column_double (data->stmt, col);
}

APIEXPORT int WINAPI sqlite_free_query (int handle)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    if (data->stmt)
        sqlite3_finalize (data->stmt);
    if (data->s)
        sqlite3_close (data->s);
    free (data);
    return 1;
}


APIEXPORT void WINAPI sqlite_set_busy_timeout (int ms)
{
    busy_timeout = ms;
}


APIEXPORT int WINAPI sqlite_set_journal_mode (const wchar_t* mode)
{
    if (!mode)
        return -1;

    if (journal_statement[0] != '\0')
        journal_statement[0]  = '\0';

    char modebuf[128];
    const char* mode_ansi = unicode_to_ansi_str(mode, modebuf, sizeof(modebuf));
    if (!mode_ansi)
        return -ERROR_BUF_TOO_SMALL;

    sprintf(journal_statement, "PRAGMA journal_mode=%s;", mode_ansi);

    return 0;
}

#ifdef __cplusplus
}
#endif
