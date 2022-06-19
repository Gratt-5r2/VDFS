#include "Vdfs32.h"


#ifdef ALLOWVORBIS
#ifdef VDFS32G
#pragma comment( lib, "libogg_static.lib" )
#pragma comment( lib, "libvorbis_static.lib" )
#pragma comment( lib, "libvorbisfile_static.lib" )
#endif


namespace Vdfs32 {
  uint OggVorbis::UnpackBuffer( char* buffer, uint size ) {
    char* Buffer = (char*)buffer;
    ulong32 Size = size;
    uint32& Offset = ogg.Cursor;

    if( Offset < sizeof( WaveFile ) ) {
      size_t ToRead = sizeof( WaveFile ) - Offset;
      if( size < ToRead )
        ToRead = size;

      memcpy( buffer, &ogg.Wave, ToRead );

      Buffer += ToRead;
      Size -= ToRead;
      Offset += ToRead;
    }

    if( Size ) {
      uint32 Part = (Offset - sizeof( WaveFile )) % ogg.Wave.Format.BlockAlign;
      if( Part ) {
        Buffer = new char[size + Part];
        Offset -= Part;
        Size += Part;
      }

      if( ov_pcm_seek( &ogg.Vorbis, (Offset - sizeof( WaveFile )) / ogg.Wave.Format.BlockAlign ) ) {
        if( Part )
          delete[] Buffer;
        return 0;
      }

      int default_stream = -1;

      uint32 Readed = 0;
      while( Readed < Size ) {
        if( EventDeleting.IsOpened() )
          return 0;

        int res = ov_read( &ogg.Vorbis, &Buffer[Readed], Size - Readed, 0, 2, 1, &default_stream );
        if( res <= 0 ) {
          if( Part ) {
            memcpy( &buffer[Part + Readed], &Buffer[Part], Readed - Part );
            delete[] Buffer;
          }

          return (size - (Size - Part)) + Readed;
        }

        Readed += res;
        Offset += res;
      }

      if( Part ) {
        memcpy( &((byte*)buffer)[size - Size], &Buffer[Part], size );
        delete[] Buffer;
      }
    }

    return size;
  }

  void OggVorbis::Unpack() {
    EventDeleting.Wait();
    // Sleep( 5000 );

    uint size = ogg.Wave.Header.RiffSize + 8;
    ogg.Unpacked = new byte[size];
    if( ogg.Unpacked != Null )
      UnpackBuffer( (char*)ogg.Unpacked, size );

    if( EventDeleting.IsOpened() ) {
      ClearPacked();
    }
  }

  void OggVorbis::ClearPacked() {
    if( ogg.Unpacked ) {
      long dataSize = shi_msize( ogg.Unpacked );
      delete[] ogg.Unpacked;
      ogg.Unpacked = Null;

      cmd << Col16( CMD_GREEN ) << "Vdfs: clear unpacked OGG data " << dataSize / 1024 / 1024 << " MB" << Col16() << endl;
      if( EventDeleting.IsOpened() )
        EventDeleting.Close();
    }
  }

  void OggVorbis::Pack() {
    if( !EventDeleting.IsOpened() ) {
      if( EventUnpacking.IsOpened() ) {
        EventDeleting.Open();
        return;
      }

      ClearPacked();
    }
  }

#pragma region VirtualOggFlow
  CFileVirtual_Ogg::CFileVirtual_Ogg( const CString& fullname, flow32 flow, const uint32& offset, const uint32& size, CVolume* parent )
    : CFileVirtual( fullname, flow, offset, size, parent ) {

    ogg.Callbacks.read_func  = VorbisRead;
    ogg.Callbacks.seek_func  = VorbisSeek;
    ogg.Callbacks.tell_func  = VorbisTell;
    ogg.Callbacks.close_func = VorbisClose;

    long oldFilePos = ftell( flow );
    
    fseek( flow, oldFilePos, SEEK_SET );
  }

  uint32 CFileVirtual_Ogg::Open() {
    {
      FILE_MASTERMUTEX;
      EventUnpacking.Wait();
      EventDeleting.Wait();
      OpenPassive();
    }
    return CFileVirtual::Open();
  }

  bool CFileVirtual_Ogg::CheckSoundFormatOgg() {
    CFileVirtual::Seek( 0 );

    bool IsOpenCallbacks = !ov_open_callbacks( (void*)this, &ogg.Vorbis, Null, 0, ogg.Callbacks );
    if( !IsOpenCallbacks )
      return false;

    vorbis_info* info = ov_info( &ogg.Vorbis, -1 );
    ulong32      pcm  = ov_pcm_total( &ogg.Vorbis, -1 );
    
    ogg.Wave.Format.Channels        = info->channels;
    ogg.Wave.Format.SamplesPerSec   = info->rate;
    ogg.Wave.Format.BlockAlign      = ogg.Wave.Format.Channels * ogg.Wave.Format.BitsPerSample / 8;
    ogg.Wave.Format.AvgBytesPerSec  = ogg.Wave.Format.SamplesPerSec * ogg.Wave.Format.BlockAlign;
    ogg.Wave.dataChunk.ChunkSize    = pcm * ogg.Wave.Format.BlockAlign;
    ogg.Wave.Header.RiffSize        = ogg.Wave.dataChunk.ChunkSize + sizeof( WaveFile ) - 8;
    return true;
  }

  bool CFileVirtual_Ogg::IsOgg() {
    FILE_MASTERMUTEX;
    return ogg.IsVorbis;
  }

  uint32 CFileVirtual_Ogg::Close() {
    CFileVirtual::Close();
    FILE_MASTERMUTEX;
    ClosePassive();
    return True;
  }

  void CFileVirtual_Ogg::OpenPassive() {
    char Tag[4];
    VdfsSetOffset( flFile, meta.ulOffset );
    VdfsRead( flFile, &Tag[0], 4 );

    if( CString( Tag, 4 ).StartWith( "Ogg" ) )
      ogg.IsVorbis = CheckSoundFormatOgg();

    Seek( 0 );
  }

  void CFileVirtual_Ogg::ClosePassive() {
    if( ogg.IsVorbis )
      ov_clear( &ogg.Vorbis );

    memset( &ogg.Vorbis, 0, sizeof( OggVorbis_File ) );
  }

  uint32 CFileVirtual_Ogg::Read( HBuffer buffer, const uint32& size ) {
    if( !IsOgg() )
      return CFileVirtual::Read( buffer, size );

    FILE_MASTERMUTEX;
    if( ogg.Unpacked ) {
      cmd << Col16( CMD_GREEN ) << "Vdfs: copy OGG data from virtual unpacked memory " << Name() << Col16() << endl;
      memcpy( buffer, ogg.Unpacked + ogg.Cursor, size );
      ogg.Cursor += size;
      return size;
    }

    char*   Buffer = (char*)buffer;
    ulong32 Size   = size;
    uint32& Offset = ogg.Cursor;

    if( Offset < sizeof( WaveFile ) ) {
      size_t ToRead = sizeof( WaveFile ) - Offset;
      if( size < ToRead )
        ToRead = size;

      memcpy( buffer, &ogg.Wave, ToRead );

      Buffer += ToRead;
      Size   -= ToRead;
      Offset += ToRead;
    }

    if( Size ) {
      uint32 Part = ( Offset - sizeof( WaveFile ) ) % ogg.Wave.Format.BlockAlign;
      if( Part ) {
        Buffer   = new char[size + Part];
        Offset -= Part;
        Size   += Part;
      }

      if( ov_pcm_seek( &ogg.Vorbis, ( Offset - sizeof( WaveFile ) ) / ogg.Wave.Format.BlockAlign ) ) {
        if( Part )
          delete[] Buffer;
        return 0;
      }

      int default_stream = -1;

      uint32 Readed = 0;
      while( Readed < Size ) {
        int res = ov_read( &ogg.Vorbis, &Buffer[Readed], Size - Readed, 0, 2, 1, &default_stream );
        if( res <= 0 ) {
          if( Part ) {
            memcpy( &buffer[Part + Readed], &Buffer[Part], Readed - Part );
            delete[] Buffer;
          }
          return ( size - ( Size - Part ) ) + Readed;
        }
        Readed += res;
        Offset += res;
      }

      if( Part ) {
        memcpy( &( (byte*)buffer )[size - Size], &Buffer[Part], size );
        delete[] Buffer;
      }
    }
    return size;
  }

  uint32 CFileVirtual_Ogg::Seek( const uint32& offset, const uint32& origin ) {
    if( !IsOgg() )
      return CFileVirtual::Seek( offset, origin );

    FILE_MASTERMUTEX;
    uint32 cursor = ogg.Cursor;
    switch( origin ) {
      case SEEK_SET: 
        cursor = offset;
        break;

      case SEEK_CUR: 
        cursor += offset;
        break;

      case SEEK_END:
        cursor = Sizeof() - offset - 1;
        break;
    }

    if( cursor >= Sizeof() )
      return Invalid;

    ogg.Cursor = cursor;
    return ogg.Cursor;
  }

  uint32 CFileVirtual_Ogg::Tell() {
    if( !IsOgg() )
      return CFileVirtual::Tell();

    FILE_MASTERMUTEX;
    return ogg.Cursor;
  }

  bool32 CFileVirtual_Ogg::IsVirtual() {
    return CFileVirtual::IsVirtual();
  }

  uint32 CFileVirtual_Ogg::Sizeof() {
    if( !IsOgg() )
      return CFileVirtual::Sizeof();

    FILE_MASTERMUTEX;
    return ogg.Wave.Header.RiffSize + 8;
  }

  bool32 CFileVirtual_Ogg::IsEncrypted() {
    return CFileVirtual::IsEncrypted();
  }

  void CFileVirtual_Ogg::UnpackAsync() {
    OpenPassive();
    if( !IsOgg() ) {
      ClosePassive();
      return;
    }

    cmd << Col16( CMD_GREEN ) << "Vdfs: unpack OGG data started" << Col16() << endl;
    Thread thread( &CFileVirtual_Ogg::UnpackInstanceAsync );
    thread.Detach( this );
  }

  void CFileVirtual_Ogg::UnpackInstanceAsync( CFileVirtual_Ogg* file ) {
    if( file->EventUnpacking.IsOpened() )
      return;

    file->EventUnpacking.Open();
    file->Unpack();
    file->ClosePassive();
    file->EventUnpacking.Close();
  }

  CFileVirtual_Ogg::~CFileVirtual_Ogg() {

  }


  uint32 CFileVirtual_Ogg::VorbisRead( void32 buffer, uint32 size, uint32 length, void32 file ) {
    return ((CFileVirtual_Ogg*)file)->CFileVirtual::Read( (HBuffer)buffer, size * length );
  }

  int32 CFileVirtual_Ogg::VorbisSeek( void32 file, int64  offset, int32  whence ) {
    return ((CFileVirtual_Ogg*)file)->CFileVirtual::Seek( offset, whence );
  }

  long32 CFileVirtual_Ogg::VorbisTell( void32 file ) {
    return ((CFileVirtual_Ogg*)file)->CFileVirtual::Tell();
  }

  int32 CFileVirtual_Ogg::VorbisClose( void32 file ) {
    return 0;
  }
#pragma endregion




#pragma region PhysicalOggFlow
  CFilePhysical_Ogg::CFilePhysical_Ogg( const CString& baseDirectory, const CString& fileName )
    : CFilePhysical( baseDirectory, fileName ) {

    ogg.Callbacks.read_func  = VorbisRead;
    ogg.Callbacks.seek_func  = VorbisSeek;
    ogg.Callbacks.tell_func  = VorbisTell;
    ogg.Callbacks.close_func = VorbisClose;
  }

  uint32 CFilePhysical_Ogg::Open() {
    {
      FILE_MASTERMUTEX;
      EventUnpacking.Wait();
      EventDeleting.Wait();
      CFilePhysical::Open();
      OpenPassive();
    }
    return True;
  }

  bool CFilePhysical_Ogg::CheckSoundFormatOgg() {
    CFilePhysical::Seek( 0 );

    FILE_MASTERMUTEX;
    bool IsOpenCallbacks = !ov_open_callbacks( (void*)this, &ogg.Vorbis, Null, 0, ogg.Callbacks );
    if( !IsOpenCallbacks ) {
      return false;
    }

    vorbis_info* info = ov_info( &ogg.Vorbis, -1 );
    ulong32      pcm  = ov_pcm_total( &ogg.Vorbis, -1 );

    ogg.Wave.Format.Channels        = info->channels;
    ogg.Wave.Format.SamplesPerSec   = info->rate;
    ogg.Wave.Format.BlockAlign      = ogg.Wave.Format.Channels * ogg.Wave.Format.BitsPerSample / 8;
    ogg.Wave.Format.AvgBytesPerSec  = ogg.Wave.Format.SamplesPerSec * ogg.Wave.Format.BlockAlign;
    ogg.Wave.dataChunk.ChunkSize    = pcm * ogg.Wave.Format.BlockAlign;
    ogg.Wave.Header.RiffSize        = ogg.Wave.dataChunk.ChunkSize + sizeof( WaveFile ) - 8;

    return true;
  }

  bool CFilePhysical_Ogg::IsOgg() {
    FILE_MASTERMUTEX;
    return ogg.IsVorbis;
  }

  uint32 CFilePhysical_Ogg::Close() {
    CFilePhysical::Close();
    FILE_MASTERMUTEX;
    ClosePassive();
    return True;
  }

  void CFilePhysical_Ogg::OpenPassive() {
    if( flFile == Null ) {
      string fullFileName = GameRoot() + sFullName;
      VdfsOpenFile( flFile, fullFileName );
      meta.ulCursor = 0;
      VdfsEndOffset( flFile, 0 );
      meta.ulSizeof = (uint32)VdfsGetOffset( flFile );
      VdfsSetOffset( flFile, 0 );
      ogg.IsPassive = True;

    }

    char Tag[4];
    VdfsSetOffset( flFile, 0 );
    VdfsRead( flFile, &Tag[0], 4 );

    if( CString( Tag, 4 ).StartWith( "Ogg" ) )
      ogg.IsVorbis = CheckSoundFormatOgg();

    Seek( 0 );
  }

  void CFilePhysical_Ogg::ClosePassive() {
    if( ogg.IsVorbis )
      ov_clear( &ogg.Vorbis );

    memset( &ogg.Vorbis, 0, sizeof( OggVorbis_File ) );

    if( ogg.IsPassive ) {
      VdfsCloseFile( flFile );
      ogg.IsPassive = False;
    }
  }

  uint32 CFilePhysical_Ogg::Read( HBuffer buffer, const uint32& size ) {
    
    if( !IsOgg() )
      return CFilePhysical::Read( buffer, size );

    FILE_MASTERMUTEX;
    if( ogg.Unpacked ) {
      cmd << Col16( CMD_GREEN ) << "Vdfs: copy OGG data from physical unpacked memory " << Name() << Col16() << endl;
      memcpy( buffer, ogg.Unpacked + ogg.Cursor, size );
      ogg.Cursor += size;
      return size;
    }

    char*   Buffer = (char*)buffer;
    ulong32 Size   = size;
    uint32& Offset = ogg.Cursor;

    if( Offset < sizeof( WaveFile ) ) {
      size_t ToRead = sizeof( WaveFile ) - Offset;
      if( size < ToRead )
        ToRead = size;

      memcpy( buffer, &ogg.Wave, ToRead );

      Buffer += ToRead;
      Size   -= ToRead;
      Offset += ToRead;
    }

    if( Size ) {
      uint32 Part = ( Offset - sizeof( WaveFile ) ) % ogg.Wave.Format.BlockAlign;
      if( Part ) {
        Buffer   = new char[size + Part];
        Offset -= Part;
        Size   += Part;
      }

      if( ov_pcm_seek( &ogg.Vorbis, ( Offset - sizeof( WaveFile ) ) / ogg.Wave.Format.BlockAlign ) ) {
        if( Part )
          delete[] Buffer;
        return 0;
      }

      int default_stream = -1;

      uint32 Readed = 0;
      while( Readed < Size ) {
        int res = ov_read( &ogg.Vorbis, &Buffer[Readed], Size - Readed, 0, 2, 1, &default_stream );
        if( res <= 0 ) {
          if( Part ) {
            memcpy( &buffer[Part + Readed], &Buffer[Part], Readed - Part );
            delete[] Buffer;
          }
          return ( size - ( Size - Part ) ) + Readed;
        }
        Readed += res;
        Offset += res;
      }

      if( Part ) {
        memcpy( &( (byte*)buffer )[size - Size], &Buffer[Part], size );
        delete[] Buffer;
      }
    }
    return size;
  }

  uint32 CFilePhysical_Ogg::Seek( const uint32& offset, const uint32& origin ) {
    if( !IsOgg() )
      return CFilePhysical::Seek( offset, origin );

    FILE_MASTERMUTEX;
    uint32 cursor = ogg.Cursor;
    switch( origin ) {
      case SEEK_SET: 
        cursor = offset;
        break;

      case SEEK_CUR: 
        cursor += offset;
        break;

      case SEEK_END:
        cursor = Sizeof() - offset - 1;
        break;
    }

    if( cursor >= Sizeof() )
      return Invalid;

    ogg.Cursor = cursor;
    return ogg.Cursor;
  }

  uint32 CFilePhysical_Ogg::Tell() {
    if( !IsOgg() )
      return CFilePhysical::Tell();

    FILE_MASTERMUTEX;
    return ogg.Cursor;
  }

  bool32 CFilePhysical_Ogg::IsVirtual() {
    return CFilePhysical::IsVirtual();
  }

  uint32 CFilePhysical_Ogg::Sizeof() {
    if( !IsOgg() )
      return CFilePhysical::Sizeof();

    FILE_MASTERMUTEX;
    return ogg.Wave.Header.RiffSize + 8;
  }

  bool32 CFilePhysical_Ogg::IsEncrypted() {
    return CFilePhysical::IsEncrypted();
  }

  void CFilePhysical_Ogg::UnpackAsync() {
    OpenPassive();
    if( !IsOgg() ) {
      ClosePassive();
      return;
    }

    cmd << Col16( CMD_GREEN ) << "Vdfs: unpack OGG data started" << Col16() << endl;
    Thread thread( &CFilePhysical_Ogg::UnpackInstanceAsync );
    thread.Detach( this );
  }

  void CFilePhysical_Ogg::UnpackInstanceAsync( CFilePhysical_Ogg* file ) {
    if( file->EventUnpacking.IsOpened() )
      return;

    file->EventUnpacking.Open();
    file->Unpack();
    file->ClosePassive();
    file->EventUnpacking.Close();
  }

  CFilePhysical_Ogg::~CFilePhysical_Ogg() {

  }


  uint32 CFilePhysical_Ogg::VorbisRead( void32 buffer, uint32 size, uint32 length, void32 file ) {
    return ((CFilePhysical_Ogg*)file)->CFilePhysical::Read( (HBuffer)buffer, size * length );
  }

  int32 CFilePhysical_Ogg::VorbisSeek( void32 file, int64  offset, int32  whence ) {
    return ((CFilePhysical_Ogg*)file)->CFilePhysical::Seek( offset, whence );
  }

  long32 CFilePhysical_Ogg::VorbisTell( void32 file ) {
    return ((CFilePhysical_Ogg*)file)->CFilePhysical::Tell();
  }

  int32 CFilePhysical_Ogg::VorbisClose( void32 file ) {
    return 0;
  }
#pragma endregion
}
#endif