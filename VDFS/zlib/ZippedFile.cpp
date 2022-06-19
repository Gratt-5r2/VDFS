#include "Vdfs32.h"

#ifdef ALLOWZIPPED
namespace Vdfs32 {
  ulong ThreadsCountDefault = 1;
  ulong AllowMultithreading = 0;

  CFileVirtual_Zipped::CFileVirtual_Zipped( const CString& fullname, flow32 flow, const uint32& offset, const uint32& size, CVolume* parent ) :
    CFileVirtual( fullname, flow, offset, size, parent ) {
    ZippedReader = Null;
  }

  uint32 CFileVirtual_Zipped::Open() {
    FILE_MASTERMUTEX;
    if( !AllowMultithreading || Name().EndWith( ".ZEN" ) )
      ZIPPED_THREADS_COUNT = 1;
    else
      ZIPPED_THREADS_COUNT = ThreadsCountDefault;

    ZippedReader = new ZippedStreamReader( flFile, meta.ulOffset );

    ZippedReader->Seek( meta.ulCursor );
    return Success;
  }

  uint32 CFileVirtual_Zipped::Close() {
    FILE_MASTERMUTEX;
    ZippedReader->Close( false );
    ZippedReader = Null;
    return Success;
  }

  uint32 CFileVirtual_Zipped::Read( HBuffer buffer, const uint32& size ) {
    int readed = ZippedReader->Read( (byte*)buffer, size );
    return readed;
  }

  uint32 CFileVirtual_Zipped::Seek( const uint32& offset, const uint32& origin ) {
    return ZippedReader->Seek( offset, origin );
  }

  uint32 CFileVirtual_Zipped::Tell() {
    return ZippedReader->Tell();
  }

  CFileVirtual_Zipped::~CFileVirtual_Zipped() {

  }
}
#endif