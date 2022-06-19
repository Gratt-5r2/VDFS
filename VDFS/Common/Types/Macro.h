#pragma once

#define untyped_ptr		...
#define memsize(dst)	 /* smart heap !!! */ shi_msize(dst)
#define True          (1)
#define False			    (0)
#define Null			    (nullptr)
#define None			    (0x00000000)
#define Invalid			  (0xFFFFFFFF)
#define NotFound		  Invalid
#define NullStrA		  ((char*)None)
#define NullStrW		  ((wchar_t*)None)
#define Success			  True
#define Failed			  False

#define I32 (int32)
#define I64 (int64)
#define U32 (uint32)
#define U64 (uint64)
#define F32 (real32)
#define F64 (real64)

#define MemDel(obj)		{ /* smart heap !!! */ shi_free(obj); obj = Null; }
#define DeleteObject(obj){ delete obj; obj = Null; }

#ifdef UNICODE
#define NullStr			NullStrW
#else
#define NullStr			NullStrA
#endif

#define ASTAPI
#define UNSAFE
#define shi_free free
#define shi_malloc malloc
#define shi_realloc realloc
#define shi_msize _msize

#define ASSERT(c, t) if( !(c) ) Message::Fatal(t)