#include "sqlite3_wrapper.h"

int main(int argc, char* argv[])
{
  int dbg = argc > 1 ? atoi(argv[1]) : 1;
  const wchar_t* path = L"c:/temp";

  int err = sqlite_initialize(path);
  printf("Err: %d\n", err);
  return 0;
}