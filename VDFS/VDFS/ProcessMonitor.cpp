#include <Windows.h>

void ProcessMonitor() {
  while( true ) {
    short lalt = GetAsyncKeyState( VK_MENU );
    short ctrl = GetAsyncKeyState( VK_LCONTROL );
    short keyq = GetAsyncKeyState( 0x51 );

    if( lalt && ctrl && keyq )
      break;

    Sleep( 250 );
  }

  DWORD dwProcessId = GetCurrentProcessId();
  HANDLE hProcess;
  DWORD dwError;

  hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessId );
  if( hProcess == NULL )
  {
    if( GetLastError() != ERROR_ACCESS_DENIED )
      return;

    OSVERSIONINFO osvi;

    osvi.dwOSVersionInfoSize = sizeof( osvi );
    GetVersionEx( &osvi );

    if( osvi.dwPlatformId != VER_PLATFORM_WIN32_NT )
      return;

    TOKEN_PRIVILEGES Priv, PrivOld;
    DWORD cbPriv = sizeof( PrivOld );
    HANDLE hToken;

    if( !OpenThreadToken( GetCurrentThread(),
      TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
      FALSE, &hToken ) )
    {
      if( GetLastError() != ERROR_NO_TOKEN )
        return;

      if( !OpenProcessToken( GetCurrentProcess(),
        TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
        &hToken ) )
        return;
    }

    Priv.PrivilegeCount = 1;
    Priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &Priv.Privileges[0].Luid );

    if( !AdjustTokenPrivileges( hToken, FALSE, &Priv, sizeof( Priv ), &PrivOld, (LPDWORD)&cbPriv ) ) {
      dwError = GetLastError();
      CloseHandle( hToken );
      return;
    }

    if( GetLastError() == ERROR_NOT_ALL_ASSIGNED ) {
      CloseHandle( hToken );
      return;
    }

    hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, dwProcessId );
    dwError = GetLastError();

    AdjustTokenPrivileges( hToken, FALSE, &PrivOld, sizeof( PrivOld ), NULL, NULL );
    CloseHandle( hToken );

    if( hProcess == NULL )
      return;
  }

  if( !TerminateProcess( hProcess, (UINT)-1 ) ) {
    dwError = GetLastError();
    CloseHandle( hProcess );
    return;
  }

  CloseHandle( hProcess );
}