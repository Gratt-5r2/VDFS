#include <UnionAfx.h>

extern "C" {
#ifdef UNION
  HANDLE ASTAPI GetConsoleHandle() {
    return cmd.GetHandle();
  }
#else
  inline void* ImportShw32( const char* symbolName ) {
    HMODULE module = GetModuleHandle( "shw32.dll" );
    void* proc = GetProcAddress( module, symbolName );
    // Message::Warning( string::Hex32( (uint)proc ) );
    return proc;
  }

  HANDLE GetConsoleHandle() {
    static HANDLE( *__imp_GetConsoleHandle )() = (HANDLE(*)())ImportShw32( "GetConsoleHandle" );
    if( __imp_GetConsoleHandle )
      return __imp_GetConsoleHandle();

    return INVALID_HANDLE_VALUE;
  }

  //bool PrintConsole( const wchar_t* str, const uint& length ) {
  //  static HANDLE( *__imp_PrintConsole )() = (HANDLE( * )())ImportShw32( "PrintConsole" );
  //  if( __imp_PrintConsole )
  //    return __imp_PrintConsole;
  //
  //  return INVALID_HANDLE_VALUE;
  //}
#endif
}

namespace Common {

  bool CCmd::DontUpdateWindowPos = false;
  bool CCmd::UsedHostedWindow = false;

  ASTAPI CCmd cmd (False);

#ifndef UNION
  CPID  CCmd::cpDefaultCP = ANSI_CODEPAGE_CYRILLIC;
#endif
  HANDLE CCmd::hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
  uint32 CCmd::ulRef    = 0;

  CCmd::CCmd (bool_t shwCsl) {
    bHasHandle = shwCsl;
    if (bHasHandle)
      if (ulRef++ == 0)
        Show ();
  }

  HWND FindConsoleHostedWindow( const string& defaultName, const string& specificName ) {
    HWND window = FindWindowA( "ConsoleWindowClass", defaultName );
    if( window )
      return window;

    return FindWindowA( "ConsoleWindowClass", specificName );
  }

  bool_t OpenConsole() {
    return AllocConsole();
  }

  void CCmd::Show () {
    if (ulRef++ > 1)
      return;

    if ( OpenConsole()) {
      int  hCrt = _open_osfhandle((long)GetStdHandle(STD_INPUT_HANDLE), _O_TEXT);
      *stdin = *(::_fdopen(hCrt, "r"));
      ::setvbuf(stdin, NULL, _IONBF, 0);
      
      hCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
      *stdout = *(::_fdopen(hCrt, "w"));
      ::setvbuf(stdout, NULL, _IONBF, 0);
      
      hCrt = _open_osfhandle( (long)GetStdHandle( STD_ERROR_HANDLE ), _O_TEXT );
      *stderr = *(::_fdopen( hCrt, "r" ));
      ::setvbuf( stderr, NULL, _IONBF, 0 );

      //hConsole = CreateFileW( L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 0, 0, NULL );

      // FILE* fileHandle;
      // freopen_s( &fileHandle, "CONIN$", "r",  stdin  );
      // freopen_s( &fileHandle, "CONOUT$", "w", stderr );
      // freopen_s( &fileHandle, "CONOUT$", "w", stdout );
    }
    // hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

    wchar_t buff [260];
    GetModuleFileNameW (NULL, buff, 260);
    SetConsoleTitleW (buff);
    Clear();
  }

  void CCmd::Hide () {
    if( !UsedHostedWindow )
      FreeConsole();
  }

  void CCmd::Release () {
    if (ulRef > 0)
      if (--ulRef == 0)
        Hide ();
  }

  void CCmd::Clear () {
    system ("cls");
  }

  HANDLE CCmd::GetHandle () {
    return GetStdHandle( STD_OUTPUT_HANDLE );
  }

  CCmd& CCmd::Write( const wchar_t* str, const uint& length ) {
    DWORD dw;
    WriteConsoleW( CCmd::GetHandle(), str, length, &dw, Null );
    return *this;
  }

  CCmd& CCmd::Print (const CStringA& str, const CPID& cp) {
    return Print (str.AToW(cp));
  }

  CCmd& CCmd::Print (const CStringW& str) {
    Write( str.GetVector(), str.Length() );
    return *this;
  }

  CCmd& CCmd::Print (const text str, const CPID& cp) {
    return Print (CStringA(str).AToW(cp));
  }

  CCmd& CCmd::Print (const wtext str) {
    Write( str, lstrlenW( str ) );
    return *this;
  }

  CCmd& CCmd::operator << (const CStringA& str) {
    return Print (str);
  }

  CCmd& CCmd::operator << (const CStringW& str) {
    return Print (str);
  }

  CCmd& CCmd::operator << (const text str) {
    return Print (str);
  }

  CCmd& CCmd::operator << (const wtext str) {
    return Print (str);
  }

  CCmd& CCmd::operator << (const int32& val) {
    return Print (W val);
  }

  CCmd& CCmd::operator << (const uint32& val) {
    return Print (W (uint32)val);
  }

  CCmd& CCmd::operator << (const long32& val){
    return Print (W val);
  }

  CCmd& CCmd::operator << (const ulong32& val){
    return Print (W val);
  }

  CCmd& CCmd::operator << (const int64& val){
    return Print (W val);
  }

  CCmd& CCmd::operator << (const uint64& val){
    return Print (W val);
  }

  CCmd& CCmd::operator << (const real32& val){
    return Print (W val);
  }

  CCmd& CCmd::operator << (const real64& val){
    return Print (W val);
  }

  CCmd& CCmd::operator << (const Col16& val){
    SetConsoleTextAttribute (CCmd::GetHandle(), val.front | val.back * 16);
    return *this;
  }

  CCmd& CCmd::operator << ( const ECmdColorID& col ) {
    if( col == CMD_RESET )
      return CCmd::operator <<( Col16() );

    return CCmd::operator <<( Col16( col ) );
  }

  CCmd& CCmd::operator << (iofunc fnc){
    fnc (*this);
    return *this;
  }

  CCmd::~CCmd () {
    if (bHasHandle && ulRef > 0)
      if (--ulRef == 0)
        Hide ();
  }

  ThreadLocker logMutex;

  inline const wchar_t* GetLogLineBreakW() {
    static const wchar_t crlf[] = { (wchar_t)0x000D, (wchar_t)0x000A, (wchar_t)0x0000 };
    return crlf;
  }

  void CLogger::PrintTime() {
    time_t rawtime;
    time( &rawtime );
    tm* timeinfo = localtime( &rawtime );
    int h = timeinfo->tm_hour;
    int m = timeinfo->tm_min;
    int s = timeinfo->tm_sec;
    CStringW format = CStringW::Combine( L"%i:%i:%i   ", h, m, s );
    fwrite( format, 2, format.Length(), file );
  }

  CLogger::CLogger( string name ) {
    fileName = name;
    nextLine = true;

    time_t rawtime;
    time( &rawtime );
    tm* timeinfo = localtime( &rawtime );
    int y = timeinfo->tm_year + 1900;
    int m = timeinfo->tm_mon + 1;
    int d = timeinfo->tm_mday;
    CStringW format = CStringW::Combine( L"%c============== Log begin %i.%i.%i ==============\n", (wchar_t)0xFEFF, d, m, y );
    {
      file = fopen( fileName, "wb" );
      fwrite( format, 2, format.Length(), file );
      fflush( file );
    }
  }

  CCmd& CLogger::Write( const wchar_t* str, const uint& length ) {
    logMutex.Enter();
    if( !file ) {
      // cmd << Col16( CMD_RED | CMD_INT ) << "Can not open log file '" << fileName << "'" << endl;
      // cmd << str << Col16() << endl;
      logMutex.Leave();
      return *this;
    }

    if( str == L"\r\n" ) {
      nextLine = true;
      fwrite( GetLogLineBreakW(), 2, 2, file );
      logMutex.Leave();
      return *this;
    }

    if( nextLine ) {
      PrintTime();
      nextLine = false;
    }

    fwrite( str, 2, length, file );
    fflush( file );
    logMutex.Leave();
    return *this;
  }

  CLogger::~CLogger() {
    if( file ) {
      CStringW endLog = L"\r\n============== Log end ==============";
      fwrite( endLog, 2, endLog.Length(), file );
      fclose( file );
    }
  }

  void __stdcall endl( CCmd& console ) {
    console.Print( L"\r\n" );
  }

  void __stdcall tab( CCmd& console ) {
    console.Print( L"\t" );
  }
}