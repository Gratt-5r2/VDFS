#pragma once

namespace VDFS {
struct VolumeHeader {
    struct Timestamp {
      uint Second : 5; // 60 / 32
      uint Minute : 6; // 60 / 64
      uint Hour   : 5; // 24 / 32
      uint Day    : 5; // 31 / 32
      uint Month  : 4; // 12 / 16
      uint Year   : 7; // 1980 + 128

      operator const uint32& () const {
        return *(uint32*)this;
      }

      const string ToString() const {
        string h = string( Hour        ).PadLeft( 2, '0' );
        string m = string( Minute      ).PadLeft( 2, '0' );
        string s = string( Second * 2  ).PadLeft( 2, '0' );
        string d = string( Day         ).PadLeft( 2, '0' );
        string M = string( Month       ).PadLeft( 2, '0' );
        string y = string( 1980 + Year );
        return string::Combine( "%s:%s:%s  %s.%s.%s", h, m, s, d, M, y );
      }
    };

    char Comment[256];
    char Label[16];
    uint EntriesCount;
    uint FilesCountOnly;
    Timestamp Timestamp;
    uint Sizeof;
    uint DataStart;
    uint Flags;
  };
}