#pragma once
#include "Include/codec.h"
#include "Include/vorbisenc.h"
#include "Include/vorbisfile.h"

namespace Vdfs32 {

#pragma pack(push, 1)

  struct WaveFileHeader {
    byte    RIFF_SIG[4];
    ulong32 RiffSize;
    byte    WAVE_SIG[4];
  };

  struct RiffChunk {
    byte    CHUNK_NAME[4];
    ulong32 ChunkSize;
  };

  struct WaveFormatEx {
    uint16 FormatTag;
    uint16 Channels;
    uint32 SamplesPerSec;
    uint32 AvgBytesPerSec;
    uint16 BlockAlign;
    uint16 BitsPerSample;
  };

  struct WaveFile {
    WaveFileHeader Header;
    RiffChunk      fmtChunk;
    WaveFormatEx   Format;
    RiffChunk      dataChunk;
  };

#pragma pack(pop)

  struct OggVorbis {
    Event EventUnpacking;
    Event EventDeleting;

    struct {
      OggVorbis_File Vorbis;
      ov_callbacks   Callbacks;
      WaveFile       Wave;
      uint32         Cursor;
      bool32         IsVorbis;
      bool32         IsPassive;
      byte*          Unpacked;
    } 
    ogg;

    OggVorbis() {
      ogg.IsVorbis = False;
      ogg.IsPassive = False;
      ogg.Unpacked = Null;
      memset( &ogg.Vorbis, 0, sizeof( OggVorbis_File ) );
      memset( &ogg.Wave, 0, sizeof( WaveFile ) );

      memcpy( &ogg.Wave.Header.RIFF_SIG,      "RIFF", 4 );
      memcpy( &ogg.Wave.Header.WAVE_SIG,      "WAVE", 4 );
      memcpy( &ogg.Wave.fmtChunk.CHUNK_NAME,  "fmt ", 4 );
      memcpy( &ogg.Wave.dataChunk.CHUNK_NAME, "data", 4 );

      ogg.Wave.fmtChunk.ChunkSize   = sizeof( WaveFormatEx );
      ogg.Wave.Format.FormatTag     = 1;
      ogg.Wave.Format.BitsPerSample = 16;
    }

    void Unpack();
    void Pack();
  protected:

    uint UnpackBuffer( char* buffer, uint size );
    void ClearPacked();
  };
} 