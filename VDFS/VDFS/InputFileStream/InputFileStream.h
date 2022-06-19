#pragma once


namespace VDFS {
  struct IFILE : VDFSObject {
    FILE* Stream;

    virtual void Open();
    virtual int Close();
    virtual int SetPosition( const long& position, const long& seek = SEEK_SET );
    virtual long GetPosition();
    virtual long GetSize();
    virtual long Read( void* memory, const long& length );
    static IFILE* Open( const string& fileName, const bool& lock = false );

  private:
    IFILE( FILE* stream );
    virtual ~IFILE();
  };
}