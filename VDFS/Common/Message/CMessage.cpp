#include <UnionAfx.h>

// sequel .data:008B50C8 ; HINSTANCE hInstApp
namespace Common {
  EXTERN_C {
  int32 CMessageA::Box( HWND hwnd, const CStringA & msg, const CStringA & title, UINT uType ) {
    return MessageBoxA( hwnd, msg, title, uType );
  }

  void CMessageA::Box( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    Box( Null, msg, title, (TopMost ? MB_TOPMOST : 0) );
  }

  void CMessageA::Info( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONINFORMATION | (TopMost ? MB_TOPMOST : 0) );
  }

  void CMessageA::Error( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONHAND | (TopMost ? MB_TOPMOST : 0) );
  }

  void CMessageA::Warning( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONEXCLAMATION | (TopMost ? MB_TOPMOST : 0) );
  }

  bool32 CMessageA::Choice( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    int nResult = Box( Null, msg, title, MB_OKCANCEL | MB_ICONQUESTION | (TopMost ? MB_TOPMOST : 0) );
    switch( nResult ) {
      case IDOK: return TRUE;
      case IDCANCEL: return FALSE;
      default: return -1;
    }
  }

  bool32 CMessageA::Question( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    int nResult = Box( Null, msg, title, MB_YESNO | MB_ICONQUESTION | (TopMost ? MB_TOPMOST : 0) );
    switch( nResult ) {
      case IDYES: return TRUE;
      case IDNO: return FALSE;
      default: return -1;
    }
  }

  bool32 CMessageA::Retry( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    int nResult = Box( Null, msg, title, MB_RETRYCANCEL | MB_ICONEXCLAMATION | (TopMost ? MB_TOPMOST : 0) );
    switch( nResult ) {
      case IDRETRY:  return TRUE;
      case IDCANCEL:  return FALSE;
      default:    return -1;
    }
  }

  void CMessageA::Fatal( const CStringA & msg, const CStringA & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONHAND | (TopMost ? MB_TOPMOST : 0) );
    exit( 0 );
  }

  int32 CMessageW::Box( HWND hwnd, const CStringW & msg, const CStringW & title, UINT uType ) {
    return MessageBoxW( hwnd, msg, title, uType );
  }

  void CMessageW::Box( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    Box( Null, msg, title, (TopMost ? MB_TOPMOST : 0) );
  }

  void CMessageW::Info( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONINFORMATION | (TopMost ? MB_TOPMOST : 0) );
  }

  void CMessageW::Error( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONHAND | (TopMost ? MB_TOPMOST : 0) );
  }

  void CMessageW::Warning( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONEXCLAMATION | (TopMost ? MB_TOPMOST : 0) );
  }

  bool32 CMessageW::Choice( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    int nResult = Box( Null, msg, title, MB_OKCANCEL | MB_ICONQUESTION | (TopMost ? MB_TOPMOST : 0) );
    switch( nResult ) {
      case IDOK: return TRUE;
      case IDCANCEL: return FALSE;
      default: return -1;
    }
  }

  bool32 CMessageW::Question( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    int nResult = Box( Null, msg, title, MB_YESNO | MB_ICONQUESTION | (TopMost ? MB_TOPMOST : 0) );
    switch( nResult ) {
      case IDYES: return TRUE;
      case IDNO: return FALSE;
      default: return -1;
    }
  }

  bool32 CMessageW::Retry( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    int nResult = Box( Null, msg, title, MB_RETRYCANCEL | MB_ICONEXCLAMATION | (TopMost ? MB_TOPMOST : 0) );
    switch( nResult ) {
      case IDRETRY: return TRUE;
      case IDCANCEL: return FALSE;
      default: return -1;
    }
  }

  void CMessageW::Fatal( const CStringW & msg, const CStringW & title, bool32 TopMost ) {
    Box( Null, msg, title, MB_ICONHAND | (TopMost ? MB_TOPMOST : 0) );
    exit( 0 );
  }
  }
}