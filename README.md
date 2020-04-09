sqlite3-mt4-wrapper - An sqlite3 binding for MT4,5
==================================================

This fork is non-backward compatible with the master.
It contains many enhancements and bug fixes.


## Howto

**NOTICE: This instruction is only for MT4 build 600 or later!**
For pre-600 builds use dll and header from tag https://github.com/Shmuma/sqlite3-mt4-wrapper/tree/pre-600

1. Download [zip of master](https://github.com/Shmuma/sqlite3-mt4-wrapper/archive/master.zip)
2. Extract it
3. Copy all contents under ``MQL4`` directory, to ``<TERMINAL_DATA_PATH>/MQL4``
    * See the "Terminal data path" section below
4. In your EA/Indicator/Script, add following include

```cpp
#include <sqlite.mqh>
```
5. Add the following code

```cpp

void Test(string path_to_dbfile) {
  SQLite db;
  if (!db.OpenReadOnly(path_to_dbfile)) {
    Alert(StringFormat("Cannot open database %s: %s", path_to_dbfile, db.ErrMsg()));
    return;
  }

  string sql = StringFormat(
                  "SELECT "
                  "  ticket,ts_open,lots,px_open,sl,tp,ts_close,px_close,"
                  "  magic,fees,swap,profit,comment,type "
                  "FROM trade "
                  "WHERE symbol like '%s%%' and type < 2 "
                  "ORDER BY ts_open",
                  _Symbol
               );
  
  SQLiteQuery* query = db.Prepare(sql);
  if (!query) {
    Alert(StringFormat("Invalid query: %s", db.ErrMsg()));
    return;
  }

  int pip_points = PipPoints(_Symbol);
  int count      = 0;
  int tzoff      = (int)(TimeCurrent()-TimeGMT());

  for(; query.Next() == SQLITE_ROW; ++count) {
    int      ticket   = query.GetInt(0);
    datetime ts_open  = query.GetDatetime(1)+tzoff;
    double   lots     = query.GetDouble(2);
    double   px_open  = query.GetDouble(3);
    double   sl       = query.GetDouble(4);
    double   tp       = query.GetDouble(5);
    datetime ts_close = query.GetDatetime(6)+tzoff;
    double   px_close = query.GetDouble(7);
    double   fees     = query.GetDouble(9);
    double   swap     = query.GetDouble(10);
    double   profit   = query.GetDouble(11);
    string   comment  = query.GetString(12);
    int      type     = query.GetInt(13);

    double   tot_prof = fees+swap+profit;
    double   pips     = MathAbs(px_close-px_open)/_Point/pip_points;

    string   side     = type < 1 ? "buy" : "sell";
    PrintFormat("#%d %s %.2f at %.5f close at %.5f (%.1fp) %s $%.2f%s"),
                ticket, side, lots,
                px_open, px_close, pips,
                tot_prof >= 0 ? "profit" : "loss", tot_prof,
                StringLen(comment) ? "" : ", cmt="+comment);
  }

  delete query;
}
```
6. sqlite wrapper functions

## Database file

Database file is by default stored to ``<TERMINAL_DATA_PATH>\MQL4\Files\SQLite``.

If you specify a full path as database filename, it's used.

## Terminal data path

TERMINAL_DATA_PATH can be known by the following instruction.

1. Open MT4
2. Open [File] menu
3. Click "Open Data Folder"

## Sample

Many sample scripts in under ``MQL4/Scripts``.

## Precautions
### Argument mess

MT4 build 610 has a weird bug when dll function with two string arguments gets corrupted when both values are variables. In that case, inside dll, second argument is the same as the first. In sqlite-wrapper two routines are affected: sqlite_exec and sqlite_table_exists. The simple temparary workaround of this (I hope it will be fixed in latest MT4) is to add empty string to a second argument, for examlpe:
```
bool do_check_table_exists (string db, string table)
{
    int res = sqlite_table_exists (db, table + "");
```
