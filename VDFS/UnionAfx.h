#pragma once
#pragma warning(disable:4005)
#pragma warning(disable:4099)
#pragma warning(disable:4800)
#pragma warning(disable:4731)
#pragma warning(disable:4722)

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

#define XCALL(uAddr)			  \
	__asm { mov esp, ebp	  }	\
	__asm { pop ebp	        }	\
	__asm { mov eax, uAddr	}	\
	__asm { jmp eax			    }

#define _O_TEXT 0x4000
#define _O_BINARY 0x8000
#include <time.h>
#include <new.h>

#include "Common\Common.h"
using namespace Common;

// #include "Detours\detours.h"
// #pragma comment (lib, "Detours\\Detours.lib")
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)

// #include "HookInvoker.h"