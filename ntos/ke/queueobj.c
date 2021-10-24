///
/// Impliment kernel queue objects
///
///
///
#include "ki.h"



///
/// Initilize Kernel object queue
///
///
VOID 
KeInitializeQueue (
	IN PRKQUEUE Queue,
	IN ULONG count OPTIONAL)

{
  Queue->Header.SignalState = 0;
  Queue->CurrentCount = 0;
  InitializeListHead(&Queue->Header.WaitListHead);
  InitializeListHead(&Queue->EntryListHead);
  InitializeListHead(&Queue->ThreadListHead);
  Queue->Header.Type = 4;
  Queue->Header.Size = 10;
  if ( ARGUMENT_PRESENT((PVOID)(ULONG_PTR)Count) )
  {
    Queue->MaximumCount = Count;
  }
  else
  {
    Queue->MaximumCount = 1;
  }
}




///
/// Insert a specified entry in the object entry list and try to satisfy
///
///


LONG 
KeInsertQueue (
	IN PRKQUEUE Queue, 
	PLIST_ENTRY Entry)
{
  KIRQL OldIrql; // bl v2
  LONG OldState; // eax  v3
  int v4; // ecx
  int v5; // esi

  if ( Queue->Header.Type != 4 ) 
  {
    RtlAssert("(Queue)->Header.Type == QueueObject", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x7Cu, 0);
  }
  
  if ( *((_BYTE *)&KiPCR + 36) > 2u )
  {  
	RtlAssert("KeGetCurrentIrql() <= DISPATCH_LEVEL", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x7Du, 0);
  }
  
  OldIrql = KeRaiseIrqlToDpcLevel(); // v2
  OldState = KiInsertQueue(Queue, Entry, 0); // v3
  LOBYTE(v4) = v2;
  v5 = OldState; // v3
  KiUnlockDispatcherDatabase(v4);
  return v5;
}




///
/// Insert a specified entry in the object entry list and try to satisfy
/// 
///

LONG KeInsertHeadQueue (
	IN PRKQUEUE Queue, 
	PLIST_ENTRY Entry)
{
  KIRQL OldIrql; // bl  v2
  LONG OldState; // eax v3
  int v4; // ecx
  int v5; // esi

  if ( Queue->Header.Type != 4 )
  {
    RtlAssert("(Queue)->Header.Type == QueueObject", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0xB5u, 0);
  }
  
  if ( *((_BYTE *)&KiPCR + 36) > 2u )
  {
	RtlAssert("KeGetCurrentIrql() <= DISPATCH_LEVEL", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0xB6u, 0);
  }
  
  OldIrql = KeRaiseIrqlToDpcLevel();
  OldState = KiInsertQueue(Queue, Entry, TRUE); //v3   1u
  LOBYTE(v4) = OldIrql; //v
  v5 = v3;
  KiUnlockDispatcherDatabase(v4);
  return v5;
}



///
/// Remove next entry in object queue
///
/// TODO
PLIST_ENTRY 
KeRemoveQueue(
	IN PRKQUEUE Queue, 
	IN KPROCESSOR_MODE WaitMode, 
	IN PLARGE_INTEGER Timeout OPTIONAL)
{
  PRKTHREAD Thread; // esi // v4
  PRKQUEUE OldQueue; // ecx  /v5
  _LIST_ENTRY *v6; // ebx
  _LIST_ENTRY *v7; // eax
  _LIST_ENTRY **v8; // edx
  _LIST_ENTRY *v9; // ecx
  _LARGE_INTEGER *v10; // ecx
  PLIST_ENTRY Entry; // ebx v11
  _LIST_ENTRY *v12; // ecx
  _LIST_ENTRY *v13; // ecx
  LONG_PTR WaitStatus; // eax  v14
  KIRQL OldIrql; // al v15
  unsigned int v16; // eax
  KWAIT_BLOCK StackWaitBlock; // [esp+Ch] [ebp-28h] BYREF
  LARGE_INTEGER NewTime; // [esp+24h] [ebp-10h] BYREF
  LARGE_INTEGER DueTime; // [esp+2Ch] [ebp-8h] BYREF
  PLARGE_INTEGER OriginalTime; // [esp+3Ch] [ebp+8h]

  if (Queue->Header.Type != 4)
  {
    RtlAssert("(Queue)->Header.Type == QueueObject", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x100u, 0);
  }
  if (*((_BYTE *)&KiPCR + 36) > 2u)
  {  
  RtlAssert("KeGetCurrentIrql() <= DISPATCH_LEVEL", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x101u, 0);
  }
  
  Thread = *(&KiPCR + 10); // v4
  if ( *(_BYTE *)(*(&KiPCR + 10) + 86) )
  {
    *(_BYTE *)(*(&KiPCR + 10) + 86) = 0;
  }
  else
  {
    *(_BYTE *)(Thread + 84) = KeRaiseIrqlToDpcLevel(); // v4
  }
  
  OldQueue = Thread->Queue; // v5 v4
  Thread->Queue) = Queue; // v4
  
  if ( Queue == OldQueue ) // v5
  {
    Queue->CurrentCount -= 1; // --
  }
  else
  {
    v6 = (_LIST_ENTRY *)(Thread + 124); // v4
    if ( OldQueue != NULL ) //v5
    {
      v7 = v6->Flink;
      v8 = *(_LIST_ENTRY ***)(Thread + 128); // v4
      *v8 = v6->Flink;
      v7->Blink = (_LIST_ENTRY *)v8;
      KiActivateWaiterQueue(OldQueue); //v5
    }
    v9 = Queue->ThreadListHead.Blink;
    v6->Flink = &Queue->ThreadListHead;
    *(_DWORD *)(Thread + 128) = v9; // v4
    v9->Flink = v6;
    Queue->ThreadListHead.Blink = v6;
  }
  v10 = Timeout;
  OriginalTime = Timeout;
  while ( true ) // 1
  {
    Entry = Queue->EntryListHead.Flink;
    if ((Entry != &Queue->EntryListHead) && (Queue->CurrentCount < Queue->MaximumCount) )
    {
      Queue->Header.SignalState -= 1; // --
      Queue->CurrentCount += 1; // ++
      v16 = *(_DWORD *)Entry;
      v10 = *(_LARGE_INTEGER **)(Entry + 4);
      v10->LowPart = *(_DWORD *)Entry;
      *(_DWORD *)(v16 + 4) = v10;
      *(_DWORD *)Entry = 0;
      goto LABEL_36;
    }
    if ( *(_BYTE *)(Thread + 73) && !*(_BYTE *)(Thread + 84) ) // v4
    {
      Queue->CurrentCount += 1; // ++
      LOBYTE(v10) = *(_BYTE *)(Thread + 84); // v4
      KiUnlockDispatcherDatabase(v10);
      goto LABEL_31;
    }
    if ( WaitMode && *(_BYTE *)(Thread + 74) ) // v4
    {
      Entry = 192;
      goto LABEL_35;
    }
    *(_DWORD *)(Thread + 80) = 0; // v4
    *(_DWORD *)(Thread + 88) = &StackWaitBlock; // v4
    StackWaitBlock.WaitKey = 0;
    StackWaitBlock.Object = Queue;
    StackWaitBlock.WaitType = 1;
    StackWaitBlock.Thread = Thread; // v4
    if ( !v10 )
    {
      StackWaitBlock.NextWaitBlock = &StackWaitBlock;
      goto LABEL_26;
    }
    if ( !v10->QuadPart )
      break;
    StackWaitBlock.NextWaitBlock = (_KWAIT_BLOCK *)(Thread + 176); // v4
    *(_DWORD *)(Thread + 144) = Thread + 176; // v4
    *(_DWORD *)(Thread + 148) = Thread + 176; // v4
    *(_DWORD *)(Thread + 192) = &StackWaitBlock; // v4
    if ( !KiInsertTreeTimer((_KTIMER *)(Thread + 136), *v10) ) // v4
	{
	  break;
    }
	DueTime = *(_LARGE_INTEGER *)(Thread + 152); // v4
LABEL_26:
    v12 = Queue->Header.WaitListHead.Blink;
    StackWaitBlock.WaitListEntry.Flink = &Queue->Header.WaitListHead;
    StackWaitBlock.WaitListEntry.Blink = v12;
    v12->Flink = (_LIST_ENTRY *)&StackWaitBlock;
    Queue->Header.WaitListHead.Blink = (_LIST_ENTRY *)&StackWaitBlock;
    *(_BYTE *)(Thread + 85) = WaitMode; // v4
    *(_BYTE *)(Thread + 47) = 0; // v4
    *(_BYTE *)(Thread + 87) = 15; // v4
    *(_DWORD *)(Thread + 100) = KeTickCount; // v4
    *(_BYTE *)(Thread + 44) = 5; // v4
    v13 = KiWaitInListHead.Blink;
    *(_DWORD *)(Thread + 92) = &KiWaitInListHead; // v4
    *(_DWORD *)(Thread + 96) = v13; // v4
    v13->Flink = (_LIST_ENTRY *)(Thread + 92); // v4
    KiWaitInListHead.Blink = (_LIST_ENTRY *)(Thread + 92); // v4
	
    if ( *(_BYTE *)(Thread + 84) > 2u ) // v4
	{    
	RtlAssert("Thread->WaitIrql <= DISPATCH_LEVEL", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x1DBu, 0);
    }
	WaitStatus = KiSwapThread();
    Thread->WaitReason = 0; // v4
    if (WaitStatus != STATUS_KERNEL_APC)
	{
      return (PLIST_ENTRY)WaitStatus;
    }
	
	if ( ARGUMENT_PRESENT(Timeout) )
	{
	Timeout = KiComputeWaitInterval(OriginalTime, &DueTime, &NewTime);
	}
	
	
LABEL_31:
    OldIrql = KeRaiseIrqlToDpcLevel();
    v10 = Timeout;
    *(_BYTE *)(Thread + 84) = OldIrql; // v4
    --Queue->CurrentCount;
  }
  Entry = 258;
LABEL_35:
  ++Queue->CurrentCount;
LABEL_36:
  LOBYTE(v10) = *(_BYTE *)(Thread + 84); // v4
  KiUnlockDispatcherDatabase(v10);
  return Entry;
}



///
/// Goes through the specified 'queue' to run maintinence
///
/// TODO

PLIST_ENTRY 
KeRundownQueue (
	IN PRKQUEUE Queue
	)
{
  KIRQL v1; // al
  KIRQL OldIrql; // ecx int  v2
  PLIST_ENTRY FirstEntry; // ebx   _KQUEUE *v3;
  _LIST_ENTRY *v4; // edx
  _LIST_ENTRY *v5; // edi
  _LIST_ENTRY *v6; // esi
  _LIST_ENTRY *v7; // edx
  _LIST_ENTRY *v8; // edx

  if ( Queue->Header.Type != 4 )
  {
    RtlAssert("(Queue)->Header.Type == QueueObject", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x22Bu, 0);
  }
  if ( *((_BYTE *)&KiPCR + 36) > 2u )
  {
    RtlAssert("KeGetCurrentIrql() <= DISPATCH_LEVEL", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x22Cu, 0);
  }
  
  v1 = KeRaiseIrqlToDpcLevel();
  
  FirstEntry = Queue->EntryListHead.Flink; //v3
  
  if ( FirstEntry == &Queue->EntryListHead ) //v3
  {
    FirstEntry = 0; //v3
  }
  else
  {
    v4 = Queue->EntryListHead.Blink;
    v4->Flink = FirstEntry;  // (_LIST_ENTRY *)
    FirstEntry->Header.SignalState = (int)v4;
  }
  
  v5 = &Queue->ThreadListHead;
  
  while ( 1 )
  {
    v8 = v5->Flink;
  if ( v5->Flink == v5 ) {
      break;
  }
    v8[-1].Blink = 0;
    v6 = v8->Flink;
    v7 = v8->Blink;
    v7->Flink = v6;
    v6->Blink = v7;
  }
  LOBYTE(OldIrql) = v1;
  KiUnlockDispatcherDatabase(OldIrql);
  return FirstEntry; // v3
}




//
// Called when a thread is about to enter a wait state
//
// TODO
//
VOID
FASTCALL
KiActivateWaiterQueue(
	IN PRKQUEUE Queue
	)
{
  PRLIST_ENTRY Entry; // edx  //_KQUEUE *v1
  PRLIST_ENTRY WaitEntry; // edi  //_LIST_ENTRY *v2
  int v3; // eax  TODO
  _DWORD *v4; // esi  TODO

  if (Queue->CurrentCount < Queue->MaximumCount)
  {
    Entry = Queue->EntryListHead.Flink; //v1
    WaitEntry = Queue->Header.WaitListHead.Blink;//v2
    if ( (Entry != &Queue->EntryListHead) && (WaitEntry != &Queue->Header.WaitListHead) ) // v1  v2
    {
      v3 = *(_DWORD *)&Entry->Header.Type;  //TODO
      v4 = (_DWORD *)Entry->Header.SignalState;  //TODO
      *v4 = *(_DWORD *)&Entry->Header.Type;  //TODO
      *(_DWORD *)(v3 + 4) = v4;  //TODO
      *(_DWORD *)&Entry->Header.Type = 0; // TODO
      Queue->Header.SignalState;
      KiUnwaitThread(WaitEntry[1].Flink, (LONG_PTR)Entry, 0);
    }
  }
  return;
}


///
/// Insert specified Entry to queue object entry list
///
/// LOTS TODO

LONG 
FASTCALL 
KiInsertQueue(
	IN PRKQUEUE Queue, 
	IN PLIST_ENTRY Entry, 
	IN BOOLEAN Head
	)
{
  PLIST_ENTRY WaitEntry; // eax v5
  int v6; // ecx
  _DWORD *v7; // edx
  PRKTHREAD Thread; // ecx  v8
  _LIST_ENTRY *v9; // eax
  _LIST_ENTRY *v10; // edx
  _LIST_ENTRY *v11; // edx
  _LIST_ENTRY *v12; // esi
  _LIST_ENTRY *v13; // eax
  _LIST_ENTRY *v14; // ecx
  _LIST_ENTRY *v15; // ecx
  LONG OldState; // [esp+Ch] [ebp-4h]   int v17

  if ( Queue->Header.Type != 4 )
  {
    RtlAssert("(Queue)->Header.Type == QueueObject", "d:\\xbox-apr03\\private\\ntos\\ke\\queueobj.c", 0x2BFu, 0);
  }
  OldState = Queue->Header.SignalState; // v17
  WaitEntry = Queue->Header.WaitListHead.Blink; // v5
  if ( WaitEntry == &Queue->Header.WaitListHead
    || Queue->CurrentCount >= Queue->MaximumCount
    || *(_KQUEUE **)(*(&KiPCR + 10) + 120) == Queue && *(_BYTE *)(*(&KiPCR + 10) + 87) == 15 ) // TODO
  {
    Queue->Header.SignalState = OldState + 1; // v17
    v13 = &Queue->EntryListHead;
    if ( Head )
    {
      v14 = v13->Flink;
      Entry->Flink = v13->Flink;
      Entry->Blink = v13;
      v14->Blink = Entry;
      v13->Flink = Entry;
    }
    else
    {
      v15 = Queue->EntryListHead.Blink;
      Entry->Flink = v13;
      Entry->Blink = v15;
      v15->Flink = Entry;
      Queue->EntryListHead.Blink = Entry;
    }
  }
  else
  {
    v6 = &WaitEntry->Header.Type;
    v7 = WaitEntry->Header.SignalState;
    *v7 = )&WaitEntry->Header.Type;
    (v6 + 4) = v7;  //v 7
    Thread = WaitEntry->Header.WaitListHead.Flink; // v8
    v9 = Thread->WaitListEntry.Flink; // v8
    v10 = Thread->WaitListEntry.Blink; // v8
    Thread->WaitStatus = (LONG_PTR)Entry; // v8
    v10->Flink = v9;
    v9->Blink = v10;
    ++Queue->CurrentCount;
    Thread->WaitReason = 0; // v8
    if ( Thread->Timer.Header.Inserted == TRUE ) // v8
    {
      v11 = Thread->Timer.TimerListEntry.Flink; // v8
      v12 = Thread->Timer.TimerListEntry.Blink; // v8
      Thread->Timer.Header.Inserted = FALSE; // v8
      v12->Flink = v11;
      v11->Blink = v12;
      Thread->Timer.TimerListEntry.Flink = 0; // v8
      Thread->Timer.TimerListEntry.Blink = 0; // v8
    }
    KiReadyThread(Thread); // v8
  }
  return OldState;
}