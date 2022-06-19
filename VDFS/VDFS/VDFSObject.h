#pragma once

namespace VDFS {
  class VDFSObject {
  protected:
    int ReferencesCount;
    VDFSObject() {
      ReferencesCount = 1;
    }
  public:
    int Lock() {
      return ++ReferencesCount;
    }

    int Release() {
      if( --ReferencesCount <= 0 ) {
        delete this;
        return 0;
      }

      return ReferencesCount;
    }

    const int GetReferencesCount() const {
      return ReferencesCount;
    }

    virtual ~VDFSObject() {
    }
  };
}