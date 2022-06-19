#include <UnionAfx.h>
#include "VDFS\VDFS.h"

#pragma comment( lib, "libogg_static.lib" )
#pragma comment( lib, "libvorbis_static.lib" )
#pragma comment( lib, "libvorbisfile_static.lib" )

namespace VDFS {
  FileStreamOgg::FileStreamOgg( File* file, IFILE* handle ) : FileStream( file, handle ) {
    OggS = OpenOgg();
  }


  bool FileStreamOgg::OpenOgg() {
    FileStream::SetPosition(0);

    char sig[4];
    if( FileStream::Read( sig, 4 ) == 4 && memcmp( sig, "OggS", 4 ) == 0 ) {
      FileStream::SetPosition( 0 );
      InitCallbacks();
      InitWave();
      if( InitStream() )
        return true;
    }

    FileStream::SetPosition( 0 );
    return NULL;
  }


  void FileStreamOgg::InitCallbacks() {
    ZeroMemory( &Vorbis, sizeof( OggVorbis_File ) );
    Callbacks.read_func  = VorbisRead;
    Callbacks.seek_func  = VorbisSeek;
    Callbacks.close_func = VorbisClose;
    Callbacks.tell_func  = VorbisTell;
  }


  void FileStreamOgg::InitWave() {
    ZeroMemory( &Wave, sizeof( WaveFile ) );
    memcpy( &Wave.RiffSig,   "RIFF", 4 );
    memcpy( &Wave.WaveSig,   "WAVE", 4 );
    memcpy( &Wave.FmtChunk,  "fmt ", 4 );
    memcpy( &Wave.DataChunk, "data", 4 );

    Wave.FmtSize              = sizeof( WaveFormat );
    Wave.Format.FormatTag     = 1;
    Wave.Format.BitsPerSample = 16;
  }


  bool FileStreamOgg::InitStream() {
    if( ov_open_callbacks( this, &Vorbis, Null, 0, Callbacks ) == 0 && ov_streams( &Vorbis ) == 1 ) {
      vorbis_info* info          = ov_info( &Vorbis, -1 );
      Wave.Format.Channels       = info->channels;
      Wave.Format.SamplesPerSec  = info->rate;
      Wave.Format.BlockAlign     = Wave.Format.Channels * Wave.Format.BitsPerSample / 8;
      Wave.Format.AvgBytesPerSec = Wave.Format.SamplesPerSec * Wave.Format.BlockAlign;
      Wave.DataSize              = (uint)ov_pcm_total( &Vorbis, -1 ) * Wave.Format.BlockAlign;
      Wave.RiffSize              = Wave.DataSize + sizeof( WaveFile ) - 8;
      Position                   = 0;
      return true;
    }

    return false;
  }


  long FileStreamOgg::GetSize() const {
    if( !OggS )
      return FileStream::GetSize();

    return Wave.RiffSize + 8;
  }


  long FileStreamOgg::GetPosition() const {
    if( !OggS )
      return FileStream::GetPosition();

    return Position;
  }


  int FileStreamOgg::SetPosition( const long& position, const uint& origin ) {
    if( !OggS )
      return FileStream::SetPosition( position, origin );

    if( position > GetSize() )
      return False;

    Position = position;
    return True;
  }


  long FileStreamOgg::Read( void* data, const uint& length ) {
    if( !OggS )
      return FileStream::Read( data, length );

    char* Buffer = (char*)data;
    ulong Size = length;
    if( Position < sizeof( WaveFile ) ) {
      size_t ToRead = sizeof( WaveFile ) - Position;
      if( length < ToRead )
        ToRead = length;

      memcpy( data, &Wave, ToRead );

      Buffer += ToRead;
      Size -= ToRead;
      Position += ToRead;
    }

    if( Size ) {
      uint Part = (Position - sizeof( WaveFile )) % Wave.Format.BlockAlign;
      if( Part ) {
        Buffer = new char[length + Part];
        Position -= Part;
        Size += Part;
      }

      if( ov_pcm_seek( &Vorbis, (Position - sizeof( WaveFile )) / Wave.Format.BlockAlign ) ) {
        if( Part )
          delete[] Buffer;
        return 0;
      }

      int default_stream = -1;

      uint Readed = 0;
      while( Readed < Size ) {
        int res = ov_read( &Vorbis, &Buffer[Readed], Size - Readed, 0, 2, 1, &default_stream );
        if( res <= 0 ) {
          if( Part ) {
            memcpy( data, &Buffer[Part], Readed - Part );
            delete[] Buffer;
          }
          return (length - (Size - Part)) + Readed;
        }
        Readed += res;
        Position += res;
      }

      if( Part ) {
        memcpy( &((byte*)data)[length - Size], &Buffer[Part], length );
        delete[] Buffer;
      }
    }
    return length;
  }


  void FileStreamOgg::Close( void ) {
    if( OggS ) {
      if( Vorbis.datasource )
        ov_clear( &Vorbis );
      
      OggS = false;
    }

    FileStream::Close();
  }


  FileStreamOgg::~FileStreamOgg( void ) {
    Close();
  }


  size_t VorbisRead( void* ptr, size_t size, size_t count, void* streamPtr ) { // TODO
    FileStream* stream = (FileStream*)streamPtr;
    size_t length = size * count;

    size_t lengthMax = (size_t)(stream->FileStream::GetSize() - stream->FileStream::GetPosition());
    if( length > lengthMax ) {
      count = lengthMax / size;
      length = size * count;
    }

    if( stream->FileStream::Read( ptr, length ) == length )
      return count;

    return 0;
  }


  int VorbisSeek( void* streamPtr, ogg_int64_t position, int origin ) {
    FileStream* stream = (FileStream*)streamPtr;
    switch( origin ) {
    case SEEK_SET:
      stream->FileStream::SetPosition( (ulong)position );
      break;
    case SEEK_CUR:
      stream->FileStream::SetPosition( stream->FileStream::GetPosition() + (long)position );
      break;
    case SEEK_END:
      stream->FileStream::SetPosition( stream->FileStream::GetSize() - (long)position - 1 );
      break;
    }

    return (int)stream->FileStream::GetPosition();
  }


  int VorbisClose( void* streamPtr ) {
    return 0;
  }


  long VorbisTell( void* streamPtr ) {
    FileStream* stream = (FileStream*)streamPtr;
    return stream->FileStream::GetPosition();
  }
}