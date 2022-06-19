#pragma once
#include <Vorbis/Include/codec.h>
#include <Vorbis/Include/vorbisenc.h>
#include <Vorbis/Include/vorbisfile.h>

namespace VDFS {
#pragma pack(push, 1)
  struct WaveFormat {
    ushort FormatTag;
    ushort Channels;
    uint SamplesPerSec;
    uint AvgBytesPerSec;
    ushort BlockAlign;
    ushort BitsPerSample;
  };


  struct WaveFile {
    char RiffSig[4];
    uint RiffSize;
    char WaveSig[4];
    char FmtChunk[4];
    uint FmtSize;
    WaveFormat Format;
    char DataChunk[4];
    uint DataSize;
  };
#pragma pack(pop)


  extern size_t VorbisRead( void* ptr, size_t size, size_t nmemb, void* datasource );
  extern int VorbisSeek( void* datasource, ogg_int64_t offset, int whence );
  extern int VorbisClose( void* datasource );
  extern long VorbisTell( void* datasource );


  class FileStreamOgg : public FileStream {
    friend class File;

  protected:
    OggVorbis_File Vorbis;
    ov_callbacks Callbacks;
    WaveFile Wave;
    long Position;
    bool OggS;

    bool OpenOgg();
    void InitCallbacks();
    void InitWave();
    bool InitStream();
    FileStreamOgg( File* file, IFILE* handle );

  public:
    virtual long GetSize() const;
    virtual long GetPosition() const;
    virtual int SetPosition( const long& position, const uint& origin = SEEK_SET );
    virtual long Read( void* data, const uint& length );
    virtual void Close();
    virtual ~FileStreamOgg( void );
  };
}