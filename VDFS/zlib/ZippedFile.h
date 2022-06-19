#pragma once
#include "ZippedStream/ZippedAfx.h"

namespace Vdfs32 {
  extern ulong ThreadsCountDefault;
  extern ulong AllowMultithreading;

  class CFileVirtual_Zipped : public CFileVirtual {
    VDF_FRIEND_CLASSES
    ZippedStreamReader* ZippedReader;
    ulong ThreadsCount;
  public:

    CFileVirtual_Zipped( const CString& fullname, flow32 flow, const uint32& offset, const uint32& size, CVolume* parent );

    virtual uint32 Open();
    virtual uint32 Close();
    virtual uint32 Read( HBuffer buffer, const uint32& size );
    virtual uint32 Seek( const uint32& offset, const uint32& origin = SEEK_SET );
    virtual uint32 Tell();

  protected:
    virtual ~CFileVirtual_Zipped();
  };
}