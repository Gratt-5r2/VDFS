#include <UnionAfx.h>
#include "VDFS\VDFS.h"

namespace VDFS {
  uint DecompressedCacheSize = 20;
  uint UnzipThreadsCount = 2;
  bool UnzipMultithreading = false;
  bool TerminateProcessHotkey = true;
  bool UseErrorVisuals = false;
  char ModName[64];

  template<typename T>
  T ImportProc( const string& dllName, const string& funcName ) {
    HMODULE module = GetModuleHandle( dllName );
    return module ? (T)GetProcAddress( module, funcName ) : Null;
  }

  int ReadOptionInt( const char* section, const char* parameter, const int& default ) {
    auto proc = ImportProc<int(*)(const char*, const char*, const int&)>( "shw32.dll", "ReadOptionInt" );
    return proc ? proc( section, parameter, default ) : default;
  }

  bool ReadOptionBool( const char* section, const char* parameter, const bool& default ) {
    auto proc = ImportProc<bool(*)(const char*, const char*, const bool&)>( "shw32.dll", "ReadOptionBool" );
    return proc ? proc( section, parameter, default ) : default;
  }

  void LoadAutorunFile( const char* name ) {
    auto proc = ImportProc<void(*)(const char*)>( "shw32.dll", "LoadAutorunFile" );
    if( proc )
      proc( name );
  }
}