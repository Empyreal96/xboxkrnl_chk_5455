///
///
/// Startup animation impl
///
/// empyreal96
///

#include <stddef.h>

#pragma code_seg("INIT")
#pragma data_seg("INIT_RW")
#pragma const_seg("INIT_RD")

#include "ntos.h"
#include "stdio.h"
#include "stdlib.h"
#include "wtypes.h"

#include "ani.h"

// Tell linker to put startup animation code and data into INIT section
#pragma comment(linker, "/merge:INIT_RW=INIT")
#pragma comment(linker, "/merge:D3D=INIT")
#pragma comment(linker, "/merge:D3D_RD=INIT")
#pragma comment(linker, "/merge:D3D_RW=INIT")
#pragma comment(linker, "/merge:XGRPH=INIT")
#pragma comment(linker, "/merge:XGRPH_RD=INIT")

HANDLE g_hThread;
VOID AnipStartAnimationThread(PKSTART_ROUTINE StartRoutine, PVOID StartContext);
VOID AnipRunAnimation();

///
///
/// Starts animation in background thread
///
///



void AniStartAnimation(BOOLEAN fShort)
{
	NTSTATUS Status
  if ( gBootAnimation_DoAnimation )
  {
    g_bShortVersion = fShort;
    if ( PsCreateSystemThreadEx(&g_hThread,
								0,
								0x4000, //0x4000u   Stack size 16K
								0,
								NULL,
								NULL,
								NULL,
								FALSE,
								FALSE,
								AnipStartAnimationThread) < 0 );
	if (!NT_SUCCESS(Status))
	{
		// thread creation fail
      g_hThread = NULL;
	}
  }
}


///
/// Stop animation
///
///

void 
AniTerminateAnimation()
{
  void *result; // eax

  result = g_hThread;
  if (g_hThread)
  {
	NTSTATUS Status;
    
	NtWaitForSingleObjectEx(g_hThread, KernelMode, FALSE, NULL); // 0, 0, 0
    result = NtClose(g_hThread);
    g_hThread = NULL;
  }
  return result;
}



///
/// Block till animation has finished
///

void AniBlockOnAnimation()
{
  KWAIT_BLOCK WaitBlocks[2]; // [esp+0h] [ebp-3Ch] BYREF
  PVOID WaitObjects[2]; // [esp+30h] [ebp-Ch] BYREF
  PETHREAD ThreadObject; // [esp+38h] [ebp-4h] BYREF
  NTSTATUS Status;
  if ( g_hThread )
  {
	  Status = ObReferenceObjectByHandle(g_hThread, &PsThreadObjectType, (PVOID*)&ThreadObject); 
    if ( NT_SUCCESS(Status))
    {
      WaitObjects[0] = ThreadObject;
      WaitObjects[1] = &g_EventLogoWaiting;
      KeWaitForMultipleObjects(2, WaitObjects, WaitAny, Executive, KernelMode, FALSE, NULL, WaitBlocks); // 0, 0, 0,
      ObfDereferenceObject(ThreadObject);
    }
  }
}


///
/// "MemAllocContiguous" not sure on this purpose, no source documentation
///

void *MemAllocContiguous(size_t Size, DWORD Alignment)
{
  gcMemAllocsContiguous++;
  return MmAllocateContiguousMemoryEx(
			Size, 
			0, 
			0x3FFB000u, //0x3FFB000u   could be "AGP_APERTURE_BYTES - NV_INSTANCE_SIZE"
			Alignment, 
			0x404u);  //0x404u     could be "PAGE_READWRITE | PAGE_WRITECOMBINE"
}


///
/// "MemFreeContiguous" no source documentation
///
void MemFreeContiguous(void *pv)
{
  if ( gcMemAllocsContiguous <= 0 )
  {
    __debugbreak(); // I think this is "AnipBreak()"
  }
  gcMemAllocsContiguous--;
  MmFreeContiguousMemory(pv);
}


///
/// Main Animation call
///
VOID /*__noreturn*/ AnipStartAnimationThread(
		PKSTART_ROUTINE StartRoutine, 
		PVOID StartContext)
{
  AnipRunAnimation();
  PsTerminateSystemThread(0);
  __debugbreak(); // I think this is "AnipBreak()"
}


