#include <UnionAfx.h>
#include "VDFS\VDFS.h"


namespace VDFS {
  void IFILE::Open() {
    Lock();
  }


  IFILE* IFILE::Open( const string& fileName, const bool& lock ) {
    FILE* stream = Null;
    fopen_s( &stream, fileName, lock ? "rb+" : "rb" );
    if( !stream )
      return Null;

    return new IFILE( stream );
  }


  int IFILE::Close() {
    return Release();
  }


  int IFILE::SetPosition( const long& position, const long& seek ) {
    return fseek( Stream, position, seek );
  }


  long IFILE::GetPosition() {
    return ftell( Stream );
  }


  long IFILE::GetSize() {
    long pos = GetPosition();
    SetPosition( 0, SEEK_END );
    long length = GetPosition();
    SetPosition( pos, SEEK_SET );
    return length;
  }


  long IFILE::Read( void* memory, const long& length ) {
    return fread( memory, 1, length, Stream );
  }


  IFILE::IFILE( FILE* stream ) : VDFSObject() {
    Stream = stream;
  }


  IFILE::~IFILE() {
    if( Stream )
      fclose( Stream );
  }
}