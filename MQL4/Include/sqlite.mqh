//+------------------------------------------------------------------+
//|                                     SQLite interface for MT{4,5} |
//|                                          Author: Serge Aleynikov |
//+------------------------------------------------------------------+
//| Derived from project by implementing many enhancements:          |
//| https://github.com/Shmuma/sqlite3-mt4-wrapper                    |
//+------------------------------------------------------------------+
#ifndef _SQLITE_MQH_
#define _SQLITE_MQH_

//+------------------------------------------------------------------+
enum ENUM_SQLITE_JOURN {
  JOURN_DELETE,
  JOURN_TRUNCATE,
  JOURN_PERSIST,
  JOURN_MEMORY,
  JOURN_WAL,
  JOURN_OFF
};

enum ENUM_SQLITE_COLTYPE {
  SQLITE_INTEGER  = 1,
  SQLITE_FLOAT    = 2,
  SQLITE_TEXT     = 3,
  SQLITE_BLOB     = 4,
  SQLITE_NULL     = 5
};

#define SQLITE_OK           0   /* Successful result */
/* beginning-of-error-codes */
#define SQLITE_ERROR        1   /* SQL error or missing database */
#define SQLITE_INTERNAL     2   /* Internal logic error in SQLite */
#define SQLITE_PERM         3   /* Access permission denied */
#define SQLITE_ABORT        4   /* Callback routine requested an abort */
#define SQLITE_BUSY         5   /* The database file is locked */
#define SQLITE_LOCKED       6   /* A table in the database is locked */
#define SQLITE_NOMEM        7   /* A malloc() failed */
#define SQLITE_READONLY     8   /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT    9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR       10   /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT     11   /* The database disk image is malformed */
#define SQLITE_NOTFOUND    12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL        13   /* Insertion failed because database is full */
#define SQLITE_CANTOPEN    14   /* Unable to open the database file */
#define SQLITE_PROTOCOL    15   /* Database lock protocol error */
#define SQLITE_EMPTY       16   /* Database is empty */
#define SQLITE_SCHEMA      17   /* The database schema changed */
#define SQLITE_TOOBIG      18   /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT  19   /* Abort due to constraint violation */
#define SQLITE_MISMATCH    20   /* Data type mismatch */
#define SQLITE_MISUSE      21   /* Library used incorrectly */
#define SQLITE_NOLFS       22   /* Uses OS features not supported on host */
#define SQLITE_AUTH        23   /* Authorization denied */
#define SQLITE_FORMAT      24   /* Auxiliary database format error */
#define SQLITE_RANGE       25   /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB      26   /* File opened that is not a database file */
#define SQLITE_NOTICE      27   /* Notifications from sqlite3_log() */
#define SQLITE_WARNING     28   /* Warnings from sqlite3_log() */
#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

#define SQLITE_OPEN_READONLY         0x00000001  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_READWRITE        0x00000002  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE           0x00000004  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE_RW        (SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE)
#define SQLITE_OPEN_DELETEONCLOSE    0x00000008  /* VFS only */
#define SQLITE_OPEN_EXCLUSIVE        0x00000010  /* VFS only */
#define SQLITE_OPEN_AUTOPROXY        0x00000020  /* VFS only */
#define SQLITE_OPEN_URI              0x00000040  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_MEMORY           0x00000080  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_MAIN_DB          0x00000100  /* VFS only */
#define SQLITE_OPEN_TEMP_DB          0x00000200  /* VFS only */
#define SQLITE_OPEN_TRANSIENT_DB     0x00000400  /* VFS only */
#define SQLITE_OPEN_MAIN_JOURNAL     0x00000800  /* VFS only */
#define SQLITE_OPEN_TEMP_JOURNAL     0x00001000  /* VFS only */
#define SQLITE_OPEN_SUBJOURNAL       0x00002000  /* VFS only */
#define SQLITE_OPEN_MASTER_JOURNAL   0x00004000  /* VFS only */
#define SQLITE_OPEN_NOMUTEX          0x00008000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_FULLMUTEX        0x00010000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_SHAREDCACHE      0x00020000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_PRIVATECACHE     0x00040000  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_WAL              0x00080000  /* VFS only */

class SQLiteQuery;

//+------------------------------------------------------------------+
//| Database class                                                   |
//+------------------------------------------------------------------+
struct SQLite
{
                SQLite()              : m_db(0)       { Init();                       }
                SQLite(string dbname) : m_db(0)       { if (Init()) Open(dbname);     }
                SQLite(string a_db,int flags):m_db(0) { if (Init()) Open(a_db,flags); }
               ~SQLite()                              { Close();                      }

  bool          IsOpen() const                        { return m_db != 0;             }
  bool          Open()                                { return Open(m_dbname);        }
  bool          Open(string fname);
  bool          Open(uint flags)                      { return Open(m_dbname, flags); }
  bool          Open(string fname, uint flags);
  bool          OpenReadOnly()                        { return OpenReadOnly(m_dbname);}
  bool          OpenReadOnly(string fname)            { return Open(fname, SQLITE_OPEN_READONLY); }
  void          Close()                               { if (!m_db) return; sqlite_close(m_db); m_db=0; }
  void          JournalMode(ENUM_SQLITE_JOURN mode);

  bool          HasTable(string table)                { return sqlite_table_exists(m_db,table)>0;  }
  string        Filename() const;

  // Create a query object.
  // Returns NULL on error.
  // The query must be explicitely deleted after use.
  SQLiteQuery*  Prepare (string sql, int& cols)     { return SQLiteQuery::Create(m_db,sql,cols); }
  SQLiteQuery*  Prepare (string sql)                { int    cols; return Prepare(sql,cols);     }

  // Execute an SQL query or DDL.
  // Returns SQLITE_OK(0) on success or negative value on error.
  int           ExecDDL (string sql)                { return sqlite_exec(m_db, sql);             }

  // Number of rows affected by last CRUD operation
  int           Changes() const                     { return sqlite_changes(m_db);               }

  int           ErrCode() const                     { return sqlite_errcode(m_db);               }
  string        ErrMsg()  const                     { return sqlite_errmsg(m_db);                }

  int           BeginTransaction()                  { return ExecDDL("BEGIN TRANSACTION");       }
  int           EndTransaction(bool commit=true)    { return ExecDDL(commit?"COMMIT":"ROLLBACK");}

  int           Handle() const { return m_db; }
private:
  string        m_dbname;
  int           m_db;

  bool          Init();
};

//+------------------------------------------------------------------+
//| Query class                                                      |
//+------------------------------------------------------------------+
class SQLiteQuery
{
public:
  static SQLiteQuery* Create(int    db,       string sql, int& cols);
  static SQLiteQuery* Create(string db_fname, string sql, int& cols);

         SQLiteQuery(int handle = -1) : m_handle(handle) {}
        ~SQLiteQuery() { if (m_handle >= 0) sqlite_free_query(m_handle); }

  // Reset a prepared statement object
  bool   Reset() { return !!sqlite_reset(m_handle); }

  // NOTE: 'col' is 1-based in all functions below:
  bool   Bind(int col, int    value) { return !!sqlite_bind_int   (m_handle,col,value); }
  bool   Bind(int col, long   value) { return !!sqlite_bind_int64 (m_handle,col,value); }
  bool   Bind(int col, datetime val) { return Bind(col, (long)val);                     }
  bool   Bind(int col, double value) { return !!sqlite_bind_double(m_handle,col,value); }
  bool   Bind(int col, const char& v[])   { return !!sqlite_bind_text(m_handle, col, v); }
  bool   Bind(int col, string v) { return v==NULL ? BindNull(col) : !!sqlite_bind_text16(m_handle,col,v); }
  bool   BindNull(int col)           { return !!sqlite_bind_null(m_handle, col);        }

  bool   Bind(string var, int    v)  { int n; return ParIdx(var, n) && Bind(n, v);      }
  bool   Bind(string var, long   v)  { int n; return ParIdx(var, n) && Bind(n, v);      }
  bool   Bind(string var, datetime v){ int n; return ParIdx(var, n) && Bind(n, v);      }
  bool   Bind(string var, double v)  { int n; return ParIdx(var, n) && Bind(n, v);      }
  bool   Bind(string var, string v)  { int n; return ParIdx(var, n) && Bind(n, v);      }
  bool   BindNull(string var)        { int n; return ParIdx(var, n) && BindNull(n);     }

  // If `cond` is true, the `val` is bound, otherwise NULL is stored.
  template <typename T>
  bool   BindIf(int col, bool cond, T val) { return cond ? Bind(col,val):BindNull(col); }

  bool     Exec()                    { return sqlite_step(m_handle)==SQLITE_DONE;       }
  int      Next()                    { return sqlite_step(m_handle);                    }

  // NOTE: 'col' is 0-based in all functions below:
  string   GetString  (int col) const{ return sqlite_get_col       (m_handle, col);     }
  int      GetInt     (int col) const{ return sqlite_get_col_int   (m_handle, col);     }
  long     GetLong    (int col) const{ return sqlite_get_col_int64 (m_handle, col);     }
  datetime GetDatetime(int col) const{ return (datetime)GetLong(col);                   }
  double   GetDouble  (int col) const{ return sqlite_get_col_double(m_handle, col);     }

  bool     IsNull(int col)      const{ return GetColType(col) == SQLITE_NULL;           }
  bool     IsNotNull(int col)   const{ return GetColType(col) != SQLITE_NULL;           }

  ENUM_SQLITE_COLTYPE GetColType(int col) const { return (ENUM_SQLITE_COLTYPE)sqlite_get_col_type(m_handle, col); }

private:
  int      m_handle;

  bool ParIdx(string param, int& idx) { idx = sqlite_bind_param_index(m_handle, param); return !!idx; }
};

//+------------------------------------------------------------------+
//| DLL imports                                                      |
//+------------------------------------------------------------------+
#ifdef __MQL4__
#import "MQT/mqt-sqlite3.x86.dll"
#else
#import "MQT/mqt-sqlite3.x64.dll"
#endif
  int    sqlite_initialize      (string terminal_data_path);
  bool   sqlite_get_fname       (string db_fname, string& path, int pathlen);
  int    sqlite_open            (string db_filename);
  int    sqlite_open_v2         (string db_filename, uint flags=0);
  void   sqlite_close           (int    db);
  int    sqlite_errcode         (int    db);
  string sqlite_errmsg          (int    db);

  int    sqlite_changes         (int    db);

  int    sqlite_exec            (int    db,       string sql);
  int    sqlite_exec2           (string db_fname, string sql);
  int    sqlite_table_exists    (int    db,       string table);
  int    sqlite_table_exists2   (string db_fname, string table);

  int    sqlite_query           (int    db,       string sql, int& cols);
  int    sqlite_query2          (string db_fname, string sql, int& cols);
  int    sqlite_reset           (int handle);
  int    sqlite_bind_int        (int handle, int col, int bind_value);
  int    sqlite_bind_int64      (int handle, int col, long bind_value);
  int    sqlite_bind_double     (int handle, int col, double bind_value);
  int    sqlite_bind_text       (int handle, int col, const char&   bind_value[]);
  int    sqlite_bind_text16     (int handle, int col, const string& bind_value);
  int    sqlite_bind_null       (int handle, int col);
  int    sqlite_bind_param_count(int handle);
  int    sqlite_bind_param_index(int handle, string bind_value);
  bool   sqlite_step            (int handle);
  int    sqlite_get_col_type    (int handle, int col);
  string sqlite_get_col         (int handle, int col);
  int    sqlite_get_col_int     (int handle, int col);
  long   sqlite_get_col_int64   (int handle, int col);
  double sqlite_get_col_double  (int handle, int col);
  int    sqlite_free_query      (int handle);
  void   sqlite_set_busy_timeout(int ms);
  void   sqlite_set_journal_mode(string mode);
#import

//+------------------------------------------------------------------+
//| IMPLEMENTATION                                                   |
//+------------------------------------------------------------------+
bool SQLite::Init()
{
  int error = sqlite_initialize(TerminalInfoString(TERMINAL_DATA_PATH));
  if (error == 0)
    return true;

  Alert("ERROR: sqlite initialization failed: " + IntegerToString(error));
  return false;
}

//+------------------------------------------------------------------+
bool SQLite::Open(string fname)
{
  if (IsOpen()) return true;
  m_dbname = fname;
  m_db     = sqlite_open(fname);
  return m_db != 0;
}

//+------------------------------------------------------------------+
bool SQLite::Open(string fname, uint flags)
{
  if (IsOpen()) return true;
  m_dbname = fname;
  m_db     = sqlite_open_v2(fname, flags);
  return m_db != 0;
}

//+------------------------------------------------------------------+
string SQLite::Filename() const
{
  string s; StringInit(s,1024);
  return sqlite_get_fname(m_dbname, s, 1024) ? s : NULL;
}

//+------------------------------------------------------------------+
void SQLite::JournalMode(ENUM_SQLITE_JOURN mode)
{
  static const string s_mode[] = {"DELETE","TRUNCATE","PERSIST","MEMORY","WAL","OFF"};
  sqlite_set_journal_mode(s_mode[mode]);
}

//+------------------------------------------------------------------+
SQLiteQuery* SQLiteQuery::Create(int dbh,string sql,int& cols)
{
  int handle = sqlite_query(dbh, sql, cols);
  return handle < 0 ? NULL : new SQLiteQuery(handle);
}

//+------------------------------------------------------------------+
SQLiteQuery* SQLiteQuery::Create(string db_fname,string sql,int& cols)
{
  int    handle = sqlite_query2(db_fname, sql, cols);
  return handle < 0 ? NULL : new SQLiteQuery(handle);
}

#endif // _SQLITE_MQH_
