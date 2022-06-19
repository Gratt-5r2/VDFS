#pragma once

#include <TlHelp32.h>
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)
namespace Common
{
  struct TModuleInfo
  {
    HMODULE hModule;
    uint32  ulBase;
    uint32  ulSize;

    static inline uint32 GetReturnAddress() {
      return (uint32)_ReturnAddress();
    }

    static inline bool BelongToModule( TModuleInfo* module, const uint32& address ) {
      return address >= module->ulBase && address < module->ulBase + module->ulSize;
    }

    static inline TModuleInfo* GetModuleInfo( const uint32& address ) {
      HANDLE l_hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId() );
      if( l_hSnapshot != INVALID_HANDLE_VALUE ) {
        MODULEENTRY32 l_mModuleEntry32;
        l_mModuleEntry32.dwSize = sizeof( MODULEENTRY32 );
        if( Module32First( l_hSnapshot, &l_mModuleEntry32 ) ) {
          do {
            TModuleInfo l_mInfo;
            l_mInfo.ulBase = (uint32)l_mModuleEntry32.modBaseAddr;
            l_mInfo.ulSize = (uint32)l_mModuleEntry32.modBaseSize;
            if( BelongToModule( &l_mInfo, address ) ) {
              l_mInfo.hModule = l_mModuleEntry32.hModule;
              CloseHandle( l_hSnapshot );
              return new TModuleInfo( l_mInfo );
            }
          }
          while( Module32Next( l_hSnapshot, &l_mModuleEntry32 ) );
        }
      }
      CloseHandle( l_hSnapshot );
      return Null;
    }

    static inline TModuleInfo* GetModuleCaller( const uint32& address ) {
      static CArray<TModuleInfo*> l_arrModules;

      for( uint32 i = 0; i < l_arrModules.GetNum(); i++ ) {
        if( BelongToModule( l_arrModules[i], address ) ) {
          return l_arrModules[i];
        }
      }
      TModuleInfo* l_pModuleInfo = GetModuleInfo( address );
      if( l_pModuleInfo ) {
        l_arrModules.Insert( l_pModuleInfo );
      }
      return l_pModuleInfo;
    }
  };

  inline double loga( const double& a, const double& b ) {
    return log( a ) / log( b );
  }

  inline double logceil( const double& a, const double&  b ) {
    return pow( 2.0, ceil( loga( a, b ) ) );
  }

  inline double logfloor( const double& a, const double& b ) {
    return pow( 2.0, floor( loga( a, b ) ) );
  }
}