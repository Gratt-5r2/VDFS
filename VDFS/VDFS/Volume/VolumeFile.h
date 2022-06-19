#pragma once


namespace VDFS {
  enum {
    FileEntryType_Directory = 0x00000000,
    FileEntryType_File      = 0x00000020,
    FileEntryFlag_Break     = 0x40000000,
    FileEntryFlag_Continue  = 0x80000000
  };


  struct FileEntry {
    char Name[64];
    union {
      uint Position;
      uint Index;
    };
    uint Size;
    uint Flags;
    uint Type;

    bool IsDirectory( const VolumeHeader& header ) const {
      return Position < header.DataStart + header.EntriesCount * sizeof( FileEntry );
    }

    bool IsFile( const VolumeHeader& header ) const {
      return Position >= header.DataStart + header.EntriesCount * sizeof( FileEntry );
    }

    bool IsLastFile() const {
      return (Flags & FileEntryFlag_Break) == FileEntryFlag_Break;
    }

    string GetName() const {
      return string( Name, 64 ).ShrinkFront();
    }
  };
}