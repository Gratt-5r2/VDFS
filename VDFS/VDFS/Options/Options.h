#pragma once

namespace VDFS {
  extern uint DecompressedCacheSize;
  extern uint UnzipThreadsCount;
  extern bool UnzipMultithreading;
  extern bool TerminateProcessHotkey;
  extern bool UseErrorVisuals;
  extern char ModName[64];

  int ReadOptionInt( const char* section, const char* parameter, const int& default );
  bool ReadOptionBool( const char* section, const char* parameter, const bool& default );
  void LoadAutorunFile( const char* name );
}