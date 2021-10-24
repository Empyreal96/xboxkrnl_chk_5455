///
/// Init kernel data, thread, process and proc control
///
///

#include "ki.h"

VOID
KiSwapGDT (
    VOID
    );


///
/// Function below takes control after bootstrapping, but before system initalized.
/// Used to init various Kernel objects
///
///

VOID KiInitializeKernel(
	VOID
	)
{
  PFX_SAVE_AREA NpxFrame; // edx    char *v0
  PVOID DpcStack; // eax    void *v1

  if ( ((unsigned __int8)(KiPCR + 9) & 0xF) != 0 ) {   // TODO
    RtlAssert(
      "(((ULONG_PTR)&Prcb->NpxSaveArea) % 16) == 0",
      "d:\\xbox-apr03\\private\\ntos\\ke\\i386\\kernlini.c",
      0xC7u,
      0);
  }
  *(KiPCR + 21) = (unsigned int)(&KiPCR + 20); // TODO
  *(KiPCR + 20) = (unsigned int)(&KiPCR + 20); // TODO
  KiPCR->DpcRoutineActive = 0;
  KfLowerIrql(APC_LEVEL); // 1u
  KiInitSystem();
  KeInitializeProcess(&KiIdleProcess, (KPRIORITY)0);
  KiIdleProcess.ThreadQuantum = MAXCHAR; //127
  KeInitializeProcess(&KiSystemProcess, NORMAL_BASE_PRIORITY);  //8
  KiSystemProcess.ThreadQuantum = THREAD_QUANTUM; //60
  KeInitializeThread(&KiIdleThread, KiDoubleFaultStackLimit, /*0x3000u*/KERNEL_STACK_SIZE, 0, (PKSYSTEM_ROUTINE)NULL,
                       (PKSTART_ROUTINE)NULL, (PVOID)NULL, &KiIdleProcess);
  KiIdleThread.Priority = HIGH_PRIORITY; //31
  KiIdleThread.State = Running; //2
  KiIdleThread.WaitIrql = DISPATCH_LEVEL; //2
  NpxFrame = (PFX_SAVE_AREA)(((ULONG)(KiIdleThread.StackBase) - sizeof(FX_SAVE_AREA))); //528
  memset((char *)KiIdleThread.StackBase - 528, 0, 0x210u);
  NpxFrame->FloatSave.ControlWord = 639; //(_WORD *)   TODO
  NpxFrame->FloatSave.MXCsr = 8064; //(_DWORD *)  (NpxFrame + 6)?
  KiPCR->CurrentThread = &KiIdleThread;
  KiPCR->NextThread = (PKTHREAD)NULL; // 0
  KiPCR->IdleThread = &KiIdleThread;
  KiPCR->NtTib.StackBase = (PCHAR)KiIdleThread.StackBase - sizeof(FX_SAVE_AREA); // 528
 
  ExpInitializeExecutive();
  DpcStack = MmCreateKernelStack(KERNEL_STACK_SIZE, FALSE); // (0x3000u, 0)
  if (DpcStack == NULL ) {
    KeBugCheckEx(NO_PAGES_AVAILABLE, 1, 0, 0, 0); // 0x4Du, 1u
  }
  KiPCR->DpcStack = DpcStack;
  KfRaiseIrql(2u); // TODO
  KeSetPriorityThread(&KiIdleThread, (KPRIORITY)0);
if (KiPCR->NextThread == (PKTHREAD)NULL) {
    KiIdleSummary |= 1; //1u
	}
  KfRaiseIrql(0x1Fu); // return    TODO
}


///
/// call to edit GDT
///
/// TODO
unsigned __int8 *__stdcall KiSwapGDT()
{
  UCHAR *result; // eax  unsigned __int8
  unsigned int v1; // ecx
  UCHAR v2; // dl   unsigned __int8

  result = KiGDT;
  if ( KiGDT < (UCHAR *)KiDataSectionDigest )
  {
    result = (unsigned __int8 *)&unk_8005FBAE;
    v1 = ((unsigned int)((char *)KiDataSectionDigest - (char *)KiGDT - 1) >> 3) + 1;
    do
    {
      v2 = *(result - 1);
      *(result - 1) = *result;
      *result = result[1];
      result[1] = v2;
      result += 8;
      --v1;
    }
    while ( v1 );
  }
  return result;
}