#pragma once

#include "ZippedBuffer.h"



class ZSTREAMAPI ZippedBlockBase {
  friend class ZippedStreamBase;
  friend class ZippedStreamReader;
  friend class ZippedStreamWriter;
  friend class ZippedBlockStack;
protected:
  struct {
    ulong LengthSource;
    ulong LengthCompressed;
    ulong BlockSize;
  }
  Header;

  ulong Position;
  ZippedBuffer Buffer;
  FILE* BaseStream;
  ulong BasePosition;

public:
  ZippedBlockBase( FILE* baseStream, const ulong& position );
  virtual long Tell();
  virtual long Seek( const long& offset, const uint& origin = SEEK_SET );
  virtual bool Compress( const bool& clearSource = true );
  virtual bool Decompress( const bool& clearCompressed = true );
  virtual bool IsCompressed();
  virtual bool IsDecompressed();
  virtual void CommitHeader() = 0;
  virtual void CommitData() = 0;
  virtual void SetBlockSize( const ulong& length ) = 0;
  virtual ulong GetFileSize() = 0;
  virtual ulong Read( byte* buffer, const ulong& length ) = 0;
  virtual ulong Write( byte* buffer, const ulong& length ) = 0;
  virtual bool EndOfBlock() = 0;
  virtual bool CacheIn( const ulong& position = -1 ) = 0;
  virtual void CacheOut() = 0;
  virtual bool Cached() = 0;
  virtual ~ZippedBlockBase();
};



class ZSTREAMAPI ZippedBlockReader : public ZippedBlockBase {
private:
  friend class ZippedBlockReaderCache;
  bool IsCached;

public:
  ZippedBlockReader( FILE* baseStream, const ulong& position );
  virtual bool Decompress( const bool& clearCompressed = true );
  virtual void CommitHeader();
  virtual void CommitData();
  virtual ulong GetFileSize();
  virtual void SetBlockSize( const ulong& length );
  virtual ulong Read( byte* buffer, const ulong& length );
  virtual ulong Write( byte* buffer, const ulong& length );
  virtual bool EndOfBlock();
  virtual bool CacheIn( const ulong& position = -1 );
  virtual void CacheOut();
  virtual bool Cached();
  virtual ~ZippedBlockReader();
};



class ZSTREAMAPI ZippedBlockWriter : public ZippedBlockBase {
public:
  ZippedBlockWriter( FILE* baseStream, const ulong& position = 0 );
  virtual void CommitHeader();
  virtual void CommitData();
  virtual ulong GetFileSize();
  virtual void SetBlockSize( const ulong& length );
  virtual ulong Read( byte* buffer, const ulong& length );
  virtual ulong Write( byte* buffer, const ulong& length );
  virtual bool EndOfBlock();
  virtual bool CacheIn( const ulong& position = -1 );
  virtual void CacheOut();
  virtual bool Cached();
};



const uint StackSizeMax = 1024;

class ZSTREAMAPI ZippedBlockReaderCache {
public:
  friend class ZippedBlockReader;
  ulong CacheSizeMax;
  ulong CacheSize;
  ZippedBlockReader* Stack[StackSizeMax];
  uint StackSize;

  void Move( const uint& toID, const uint& fromID, const uint& count );
  void Push( ZippedBlockReader* block );
  void Pop( ZippedBlockReader* block );
  ZippedBlockReaderCache();

public:
  uint GetBlocksCount();
  bool CacheIn( ZippedBlockReader* block );
  void CacheOut( ZippedBlockReader* block );
  void CacheInvalidate( ZippedBlockReader* block );
  void CacheOutLast();
  void CacheReduce();
  void SetMemoryLimit( const ulong& size );
  ZippedBlockReader* GetTopBlock();
  static ZippedBlockReaderCache* GetInstance();

  void ShowDebug();
};
