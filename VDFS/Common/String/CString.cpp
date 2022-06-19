#define CORE_SECURE_DISABLE
#include <UnionAfx.h>
#include <regex>

#ifdef DONT_USE_VIRTUAL_SYSTEM
#define ASSERT(a)
#define ASSERT_ERROR(a)
#endif

namespace Common {

  ASTAPI uint ANSI_CODEPAGE_DEFAULT = ANSI_CODEPAGE_CYRILLIC;

  enum {
    BOM_None,
    BOM_LE,
    BOM_UTF8
  };

  inline float resf( float f1, float f2 ) {
    return powf( 2.0f, ceilf( logf( f1 ) / logf( f2 ) ) );
  }

  inline float res10f( float f1 ) {
    static float f2 = log( 10.0f );
    return powf( 2.0f, ceilf( logf( f1 ) / f2 ) );
  }

  inline float res2f( float f1 ) {
    static float f2 = log( 2.0f );
    return powf( 2.0f, ceilf( logf( f1 ) / f2 ) );
  }

  inline uint32 clamp( uint32 src, uint32 min, uint32 max ) {
    return
      src > max ? max :
      src < min ? min :
      src;
  }



  template <typename T>
  inline void str_malloc( void32& ptr, size_t sz ) {
    ptr = (T*)::/* smart heap !!! */ shi_malloc( ++sz * sizeof( T ) );
  }

  template <typename T>
  inline void str_realloc( void32& ptr, size_t sz ) {
    ptr = (T*)::/* smart heap !!! */ shi_realloc( ptr, ++sz * sizeof( T ) );
  }

  template <typename T>
  inline void str_copy( void32 ptr, const void32 src, size_t sz ) {
    ::memcpy( ptr, src, sz * sizeof( T ) );
  }

  template <typename T>
  inline void str_free( void32& ptr ) {
    ::/* smart heap !!! */ shi_free( ptr );
  }

  template <typename T>
  inline void str_move( void32& ptr, size_t to, size_t from, size_t len ) {
    memmove( &((T*)ptr)[to], &((T*)ptr)[from], len * sizeof( T ) );
  }


#pragma region CStringBaseDefinition

  CStringBase::CStringBase() {
    ulRange = UINT_MAX;
    lpPtr = Null;
    ulLength = 0;
    ulReserved = 0;
  }

  const CStringA& CStringBase::ReinterpretToA() const {
    return *reinterpret_cast<const CStringA*>(this);
  }

  CStringA& CStringBase::ReinterpretToA() {
    return *reinterpret_cast<CStringA*>(this);
  }

  const CStringW& CStringBase::ReinterpretToW() const {
    return *reinterpret_cast<const CStringW*>(this);
  }

  CStringW& CStringBase::ReinterpretToW() {
    return *reinterpret_cast<CStringW*>(this);
  }

  CStringBase::~CStringBase() {
    /* smart heap !!! */ shi_free( lpPtr );
  }

#pragma endregion CStringBaseDefinition



#pragma region CStringADefinition

#pragma warning (disable: 4244)
  void CStringA::Allocate() {
    ulLength = clamp( ulLength, 0, ulRange );
    if( ulLength + 1 > ulReserved ) {
      ulReserved = clamp( res2f( ulLength + 1 ), 16, ulRange );
      str_realloc<char>( lpPtr, ulReserved );
    }
    ((char*)lpPtr)[ulLength] = 0;
  }

  void CStringA::Free() {
    ulLength = clamp( ulLength, 0, ulRange );
    if( ulLength + 1 < ulReserved / 2 ) {
      ulReserved = clamp( res2f( ulLength + 1 ), 16, ulRange );
      str_realloc<char>( lpPtr, ulReserved );
    }
    ((char*)lpPtr)[ulLength] = 0;
  }
#pragma warning (default: 4244)

  CStringA::CStringA() {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = 0;
    Allocate();
  }

  CStringA::CStringA( const CStringA& ptr ) {
    ulRange = ptr.ulRange;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = ptr.ulLength;
    Allocate();
    str_copy<char>( lpPtr, ptr.lpPtr, ulLength );
  }

  CStringA::CStringA( const char& ptr ) {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = 1;
    Allocate();
    ((char*)lpPtr)[0] = ptr;
  }

  CStringA::CStringA( const char* ptr ) {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( ptr );
    Allocate();
    str_copy<char>( lpPtr, (void32)ptr, ulLength );
  }

  CStringA::CStringA( const char* ptr, const uint32& len ) {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = len;
    Allocate();
    str_copy<char>( lpPtr, (void32)ptr, ulLength );
  }

  CStringA::CStringA( const byte& ptr ) {
    char buff[9];
    _itoa_s( ptr, buff, 9, 16 );
    uint32 ulBufLen = lstrlenA( buff );
    for( uint32 i = 0; buff[i]; i++ )
      buff[i] = toupper( buff[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = 4;
    Allocate();

    str_copy<char>( lpPtr, "0x00", ulLength );
    str_copy<char>( (char*)lpPtr + (ulLength - ulBufLen), buff, ulBufLen );
  }

  CStringA::CStringA( const int32& ptr, const uint32& radix ) {
    char buf[33];
    _itoa_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = toupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();

    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const uint32& ptr, const uint32& radix ) {
    char buf[33];
    _ultoa_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = toupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();

    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const long32& ptr, const uint32& radix ) {
    char buf[33];
    _itoa_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = toupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();

    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const ulong32& ptr, const uint32& radix ) {
    char buf[33];
    _ultoa_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = toupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();

    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const int64& ptr, const uint32& radix ) {
    char buf[65];
    _i64toa_s( ptr, buf, 65, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = toupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();

    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const uint64& ptr, const uint32& radix ) {
    char buf[65];
    _ui64toa_s( ptr, buf, 65, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = toupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();

    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const real32& ptr, const uint32& dig ) {
    char buf[40];
    {
      char buff[8];
      sprintf( buff, "%%.%if", dig );
      sprintf( buf, buff, ptr );
    }
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();
    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA::CStringA( const real64& ptr, const uint32& dig ) {
    char buf[80];
    {
      char buff[8];
      sprintf( buff, "%%.%if", dig );
      sprintf( buf, buff, ptr );
    }
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenA( buf );
    Allocate();
    str_copy<char>( lpPtr, buf, ulLength );
  }

  CStringA CStringA::operator + ( const CStringA& ptr ) const {
    return CStringA( *this ) += ptr;
  }

  CStringA CStringA::operator + ( const char* ptr ) const {
    return CStringA( *this ) += ptr;
  }

  CStringA& CStringA::operator += ( const CStringA& ptr ) {
    uint32 ulStartCpy = ulLength;
    ulLength += ptr.ulLength;
    Allocate();
    str_copy<char>( (char*)lpPtr + ulStartCpy, ptr.lpPtr, ptr.ulLength );
    return *this;
  }

  CStringA& CStringA::operator += ( const char* ptr ) {
    uint32 ulPtrlen = lstrlenA( ptr );
    uint32 ulStartCpy = ulLength;
    ulLength += ulPtrlen;
    Allocate();
    str_copy<char>( (char*)lpPtr + ulStartCpy, (void32)ptr, ulPtrlen );
    return *this;
  }

  CStringA& CStringA::operator = ( const CStringA& ptr ) {
    ulRange = ptr.ulRange;
    ulReserved = 0;
    ulLength = ptr.ulLength;
    Allocate();
    str_copy<char>( lpPtr, ptr.lpPtr, ulLength );
    return *this;
  }

  CStringA& CStringA::operator = ( const char* ptr ) {
    ulReserved = 0;
    ulLength = lstrlenA( ptr );
    Allocate();
    str_copy<char>( lpPtr, (void32)ptr, ulLength );
    return *this;
  }

  bool32 CStringA::operator == ( const CStringA& ptr ) const {
    return CompareI( ptr );
  }

  bool32 CStringA::operator == ( const char* ptr ) const {
    return CompareI( ptr );
  }

  bool32 CStringA::operator != ( const CStringA& ptr ) const {
    return !CompareI( ptr );
  }

  bool32 CStringA::operator != ( const char* ptr ) const {
    return !CompareI( ptr );
  }

  bool CStringA::operator > ( const CStringA& other ) const {
    uint llength = Length();
    uint rlength = other.Length();
    uint length = min( llength, rlength );

    for( uint i = 0; i < length; i++ ) {
      byte lchar = toupper( operator[]( i ) );
      byte rchar = toupper( other[i] );

      if( lchar != rchar )
        return lchar > rchar;
    }

    return llength > rlength;
  }

  bool CStringA::operator < ( const CStringA& other ) const {
    uint llength = Length();
    uint rlength = other.Length();
    uint length = min( llength, rlength );

    for( uint i = 0; i < length; i++ ) {
      byte lchar = toupper( operator[]( i ) );
      byte rchar = toupper( other[i] );

      if( lchar != rchar )
        return lchar < rchar;
    }

    return llength < rlength;
  }

  char& CStringA::First() {
    return ((char*)lpPtr)[0];
  }

  const char& CStringA::First() const {
    return ((char*)lpPtr)[0];
  }

  char& CStringA::Last() {
    return ((char*)lpPtr)[ulLength - 1];
  }

  const char& CStringA::Last() const {
    return ((char*)lpPtr)[ulLength - 1];
  }

  char& CStringA::operator [] ( const int& idx ) {
    return ((char*)lpPtr)[(uint32)idx];
  }

  const char& CStringA::operator [] ( const int& idx ) const {
    return ((char*)lpPtr)[(uint32)idx];
  }

  CStringA::operator const char* () const {
    return (char*)lpPtr;
  }

  uint32 CStringA::ToInt32( const uint32& radix ) const {
    return strtoul( (char*)lpPtr, NULL, radix );
  }

  uint64 CStringA::ToInt64( const uint32& radix ) const {
    return _strtoui64( (char*)lpPtr, NULL, radix );
  }

  real32 CStringA::ToReal32() const {
    return (float)atof( (char*)lpPtr );
  }

  real64 CStringA::ToReal64() const {
    return atof( (char*)lpPtr );
  }

  bool32 CStringA::IsNumber() const {
    return MatchesMask( "0123456789.,-" );
  }

  bool32 CStringA::IsHexNumber() const {
    if( !MatchesMask( "xX1234567890abcdefABCDEFhH" ) )
      return false;

    if( StartWith( "0x" ) || StartWith( "0X" ) )
      return true;

    if( EndWith( "h" ) || EndWith( "H" ) )
      return true;

    return false;
  }

  const char* CStringA::GetVector() const {
    return (char*)lpPtr;
  }

  uint32 CStringA::Length() const {
    return ulLength;
  }

  uint32 CStringA::Allocated() const {
    return ulReserved;
  }

  bool32 CStringA::IsEmpty() const {
    return ulLength == 0;
  }

#include <TCHAR.H>

  bool32 CStringA::Compare( const CStringA& ptr ) const {
    if( !ulLength && !ptr.ulLength )
      return True;
    if( ulLength != ptr.ulLength )
      return FALSE;
    return _tcscmp( (char*)lpPtr, (char*)ptr.lpPtr ) == 0;
  }

  bool32 CStringA::Compare( const char* ptr ) const {
    if( !ulLength && ptr[0] == 0 )
      return True;
    if( !ulLength || ptr == Null )
      return False;
    return _tcscmp( (char*)lpPtr, ptr ) == 0;
  }

  bool32 CStringA::CompareI( const CStringA& ptr ) const {
    if( !ulLength && !ptr.ulLength )
      return True;
    if( !ulLength || ulLength != ptr.ulLength )
      return FALSE;
    return _tcsicmp( (char*)lpPtr, (char*)ptr.lpPtr ) == 0;
  }

  bool32 CStringA::CompareI( const char* ptr ) const {
    if( !ulLength && ptr[0] == 0 )
      return True;
    if( !ulLength || ptr == Null )
      return False;
    return _tcsicmp( (char*)lpPtr, ptr ) == 0;
  }

  bool32 CStringA::CompareMasked( const CStringA& ptr ) const {
    Array<CStringA> tokens = ptr.Split( "*" );

    uint index = 0;
    for( uint i = 0; i < tokens.GetNum(); i++ ) {
      if( tokens[i].IsEmpty() )
        continue;

      index = Search( tokens[i], index, True );
      if( index == Invalid )
        return false;
    }

    return true;
  }

  bool32 CStringA::CompareMaskedI( const CStringA& ptr ) const {
    Array<CStringA> tokens = ptr.Split( "*" );

    uint index = 0;
    for( uint i = 0; i < tokens.GetNum(); i++ ) {
      if( tokens[i].IsEmpty() )
        continue;

      index = SearchI( tokens[i], index, True );
      if( index == Invalid )
        return false;
    }

    return true;
  }

  uint32 CStringA::Search( const CStringA& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        if( ((char*)lpPtr)[i + j] != ((char*)ptr.lpPtr)[j] )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return i + ptr.ulLength;
          return i;
        }
      }
    }
    return NotFound;
  }

  uint32 CStringA::SearchI( const CStringA& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        if( toupper( ((char*)lpPtr)[i + j] ) != toupper( ((char*)ptr.lpPtr)[j] ) )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return i + ptr.ulLength;
          return i;
        }
      }
    }
    return NotFound;
  }

  uint32 CStringA::SearchReverse( const CStringA& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    uint32 submode1 = ulLength - 1;
    uint32 submode2 = ptr.ulLength - 1;
    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        char left = ((char*)lpPtr)[submode1 - i - j];
        char right = ((char*)ptr.lpPtr)[submode2 - j];
        if( left != right )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return ulLength - i;
          return (submode1 - i) - submode2;
        }
      }
    }
    return NotFound;
  }

  uint32 CStringA::SearchReverseI( const CStringA& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    uint32 submode1 = ulLength - 1;
    uint32 submode2 = ptr.ulLength - 1;
    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        char left = toupper( ((char*)lpPtr)[submode1 - i - j] );
        char right = toupper( ((char*)ptr.lpPtr)[submode2 - j] );
        if( left != right )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return ulLength - i;
          return (submode1 - i) - submode2;
        }
      }
    }
    return NotFound;
  }

  bool32 CStringA::HasWord( const CStringA& ptr ) const {
    uint32 count = 0;
    uint32 pos = 0;
    while( (pos = Search( ptr, pos, True )) != NotFound ) {
      count++;
    }
    return count;
  }

  bool32 CStringA::HasWordI( const CStringA& ptr ) const {
    uint32 count = 0;
    uint32 pos = 0;
    while( (pos = SearchI( ptr, pos, True )) != NotFound ) {
      count++;
    }
    return count;
  }

  static bool32 BelongToMaskA( const CStringA& str, const char& c ) {
    for( uint32 i = 0; i < str.Length(); i++ )
      if( c == str[i] )
        return TRUE;
    return FALSE;
  }

  bool32 CStringA::MatchesMask( const CStringA& mask ) const {
    for( uint32 i = 0; i < Length(); i++ ) {
      for( uint32 j = 0; j < mask.Length(); j++ ) {
        if( (*this)[i] == mask[j] )
          goto con;
      }
      return False;
    con:
      continue;
    }
    return True;
  }

  bool32 CStringA::StartWith( const CStringA& cmp ) const {
    if( cmp.ulLength > ulLength )
      return False;

    for( uint i = 0; i < cmp.ulLength; i++ )
      if( ((char*)lpPtr)[i] != cmp[i] )
        return False;

    return True;
  }

  bool32 CStringA::EndWith( const CStringA& cmp ) const {
    if( cmp.ulLength > ulLength )
      return False;

    for( uint i = 0; i < cmp.ulLength; i++ )
      if( ((char*)lpPtr)[ulLength - i - 1] != cmp[cmp.ulLength - i - 1] )
        return False;

    return True;
  }

  CStringA& CStringA::PadLeft( const uint& length, const char& sym ) {
    if( Length() >= length )
      return *this;

    uint count = length - Length();
    char* buffer = new char[count + 1];
    memset( buffer, sym, count );
    buffer[count] = 0;

    Put( buffer, 0 );

    delete buffer;
    return *this;
  }

  CStringA& CStringA::PadRight( const uint& length, const char& sym ) {
    if( Length() >= length )
      return *this;

    uint count = length - Length();
    char* buffer = new char[count + 1];
    memset( buffer, sym, count );
    buffer[count] = 0;

    Put( buffer, Length() );

    delete buffer;
    return *this;
  }

  CStringA CStringA::GetWord( const CStringA& mask, const int32& word_index ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint32 index = 0;
    uint32 left = 0;
    uint32 right = 0;
    int32  num = 0;
    bool32 found = FALSE;

    if( word_index > 0 ) {
      for( ;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskA( mask, ((char*)lpPtr)[index] ) )
            continue;
          left = index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskA( mask, ((char*)lpPtr)[index] ) )
            continue;
        right = index;
        found = FALSE;
        num++;

        if( num >= word_index )
          return Copy( left, right - left );
        if( index >= ulLength )
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 submode = ulLength - 1;

      for( ;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskA( mask, ((char*)lpPtr)[submode - index] ) )
            continue;
          left = ulLength - index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskA( mask, ((char*)lpPtr)[submode - index] ) )
            continue;
        right = ulLength - index;
        found = FALSE;
        num--;

        if( num <= word_index )
          return Copy( right, left - right );
        if( index >= ulLength )
          break;
      }
    }
    return NullStrA;
  }

  CStringA CStringA::GetPattern( const CStringA& lw, const CStringA& rw, const int32& word_index ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero pattern from string. Indexing start with First (1) or First less than zero (-1)" );

    int32 num = 0;

    if( word_index > 0 ) {
      uint32 left = rw.IsEmpty() ? -1 : 0;
      uint32 right = lw.IsEmpty() ? -1 : 0;

      while( (left = (lw.IsEmpty() ? 0 :
        rw.IsEmpty() ? Search( lw, left + 1, TRUE ) :
        Search( lw, right, TRUE ))) != NotFound ) {

        if( (right = (rw.IsEmpty() ? ulLength :
          lw.IsEmpty() ? Search( rw, right + 1, FALSE ) :
          Search( rw, left, FALSE ))) != NotFound ) {

          if( ++num >= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 left = rw.IsEmpty() ? ulLength + 1 : ulLength;
      uint32 right = lw.IsEmpty() ? ulLength + 1 : ulLength;

      while( (right = (rw.IsEmpty() ? ulLength :
        lw.IsEmpty() ? SearchReverse( rw, (ulLength - right + 1), FALSE ) :
        SearchReverse( rw, (ulLength - left), FALSE ))) != NotFound ) {

        if( (left = (lw.IsEmpty() ? 0 :
          rw.IsEmpty() ? SearchReverse( lw, (ulLength - left + 1), TRUE ) :
          SearchReverse( lw, (ulLength - right), TRUE ))) != NotFound ) {

          if( --num <= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }

    return NullStrA;
  }

  CStringA CStringA::GetWordEx( const CStringA& mask, const int32& word_index, const uint32& offset, uint32* rtn ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint32 index = 0;
    uint32 left = 0;
    uint32 right = 0;
    int32  num = 0;
    bool32 found = FALSE;

    if( word_index > 0 ) {
      for( index = offset;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskA( mask, ((char*)lpPtr)[index] ) )
            continue;
          left = index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskA( mask, ((char*)lpPtr)[index] ) )
            continue;
        right = index;
        found = FALSE;
        num++;

        if( num >= word_index ) {
          if( rtn )
            *rtn = left;
          return Copy( left, right - left );
        }
        if( index >= ulLength )
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 submode = ulLength - 1;

      for( index = ulLength - offset;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskA( mask, ((char*)lpPtr)[submode - index] ) )
            continue;
          left = ulLength - index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskA( mask, ((char*)lpPtr)[submode - index] ) )
            continue;
        right = ulLength - index;
        found = FALSE;
        num--;

        if( num <= word_index ) {
          if( rtn )
            *rtn = left;
          return Copy( right, left - right );
        }
        if( index >= ulLength )
          break;
      }
    }
    if( rtn )
      *rtn = Invalid;
    return NullStrA;
  }

  CStringA CStringA::GetPatternEx( const CStringA& lw, const CStringA& rw, const int32& word_index, const uint32& offset, uint32* rtn ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero pattern from string. Indexing start with First (1) or First less than zero (-1)" );

    int32 num = 0;

    if( word_index > 0 ) {
      uint32 left = rw.IsEmpty() ? offset - 1 : offset;
      uint32 right = lw.IsEmpty() ? offset - 1 : offset;

      while( (left = (lw.IsEmpty() ? 0 :
        rw.IsEmpty() ? Search( lw, left + 1, TRUE ) :
        Search( lw, right, TRUE ))) != NotFound ) {

        if( (right = (rw.IsEmpty() ? ulLength :
          lw.IsEmpty() ? Search( rw, right + 1, FALSE ) :
          Search( rw, left, FALSE ))) != NotFound ) {

          if( ++num >= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            if( rtn )
              *rtn = scpy;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 left = rw.IsEmpty() ? ulLength - offset + 1 : ulLength - offset;
      uint32 right = lw.IsEmpty() ? ulLength - offset + 1 : ulLength - offset;

      while( (right = (rw.IsEmpty() ? ulLength :
        lw.IsEmpty() ? SearchReverse( rw, (ulLength - right + 1), FALSE ) :
        SearchReverse( rw, (ulLength - left), FALSE ))) != NotFound ) {

        if( (left = (lw.IsEmpty() ? 0 :
          rw.IsEmpty() ? SearchReverse( lw, (ulLength - left + 1), TRUE ) :
          SearchReverse( lw, (ulLength - right), TRUE ))) != NotFound ) {

          if( --num <= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            if( rtn )
              *rtn = scpy;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }
    if( rtn )
      *rtn = Invalid;
    return NullStrA;
  }

  Array<CStringA> CStringA::Split( const CStringA& separator ) const {
    Array<CStringA> words;
    if( IsEmpty() )
      return words;

    uint begin = 0;
    uint end = 0;
    while( begin < Length() ) {
      end = Search( separator, begin );
      if( end == Invalid ) {
        words += &GetVector()[begin];
        break;
      }

      words += CStringA( &GetVector()[begin], end - begin );
      end += separator.Length();
      begin = end;
    }

    return words;
  }

  static int GetSymbolGroupA( const char& c ) {
    if( BelongToMaskA( " \t\n\r", c ) )
      return 0;

    if( c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') )
      return 1;

    if( BelongToMaskA( "+-*/|^%@!=&.,", c ) )
      return 2;

    if( BelongToMaskA( "()[]{}'\"", c ) )
      return 3;

    return 4;
  }

  CStringA CStringA::GetWordSmart( const int& word_index, const bool& groupSymbols ) const {
    if( word_index == 0 )
      throw std::runtime_error( "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint   index = 0;
    uint   left = 0;
    uint   right = 0;
    int    num = 0;
    bool32 found = False;

    // 0 - space
    // 1 - letters
    // 2 - operators
    // 3 - brackets
    // 4 - same spaces ???
    int oldGroup = Invalid;
    int newGroup = Invalid;

    if( word_index > 0 ) {
      for( ;; index++ ) {
        char sym = ((char*)lpPtr)[index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupA( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = index;
          found = False;
          num++;
          index--;
        }

        if( num >= word_index )
          return Copy( left, right - left );

        if( index >= ulLength )
          break;
      }
    }
    else if( word_index < 0 ) {
      uint reverser = ulLength - 1;

      for( ;; index++ ) {
        char sym = ((char*)lpPtr)[reverser - index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupA( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = ulLength - index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = ulLength - index;
          found = False;
          num--;
          index--;
        }

        if( num <= word_index )
          return Copy( right, left - right );

        if( index >= ulLength )
          break;
      }
    }

    return NullStrA;
  }

  CStringA CStringA::GetWordSmartEx( const int& word_index, const bool& groupSymbols, const uint& offset, uint* word_offset ) const {
    if( word_index == 0 )
      throw std::runtime_error( "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint   index = offset;
    uint   left = 0;
    uint   right = 0;
    int    num = 0;
    bool32 found = False;

    // 0 - space
    // 1 - letters
    // 2 - operators
    // 3 - brackets
    // 4 - same spaces ???
    int oldGroup = Invalid;
    int newGroup = Invalid;

    if( word_index > 0 ) {
      for( ;; index++ ) {
        char sym = ((char*)lpPtr)[index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupA( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = index;
          found = False;
          num++;
          index--;
        }

        if( num >= word_index ) {
          if( word_offset )
            *word_offset = left;

          return Copy( left, right - left );
        }

        if( index >= ulLength )
          break;
      }
    }
    else if( word_index < 0 ) {
      uint reverser = ulLength - 1;

      for( ;; index++ ) {
        char sym = ((char*)lpPtr)[reverser - index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupA( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = ulLength - index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = ulLength - index;
          found = False;
          num--;
          index--;
        }

        if( num <= word_index )
          return Copy( right, left - right );

        if( index >= ulLength )
          break;
      }
    }

    return NullStrA;
  }

  inline bool32 BelongToMaskA_Line( char& sym, const CStringA& line ) {
    for( uint32 i = 0; i < line.Length(); i++ ) {
      if( line[i] == sym )
        return True;
    }
    return False;
  }

  inline uint32 BelongToMaskA_Array( char& sym, char* pprev, const CStringA mask[], const uint32& cnt, const uint32& src = Invalid ) {
    if( pprev ) {
      uint32 idx = 0;
      uint32 lim = cnt;
      if( src != Invalid ) {
        idx += src;
        lim += src;
      }

      char prev = *pprev;
      for( ; idx < lim; idx++ ) {
        uint32 _idx = idx % cnt;
        const CStringA& line = mask[_idx];
        if( BelongToMaskA_Line( sym, line ) && BelongToMaskA_Line( prev, line ) )
          return src;
      }
    }

    {
      uint32 idx = 0;
      uint32 lim = cnt;
      if( src != Invalid ) {
        idx += src;
        lim += src;
      }

      for( ; idx < lim; idx++ ) {
        uint32 _idx = idx % cnt;
        for( uint32 j = 0; j < mask[_idx].Length(); j++ ) {
          if( sym == mask[_idx][j] )
            return _idx;
        }
      }
    }
    return Invalid;
  }

  inline uint32 BelongToBracketsA_Array( char& sym, const CStringA* brackets[], const uint32& brack_num ) {
    for( uint32 i = 0; i < brack_num; i++ ) {
      char* buf = &sym;
      const CStringA& sLeftBrack = brackets[i][0];
      CStringA sWord( buf, sLeftBrack.Length() );

      if( sWord.Compare( sLeftBrack ) ) {
        uint32 ulBrackCount = 1;
        const CStringA& sRightBrack = brackets[i][1];

        for( uint32 j = sLeftBrack.Length(); buf[j]; j++ ) {
          if( sRightBrack.Compare( CStringA( &buf[j], sRightBrack.Length() ) ) ) {
            if( buf[j - 1] != '\\' || (j > 1 ? (buf[j - 1] == '\\' && buf[j - 2] == '\\') : False) ) {
              if( --ulBrackCount == 0 ) {
                uint32 ulLength = &buf[j] + sRightBrack.Length() - &buf[0];
                return ulLength;
              };
              j += sRightBrack.Length() - 1;
              continue;
            }
          }
          else if( sLeftBrack.Compare( CStringA( &buf[j], sLeftBrack.Length() ) ) ) {
            if( buf[j - 1] != '\\' || (j > 1 ? (buf[j - 1] == '\\' && buf[j - 2] == '\\') : False) ) {
              ++ulBrackCount;
              j += sLeftBrack.Length() - 1;
              continue;
            }
          }
        }
        Say::Fatal( "F: Непредвиденный конец блока!" );
      }
    }
    return 0;
  }

  CStringA CStringA::GetSymbol( const CStringA* separators, const uint32& sep_num, const uint32& word_index, uint32* inline_index ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero symbol from string. Indexing start with First (1)" );

    uint32 Index = 0;
    uint32 Num = 0;

    uint32 Left = 0;
    uint32 Right = 0;

    uint32 ulOldMask = Invalid;
    uint32 ulMask = Invalid;

    for( uint32 Index = (inline_index ? *inline_index : 0); Index <= ulLength; Index++, ulOldMask = ulMask ) {
      char& sym = ((char*)lpPtr)[Index];
      char* prev = Index ? &((char*)lpPtr)[Index - 1] : Null;
      ulMask = BelongToMaskA_Array( sym, prev, separators, sep_num, ulOldMask );

      if( ulOldMask != ulMask ) {
        if( ulOldMask != Invalid )
          if( ++Num >= word_index ) {
            if( inline_index )
              *inline_index = Left;
            return Copy( Left, Right - Left );
          }
        Left = Index;
      }
      Right = Index + 1;
    }

    if( inline_index )
      *inline_index = Invalid;
    return NullStrA;
  }

  inline uint32 IsEmptyCloseBracketA( char& sym, const CStringA* brackets[], const uint32& brack_num ) {
    for( uint32 i = 0; i < brack_num; i++ ) {

      const CStringA& sRightBrack = brackets[i][1];
      char* buf = &sym;

      if( sRightBrack.Compare( CStringA( &buf[0], sRightBrack.Length() ) ) )
        return sRightBrack.Length();
    }
    return 0;
  }

  CStringA CStringA::GetSymbolEx( const CStringA separators[], const uint32& sep_num, const CStringA* brackets[], const uint32& br_num, const uint32& word_index, uint32* inline_index ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero symbol from string. Indexing start with First (1)" );

    uint32 Index = 0;
    uint32 Num = 0;

    uint32 Left = 0;
    uint32 Right = 0;

    uint32 ulOldMask = Invalid;
    uint32 ulMask = Invalid;

    for( uint32 Index = (inline_index ? *inline_index : 0); Index <= ulLength; Index++, ulOldMask = ulMask ) {
      char& sym = ((char*)lpPtr)[Index];
      uint32 ulBrackLength = BelongToBracketsA_Array( sym, brackets, br_num );
      if( ulBrackLength ) {
        if( ulMask != Invalid ) {
          if( ++Num >= word_index ) {
            if( inline_index )
              *inline_index = Left;
            return Copy( Left, Right - Left );
          }
        }

        ulOldMask = Invalid;
        ulMask = Invalid;
        Left = Index;

        if( ++Num >= word_index ) {
          if( inline_index )
            *inline_index = Left;
          return Copy( Left, ulBrackLength );
        }
        Index += ulBrackLength - 1;
      }
      else if( uint32 length = IsEmptyCloseBracketA( sym, brackets, br_num ) ) {
        if( ulMask != Invalid ) {
          if( ++Num >= word_index ) {
            if( inline_index )
              *inline_index = Left;
            return Copy( Left, Right - Left );
          }
        }

        ulOldMask = Invalid;
        ulMask = Invalid;
        Left = Index;

        if( ++Num >= word_index ) {
          if( inline_index )
            *inline_index = Left;
          return Copy( Left, length );
        }
        Index += length - 1;
      }
      else {
        char* prev = Index ? &((char*)lpPtr)[Index - 1] : Null;
        ulMask = BelongToMaskA_Array( sym, prev, separators, sep_num, ulOldMask );

        if( ulOldMask != ulMask ) {
          if( ulOldMask != Invalid )
            if( ++Num >= word_index ) {
              if( inline_index )
                *inline_index = Left;
              return Copy( Left, Right - Left );
            }
          Left = Index;
        }
      }
      Right = Index + 1;
    }

    if( inline_index )
      *inline_index = Invalid;
    return NullStrA;
  }

  CStringA& CStringA::Reverse() {
    uint32 uliterations = ulLength / 2;
    for( uint32 i = 0; i < uliterations; i++ ) {
      char tmp = ((char*)lpPtr)[i];
      ((char*)lpPtr)[i] = ((char*)lpPtr)[ulLength - 1 - i];
      ((char*)lpPtr)[ulLength - 1 - i] = tmp;
    }
    return *this;
  }

  void CStringA::Clear() {
    ulLength = 0;
    Free();
  }

  CStringA CStringA::Copy( const uint32& pos, const uint32& len ) const {
    return CStringA( &((char*)lpPtr)[pos], len );
  }

  CStringA CStringA::Copy() const {
    return CStringA( *this );
  }

  CStringA& CStringA::Cut( const uint32& pos, const uint32& len ) {
    str_move<char>( lpPtr, pos, pos + len, ulLength - (pos + len) );
    ulLength -= len;
    Free();
    return *this;
  }

  CStringA& CStringA::Cut( const uint32& len ) {
    ulLength -= len;
    Free();
    return *this;
  }

  CStringA& CStringA::Put( const CStringA& ptr, const uint32& pos ) {
    uint32 ulCpyLen = ulLength - pos;
    ulLength += ptr.ulLength;
    Allocate();
    str_move<char>( lpPtr, pos + ptr.ulLength, pos, ulCpyLen );
    str_copy<char>( &((char*)lpPtr)[pos], ptr.lpPtr, ptr.ulLength );
    return *this;
  }

  CStringA& CStringA::Put( const CStringA& str, const uint32& idx, const uint count ) {
    uint index = idx;
    for( uint i = 0; i < count; i++ ) {
      Put( str, index );
      index += str.Length();
    }

    return *this;
  }

  CStringA& CStringA::Put( const char* ptr, const uint32& pos ) {
    uint32 ulPtrLen = lstrlenA( ptr );
    uint32 ulCpyLen = ulLength - pos;
    ulLength += ulPtrLen;
    Allocate();
    str_move<char>( lpPtr, pos + ulPtrLen, pos, ulCpyLen );
    str_copy<char>( &((char*)lpPtr)[pos], (void32)ptr, ulPtrLen );
    return *this;
  }

  CStringA& CStringA::Put( const char* vec, const uint32& idx, const uint count ) {
    uint index = idx;
    uint length = strlen( vec );
    for( uint i = 0; i < count; i++ ) {
      Put( vec, index );
      index += length;
    }

    return *this;
  }

  CStringA& CStringA::Replace( const CStringA& ptr1, const CStringA& ptr2, bool looped ) {
    bool bFirstPass = true;
    uint32 idx = 0;
    while( true ) {
      idx = Search( ptr1, looped ? idx : bFirstPass ? 0 : idx + ptr2.Length() );
      bFirstPass = false;
      if( idx == Invalid )
        break;
      Cut( idx, ptr1.Length() );
      Put( ptr2, idx );
    }
    return *this;
  }

  CStringA& CStringA::ShrinkFront() {
    uint32 ulLen = 0;
    uint32 ulPos = ulLength;
    while( ulPos-- > 0 ) {
      if( ((char*)lpPtr)[ulPos] == ' ' ||
        ((char*)lpPtr)[ulPos] == '\t' ||
        ((char*)lpPtr)[ulPos] == '\r' ||
        ((char*)lpPtr)[ulPos] == '\n' ) ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringA& CStringA::ShrinkFront( char sym ) {
    uint32 ulLen = 0;
    uint32 ulPos = ulLength;
    while( ulPos-- > 0 ) {
      if( ((char*)lpPtr)[ulPos] == sym )
        ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringA& CStringA::ShrinkBack() {
    uint32 ulLen = 0;
    for( uint32 ulPos = 0; ulPos < ulLength; ulPos++ ) {
      if( ((char*)lpPtr)[ulPos] == ' ' ||
        ((char*)lpPtr)[ulPos] == '\t' ||
        ((char*)lpPtr)[ulPos] == '\r' ||
        ((char*)lpPtr)[ulPos] == '\n' ) ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      str_move<char>( lpPtr, 0, ulLen, ulLength - ulLen );
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringA& CStringA::ShrinkBack( char sym ) {
    uint32 ulLen = 0;
    for( uint32 ulPos = 0; ulPos < ulLength; ulPos++ ) {
      if( ((char*)lpPtr)[ulPos] == sym )
        ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      str_move<char>( lpPtr, 0, ulLen, ulLength - ulLen );
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringA& CStringA::Shrink() {
    ShrinkFront();
    ShrinkBack();
    return *this;
  }

  CStringA& CStringA::Shrink( char sym ) {
    ShrinkFront( sym );
    ShrinkBack( sym );
    return *this;
  }

  CStringA& CStringA::Upper() {
    for( uint32 i = 0; i < ulLength; i++ )
      ((char*)lpPtr)[i] = toupper( ((char*)lpPtr)[i] );
    return *this;
  }

  CStringA CStringA::GetUpper() const {
    return CStringA( *this ).Upper();
  }

  CStringA CStringA::Completion( const CStringA& end, const bool& ignoreEmpty ) const {
    if( ignoreEmpty && IsEmpty() )
      return *this;

    CStringA str( *this );
    if( !str.EndWith( end ) )
      str += end;

    return str;
  }

  CStringA CStringA::Completion( const char& end, const bool& ignoreEmpty ) const {
    if( ignoreEmpty && IsEmpty() )
      return *this;

    CStringA str( *this );
    if( str.Last() != end )
      str += end;

    return str;
  }

  CStringA& CStringA::Lower() {
    for( uint32 i = 0; i < ulLength; i++ )
      ((char*)lpPtr)[i] = tolower( ((char*)lpPtr)[i] );
    return *this;
  }

  CStringA& CStringA::LowerFirstUpper() {
    if( ulLength )
      ((char*)lpPtr)[0] = toupper( ((char*)lpPtr)[0] );

    for( uint32 i = 1; i < ulLength; i++ )
      ((char*)lpPtr)[i] = tolower( ((char*)lpPtr)[i] );

    return *this;
  }

  //static const auto locale = std::locale::global( std::locale( "" ) );
  static std::locale originLcl;
  static std::locale specialLcl( std::locale( "" ) );

  Array<CStringA> CStringA::Regex_Search( const CStringA& pattern ) const {
    std::locale::global( specialLcl );

    Array<CStringA> collection;
    try {
      std::string s( GetVector() );
      std::smatch m;
      std::regex  e( pattern.GetVector() );

      while( std::regex_search( s, m, e ) ) {
        string r = m.str().c_str();
        s = m.suffix().str();
        if( r.IsEmpty() )
          break;

        collection += r;
      }
    }
    catch( std::regex_error e ) {
      cmd << e.what() << endl;
    }

    std::locale::global( originLcl );
    return collection;
  }

  CStringA CStringA::Regex_Match( const CStringA& pattern ) const {
    std::locale::global( specialLcl );

    try {
      std::string s( GetVector() );
      std::smatch m;
      std::regex  e( pattern );

      if( !std::regex_match( s, m, e ) )
        return NullStrA;

      std::locale::global( originLcl );
      return m.str().c_str();
    }
    catch( std::regex_error e ) {
      cmd << e.what() << endl;
    }

    std::locale::global( originLcl );
    return NullStrA;
  }

  void CStringA::Regex_Replace( const CStringA& pattern, const CStringA& newSubStr ) {
    std::locale::global( specialLcl );
    try {
      std::string s( GetVector() );
      std::regex  e( pattern );
      std::string f( newSubStr );

      auto r = std::regex_replace( s, e, f );
      (*this) = r.c_str();
    }
    catch( std::regex_error e ) {
      cmd << e.what() << endl;
    }

    std::locale::global( originLcl );
  }

  static void CreateDirs( CStringA path ) {
    CStringA sub_path = path.GetPattern( "", "\\", -1 );
    if( !sub_path.IsEmpty() )
      CreateDirs( sub_path );
    CreateDirectoryA( path, 0 );
  }

  bool32 CStringA::WriteToFile( const CStringA& fname, const bool32& createDirs ) const {
    if( createDirs )
      CreateDirs( fname.GetPattern( "", "\\", -1 ) );

    FILE* hFile = fopen( fname, "w+b" );
    if( !hFile )
      return False;
    _setmode( _fileno( hFile ), _O_BINARY );

    bool32 bWrite = (fwrite( lpPtr, sizeof( char ), ulLength, hFile ) > 0);
    fclose( hFile );
    return bWrite;
  }

  bool32 CStringA::WriteToFileUTF8( const CStringA& fname, const bool32& createDirs ) const {
    if( createDirs )
      CreateDirs( fname.GetPattern( "", "\\", -1 ) );

    UTF8StringConverter converter;
    converter.ToUTF8( *this );
    return converter.WriteToFile( fname );
  }

  bool32 CStringA::ReadFromFile( const CStringA& fname ) {
    if( _access( fname, None ) == NotFound )
      return False;

    FILE* file = fopen( fname, "rb" );
    if( !file )
      return False;

    _setmode( _fileno( file ), _O_BINARY );
    fseek( file, 0, SEEK_END );
    uint fileLength = ftell( file );
    fseek( file, 0, SEEK_SET );

    // Читаем BOM
    uint BOM = 0;
    fread( &BOM, 1, 3, file );
    fseek( file, 0, SEEK_SET );

    // Определяем формат файла
    uint fileType = 0x00000000;
    if( (BOM & 0x00FFFF) == 0x00FEFF ) fileType = BOM_LE;
    else if( (BOM & 0xFFFFFF) == 0xBFBBEF ) fileType = BOM_UTF8;

    // Преобразование Юникода в Анси
    if( fileType == BOM_LE ) {
      fclose( file );

      CStringW uni;
      if( !uni.ReadFromFile( fname.AToW() ) )
        return False;

      *this = uni.WToA();
      return True;
    }
    // Преобразование Ютф в Анси
    else if( fileType == BOM_UTF8 ) {
      fclose( file );

      UTF8StringConverter converter;
      if( !converter.ReadFromFile( fname ) )
        return False;

      converter.ToAnsi( *this );
      return True;
    }

    ulLength = fileLength;

    Allocate();
    bool32 readed = (fread( lpPtr, 1, fileLength, file ) > 0);
    fclose( file );

    return readed;
  }

  bool32 CStringA::ReadFromFile( const CStringW& fname ) {
    return ReadFromFile( fname.WToA() );
  }

  bool32 CStringA::AsFileExists( const int& accessMode ) const {
    return _access( *this, accessMode ) == 0;
  }

  CStringA CStringA::GetFileName() const {
    return GetWord( "\\", -1 );
  }

  CStringA CStringA::GetDirectoryName() const {
    int count = 1 + (Last() == '\\' ? 1 : 0);
    return GetWord( "\\", -count );
  }

  CStringA CStringA::GetDirectory() const {
    int count = 1 + (Last() == '\\' ? 1 : 0);
    return GetPattern( "", "\\", -count ) + "\\";
  }

  CStringA CStringA::GetExtension() const {
    return GetWord( ".", -1 );
  }

  CStringA CStringA::RemoveExtension() const {
    return GetPattern( "", ".", -1 );
  }

  CStringA CStringA::GetFullPath() const {
    if( StartWith( "\\\\" ) )
      return *this;

    if( GetVector()[1] == ':' )
      return *this;

    char buffer[MAX_PATH];
    GetCurrentDirectory( MAX_PATH, buffer );
    string currentDirectory = buffer;
    return string::Combine( "%s\\%s", currentDirectory, *this );
  }

  CStringA CStringA::Hex32( const uint32& val ) {
    char conv[33];
    _ultoa_s( val, conv, 33, 16 );
    uint32 ulLen = lstrlenA( conv );
    for( uint32 i = 0; i < ulLen; i++ )
      conv[i] = toupper( conv[i] );

    char temp[11];
    str_copy<char>( temp, "0x00000000", 11 );

    str_copy<char>( temp + (10 - ulLen), conv, ulLen );
    return CStringA( temp, 10 );
  }

  CStringA CStringA::Hex64( const uint64& val ) {
    char conv[65];
    _ui64toa( val, conv, 16 );
    uint32 ulLen = lstrlenA( conv );
    for( uint32 i = 0; i < ulLen; i++ )
      conv[i] = toupper( conv[i] );

    char temp[19];
    str_copy<char>( temp, "0x0000000000000000", 19 );

    str_copy<char>( temp + (18 - ulLen), conv, ulLen );
    return CStringA( temp, 18 );
  }

  CStringW CStringA::AToW( const uint32& srcCodePage ) const {
    wchar_t* buffw = new wchar_t[ulLength];
    MultiByteToWideChar( srcCodePage, 0, (char*)lpPtr, ulLength, buffw, ulLength );

    CStringW newStr( buffw, ulLength );
    delete buffw;
    return newStr;
  }

  text CStringA::ToChar() {
    return (text)lpPtr;
  }

  CStringA CStringA::Combine( const text format ... ) {
    uint par;
    try {
      CStringA sResult;
      text* array = (text*)(uint32( &format ) + 4);

      for( uint32 i = 0, o = 0; format[i] != 0; i++ ) {
        par = i;
        void32 varg = (void32)(((uint32)array) + o);

        if( format[i] == '%' ) {
          switch( format[i + 1] ) {
            case FMT_SINT32:
            {
              sResult += CStringA( *(int32*)varg );
              o += sizeof( int32 );
              i++;
              break;
            };

            case FMT_SINT64:
            {
              sResult += CStringA( *(int64*)varg );
              o += sizeof( int64 );
              i++;
              break;
            };

            case FMT_UINT32:
            {
              sResult += CStringA( *(uint32*)varg );
              o += sizeof( uint32 );
              i++;
              break;
            };

            case FMT_UINT64:
            {
              sResult += CStringA( *(uint64*)varg );
              o += sizeof( uint64 );
              i++;
              break;
            };

            case FMT_HEX32:
            {
              sResult += CStringA::Hex32( *(int32*)varg );
              o += sizeof( int32 );
              i++;
              break;
            };

            case FMT_HEX64:
            {
              sResult += CStringA::Hex64( *(int64*)varg );
              o += sizeof( int64 );
              i++;
              break;
            };

            case FMT_REAL32:
            {
              double val = *(double*)varg;
              sResult += CStringA( val );
              o += sizeof( double );
              i++;
              break;
            };

            case FMT_REAL64:
            {
              double val = *(double*)varg;
              sResult += CStringA( val );
              o += sizeof( real64 );
              i++;
              break;
            };

            case FMT_BOOL:
            {
              bool v = *(int*)varg;
              sResult += CStringA( v ? "true" : "false" );
              o += sizeof( int );
              i++;
              break;
            };

            case FMT_CHAR:
            {
              sResult += CStringA( (char)*(int*)varg );
              o += sizeof( int );
              i++;
              break;
            };

            case FMT_BYTE:
            {
              sResult += CStringA( *(int*)varg );
              o += sizeof( int );
              i++;
              break;
            };

            case FMT_TEXT:
            {
              sResult += CStringA( *(text*)varg );
              o += sizeof( text );
              i++;
              break;
            };

            case FMT_STRING:
            {
              CStringBase& str = *(CStringBase*)varg;
              if( str.IsA() ) {
                sResult += str.ReinterpretToA();
                o += sizeof( CStringA );
              }
              else {
                sResult += str.ReinterpretToW().WToA();
                o += sizeof( CStringW );
              }
              i++;
              break;
            };

            case FMT_TITLE:
            {
              sResult += "\n_____________________\n\n";
              i++;
              break;
            };

            case FMT_ZSTRING:
            {
              CStringA& str = *(CStringA*)varg;
              sResult += str;
              o += sizeof( CStringA );
              i++;
              break;
            };

            case '%':
            {
              //i++;
            };

            default:
            {
              sResult += "%";
              i++;
              break;
            };
          }
        }
        else
          sResult += format[i];
      }
      return sResult;
    }
    catch( std::exception e ) {
      return "string::Combine\n" + A par + " argument is invalid.";
    }
  }

  CStringA::~CStringA() {
  }

#pragma endregion CStringADefinition



#pragma region CStringWDefinition

#pragma warning (disable: 4244)
  void CStringW::Allocate() {
    ulLength = clamp( ulLength, 0, ulRange );
    if( ulLength + 1 > ulReserved ) {
      ulReserved = clamp( res2f( ulLength + 1 ), 16, ulRange );
      str_realloc<wchar_t>( lpPtr, ulReserved );
    }
    ((wchar_t*)lpPtr)[ulLength] = 0;
  }

  void CStringW::Free() {
    ulLength = clamp( ulLength, 0, ulRange );
    if( ulLength + 1 < ulReserved / 2 ) {
      ulReserved = clamp( res2f( ulLength + 1 ), 16, ulRange );
      str_realloc<wchar_t>( lpPtr, ulReserved );
    }
    ((wchar_t*)lpPtr)[ulLength] = 0;
  }
#pragma warning (default: 4244)

  CStringW::CStringW() {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = 0;
    Allocate();
  }

  CStringW::CStringW( const CStringW& ptr ) {
    ulRange = ptr.ulRange;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = ptr.ulLength;
    Allocate();
    str_copy<wchar_t>( lpPtr, ptr.lpPtr, ulLength );
  }

  CStringW::CStringW( const wchar_t& ptr ) {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = 1;
    Allocate();
    ((wchar_t*)lpPtr)[0] = ptr;
  }

  CStringW::CStringW( const wchar_t* ptr ) {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( ptr );
    Allocate();
    str_copy<wchar_t>( lpPtr, (void32)ptr, ulLength );
  }

  CStringW::CStringW( const wchar_t* ptr, const uint32& len ) {
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = len;
    Allocate();
    str_copy<wchar_t>( lpPtr, (void32)ptr, ulLength );
  }

  CStringW::CStringW( const byte& ptr ) {
    wchar_t buff[9];
    _itow_s( ptr, buff, 9, 16 );
    uint32 ulBufLen = lstrlenW( buff );
    for( uint32 i = 0; buff[i]; i++ )
      buff[i] = towupper( buff[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = 4;
    Allocate();

    str_copy<wchar_t>( lpPtr, L"0x00", ulLength );
    str_copy<wchar_t>( (wchar_t*)lpPtr + (ulLength - ulBufLen), buff, ulBufLen );
  }

  CStringW::CStringW( const int32& ptr, const uint32& radix ) {
    wchar_t buf[33];
    _itow_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = towupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();

    str_copy<wchar_t>( lpPtr, buf, ulLength );
  }

  CStringW::CStringW( const uint32& ptr, const uint32& radix ) {
    wchar_t buf[33];
    _ultow_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = towupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();

    str_copy<wchar_t>( lpPtr, buf, ulLength );
  }

  CStringW::CStringW( const long32& ptr, const uint32& radix ) {
    wchar_t buf[33];
    _itow_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = towupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();

    str_copy<wchar_t>( lpPtr, buf, ulLength );
  }

  CStringW::CStringW( const ulong32& ptr, const uint32& radix ) {
    wchar_t buf[33];
    _ultow_s( ptr, buf, 33, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = towupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();

    str_copy<wchar_t>( lpPtr, buf, ulLength );
  }

  CStringW::CStringW( const int64& ptr, const uint32& radix ) {
    wchar_t buf[65];
    _i64tow_s( ptr, buf, 65, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = towupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();

    str_copy<wchar_t>( lpPtr, buf, ulLength );
  }

  CStringW::CStringW( const uint64& ptr, const uint32& radix ) {
    wchar_t buf[65];
    _ui64tow_s( ptr, buf, 65, radix );
    if( radix > 10 )
      for( uint32 i = 0; buf[i]; i++ )
        buf[i] = towupper( buf[i] );

    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();

    str_copy<wchar_t>( lpPtr, buf, ulLength );
  }

  CStringW::CStringW( const real32& ptr, const uint32& dig ) {
    wchar_t buf[40];
    {
      wchar_t buff[8];
      swprintf( buff, L"%%.%if", dig );
      swprintf( buf, buff, ptr );
    }
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();
    str_copy<wchar_t>( lpPtr, buf, ulLength );

    for( uint i = 0; i < ulLength; i++ ) {
      if( ((wchar_t*)lpPtr)[i] == ',' ) {
        ((wchar_t*)lpPtr)[i] = '.';
        break;
      }
    }

  }

  CStringW::CStringW( const real64& ptr, const uint32& dig ) {
    wchar_t buf[80];
    {
      wchar_t buff[8];
      swprintf( buff, L"%%.%if", dig );
      swprintf( buf, buff, ptr );
    }
    ulRange = UINT_MAX;
    lpPtr = NULL;
    ulReserved = 0;
    ulLength = lstrlenW( buf );
    Allocate();
    str_copy<wchar_t>( lpPtr, buf, ulLength );

    for( uint i = 0; i < ulLength; i++ ) {
      if( ((wchar_t*)lpPtr)[i] == ',' ) {
        ((wchar_t*)lpPtr)[i] = '.';
        break;
      }
    }

  }

  CStringW CStringW::operator + ( const CStringW& ptr ) const {
    return CStringW( *this ) += ptr;
  }

  CStringW CStringW::operator + ( const wchar_t* ptr ) const {
    return CStringW( *this ) += ptr;
  }

  CStringW& CStringW::operator += ( const CStringW& ptr ) {
    uint32 ulStartCpy = ulLength;
    ulLength += ptr.ulLength;
    Allocate();
    str_copy<wchar_t>( (wchar_t*)lpPtr + ulStartCpy, ptr.lpPtr, ptr.ulLength );
    return *this;
  }

  CStringW& CStringW::operator += ( const wchar_t* ptr ) {
    uint32 ulPtrlen = lstrlenW( ptr );
    uint32 ulStartCpy = ulLength;
    ulLength += ulPtrlen;
    Allocate();
    str_copy<wchar_t>( (wchar_t*)lpPtr + ulStartCpy, (void32)ptr, ulPtrlen );
    return *this;
  }

  CStringW& CStringW::operator = ( const CStringW& ptr ) {
    ulRange = ptr.ulRange;
    ulReserved = 0;
    ulLength = ptr.ulLength;
    Allocate();
    str_copy<wchar_t>( lpPtr, ptr.lpPtr, ulLength );
    return *this;
  }

  CStringW& CStringW::operator = ( const wchar_t* ptr ) {
    ulReserved = 0;
    ulLength = lstrlenW( ptr );
    Allocate();
    str_copy<wchar_t>( lpPtr, (void32)ptr, ulLength );
    return *this;
  }

  bool32 CStringW::operator == ( const CStringW& ptr ) const {
    return CompareI( ptr );
  }

  bool32 CStringW::operator == ( const wchar_t* ptr ) const {
    return CompareI( ptr );
  }

  bool32 CStringW::operator != ( const CStringW& ptr ) const {
    return !CompareI( ptr );
  }

  bool32 CStringW::operator != ( const wchar_t* ptr ) const {
    return !CompareI( ptr );
  }

  bool CStringW::operator > ( const CStringW& other ) const {
    uint llength = Length();
    uint rlength = other.Length();
    uint length = min( llength, rlength );

    for( uint i = 0; i < length; i++ ) {
      uint16 lchar = towupper( operator[]( i ) );
      uint16 rchar = towupper( other[i] );

      if( lchar != rchar )
        return lchar > rchar;
    }

    return llength > rlength;
  }

  bool CStringW::operator < ( const CStringW& other ) const {
    uint llength = Length();
    uint rlength = other.Length();
    uint length = min( llength, rlength );

    for( uint i = 0; i < length; i++ ) {
      byte lchar = (byte)towupper( operator[]( i ) );
      byte rchar = (byte)towupper( other[i] );

      if( lchar != rchar )
        return lchar < rchar;
    }

    return llength < rlength;
  }

  wchar_t& CStringW::First() {
    return ((wchar_t*)lpPtr)[0];
  }

  const wchar_t& CStringW::First() const {
    return ((wchar_t*)lpPtr)[0];
  }

  wchar_t& CStringW::Last() {
    return ((wchar_t*)lpPtr)[ulLength - 1];
  }

  const wchar_t& CStringW::Last() const {
    return ((wchar_t*)lpPtr)[ulLength - 1];
  }

  wchar_t& CStringW::operator [] ( const int& idx ) {
    return ((wchar_t*)lpPtr)[(uint32)idx];
  }

  const wchar_t& CStringW::operator [] ( const int& idx ) const {
    return ((wchar_t*)lpPtr)[(uint32)idx];
  }

  CStringW::operator const wchar_t* () const {
    return (wchar_t*)lpPtr;
  }

  int32 CStringW::ToInt32( const uint32& radix ) const {
    char buff[33];
    uint32 len = wcstombs( buff, (wchar_t*)lpPtr, ulLength );
    buff[len] = 0;
    return strtol( buff, NULL, radix );
  }

  int64 CStringW::ToInt64( const uint32& radix ) const {
    char buff[65];
    uint32 len = wcstombs( buff, (wchar_t*)lpPtr, ulLength );
    buff[len] = 0;
    return _strtoi64( buff, NULL, radix );
  }

  real32 CStringW::ToReal32() const {
    for( uint i = 0; i < ulLength; i++ ) {
      if( ((wchar_t*)lpPtr)[i] == '.' ) {
        ((wchar_t*)lpPtr)[i] = ',';
        break;
      }
    }

    return (float)_wtof( (wchar_t*)lpPtr );
  }

  real64 CStringW::ToReal64() const {
    for( uint i = 0; i < ulLength; i++ ) {
      if( ((wchar_t*)lpPtr)[i] == '.' ) {
        ((wchar_t*)lpPtr)[i] = ',';
        break;
      }
    }

    return _wtof( (wchar_t*)lpPtr );
  }

  bool32 CStringW::IsNumber() const {
    for( uint32 i = 0; i < Length(); i++ ) {
      if( !isdigit( ((wchar_t*)lpPtr)[i] ) && ((wchar_t*)lpPtr)[i] != '-' && ((wchar_t*)lpPtr)[i] != '.' ) {
        return false;
      }
    }
    return true;
  }

  bool32 CStringW::IsHexNumber() const {
    if( ((wchar_t*)lpPtr)[0] != L'0' )
      return false;

    return MatchesMask( L"xX1234567890abcdefABCDEF" );
  }

  wtext CStringW::ToWChar() {
    return (wchar_t*)lpPtr;
  }

  const wchar_t* CStringW::GetVector() const {
    return (wchar_t*)lpPtr;
  }

  uint32 CStringW::Length() const {
    return ulLength;
  }

  uint32 CStringW::Allocated() const {
    return ulReserved;
  }

  bool32 CStringW::IsEmpty() const {
    return ulLength == 0;
  }

  bool32 CStringW::Compare( const CStringW& ptr ) const {
    if( ulLength != ptr.ulLength )
      return FALSE;
    return lstrcmpW( (wchar_t*)lpPtr, (wchar_t*)ptr.lpPtr ) == 0;
  }

  bool32 CStringW::Compare( const wchar_t* ptr ) const {
    return lstrcmpW( (wchar_t*)lpPtr, ptr ) == 0;
  }

  bool32 CStringW::CompareI( const CStringW& ptr ) const {
    if( ulLength != ptr.ulLength )
      return FALSE;
    return lstrcmpiW( (wchar_t*)lpPtr, (wchar_t*)ptr.lpPtr ) == 0;
  }

  bool32 CStringW::CompareI( const wchar_t* ptr ) const {
    return lstrcmpiW( (wchar_t*)lpPtr, ptr ) == 0;
  }

  bool32 CStringW::CompareMasked( const CStringW& ptr ) const {
    Array<CStringW> tokens = ptr.Split( L"*" );

    uint index = 0;
    for( uint i = 0; i < tokens.GetNum(); i++ ) {
      if( tokens[i].IsEmpty() )
        continue;

      index = Search( tokens[i], index, True );
      if( index == Invalid )
        return false;
    }

    return true;
  }

  bool32 CStringW::CompareMaskedI( const CStringW& ptr ) const {
    Array<CStringW> tokens = ptr.Split( L"*" );

    uint index = 0;
    for( uint i = 0; i < tokens.GetNum(); i++ ) {
      if( tokens[i].IsEmpty() )
        continue;

      index = SearchI( tokens[i], index, True );
      if( index == Invalid )
        return false;
    }

    return true;
  }

  uint32 CStringW::Search( const CStringW& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        if( ((wchar_t*)lpPtr)[i + j] != ((wchar_t*)ptr.lpPtr)[j] )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return i + ptr.ulLength;
          return i;
        }
      }
    }
    return NotFound;
  }

  uint32 CStringW::SearchI( const CStringW& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        if( towupper( ((wchar_t*)lpPtr)[i + j] ) != towupper( ((wchar_t*)ptr.lpPtr)[j] ) )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return i + ptr.ulLength;
          return i;
        }
      }
    }
    return NotFound;
  }

  uint32 CStringW::SearchReverse( const CStringW& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    uint32 submode1 = ulLength - 1;
    uint32 submode2 = ptr.ulLength - 1;
    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        wchar_t left = ((wchar_t*)lpPtr)[submode1 - i - j];
        wchar_t right = ((wchar_t*)ptr.lpPtr)[submode2 - j];
        if( left != right )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return ulLength - i;
          return (submode1 - i) - submode2;
        }
      }
    }
    return NotFound;
  }

  uint32 CStringW::SearchReverseI( const CStringW& ptr, const uint32& pos, const bool32& end ) const {
    if( ptr.ulLength > ulLength ) return NotFound;
    if( ptr.ulLength == 0 || ulLength == 0 ) return NotFound;

    uint32 submode1 = ulLength - 1;
    uint32 submode2 = ptr.ulLength - 1;
    for( uint32 i = pos; i < ulLength; i++ ) {

      for( uint32 j = 0; i + ptr.ulLength <= ulLength; j++ ) {
        wchar_t left = towupper( ((wchar_t*)lpPtr)[submode1 - i - j] );
        wchar_t right = towupper( ((wchar_t*)ptr.lpPtr)[submode2 - j] );
        if( left != right )
          break;

        if( j >= ptr.ulLength - 1 ) {
          if( end )
            return ulLength - i;
          return (submode1 - i) - submode2;
        }
      }
    }
    return NotFound;
  }

  bool32 CStringW::HasWord( const CStringW& ptr ) const {
    uint32 count = 0;
    uint32 pos = 0;
    while( (pos = Search( ptr, pos, True )) != NotFound ) {
      count++;
    }
    return count;
  }

  bool32 CStringW::HasWordI( const CStringW& ptr ) const {
    uint32 count = 0;
    uint32 pos = 0;
    while( (pos = SearchI( ptr, pos, True )) != NotFound ) {
      count++;
    }
    return count;
  }

  static bool32 BelongToMaskW( const CStringW& str, const wchar_t& c ) {
    for( uint32 i = 0; i < str.Length(); i++ )
      if( c == str[i] )
        return TRUE;
    return FALSE;
  }

  bool32 CStringW::MatchesMask( const CStringW& mask ) const {
    for( uint32 i = 0; i < Length(); i++ ) {
      for( uint32 j = 0; j < mask.Length(); j++ ) {
        if( (*this)[i] == mask[j] )
          goto con;
      }
      return False;
    con:
      continue;
    }
    return True;
  }

  bool32 CStringW::StartWith( const CStringW& cmp ) const {
    if( cmp.ulLength > ulLength )
      return False;

    for( uint i = 0; i < cmp.ulLength; i++ )
      if( ((wchar_t*)lpPtr)[i] != cmp[i] )
        return False;

    return True;
  }

  bool32 CStringW::EndWith( const CStringW& cmp ) const {
    if( cmp.ulLength > ulLength )
      return False;

    for( uint i = 0; i < cmp.ulLength; i++ )
      if( ((wchar_t*)lpPtr)[ulLength - i - 1] != cmp[cmp.ulLength - i - 1] )
        return False;

    return True;
  }

  CStringW CStringW::GetWord( const CStringW& mask, const int32& word_index ) const {
    uint32 index = 0;
    uint32 left = 0;
    uint32 right = 0;
    int32  num = 0;
    bool32 found = FALSE;

    if( word_index > 0 ) {
      for( ;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskW( mask, ((wchar_t*)lpPtr)[index] ) )
            continue;
          left = index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskW( mask, ((wchar_t*)lpPtr)[index] ) )
            continue;
        right = index;
        found = FALSE;
        num++;

        if( num >= word_index )
          return Copy( left, right - left );
        if( index >= ulLength )
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 submode = ulLength - 1;

      for( ;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskW( mask, ((wchar_t*)lpPtr)[submode - index] ) )
            continue;
          left = ulLength - index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskW( mask, ((wchar_t*)lpPtr)[submode - index] ) )
            continue;
        right = ulLength - index;
        found = FALSE;
        num--;

        if( num <= word_index )
          return Copy( right, left - right );
        if( index >= ulLength )
          break;
      }
    }
    return NullStrW;
  }

  CStringW CStringW::GetPattern( const CStringW& lw, const CStringW& rw, const int32& word_index ) const {
    int32 num = 0;

    if( word_index > 0 ) {
      uint32 left = rw.IsEmpty() ? -1 : 0;
      uint32 right = lw.IsEmpty() ? -1 : 0;

      while( (left = (lw.IsEmpty() ? 0 :
        rw.IsEmpty() ? Search( lw, left + 1, TRUE ) :
        Search( lw, right, TRUE ))) != NotFound ) {

        if( (right = (rw.IsEmpty() ? ulLength :
          lw.IsEmpty() ? Search( rw, right + 1, FALSE ) :
          Search( rw, left, FALSE ))) != NotFound ) {

          if( ++num >= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 left = rw.IsEmpty() ? ulLength + 1 : ulLength;
      uint32 right = lw.IsEmpty() ? ulLength + 1 : ulLength;

      while( (right = (rw.IsEmpty() ? ulLength :
        lw.IsEmpty() ? SearchReverse( rw, (ulLength - right + 1), FALSE ) :
        SearchReverse( rw, (ulLength - left), FALSE ))) != NotFound ) {

        if( (left = (lw.IsEmpty() ? 0 :
          rw.IsEmpty() ? SearchReverse( lw, (ulLength - left + 1), TRUE ) :
          SearchReverse( lw, (ulLength - right), TRUE ))) != NotFound ) {

          if( --num <= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }

    return NullStrW;
  }

  CStringW CStringW::GetWordEx( const CStringW& mask, const int32& word_index, const uint32& offset, uint32* rtn ) const {
    ASSERT( word_index != 0, "Невозможно извлечь 0-е слово из строки." );

    uint32 index = 0;
    uint32 left = 0;
    uint32 right = 0;
    int32  num = 0;
    bool32 found = FALSE;

    if( word_index > 0 ) {
      for( index = offset;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskW( mask, ((wchar_t*)lpPtr)[index] ) )
            continue;
          left = index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskW( mask, ((wchar_t*)lpPtr)[index] ) )
            continue;
        right = index;
        found = FALSE;
        num++;

        if( num >= word_index ) {
          if( rtn )
            *rtn = left;
          return Copy( left, right - left );
        }
        if( index >= ulLength )
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 submode = ulLength - 1;

      for( index = ulLength - offset;; index++ ) {
        if( !found ) {
          if( index >= ulLength )
            break;
          if( BelongToMaskW( mask, ((wchar_t*)lpPtr)[submode - index] ) )
            continue;
          left = ulLength - index;
          found = TRUE;
        }
        if( index < ulLength )
          if( !BelongToMaskW( mask, ((wchar_t*)lpPtr)[submode - index] ) )
            continue;
        right = ulLength - index;
        found = FALSE;
        num--;

        if( num <= word_index ) {
          if( rtn )
            *rtn = left;
          return Copy( right, left - right );
        }
        if( index >= ulLength )
          break;
      }
    }
    if( rtn )
      *rtn = Invalid;
    return NullStrW;
  }

  CStringW CStringW::GetPatternEx( const CStringW& lw, const CStringW& rw, const int32& word_index, const uint32& offset, uint32* rtn ) const {
    ASSERT( word_index != 0, "Невозможно извлечь 0-е слово из строки." );

    int32 num = 0;

    if( word_index > 0 ) {
      uint32 left = rw.IsEmpty() ? offset - 1 : offset;
      uint32 right = lw.IsEmpty() ? offset - 1 : offset;

      while( (left = (lw.IsEmpty() ? 0 :
        rw.IsEmpty() ? Search( lw, left + 1, TRUE ) :
        Search( lw, right, TRUE ))) != NotFound ) {

        if( (right = (rw.IsEmpty() ? ulLength :
          lw.IsEmpty() ? Search( rw, right + 1, FALSE ) :
          Search( rw, left, FALSE ))) != NotFound ) {

          if( ++num >= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            if( rtn )
              *rtn = scpy;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }

    if( word_index < 0 ) {
      uint32 left = rw.IsEmpty() ? ulLength - offset + 1 : ulLength - offset;
      uint32 right = lw.IsEmpty() ? ulLength - offset + 1 : ulLength - offset;

      while( (right = (rw.IsEmpty() ? ulLength :
        lw.IsEmpty() ? SearchReverse( rw, (ulLength - right + 1), FALSE ) :
        SearchReverse( rw, (ulLength - left), FALSE ))) != NotFound ) {

        if( (left = (lw.IsEmpty() ? 0 :
          rw.IsEmpty() ? SearchReverse( lw, (ulLength - left + 1), TRUE ) :
          SearchReverse( lw, (ulLength - right), TRUE ))) != NotFound ) {

          if( --num <= word_index ) {
            uint32 scpy = left;
            uint32 slen = right - left;
            if( rtn )
              *rtn = scpy;
            return Copy( scpy, slen );
          }
        }
        else
          break;
      }
    }
    if( rtn )
      *rtn = Invalid;
    return NullStrW;
  }

  static int GetSymbolGroupW( const wchar_t& c ) {
    if( BelongToMaskW( L" \t\n\r", c ) )
      return 0;

    if( c == L'_' || (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') )
      return 1;

    if( BelongToMaskW( L"+-*/|^%@!=&.,", c ) )
      return 2;

    if( BelongToMaskW( L"()[]{}'\"", c ) )
      return 3;

    return 4;
  }

  Array<CStringW> CStringW::Split( const CStringW& separator ) const {
    Array<CStringW> words;
    if( IsEmpty() )
      return words;

    uint begin = 0;
    uint end = 0;
    while( begin < Length() ) {
      end = Search( separator, begin );
      if( end == Invalid ) {
        words += &GetVector()[begin];
        break;
      }

      words += CStringW( &GetVector()[begin], end - begin );
      end += separator.Length();
      begin = end;
    }

    return words;
  }

  CStringW CStringW::GetWordSmart( const int& word_index, const bool& groupSymbols ) const {
    if( word_index == 0 )
      throw std::runtime_error( "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint   index = 0;
    uint   left = 0;
    uint   right = 0;
    int    num = 0;
    bool32 found = False;

    // 0 - space
    // 1 - letters
    // 2 - operators
    // 3 - brackets
    // 4 - same spaces ???
    int oldGroup = Invalid;
    int newGroup = Invalid;

    if( word_index > 0 ) {
      for( ;; index++ ) {
        wchar_t sym = ((wchar_t*)lpPtr)[index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupW( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = index;
          found = False;
          num++;
          index--;
        }

        if( num >= word_index )
          return Copy( left, right - left );

        if( index >= ulLength )
          break;
      }
    }
    else if( word_index < 0 ) {
      uint reverser = ulLength - 1;

      for( ;; index++ ) {
        wchar_t sym = ((wchar_t*)lpPtr)[reverser - index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupW( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = ulLength - index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = ulLength - index;
          found = False;
          num--;
          index--;
        }

        if( num <= word_index )
          return Copy( right, left - right );

        if( index >= ulLength )
          break;
      }
    }

    return NullStrW;
  }

  CStringW CStringW::GetWordSmartEx( const int& word_index, const bool& groupSymbols, const uint& offset, uint* word_offset ) const {
    if( word_index == 0 )
      throw std::runtime_error( "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint   index = offset;
    uint   left = 0;
    uint   right = 0;
    int    num = 0;
    bool32 found = False;

    // 0 - space
    // 1 - letters
    // 2 - operators
    // 3 - brackets
    // 4 - same spaces ???
    int oldGroup = Invalid;
    int newGroup = Invalid;

    if( word_index > 0 ) {
      for( ;; index++ ) {
        wchar_t sym = ((wchar_t*)lpPtr)[index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupW( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = index;
          found = False;
          num++;
          index--;
        }

        if( num >= word_index ) {
          if( word_offset )
            *word_offset = left;

          return Copy( left, right - left );
        }

        if( index >= ulLength )
          break;
      }
    }
    else if( word_index < 0 ) {
      uint reverser = ulLength - 1;

      for( ;; index++ ) {
        wchar_t sym = ((wchar_t*)lpPtr)[reverser - index];

        oldGroup = newGroup;
        newGroup = GetSymbolGroupW( sym );

        if( !found ) {
          if( newGroup == 0 )
            continue;

          if( index >= ulLength )
            break;

          if( oldGroup == newGroup )
            continue;

          left = ulLength - index;
          found = True;
        }
        else if( oldGroup != newGroup || (!groupSymbols && newGroup != 1 && newGroup != 2) ) {
          newGroup = Invalid;
          right = ulLength - index;
          found = False;
          num--;
          index--;
        }

        if( num <= word_index )
          return Copy( right, left - right );

        if( index >= ulLength )
          break;
      }
    }

    return NullStrW;
  }

  inline bool32 BelongToMaskW_Line( wchar_t& sym, const CStringW& line ) {
    for( uint32 i = 0; i < line.Length(); i++ ) {
      if( line[i] == sym )
        return True;
    }
    return False;
  }

  inline uint32 BelongToMaskW_Array( wchar_t& sym, wchar_t* pprev, const CStringW mask[], const uint32& cnt, const uint32& src = Invalid ) {
    if( pprev ) {
      uint32 idx = 0;
      uint32 lim = cnt;
      if( src != Invalid ) {
        idx += src;
        lim += src;
      }

      wchar_t prev = *pprev;
      for( ; idx < lim; idx++ ) {
        uint32 _idx = idx % cnt;
        const CStringW& line = mask[_idx];
        if( BelongToMaskW_Line( sym, line ) && BelongToMaskW_Line( prev, line ) )
          return src;
      }
    }

    {
      uint32 idx = 0;
      uint32 lim = cnt;
      if( src != Invalid ) {
        idx += src;
        lim += src;
      }

      for( ; idx < lim; idx++ ) {
        uint32 _idx = idx % cnt;
        for( uint32 j = 0; j < mask[_idx].Length(); j++ ) {
          if( sym == mask[_idx][j] )
            return _idx;
        }
      }
    }
    return Invalid;
  }

  inline uint32 BelongToBracketsW_Array( wchar_t& sym, const CStringW* brackets[], const uint32& brack_num ) {
    for( uint32 i = 0; i < brack_num; i++ ) {
      wchar_t* buf = &sym;
      const CStringW& sLeftBrack = brackets[i][0];
      CStringW sWord( buf, sLeftBrack.Length() );

      if( sWord.Compare( sLeftBrack ) ) {
        uint32 ulBrackCount = 1;
        const CStringW& sRightBrack = brackets[i][1];

        for( uint32 j = sLeftBrack.Length(); buf[j]; j++ ) {
          if( sRightBrack.Compare( CStringW( &buf[j], sRightBrack.Length() ) ) ) {
            if( buf[j - 1] != L'\\' || (j > 1 ? (buf[j - 1] == L'\\' && buf[j - 2] == L'\\') : False) ) {
              if( --ulBrackCount == 0 ) {
                uint32 ulLength = &buf[j] + sRightBrack.Length() - &buf[0];
                return ulLength;
              };
              j += sRightBrack.Length() - 1;
              continue;
            }
          }
          else if( sLeftBrack.Compare( CStringW( &buf[j], sLeftBrack.Length() ) ) ) {
            if( buf[j - 1] != L'\\' || (j > 1 ? (buf[j - 1] == L'\\' && buf[j - 2] == L'\\') : False) ) {
              ++ulBrackCount;
              j += sLeftBrack.Length() - 1;
              continue;
            }
          }
        }
        Message::Fatal( "Unexpected end of the block!" );
      }
    }
    return 0;
  }

  CStringW CStringW::GetSymbol( const CStringW* separators, const uint32& sep_num, const uint32& word_index, uint32* inline_index ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint32 Index = 0;
    uint32 Num = 0;

    uint32 Left = 0;
    uint32 Right = 0;

    uint32 ulOldMask = Invalid;
    uint32 ulMask = Invalid;

    for( uint32 Index = (inline_index ? *inline_index : 0); Index <= ulLength; Index++, ulOldMask = ulMask ) {
      wchar_t& sym = ((wchar_t*)lpPtr)[Index];
      wchar_t* prev = Index ? &((wchar_t*)lpPtr)[Index - 1] : Null;
      ulMask = BelongToMaskW_Array( sym, prev, separators, sep_num, ulOldMask );

      if( ulOldMask != ulMask ) {
        if( ulOldMask != Invalid )
          if( ++Num >= word_index ) {
            if( inline_index )
              *inline_index = Left;
            return Copy( Left, Right - Left );
          }
        Left = Index;
      }
      Right = Index + 1;
    }

    if( inline_index )
      *inline_index = Invalid;
    return NullStrW;
  }

  inline uint32 IsEmptyCloseBracketW( wchar_t& sym, const CStringW* brackets[], const uint32& brack_num ) {
    for( uint32 i = 0; i < brack_num; i++ ) {

      const CStringW& sRightBrack = brackets[i][1];
      wchar_t* buf = &sym;

      if( sRightBrack.Compare( CStringW( &buf[0], sRightBrack.Length() ) ) )
        return sRightBrack.Length();
    }
    return 0;
  }

  CStringW CStringW::GetSymbolEx( const CStringW separators[], const uint32& sep_num, const CStringW* brackets[], const uint32& br_num, const uint32& word_index, uint32* inline_index ) const {
    ASSERT( word_index != 0, "Impossible to extract Zero word from string. Indexing start with First (1) or First less than zero (-1)" );

    uint32 Index = 0;
    uint32 Num = 0;

    uint32 Left = 0;
    uint32 Right = 0;

    uint32 ulOldMask = Invalid;
    uint32 ulMask = Invalid;

    for( uint32 Index = (inline_index ? *inline_index : 0); Index <= ulLength; Index++, ulOldMask = ulMask ) {
      wchar_t& sym = ((wchar_t*)lpPtr)[Index];
      uint32 ulBrackLength = BelongToBracketsW_Array( sym, brackets, br_num );
      if( ulBrackLength ) {
        if( ulMask != Invalid ) {
          if( ++Num >= word_index ) {
            if( inline_index )
              *inline_index = Left;
            return Copy( Left, Right - Left );
          }
        }

        ulOldMask = Invalid;
        ulMask = Invalid;
        Left = Index;

        if( ++Num >= word_index ) {
          if( inline_index )
            *inline_index = Left;
          return Copy( Left, ulBrackLength );
        }
        Index += ulBrackLength - 1;
      }
      else if( uint32 length = IsEmptyCloseBracketW( sym, brackets, br_num ) ) {
        if( ulMask != Invalid ) {
          if( ++Num >= word_index ) {
            if( inline_index )
              *inline_index = Left;
            return Copy( Left, Right - Left );
          }
        }

        ulOldMask = Invalid;
        ulMask = Invalid;
        Left = Index;

        if( ++Num >= word_index ) {
          if( inline_index )
            *inline_index = Left;
          return Copy( Left, length );
        }
        Index += length - 1;
      }
      else {
        wchar_t* prev = Index ? &((wchar_t*)lpPtr)[Index - 1] : Null;
        ulMask = BelongToMaskW_Array( sym, prev, separators, sep_num, ulOldMask );

        if( ulOldMask != ulMask ) {
          if( ulOldMask != Invalid )
            if( ++Num >= word_index ) {
              if( inline_index )
                *inline_index = Left;
              return Copy( Left, Right - Left );
            }
          Left = Index;
        }
      }
      Right = Index + 1;
    }

    if( inline_index )
      *inline_index = Invalid;
    return NullStrW;
  }

  CStringW& CStringW::Reverse() {
    uint32 uliterations = ulLength / 2;
    for( uint32 i = 0; i < uliterations; i++ ) {
      wchar_t tmp = ((wchar_t*)lpPtr)[i];
      ((wchar_t*)lpPtr)[i] = ((wchar_t*)lpPtr)[ulLength - 1 - i];
      ((wchar_t*)lpPtr)[ulLength - 1 - i] = tmp;
    }
    return *this;
  }

  void CStringW::Clear() {
    ulLength = 0;
    Free();
  }

  CStringW CStringW::Copy( const uint32& pos, const uint32& len ) const {
    return CStringW( &((wchar_t*)lpPtr)[pos], len );
  }

  CStringW& CStringW::Cut( const uint32& pos, const uint32& len ) {
    str_move<wchar_t>( lpPtr, pos, pos + len, ulLength - (pos + len) );
    ulLength -= len;
    Free();
    return *this;
  }

  CStringW& CStringW::Cut( const uint32& len ) {
    ulLength -= len;
    Free();
    return *this;
  }

  CStringW& CStringW::Put( const CStringW& ptr, const uint32& pos ) {
    uint32 ulCpyLen = ulLength - pos;
    ulLength += ptr.ulLength;
    Allocate();
    str_move<wchar_t>( lpPtr, pos + ptr.ulLength, pos, ulCpyLen );
    str_copy<wchar_t>( &((wchar_t*)lpPtr)[pos], ptr.lpPtr, ptr.ulLength );
    return *this;
  }

  CStringW& CStringW::Put( const wchar_t* ptr, const uint32& pos ) {
    uint32 ulPtrLen = lstrlenW( ptr );
    uint32 ulCpyLen = ulLength - pos;
    ulLength += ulPtrLen;
    Allocate();
    str_move<wchar_t>( lpPtr, pos + ulPtrLen, pos, ulCpyLen );
    str_copy<wchar_t>( &((wchar_t*)lpPtr)[pos], (void32)ptr, ulPtrLen );
    return *this;
  }

  CStringW& CStringW::Put( const CStringW& str, const uint32& idx, const uint32 count ) {
    uint index = idx;
    for( uint i = 0; i < count; i++ ) {
      Put( str, index );
      index += str.Length();
    }

    return *this;
  }

  CStringW& CStringW::Put( const wchar_t* vec, const uint32& idx, const uint32 count ) {
    uint index = idx;
    uint length = lstrlenW( vec );
    for( uint i = 0; i < count; i++ ) {
      Put( vec, index );
      index += length;
    }

    return *this;
  }

  CStringW& CStringW::Replace( const CStringW& ptr1, const CStringW& ptr2, bool looped ) {
    bool bFirstPass = true;
    uint32 idx = 0;
    while( true ) {
      //idx = Search(ptr1, looped ? idx : idx ? idx + ptr2.Length() : idx );

      idx = Search( ptr1, looped ? idx : bFirstPass ? 0 : idx + ptr2.Length() );
      bFirstPass = false;
      if( idx == Invalid )
        break;
      Cut( idx, ptr1.Length() );
      Put( ptr2, idx );
    }
    return *this;
  }

  CStringW& CStringW::ShrinkFront() {
    uint32 ulLen = 0;
    uint32 ulPos = ulLength;
    while( ulPos-- > 0 ) {
      if( ((wchar_t*)lpPtr)[ulPos] == L' ' ||
        ((wchar_t*)lpPtr)[ulPos] == L'\t' ||
        ((wchar_t*)lpPtr)[ulPos] == L'\r' ||
        ((wchar_t*)lpPtr)[ulPos] == L'\n' ) ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringW& CStringW::ShrinkFront( wchar_t sym ) {
    uint32 ulLen = 0;
    uint32 ulPos = ulLength;
    while( ulPos-- > 0 ) {
      if( ((wchar_t*)lpPtr)[ulPos] == sym )
        ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringW& CStringW::ShrinkBack() {
    uint32 ulLen = 0;
    for( uint32 ulPos = 0; ulPos < ulLength; ulPos++ ) {
      if( ((wchar_t*)lpPtr)[ulPos] == L' ' ||
        ((wchar_t*)lpPtr)[ulPos] == L'\t' ||
        ((wchar_t*)lpPtr)[ulPos] == L'\r' ||
        ((wchar_t*)lpPtr)[ulPos] == L'\n' ) ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      str_move<wchar_t>( lpPtr, 0, ulLen, ulLength - ulLen );
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringW& CStringW::ShrinkBack( wchar_t sym ) {
    uint32 ulLen = 0;
    for( uint32 ulPos = 0; ulPos < ulLength; ulPos++ ) {
      if( ((wchar_t*)lpPtr)[ulPos] == sym )
        ulLen++;
      else
        break;
    }
    if( ulLen > 0 ) {
      str_move<wchar_t>( lpPtr, 0, ulLen, ulLength - ulLen );
      ulLength -= ulLen;
      Free();
    }
    return *this;
  }

  CStringW& CStringW::Shrink() {
    ShrinkFront();
    ShrinkBack();
    return *this;
  }

  CStringW& CStringW::Shrink( wchar_t sym ) {
    ShrinkFront( sym );
    ShrinkBack( sym );
    return *this;
  }

  CStringW& CStringW::Upper() {
    for( uint32 i = 0; i < ulLength; i++ )
      ((wchar_t*)lpPtr)[i] = towupper( ((wchar_t*)lpPtr)[i] );
    return *this;
  }

  CStringW& CStringW::Lower() {
    for( uint32 i = 0; i < ulLength; i++ )
      ((wchar_t*)lpPtr)[i] = towlower( ((wchar_t*)lpPtr)[i] );
    return *this;
  }

  CStringW& CStringW::LowerFirstUpper() {
    if( ulLength )
      ((wchar_t*)lpPtr)[0] = towupper( ((wchar_t*)lpPtr)[0] );

    for( uint32 i = 1; i < ulLength; i++ )
      ((wchar_t*)lpPtr)[i] = towlower( ((wchar_t*)lpPtr)[i] );

    return *this;
  }

  Array<CStringW> CStringW::Regex_Search( const CStringW& pattern ) const {
    std::locale::global( specialLcl );

    Array<CStringW> collection;
    try {
      std::wstring s( GetVector() );
      std::wsmatch m;
      std::wregex  e( pattern );

      while( std::regex_search( s, m, e ) ) {
        collection += m.str().c_str();
        s = m.suffix().str();
      }
    }
    catch( std::regex_error e ) {
      cmd << e.what() << endl;
    }

    std::locale::global( originLcl );
    return collection;
  }

  CStringW CStringW::Regex_Match( const CStringW& pattern ) const {
    std::locale::global( specialLcl );
    try {
      std::wstring s( GetVector() );
      std::wsmatch m;
      std::wregex  e( pattern );

      if( !std::regex_match( s, m, e ) )
        return NullStrW;

      std::locale::global( originLcl );
      return m.str().c_str();
    }
    catch( std::regex_error e ) {
      cmd << e.what() << endl;
    }

    std::locale::global( originLcl );
    return NullStrW;
  }

  void CStringW::Regex_Replace( const CStringW& pattern, const CStringW& newSubStr ) {
    std::locale::global( specialLcl );
    try {
      std::wstring s( GetVector() );
      std::wregex  e( pattern );
      std::wstring f( newSubStr );

      auto r = std::regex_replace( s, e, f );
      (*this) = r.c_str();
    }
    catch( std::regex_error e ) {
      cmd << e.what() << endl;
    }

    std::locale::global( originLcl );
  }

  static void CreateDirsW( CStringW path ) {
    CStringW sub_path = path.GetPattern( L"", L"\\", -1 );
    if( !sub_path.IsEmpty() )
      CreateDirsW( sub_path );
    CreateDirectoryW( path, 0 );
  }

  bool32 CStringW::WriteToFile( const CStringW& fname, const uint32& srcCodePage, const bool32& createDirs ) const {
    if( createDirs )
      CreateDirsW( fname.GetPattern( L"", L"\\", -1 ) );

    FILE* file = _wfopen( fname, L"w+b" );
    if( !file )
      return False;

    _setmode( _fileno( file ), _O_BINARY );

    wchar_t BOM = 0xFEFF;
    fwrite( &BOM, 1, 2, file );

    bool32 writed = (fwrite( lpPtr, sizeof( wchar_t ), ulLength, file ) > 0);
    fclose( file );

    return writed;
#if 0
    if( createDirs )
      CreateDirsW( fname.GetPattern( L"", L"\\", -1 ) );

    FILE* hFile = _wfopen( fname, L"w+b" );
    if( !hFile )
      return False;
    _setmode( _fileno( hFile ), _O_BINARY );

    bool32 bWrite = (fwrite( lpPtr, sizeof( wchar_t ), ulLength, hFile ) > 0);
    fclose( hFile );
    return bWrite;
#endif
  }

  bool32 CStringW::WriteToFile( const CStringW& fname, const bool32& createDirs ) const {
    return WriteToFile( fname, ANSI_CODEPAGE_DEFAULT, createDirs );
  }

  bool32 CStringW::WriteToFile( const CStringA& fname, const bool32& createDirs ) const {
    return WriteToFile( fname.AToW() );
#if 0
    if( createDirs )
      CreateDirs( fname.GetPattern( "", "\\", -1 ) );

    FILE* hFile = fopen( fname, "w+b" );
    if( !hFile )
      return False;
    _setmode( _fileno( hFile ), _O_BINARY );

    bool32 bWrite = (fwrite( lpPtr, sizeof( wchar_t ), ulLength, hFile ) > 0);
    fclose( hFile );
    return bWrite;
#endif
  }

  bool32 CStringW::WriteToFileUTF8( const CStringW& fname, const bool32& createDirs ) const {
    if( createDirs )
      CreateDirsW( fname.GetPattern( L"", L"\\", -1 ) );

    UTF8StringConverter converter;
    converter.ToUTF8( *this );
    return converter.WriteToFile( fname );
  }

  bool32 CStringW::ReadFromFile( const CStringW& fname, const uint32& srcCodePage ) {
    if( _waccess( fname, None ) == NotFound )
      return False;

    FILE* file = _wfopen( fname, L"rb" );
    if( !file )
      return False;

    _setmode( _fileno( file ), _O_BINARY );
    fseek( file, 0, SEEK_END );
    uint fileLength = ftell( file );
    fseek( file, 0, SEEK_SET );

    // Читаем BOM
    uint BOM = 0;
    fread( &BOM, 1, 3, file );
    fseek( file, 0, SEEK_SET );

    // Определяем формат файла
    uint fileType = 0x00000000;
    if( (BOM & 0x00FFFF) == 0x00FEFF ) fileType = BOM_LE;
    else if( (BOM & 0xFFFFFF) == 0xBFBBEF ) fileType = BOM_UTF8;

    if( fileType == BOM_LE ) {
      fileLength -= 2;
      fseek( file, 2, SEEK_SET );
    }
    else if( fileType == BOM_UTF8 ) {
      fclose( file );

      UTF8StringConverter converter;
      if( !converter.ReadFromFile( fname ) )
        return False;

      converter.ToUnicode( *this );
      return True;
    }
    else {
      CStringA ansi;
      if( !ansi.ReadFromFile( fname.WToA() ) )
        return False;

      *this = ansi.AToW( srcCodePage );
      return True;
    }

    ulLength = fileLength / sizeof( wchar_t );

    Allocate();
    bool32 readed = (fread( lpPtr, 1, fileLength, file ) > 0);
    fclose( file );

    return readed;
  }

  bool32 CStringW::ReadFromFile( const CStringW& fname ) {
    return ReadFromFile( fname, ANSI_CODEPAGE_DEFAULT );
  }

  bool32 CStringW::ReadFromFile( const CStringA& fname ) {
    return ReadFromFile( fname.AToW() );
  }

  CStringW CStringW::Hex32( const uint32& val ) {
    wchar_t conv[9];
    _ltow_s( val, conv, 9, 16 );
    uint32 ulLen = lstrlenW( conv );
    for( uint32 i = 0; i < ulLen; i++ )
      conv[i] = towupper( conv[i] );

    wchar_t temp[11];
    str_copy<wchar_t>( temp, L"0x00000000", 11 );

    str_copy<wchar_t>( temp + (10 - ulLen), conv, ulLen );
    return CStringW( temp, 10 );
  }

  CStringW CStringW::Hex64( const uint64& val ) {
    wchar_t conv[17];
    _i64tow_s( val, conv, 17, 16 );
    uint32 ulLen = lstrlenW( conv );
    for( uint32 i = 0; i < ulLen; i++ )
      conv[i] = towupper( conv[i] );

    wchar_t temp[19];
    str_copy<wchar_t>( temp, L"0x0000000000000000", 19 );

    str_copy<wchar_t>( temp + (18 - ulLen), conv, ulLen );
    return CStringW( temp, 18 );
  }

  CStringA CStringW::WToA( const uint32& srcCodePage ) const {
    char* buffa = new char[ulLength];
    WideCharToMultiByte( srcCodePage, 0, (wchar_t*)lpPtr, ulLength, buffa, ulLength, 0, 0 );

    CStringA newStr( buffa, ulLength );
    delete buffa;
    return newStr;
  }

  CStringW CStringW::Combine( const wtext format ... ) {
    CStringW sResult;
    wtext* array = (wtext*)(uint32( &format ) + 4);

    for( uint32 i = 0, o = 0; format[i] != 0; i++ ) {
      void32 varg = (void32)(((uint32)array) + o);

      if( format[i] == L'%' ) {
        switch( format[i + 1] ) {
          case FMT_SINT32:
          {
            sResult += CStringW( *(int32*)varg );
            o += sizeof( int32 );
            i++;
            break;
          };

          case FMT_SINT64:
          {
            sResult += CStringW( *(int64*)varg );
            o += sizeof( int64 );
            i++;
            break;
          };

          case FMT_UINT32:
          {
            sResult += CStringW( *(uint32*)varg );
            o += sizeof( uint32 );
            i++;
            break;
          };

          case FMT_UINT64:
          {
            sResult += CStringW( *(uint64*)varg );
            o += sizeof( uint64 );
            i++;
            break;
          };

          case FMT_HEX32:
          {
            sResult += CStringW::Hex32( *(int32*)varg );
            o += sizeof( int32 );
            i++;
            break;
          };

          case FMT_HEX64:
          {
            sResult += CStringW::Hex64( *(int64*)varg );
            o += sizeof( int64 );
            i++;
            break;
          };

          case FMT_REAL32:
          {
            sResult += CStringW( *(real64*)varg );
            o += sizeof( real64 );
            i++;
            break;
          };

          case FMT_REAL64:
          {
            sResult += CStringW( *(real64*)varg );
            o += sizeof( real64 );
            i++;
            break;
          };

          case FMT_BOOL:
          {
            bool v = *(int*)varg;
            sResult += CStringW( v ? L"true" : L"false" );
            o += sizeof( int );
            i++;
            break;
          };

          case FMT_CHAR:
          {
            sResult += CStringW( (wchar_t)*(int*)varg );
            o += sizeof( int );
            i++;
            break;
          };

          case FMT_BYTE:
          {
            sResult += CStringW( *(int*)varg );
            o += sizeof( int );
            i++;
            break;
          };

          case FMT_TEXT:
          {
            sResult += CStringW( *(wtext*)varg );
            o += sizeof( wtext );
            i++;
            break;
          };

          case FMT_STRING:
          {
            CStringBase& str = *(CStringBase*)varg;
            if( str.IsA() ) {
              sResult += str.ReinterpretToA().AToW();
              o += sizeof( CStringA );
            }
            else {
              sResult += str.ReinterpretToW();
              o += sizeof( CStringW );
            }
            i++;
            break;
          };

          case FMT_TITLE:
          {
            sResult += L"\n_____________________\n\n";
            i++;
            break;
          };

          case L'%':
          {
            //i++;
          };

          default:
          {
            sResult += L"%";
            i++;
            break;
          };
        }
      }
      else
        sResult += format[i];
    }
    return sResult;
  }

  CStringW::~CStringW() {
  }
#pragma endregion CStringWDefinition





  UTF8Char::UTF8Char() {
    Reset();
  }

  UTF8Char::UTF8Char( byte* memory ) {
    Reset();
    ToUTF8( memory );
  }

  UTF8Char::UTF8Char( uint uniw ) {
    Reset();
    ToUTF8( uniw );
  }

  UTF8Char::UTF8Char( wchar_t uni ) {
    Reset();
    ToUTF8( uni );
  }

  UTF8Char::UTF8Char( char ansi ) {
    Reset();
    ToUTF8( ansi );
  }

  void UTF8Char::Reset() {
    octet[0] = 0;
    octet[1] = 0;
    octet[2] = 0;
    octet[3] = 0;
  }

  void UTF8Char::ToUTF8( byte* memory ) {
    byte anchor = memory[0];

    // Определяем количество октетов
    // первого символа из аданного потока
    uint last = 0;
    for( uint i = 0; i < 4; i++ ) {
      if( (anchor >> (7 - i)) & 1 )
        last = i;
      else
        break;
    }

    // (Читаем октеты из потока в обратном порядке)
    for( uint i = 0; i <= last; i++ )
      octet[last - i] = memory[i];
  }

  void UTF8Char::ToUTF8( uint uniw ) {
    // Определяем является ли символ ansi
    // совместимым и заносим в первый октет 7 битов
    if( uniw < 128 ) {
      octet[0] = (byte)uniw;
      return;
    }

    // Переписываем биты 24-значного символа
    // в первые 6 бит каждого октета
    octet[0] = (uniw >> 0) & 0x3F;
    octet[1] = (uniw >> 6) & 0x3F;
    octet[2] = (uniw >> 12) & 0x3F;
    octet[3] = (uniw >> 18) & 0x3F;

    // Определяем количество октетов utf символа.
    // Каждому последнему биту октетов устанавливаем
    // положительное значение, а последнему октету
    // последние length битов.
    uint length = LengthByOctets();
    uint length_low = length - 1;
    for( uint i = 0; i <= length_low; i++ ) {
      octet[i] |= 1 << 7;
      octet[length - 1] |= 1 << (7 - i);
    }
  }

  UTF8Char::operator wchar_t() {
    return ToUnicode16();
  }

  wchar_t UTF8Char::ToUnicode16() {
    return ToUnicode24();
  }

  uint powui( uint x, uint rad ) {
    uint r = x;
    for( uint i = 0; i < rad; i++ )
      r *= x;

    return r;
  }

  uint UTF8Char::ToUnicode24() {
    uint length = Length();

    // Если количество октетов символа
    // равно одному, то извлекаем из него
    // 7 байт ASCII-совместимого символа
    if( length == 1 )
      return octet[0] & 0x7F;

    uint uniw = 0;
    uint length_low = length - 1;

    // Количество битов остатка, которое
    // необходимо взять из последнего октета
    byte dataSize = 0x7F;

    // Записываем значение октета каждые
    // 6 бит 24-разрядного символа
    for( uint i = 0; i < length; i++ ) {

      // Извлекаем остаток последнего октета за вычетом
      // 8 - (n + 1) битов-маркеров
      if( i == length_low )
        uniw |= (octet[i] & dataSize) << (6 * i);

      // Извлекаем по 6 битов
      // из каждого октета
      else {
        uniw |= (octet[i] & 0x3F) << (6 * i);

        // Вычитаем зарезервированные
        // биты последнего октета
        dataSize ^= 1 << (6 - i);
      }
    }

    return uniw;
  }

  byte UTF8Char::operator []( uint i ) {
    return octet[i];
  }

  uint UTF8Char::Length() {
    // Определяем количество октетов по
    // старшему биту каждого старшего байта
    if( (octet[3] >> 7 & 1) ) return 4;
    if( (octet[2] >> 7 & 1) ) return 3;
    if( (octet[1] >> 7 & 1) ) return 2;
    return 1;
  }

  uint UTF8Char::LengthByOctets() {
    // Определяем количество октетов, чтобы
    // в них убрались все переданные в них данные
    if( (octet[2] >> 4 & 1) || octet[3] ) return 4;
    if( (octet[1] >> 5 & 1) || octet[2] ) return 3;
    if( (octet[0] >> 6 & 1) || octet[1] ) return 2;
    return 1;
  }

  uint UTF8Char::CharSize( uint uni24 ) {
    if( uni24 <= 0xFF )
      return 1;

    if( uni24 <= 0xFFFF )
      return 2;

    return 3;
  }

  uint UTF8Char::CharSize( wchar_t uni16 ) {
    if( uni16 <= 0xFF )
      return 1;

    return 2;
  }

  void UTF8Char::Show() {
    uint length = Length();
    for( uint i = 3; i != Invalid; i-- )
      cmd << string( octet[i], 16 ) << " ";
    cmd << "  :" << length << endl;
  }

  ELineBreak GetLineBreakTypeA( const char* str ) {
    static const char crlf[] = { (char)0x0D, (char)0x0A, (char)0x00 };
    static const char cr[]   = { (char)0x0D, (char)0x00 };
    static const char lf[]   = { (char)0x0A, (char)0x00 };
    static const char eof[]  = { (char)0x00, (char)0x00 };

    if( str[0] == crlf[0] && str[1] == crlf[1] ) return LB_CRLF;
    if( str[0] == cr[0] )                        return LB_CR;
    if( str[0] == lf[0] )                        return LB_LF;
    if( str[0] == eof[0] )                       return LB_EOF;
    return LB_Unknown;
  }

  const char* GetLineBreakA( const ELineBreak& lb ) {
    static const char crlf[] = { (char)0x0D, (char)0x0A, (char)0x00 };
    static const char cr[]   = { (char)0x0D, (char)0x00 };
    static const char lf[]   = { (char)0x0A, (char)0x00 };
    static const char eof[]  = { (char)0x00, (char)0x00 };

    switch( lb ) {
      case LB_CRLF: return crlf;
      case LB_CR:   return cr;
      case LB_LF:   return lf;
      case LB_EOF:  return eof;
    }

    return crlf;
  }

  uint GetLineBreakLengthA( const ELineBreak& lb ) {
    switch( lb ) {
      case LB_CRLF: return 2;
      case LB_CR:
      case LB_LF:   return 1;
      case LB_EOF:  return 0;
    }

    return 0;
  }

  void UTF8StringConverter::ToUTF8( CStringA src, const uint32& srcCodePage ) {
    ToUTF8( src.AToW( srcCodePage ) );
  }

  void UTF8StringConverter::ToUTF8( CStringW src ) {
    for( uint i = 0; i <= src.Length(); i++ ) {
      UTF8Char utf = src[i];

      // Запись октетов в поток в обратном порядке
      uint last = utf.Length() - 1;
      for( uint j = 0; j <= last; j++ )
        utf8.Insert( utf[last - j] );
    }
  }

  void UTF8StringConverter::ToUnicode( CStringW& dst ) {
    dst.Clear();
    // Переписываем массив преобразуя
    // его символы в двухбайтовые
    for( uint i = 0; i < utf8.GetNum() - 1; ) {
      UTF8Char utf( &utf8[i] );
      wchar_t uni = utf;
      dst += uni;
      uint length = utf.Length();
      i += length;
    }
  }

  void UTF8StringConverter::ToAnsi( CStringA& dst, const uint32& srcCodePage ) {
    CStringW str;
    ToUnicode( str );
    dst = str.WToA( srcCodePage );
  }

  bool32 UTF8StringConverter::WriteToFile( CStringA fileName, const ELineBreak& lb ) {
    FILE* file = fopen( fileName, "wb+" );
    if( !file )
      return False;

    // Маркер Юникода UTF8
    uint BOM = 0xBFBBEF;
    fwrite( &BOM, 1, 3, file );

    // переписываем весь масив как есть,
    // если формат переноса строк нам не важен
    if( lb == LB_Unknown ) {
      fwrite( &utf8[0], 1, utf8.GetNum(), file );
      fclose( file );
      return False;
    }

    // Заданное завершение строк
    string lineBreak = GetLineBreakA( lb );

    // Левая и правая границы строки
    // многострочного текста
    uint left = 0;
    uint right = 0;

    // Переписываем текст построчно вставляя
    // между ними заданное завершение строк
    for( uint i = 0; i < utf8.GetNum(); i++ ) {
      char* array = (char*)&utf8[i];
      ELineBreak lb = GetLineBreakTypeA( array );

      // Найдено завершение строки
      if( lb != LB_Unknown ) {
        uint lblen = GetLineBreakLengthA( lb );
        right = i;
        fwrite( &utf8[left], 1, right - left, file );

        left = i + lblen;

        if( lblen ) {
          fwrite( lineBreak, 1, lineBreak.Length(), file );
          i += lblen - 1;
        }
      }
    }

    fclose( file );
    return True;
  }

  bool32 UTF8StringConverter::WriteToFile( CStringW fileName, const ELineBreak& lb ) {
    return WriteToFile( fileName.WToA() );
  }

  bool32 UTF8StringConverter::ReadFromFile( CStringA fileName ) {
    utf8.Clear();
    if( _access( fileName, None ) == NotFound )
      return False;

    FILE* file = fopen( fileName, "rb" );
    if( !file )
      return False;

    _setmode( _fileno( file ), _O_BINARY );
    fseek( file, 0, SEEK_END );
    uint length = ftell( file );
    fseek( file, 0, SEEK_SET );

    // Читаем BOM
    uint BOM = 0;
    fread( &BOM, 1, 3, file );
    fseek( file, 0, SEEK_SET );

    // Определяем формат файла
    uint offset = 0;
    if( (BOM & 0x00FFFF) == 0x00FEFF ) offset = 2;
    else if( (BOM & 0xFFFFFF) == 0xBFBBEF ) offset = 3;
    fseek( file, offset, SEEK_SET );

    // Резервируем место в массиве
    ArrayLocator<byte>& allocator = const_cast<ArrayLocator<byte>&>(utf8.GetArrayLocator());
    allocator.PrepareToReserveArray( length - offset + 1 );
    allocator.ActivateAllocatedMemory();

    // Пишем байты файла в массив
    bool32 readed = (fread( &utf8[0], 1, length - offset, file ) > 0);
    utf8[length] = 0;
    return readed;
  }

  bool32 UTF8StringConverter::ReadFromFile( CStringW fileName, const uint32& srcCodePage ) {
    return ReadFromFile( fileName.WToA( srcCodePage ) );
  }
}