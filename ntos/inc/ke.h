///
/// header (external) for the kernel
///
/// empyreal96
///
#ifndef _KE_
#define _KE_






///
/// Lower IRQ Level
///
/// TODO
NTHALAPI
VOID
FASTCALL
KfLowerIrql(
	IN KIRQL NewIrql
	)
{
  unsigned int v1; // kr00_4
  unsigned __int8 v2; // ah
  unsigned int v3; // [esp-4h] [ebp-4h]

  if ( NewIrql > *((_BYTE *)&KiPCR + 36) )
  {
    *((_BYTE *)&KiPCR + 36) = 31;
    KeBugCheck(0xAu);
  }
  v1 = __readeflags();
  v3 = v1;
  _disable();
  *(&KiPCR + 9) = NewIrql;
  if ( (*(_DWORD *)(4 * NewIrql - 2147370500) & HalpIRR) != 0 )
  {
    _BitScanReverse((unsigned int *)&NewIrql, *(_DWORD *)(4 * NewIrql - 2147370500) & HalpIRR);
    if ( NewIrql <= 2u )
      ((void (__cdecl *)(unsigned int))*(&SWInterruptHandlerTable + NewIrql))(v1);
    v2 = BYTE1(HalpIDR);
    __outbyte(0x21u, HalpIDR);
    __outbyte(0xA1u, v2);
    HalpIRR ^= 1 << NewIrql;
    ((void (__cdecl *)(unsigned int))*(&SWInterruptHandlerTable + NewIrql))(v1);
  }
  if ( *(&KiPCR + 159) && !NewIrql && ((unsigned __int8 (__cdecl *)(unsigned int))*(&KiPCR + 159))(v1) )
  {
    _enable();
    __debugbreak();
  }
  __writeeflags(v3);
}


///
/// Raise IRQ Level
///
/// TO-DO:
NTHALAPI
VOID
FASTCALL KfRaiseIrql(
	IN KIRQL NewIrql)
{
  KIRQL result; // al
  //result = *((_BYTE *)&KiPCR + 36);
  //*((_BYTE *)&KiPCR + 36) = NewIrql;
  //if ( result > NewIrql )
  //{
   // *((_BYTE *)&KiPCR + 36) = 0;
   // KeBugCheck(9u);
  
  result = *((_BYTE *)&KiPCR + 36);
  *(&KiPCR + 36) = NewIrql;
  if ( result > NewIrql )
  {
    *(&KiPCR + 36) = 0;
    KeBugCheck(9u);
  }
  return result;
}

