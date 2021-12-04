///
/// Config and setup for TV encoder streams
///
///
/// empyreal96
/// This file I am having hard time figuring out, mainly due to the XCALIBUR
/// changes to the kernel and unnofficial source patches used as reference

#include "avp.h"
#include "xpcicfg.h"

DECLSPEC_STICKY ULONG AvpCurrentMode;

// 
// Supported frame buffer modes.
//

#define D3DFMT_LIN_A1R5G5B5   0x00000010
#define D3DFMT_LIN_X1R5G5B5   0x0000001C
#define D3DFMT_LIN_R5G6B5     0x00000011
#define D3DFMT_LIN_A8R8G8B8   0x00000012
#define D3DFMT_LIN_X8R8G8B8   0x0000001E



///
/// Sets encoder config
///
ULONG
AvpCalcWSSCRC(IN ULONG Value)
{
  ULONG CRC; // eax //v1
  ULONG i; // ecx
  ULONG lsb; // edx  //v3
  ULONG msb; // esi

  CRC = 0x3F; // v1    0x3F=63 ?
  for ( i = 0; i < 0xE; ++i )
  {
    lsb = (CRC ^ (unsigned __int8))((Value >> i)) & 1); //v3
    msb = (32 * lsb) | (CRC >> 1) & 0xF | (16 * lsb) ^ (CRC >> 1) & 0x10; //v3  v4
    CRC = msb; //v1  v4
  }
  return Value | (CRC << 14); //v4   either msb or CRC
}



///
/// "QUOTE: Uses the current mode and macrovision state to correctly
///   set the WSS and CGMS bits."
///
/// TODO LOTS
void AvpSetWSSBits(PVOID *RegisterBase)
{
  int v1; // ecx
  unsigned int WSS; // ebx  v2
  unsigned int v3; // edi
  unsigned int v4; // ebx
  unsigned int v5; // esi
  unsigned int v6; // ebx
  unsigned __int8 Val; // [esp+4h] [ebp-4h]

  v1 = 0; // Could be WSS
  if ( (AvpCurrentMode & 0xC0000000) != 0 )
  {
    if ( (AvpCurrentMode & 0xC0000000) != 0x40000000 )
      return;
    WSS = (AvpCGMS << 12) | (8 - ((AvpCurrentMode & 0x10000000) != 0));  //V2
  }
  else
  {
    if ( (AvpCurrentMode & 0x10000000) != 0 )
      v1 = 1;// Could be WSS
    WSS = AvpCalcWSSCRC(v1 | ((AvpCGMS | (2 * (AvpMacrovisionMode & 2 | (4 * (AvpMacrovisionMode & 1))))) << 6)); //V2
  }
  if ( TVEncoderSMBusID == 0xD4 )
  {
	  /// FOCUS ENCODER
    SMB_WR(RegisterBase, 0x85, (UCHAR)((WSS & 0x0F))); //V2
    SMB_WR(RegisterBase, 0x84, (UCHAR)((WSS >> 4) & 0xFF))); //V2
    SMB_WR(RegisterBase, 0x83, (UCHAR)((WSS >> 12) & 0xFF));
    SMB_WR(RegisterBase, 0x88, (UCHAR)((WSS & 0x0F));
    SMB_WR(RegisterBase, 0x87, (UCHAR)((WSS >> 4) & 0xFF));
    SMB_WR(RegisterBase, 0x86, (UCHAR)((WSS >> 12) & 0xFF));
  }
  else if ( TVEncoderSMBusID == 0xE0 )
  {
	  
	 /// XCALIBUR ENCODER?
    v5 = AvpDump;
    AvpDump = 0;
    v6 = 2 * (WSS & 0xFFFFF | 0x1800000);
    HalWriteSMBusValue(0xE0, 7, 1, v6);
    AvpDump = v5;
    if ( v5 )
      DbgPrint("TV%04X = %04X\n", 7, v6);
  }
  else
  {
	  ///  CONEXANT ENCODER
    SMB_WR(RegisterBase, 0x60, (UCHAR)((WSS & 0x0F) | 0xC0)); //V2
    SMB_WR(RegisterBase, 0x62, (UCHAR)((WSS >> 4) & 0xFF)); //V2
    SMB_WR(RegisterBase, 0x64, (UCHAR)((WSS >> 12) & 0xFF)); //V2
  }
}


///
/// Try to enable SCART
///
/// Very different to 4400
/// TODO LOTS
VOID
AvpEnableSCART(
		IN PVOID RegisterBase, 
		IN ULONG iTV)
{
  int v2; // ecx
  unsigned __int8 *v3; // esi   // Possibly *pByte
  unsigned __int8 *v4; // edi
  unsigned __int8 v5; // al
  unsigned int v6; // ebx
  unsigned int v7; // ebx
  unsigned int v8; // esi
  unsigned int v9; // eax
  unsigned int v10; // esi
  unsigned int v11; // esi
  unsigned __int8 *v12; // esi
  unsigned __int8 *v13; // edi
  unsigned __int8 *v14; // ebx
  const unsigned __int8 *pByteMax; // [esp+Ch] [ebp-Ch]
  unsigned int DataValue; // [esp+10h] [ebp-8h] BYREF
  unsigned __int8 CommandCode[4]; // [esp+14h] [ebp-4h]

  if ( TVEncoderSMBusID == 0xD4 )
  {
	  /// FOCUS
    v2 = AvpTableRowSize[7];
    v3 = &AV_TABLE_SCARTENABLE_DATA[2 * v2];
    pByteMax = &v3[v2];
    v4 = AV_TABLE_SCARTENABLE_DATA;
    for ( iTV = (unsigned int)&AV_TABLE_SCARTENABLE_DATA[v2]; v3 < pByteMax; ++iTV )
    {
      v5 = *v4;
      v6 = AvpDump;
      AvpDump = 0;
      CommandCode[0] = v5;
      HalReadSMBusValue(TVEncoderSMBusID, v5, 0, &DataValue);
      AvpDump = v6;
      SMB_WR(RegisterBase, *v4++, *v3++ | DataValue & ~*(_BYTE *)iTV);
    }
  }
  ///
  /// XCALIBUR
  else if ( TVEncoderSMBusID == 0xE0 )
  {
    v7 = AvpDump;
    AvpDump = 0;
    HalReadSMBusValue(0xE0u, 1u, 1u, (unsigned int *)&RegisterBase);
    AvpDump = 0;
    v8 = (unsigned int)RegisterBase & 0xFCFFFFFF | 0xC00008;
    HalWriteSMBusValue(TVEncoderSMBusID, 1u, 1u, v8);
    AvpDump = v7;
    if ( v7 )
      DbgPrint("TV%04X = %04X\n", 1, v8);
    v9 = AvpDump;
    AvpDump = 0;
    iTV = v9;
    HalReadSMBusValue(TVEncoderSMBusID, 0x60u, 1u, (unsigned int *)&RegisterBase);
    AvpDump = 0;
    v10 = (unsigned int)RegisterBase & 0xFEFFFFFF;
    HalWriteSMBusValue(TVEncoderSMBusID, 0x60u, 1u, (unsigned int)RegisterBase & 0xFEFFFFFF);
    AvpDump = iTV;
    if ( iTV )
      DbgPrint("TV%04X = %04X\n", 96, v10);
    v11 = AvpDump;
    AvpDump = 0;
    HalWriteSMBusValue(TVEncoderSMBusID, 0x58u, 1u, 0);
    AvpDump = v11;
    if ( v11 )
      DbgPrint("TV%04X = %04X\n", 88, 0);
  }
  else
  {
	  /// CONEXANT
    v12 = &AV_TABLE_SCARTENABLE_DATA[iTV * AvpTableRowSize[7]];
    v13 = &v12[AvpTableRowSize[7]];
    v14 = AV_TABLE_SCARTENABLE_DATA;
    while ( v12 < v13 )
      SMB_WR(RegisterBase, *v14++, *v12++);
  }
}


/// 
///  Initilise streams to the FB > TV Encoder
///
///
ULONG
AvSetDisplayMode(
		IN PVOID RegisterBase, 
		IN ULONG Step, 
		IN ULONG Mode, 
		IN ULONG Format,
		IN ULONG Pitch, 
		IN ULONG FrameBuffer)
{
  unsigned int v6; // ecx
  unsigned int v7; // esi
  int v8; // eax
  unsigned int SelectedTVTable; // eax v9
  unsigned int v10; // eax
  unsigned int v11; // eax
  unsigned __int8 v12; // al
  unsigned int v13; // eax
  unsigned __int8 v14; // al
  unsigned int v15; // eax
  unsigned __int8 v16; // al
  unsigned int v17; // eax
  unsigned int v18; // edi
  unsigned int v19; // edi
  unsigned int v20; // esi
  unsigned int v21; // edi
  unsigned int v22; // esi
  unsigned int v24; // eax
  unsigned int v25; // eax
  unsigned int v26; // eax
  unsigned __int8 *v27; // edi
  int v28; // ecx
  unsigned __int8 *v29; // edi
  unsigned __int8 *v30; // edi
  unsigned int v31; // edi
  unsigned int v32; // edi
  unsigned int v33; // edi
  unsigned int v34; // edi
  unsigned int v35; // eax
  int v36; // eax
  unsigned __int8 *v37; // edi
  unsigned __int8 *v38; // eax
  int v39; // eax
  int v40; // ecx
  unsigned int v41; // eax
  unsigned int v42; // edi
  unsigned __int8 v43; // cl
  unsigned __int8 v44; // al
  const unsigned __int8 *v45; // ecx
  unsigned int v46; // eax
  unsigned int v47; // edi
  unsigned __int8 v48; // cl
  unsigned __int8 v49; // al
  const unsigned __int8 *v50; // ecx
  unsigned int v51; // eax
  unsigned int v52; // ecx
  unsigned __int8 v53; // al
  const unsigned __int8 *v54; // ecx
  unsigned int v55; // edi
  unsigned __int8 *v56; // edi
  unsigned __int8 v57; // al
  bool v58; // zf
  char v59; // cl
  unsigned int v60; // ecx
  unsigned __int8 v61; // al
  BOOL v62; // eax
  unsigned int v63; // edi
  char v64; // al
  unsigned __int8 v65; // al
  unsigned int v66; // ecx
  _BYTE *v67; // eax
  int v68; // eax
  unsigned int v69; // edi
  unsigned int v70; // edi
  unsigned int v71; // eax
  int v73; // [esp-4h] [ebp-30h]
  unsigned int LgsValue; // [esp+Ch] [ebp-20h]
  unsigned int iCRTC; // [esp+10h] [ebp-1Ch]
  unsigned int iCRTCa; // [esp+10h] [ebp-1Ch]
  unsigned int OutputMode; // [esp+14h] [ebp-18h]
  unsigned int OutputModea; // [esp+14h] [ebp-18h]
  unsigned int GeneralControl; // [esp+18h] [ebp-14h]
  const unsigned __int8 *pByteMax; // [esp+1Ch] [ebp-10h]
  const unsigned __int8 *pByteMaxa; // [esp+1Ch] [ebp-10h]
  const unsigned __int8 *pByteMaxb; // [esp+1Ch] [ebp-10h]
  const unsigned __int8 *pByteMaxc; // [esp+1Ch] [ebp-10h]
  const unsigned __int8 *pByteMaxd; // [esp+1Ch] [ebp-10h]
  unsigned int iTV; // [esp+20h] [ebp-Ch]
  unsigned __int8 *pRegByte; // [esp+24h] [ebp-8h]
  unsigned __int8 *pRegBytea; // [esp+24h] [ebp-8h]
  unsigned __int8 *pRegByteb; // [esp+24h] [ebp-8h]
  const unsigned __int8 *pRegBytec; // [esp+24h] [ebp-8h]
  unsigned __int8 DACs; // [esp+2Bh] [ebp-1h]
  unsigned int iRxd; // [esp+34h] [ebp+8h]
  unsigned int iRxe; // [esp+34h] [ebp+8h]
  unsigned int iRx; // [esp+34h] [ebp+8h]
  unsigned int iRxa; // [esp+34h] [ebp+8h]
  unsigned int iRxb; // [esp+34h] [ebp+8h]
  unsigned int iRxc; // [esp+34h] [ebp+8h]
  unsigned __int8 iRx_3; // [esp+37h] [ebp+Bh]

  //v6 = Mode;
  if ( !Mode )
  {
    //v6 = 251724033;
    Mode = 251724033;
  }
  
  //v7 = (Mode & 0xC0000000); //v6
  OutputMode = (Mode & 0xC0000000); //v6
  iCRTC = BYTE1(Mode); //v6
  iTV = (Mode & 0x7F); //v6 v8
  
  if ( TVEncoderSMBusID != 0xE0 )
  {
    goto LABEL_16;
  }
  
  if ( v8 == 1 && BYTE1(Mode) == 7 ) //v6
  {
    if ( (Mode & 0xC0000000) != 0x80000000 ) //v7
      goto LABEL_12;
    v8 = 4;
  }
  
  if ( (Mode & 0xC0000000) != 0x80000000 ) // v7
  {
  }

LABEL_12:
  
  if ( XboxGameRegion == 2 )
  {
      SelectedTVTable = SDTVJapanTable[v8];// v9
    
  }
  else
  {
      SelectedTVTable = SDTVTable[v8];// v9
    goto LABEL_15;
  }

  if ( XboxGameRegion == 2 )
  {
    SelectedTVTable = HDTVJapanTable[v8];// v9
  }
  else
  {
	  SelectedTVTable = HDTVTable[v8];// v9

  }
  
LABEL_15:
  iTV = SelectedTVTable; // v9

LABEL_16:
  DACs = HIBYTE(Mode & 0xF)//v6;


  switch (Format)
  {
    case 0x10u: //D3DFMT_LIN_A1R5G5B5
      goto LABEL_23;
    case 0x11u: //D3DFMT_LIN_X1R5G5B5
      GeneralControl = 1052720;  //0x00100030
      HIBYTE(Format) = 2; //CR28Depth = 2
      goto LABEL_25;
    case 0x12u: //D3DFMT_LIN_R5G6B5
      goto LABEL_21;
    case 0x1Cu: //D3DFMT_LIN_A8R8G8B8
LABEL_23:
      HIBYTE(Format) = 2; //CR28Depth = 2
      goto LABEL_24;
  }
  if ( Format != 30 )
  {
    goto LABEL_25;
  }

LABEL_21:
  HIBYTE(Format) = 3; //CR28Depth = 3

LABEL_24:
  GeneralControl = 1048624;

LABEL_25:

  Pitch >>= 3; // Pitch /= 8; ??

  if ( AvpCurrentMode == Mode ) //v6
  {
    if ( Step == 0 )
	{
      RtlAssert("Step == 0", "d:\\xbox-apr03\\private\\ntos\\av\\modeset.c", 0xCFu, 0);
    }
	REG_WR32(RegisterBase + 1704320) = GeneralControl; //*((_DWORD *)   1704320 = NV_PRAMDAC_GENERAL_CONTROL?
  }
  
	if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6817280, GeneralControl);
    }
	//v10 = AvpDump;
    AvpDump = 0;
    CRTC_WR(RegisterBase + 6296532) = 31;//*((_BYTE *)
    Step = AvpDump;
    
	if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296532, 31);
    }
	CRTC_WR(RegisterBase + 6296533) = 87; //*((_BYTE *)
    
	if ( AvpDump )
	{     
	 DbgPrint("%08X = %08X\n", 6296533, 87);
	}   
	AvpDump = Step;
    
	if ( Step )
      DbgPrint("CR%02X = %02X\n", 31, 87);
    
	//v11 = AvpDump;
    AvpDump = 0;
    CRTC_WR(RegisterBase + 6296532) = 19; //*((_BYTE *)
    Step = AvpDump;
    
	if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296532, 19);
    }
	
	//v12 = Pitch;
    CRTC_WR(RegisterBase + 6296533) = Pitch; //*((_BYTE *)
    
	if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296533, Pitch); //v12
    }
	
	AvpDump = Step;
    
	if ( Step )
	{
      DbgPrint("CR%02X = %02X\n", 19, (unsigned __int8)Pitch);
    }
	
	HIBYTE(Pitch) = (Pitch >> 3) & 0xE0;
    //v13 = AvpDump;
    AvpDump = 0;
    CRTC_WR(RegisterBase + 6296532) = 25;//*((_BYTE *)
    Step = AvpDump; // v13
    
	if ( AvpDump )
	{     
	 DbgPrint("%08X = %08X\n", 6296532, 25);
	}
   //v14 = HIBYTE(Pitch);
    CRTC_WR(RegisterBase + 6296533) = HIBYTE(Pitch); //*((_BYTE *)

    if ( AvpDump ) 
	{
      DbgPrint("%08X = %08X\n", 6296533, HIBYTE(Pitch)); //v14
	}
	
    AvpDump = Step;

    if ( Step )
	{
      DbgPrint("CR%02X = %02X\n", 25, HIBYTE(Pitch));
	}
	
    HIBYTE(Pitch) = HIBYTE(Format) | 0x80;
    //v15 = AvpDump;
    AvpDump = 0;
    CRTC_WR(RegisterBase + 6296532) = 40; //*((_BYTE *)
    Step = AvpDump; //v15

    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296532, 40);
	}
	
    //v16 = HIBYTE(Pitch);
    CRTC_WR(RegisterBase + 6296533) = HIBYTE(Pitch); //*((_BYTE *)

    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296533, HIBYTE(Pitch)); //v16
	}
	
    AvpDump = Step;

    if ( Step )
	{
      DbgPrint("CR%02X = %02X\n", 40, HIBYTE(Pitch));
	}
    //v17 = FrameBuffer;
    REG_WR32(RegisterBase + 1573376) = FrameBuffer; //*((_DWORD *)

    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6293504, FrameBuffer); // v17
	}
	
    AvSendTVEncoderOption(RegisterBase, 0xBu, 5u, 0);
    AvSendTVEncoderOption(RegisterBase, 0xEu, 0, 0);
    AvpCurrentMode = Mode;
    return 0;
  }

  if ( Step )
  {
    RtlAssert("Step == 0", "d:\\xbox-apr03\\private\\ntos\\av\\modeset.c", 0xFAu, 0);
    v6 = Mode;
  }
  
  AvpCurrentMode = Mode; //v6
  if ( TVEncoderSMBusID == 0xD4 )
  {
    //v18 = AvpDump;
    AvpDump = 0;
    HalReadSMBusValue(0xD4u, 0xA0u, 1u, &Step);
    //v73 = (unsigned __int16)Step | 0xF;
    //Step = v73;
    AvpDump = 0;
    HalWriteSMBusValue(TVEncoderSMBusID, 0xA0u, 1u, (unsigned __int16)Step | 0xF); //v73
    //AvpDump = v18;
    if ( AvpDump ) //v18
	{
      DbgPrint("TV%04X = %04X\n", 160, Step);
	}
  }
  if ( TVEncoderSMBusID == 0xE0 )
  {
    //v19 = AvpDump;
    AvpDump = 0;
    HalReadSMBusValue(0xE0u, 4u, 1u, &Step);
    AvpDump = 0;
    HalWriteSMBusValue(TVEncoderSMBusID, 4u, 1u, Step | 0xF); //v20
    //AvpDump = v19;
    if ( AvpDump ) //v19
	{
      DbgPrint("TV%04X = %04X\n", 4, Step | 0xF);
    }
	//v21 = AvpDump;
    AvpDump = 0;
    HalReadSMBusValue(TVEncoderSMBusID, 0, 1u, &Step);
    //v22 = Step & 0xFFFFFFFD;
    AvpDump = 0;
    HalWriteSMBusValue(TVEncoderSMBusID, 0, 1u, Step & 0xFFFFFFFD);
    //AvpDump = v21;
	
    if ( AvpDump ) //v21
	{
      DbgPrint("TV%04X = %04X\n", 0, Step & 0xFFFFFFFD); //v22
	}
 }
  
  __outbyte(0x80D3u, 5u);
  do
  {
    LgsValue = REG_RD32(RegisterBase + 1704360); //*((_DWORD *)
  }
  while ( !(LgsValue ^ (unsigned __int8)~(LgsValue >> 4)) & 1) != 0 );
  //v24 = AvpDump;
  AvpDump = 0;
  CRTC_WR(RegisterBase + 6296532) = 31; //*((_BYTE *)
  Step = AvpDump; //v24
  
  if ( AvpDump )
  {
  DbgPrint("%08X = %08X\n", 6296532, 31);
  CRTC_WR(RegisterBase + 6296533) = 87; //*((_BYTE *)
  }
 
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296533, 87);
  }
  
  AvpDump = Step;

  if ( Step )
  {
    DbgPrint("CR%02X = %02X\n", 31, 87);
  }

  //v25 = AvpDump;
  AvpDump = 0;
  CRTC_WR(RegisterBase + 6296532) = 33; //*((_BYTE *)
  Step = AvpDump; //v25

  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296532, 33);
  }
  
  CRTC_WR(RegisterBase + 6296533) = -1; //*((_BYTE *)

  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296533, 255);
  }
  
  AvpDump = Step;

  if ( Step )
  {
    DbgPrint("CR%02X = %02X\n", 33, 255);
  }
  
  //v26 = AvpDump;
  AvpDump = 0;
  CRTC_WR(RegisterBase + 6296532) = 40; //*((_BYTE *)
  Step = AvpDump; //v26

  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296532, 40);
  }
  
  CRTC_WR(RegisterBase + 6296533) = 0; //*((_BYTE *)

  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296533, 0);
  }
  
  AvpDump = Step;

  if ( Step )
  {
    DbgPrint("CR%02X = %02X\n", 40, 0);
  }
  REG_WR32(RegisterBase + 1704480) = 554832145; //*((_DWORD *)

  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6817920, 554832145);
  }
  
  if ( TVEncoderSMBusID == 0x8A )
  {
    SMB_WR(RegisterBase, 0xBAu, 0x80u);
    SMB_WR(RegisterBase, 0xBAu, 0x3Fu);
    SMB_WR(RegisterBase, 0x6Cu, 0x46u);
  
  }
  
  
  KeStallExecutionProcessor(1);
  
  if ( TVEncoderSMBusID == 0x8A )
  {
    SMB_WR(RegisterBase, 0x6Cu, 0xC6u);
  }
  
  if ( OutputMode == 0x80000000 )
  {
  
    if ( TVEncoderSMBusID != 0xE0 )
    {
      v27 = &AV_TABLE_HDTVREGISTERS_DATA[iTV * AvpTableRowSize[2]];
      pByteMax = &v27[AvpTableRowSize[2]];
      for ( pRegByte = AV_TABLE_HDTVREGISTERS_DATA; &AV_TABLE_HDTVREGISTERS_DATA[iTV * AvpTableRowSize[2]] < pByteMax; ++pRegByte ) //v27
        SMB_WR(RegisterBase, *pRegByte, *&AV_TABLE_HDTVREGISTERS_DATA[iTV * AvpTableRowSize[2]]++); //v27
      goto LABEL_115;
    }
  }
  else if ( TVEncoderSMBusID != 0xE0 )
  {
    //v28 = *AvpTableRowSize;
    //v29 = &AV_TABLE_COMMONTVREGISTERS_DATA[*AvpTableRowSize]; //v28
    pByteMaxa = &AV_TABLE_COMMONTVREGISTERS_DATA[*AvpTableRowSize + *AvpTableRowSize]; //v28 
    pRegBytea = AV_TABLE_COMMONTVREGISTERS_DATA;
    if ( &AV_TABLE_COMMONTVREGISTERS_DATA[*AvpTableRowSize] < pByteMaxa ) //v28
    {
      do
	  {
        SMB_WR(RegisterBase, *pRegBytea++, *&AV_TABLE_COMMONTVREGISTERS_DATA[*AvpTableRowSize]++); //v29
      }
	  while ( &AV_TABLE_COMMONTVREGISTERS_DATA[*AvpTableRowSize] < pByteMaxa );
    }
  }
  
 // v30 = &AV_TABLE_TVREGISTERS_DATA[iTV * AvpTableRowSize[1]];
  pByteMaxb = &AV_TABLE_TVREGISTERS_DATA[AvpTableRowSize[1]]; //v30
  
  for ( pRegByteb = AV_TABLE_TVREGISTERS_DATA; &AV_TABLE_TVREGISTERS_DATA[iTV * AvpTableRowSize[1]] < pByteMaxb; ++pRegByteb )
  {
    SMB_WR(RegisterBase, *pRegByteb, *&AV_TABLE_TVREGISTERS_DATA[iTV * AvpTableRowSize[1]]++);
  }
  
  if ( XboxGameRegion == 2 && (iTV == 13 || iTV == 14 || iTV == 18) )
  {
    if ( TVEncoderSMBusID == 0xE0 )
    {
      //iRxd = AvpDump;
      AvpDump = 0;
      HalReadSMBusValue(0xE0u, 1u, 1u, &Step);
      //v31 = Step & 0xFFFF7FFF;
      AvpDump = 0;
      HalWriteSMBusValue(TVEncoderSMBusID, 1u, 1u, Step & 0xFFFF7FFF);
      //AvpDump = iRxd;
      if ( AvpDump )
	  {
        DbgPrint("TV%04X = %04X\n", 1, Step & 0xFFFF7FFF); //v31
	  }
	}
    else if ( TVEncoderSMBusID == 0xD4 )
    {
     // Step = AvpDump;
      AvpDump = 0;
      HalWriteSMBusValue(0xD4u, 0x4Eu, 1u, 0x13Cu);
      //AvpDump = Step;
      if ( AvpDump )
	  {
        DbgPrint("TV%04X = %04X\n", 78, 316);
	  }
	}
    else
    {
      SMB_WR(RegisterBase, 0xA2u, 8u);
      SMB_WR(RegisterBase, 0xA4u, 0xF0u);
      SMB_WR(RegisterBase, 0xACu, 0x9Au);
    }
  }
LABEL_115:
// Enable SCART?
  pByteMaxc = (const unsigned __int8 *)(Mode & 0x20000000);
  // if (pByteMaxc != 0) { }
  if ( (Mode & 0x20000000) != 0 )
  {
    AvpEnableSCART(RegisterBase, iTV);
  }
//
  if ( TVEncoderSMBusID == 0xE0 )
  {
    if ( OutputMode == 0x80000000 )
    {
      //v32 = AvpDump;
      AvpDump = 0;
      HalWriteSMBusValue(0xE0u, 7u, 1u, 0);
      //AvpDump = v32;
      if ( !AvpDump )
        goto LABEL_122;
      DbgPrint("TV%04X = %04X\n", 7, 0);
    }
  }
    if ( !OutputMode )
    {
LABEL_124:
      //iRxe = AvpDump;
      AvpDump = 0;
      HalReadSMBusValue(TVEncoderSMBusID, 0, 1u, &Step);
     // v34 = Step | 2;
      AvpDump = 0;
      HalWriteSMBusValue(TVEncoderSMBusID, 0, 1u, Step | 2);
      //AvpDump = iRxe;
      if ( AvpDump )
	  {
        DbgPrint("TV%04X = %04X\n", 0, Step | 2); //v34
      }
	  WriteMVReg(RegisterBase, 0xFu, 0);
      goto LABEL_127;
    }
LABEL_122:
    //v33 = AvpDump;
    AvpDump = 0;
    HalWriteSMBusValue(TVEncoderSMBusID, 9u, 1u, 0);
    //AvpDump = v33;
    if ( AvpDump )
	{
      DbgPrint("TV%04X = %04X\n", 9, 0);
    }
	goto LABEL_124;
  
LABEL_127:
  __outbyte(0x80D6u, ((Mode & 0x10000000) != 0) | 4);
  __outbyte(0x80D8u, 4u);
  //v35 = AvpDump;
  AvpDump = 0;
  Step = AvpDump; //v35
  v36 = REG_OR32(RegisterBase + 1704259) | 0x10020000; //  v36 = *((_DWORD *) REG_OR32 or REG_WR08     1704259 =? NV_PRAMDAC_PLL_COEFF_SELECT ??
 // *((_DWORD *)RegisterBase + 1704259) = v36;
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6817036, v36);
  }
  
  AvpDump = Step;
  
  if ( Step )
  {
    DbgPrint("%08X = %08X (read modify write)\n", 6817036, 268566528);
  }
  
  REG_WR08(RegisterBase + 787395) = 1; //*((_BYTE *)   787395 =? NV_PRMVIO_VSE2
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 787395, 1);
  
  }
  
  REG_WR08(RegisterBase + 787394) = -29; // *((_BYTE *)    787394 =? NV_PRMVIO_MISC__WRITE
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 787394, 227);
  }
//
// basic/generic Control registers I think for SCART
  REG_WR32(RegisterBase + 1704320) = GeneralControl; // *((_DWORD *)   1704320 =? NV_PRAMDAC_GENERAL_CONTROL
 
 if ( AvpDump )
 {
    DbgPrint("%08X = %08X\n", 6817280, GeneralControl);
 }
// Unsure of below unill the end of the function 
  v37 = &AV_TABLE_DACREGISTERS_DATA[BYTE2(Mode) * AvpTableRowSize[5]];
  v38 = &v37[4 * (AvpTableRowSize[5] >> 2)];
  iRx = (unsigned int)v38;
  Step = (unsigned int)AV_TABLE_DACREGISTERS_DATA;

  if ( v37 < v38 )
  {
    do
    {
      v39 = *(_DWORD *)v37;
      v40 = *(_DWORD *)Step;
      *(_DWORD *)((char *)RegisterBase + *(_DWORD *)Step) = *(_DWORD *)v37;
      if ( AvpDump )
	  {
        DbgPrint("%08X = %08X\n", v40, v39);
      }
	  Step += 4;
      v37 += 4;
    }
    while ( (unsigned int)v37 < iRx );
  }
  if ( pByteMaxc )
  {
	  // SCART Output as RGB set here ?
	REG_WR32(RegisterBase + 1704332) = 0; //    *((_DWORD *)
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6817328, 0);
    }
	
	REG_WR32(RegisterBase + 1704497) = 0;//    *((_DWORD *)
    
	if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6817988, 0);
    }
	
	REG_WR32(RegisterBase + 1704467) = 0;//    *((_DWORD *)
    
	if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6817868, 0);
	}
  }
  
  v41 = 0; // I think result should be '=AvpSRXRegisters;'
  iRxa = 0; // From notes this should do v41 + sizeof(SOMETHING)
  do
  {
    v42 = AvpDump;
    v43 = AvpSRXRegisters[v41];
    AvpDump = 0;
	*((_BYTE *)RegisterBase + 787396) = v41; // SRX_WR(RegisterBase..... ???  
    HIBYTE(Step) = v43;
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 787396, (unsigned __int8)iRxa);
    }
	v44 = HIBYTE(Step);
    *((_BYTE *)RegisterBase + 787397) = HIBYTE(Step);// GRX_WR(RegisterBase..... ???  
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 787397, v44);
    }
	//AvpDump = v42;

    if ( AvpDump ) //v42
	{
      DbgPrint("SR%02X = %02X\n", (unsigned __int8)iRxa, HIBYTE(Step));
    }
	v41 = iRxa + 1; // = AvpARXRegisters?
    v45 = &AvpSRXRegisters[++iRxa];
  }
  while ( v45 < (const unsigned __int8 *)byte_80019821 );
  v46 = 0;
  iRxb = 0;
  do
  {
    v47 = AvpDump;
    v48 = AvpGRXRegisters[v46];
    AvpDump = 0;
    *((_BYTE *)RegisterBase + 787406) = v46;  //// ARX_WR(RegisterBase..... ???  
    HIBYTE(Step) = v48;
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 787406, (unsigned __int8)iRxb);
	}
	
    v49 = HIBYTE(Step);
    *((_BYTE *)RegisterBase + 787407) = HIBYTE(Step); // REG_WR08(RegisterBase..... ???  
	
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 787407, v49);
    }
	AvpDump = v47;
    
	if ( v47 )
	{    
	 DbgPrint("GR%02X = %02X\n", (unsigned __int8)iRxb, HIBYTE(Step));
    }
	
	v46 = iRxb + 1;
    v50 = &AvpGRXRegisters[++iRxb];
  }
  while ( v50 < (const unsigned __int8 *)byte_80019819 );
  v51 = 0;
  iRxc = 0;
  do
  {
    HIBYTE(Step) = AvpARXRegisters[v51];
    v52 = AvpDump;
    AvpDump = 0;
    *((_BYTE *)RegisterBase + 6296512) = v51;
    OutputModea = v52;
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296512, (unsigned __int8)iRxc);
    }
	v53 = HIBYTE(Step);
    *((_BYTE *)RegisterBase + 6296512) = HIBYTE(Step);
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296512, v53);
    }
	
	//AvpDump = OutputModea;
    if ( AvpDump ) //OutputModea
	{
      DbgPrint("AR%02X = %02X\n", (unsigned __int8)iRxc, HIBYTE(Step));
    }
	v51 = iRxc + 1;
    v54 = &AvpARXRegisters[++iRxc];
  }
  while ( v54 < (const unsigned __int8 *)byte_8001980D );
  *((_BYTE *)RegisterBase + 6296512) = 32;
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296512, 32);
  }
  
  v55 = AvpDump;
  AvpDump = 0;
  
  *((_BYTE *)RegisterBase + 6296532) = 17;
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296532, 17);
  }
  
  *((_BYTE *)RegisterBase + 6296533) = 0;
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296533, 0);
  }
  //AvpDump = v55;
  if ( AvpDump ) //v55
  {
    DbgPrint("CR%02X = %02X\n", 17, 0);
  }
  v56 = &AV_TABLE_CRTCREGISTERS_DATA[iCRTC * AvpTableRowSize[6]];
  pByteMaxd = &v56[AvpTableRowSize[6]];
  
  for ( pRegBytec = AV_TABLE_CRTCREGISTERS_DATA; v56 < pByteMaxd; ++pRegBytec )
  {
    v57 = *pRegBytec;
    v58 = *pRegBytec == 19;
    iRx_3 = *pRegBytec;
    HIBYTE(Step) = *v56;
    if ( v58 )
    {
      HIBYTE(Step) = Pitch;
    }
    else
    {
      if ( v57 == 25 )
      {
        v59 = (Pitch >> 3) & 0xE0;
LABEL_187:
        HIBYTE(Step) |= v59;
        goto LABEL_188;
      }
      if ( v57 == 37 )
      {
        v59 = (Pitch >> 6) & 0x20;
        goto LABEL_187;
      }
    }
LABEL_188:
    v60 = AvpDump;
    AvpDump = 0;
    *((_BYTE *)RegisterBase + 6296532) = v57;
    iCRTCa = v60;
	
    if ( AvpDump )
	{
      DbgPrint("%08X = %08X\n", 6296532, iRx_3);
    }
	
	v61 = HIBYTE(Step);
    *((_BYTE *)RegisterBase + 6296533) = HIBYTE(Step);
    
	if ( AvpDump )
    {
	 DbgPrint("%08X = %08X\n", 6296533, v61);
    }
	AvpDump = iCRTCa;
    
	if ( iCRTCa )
	{
      DbgPrint("CR%02X = %02X\n", iRx_3, HIBYTE(Step));
    }
	++v56;
  }
  v62 = (LgsValue & 1) == 0;
  REG_WR32(RegisterBase + 1704360) = v62;
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6817440, v62);
  }
  //v63 = AvpDump;
  AvpDump = 0;
  v64 = HIBYTE(Format) | 0x80;
  *((_BYTE *)RegisterBase + 6296532) = 40;
  HIBYTE(Pitch) = v64;
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296532, 40);
  }
  v65 = HIBYTE(Pitch);
  *((_BYTE *)RegisterBase + 6296533) = HIBYTE(Pitch);
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6296533, v65);
  }
  //AvpDump = v63;
  
  if ( AvpDump )
  {
    DbgPrint("CR%02X = %02X\n", 40, HIBYTE(Pitch));
  }
  v66 = 0;
  v67 = REG_RD32(RegisterBase + 6296538); //(char *)
  do
  {
    while ( (*v67 & 8) != 0 )
      ;
    while ( (*v67 & 8) == 0 )
      ;
    ++v66;
  }
  while ( v66 < 3 );
  // Still unsure to my novice eyes
  v68 = *(_DWORD *)(4 * BYTE2(Mode) - 2147172064);
  REG_WR32(RegisterBase + 1704480) = v68;
 
 if ( AvpDump )
 {
    DbgPrint("%08X = %08X\n", 6817920, v68);
 }
  KeStallExecutionProcessor(1);
  // Focus
  if ( TVEncoderSMBusID == 0x8A )
  {
	  
    v69 = AvpDump;
    AvpDump = 0;
    HalReadSMBusValue(0x8A, 0x6C, 0, &Pitch);
    AvpDump = v69;
	
    SMB_WR(RegisterBase, 0x6C, Pitch | 0x80);
    SMB_WR(RegisterBase, 0xBA, DACs | 0x20);
  }
  
  if ( TVEncoderSMBusID == 0xE0 )
  {
	  /// XCALIBUR
   // v70 = AvpDump;
    AvpDump = 0;
    HalWriteSMBusValue(0xE0, 4u, 1u, DACs);
    //AvpDump = v70;
   
   if ( AvpDump ) //v70
   {
      DbgPrint("TV%04X = %04X\n", 4, DACs);
   }
  }
  
  //v71 = FrameBuffer;
  REG_WR32(RegisterBase + 1573376) = FrameBuffer; 
  
  if ( AvpDump )
  {
    DbgPrint("%08X = %08X\n", 6293504, Framebuffer);
  }
  
  AvpSetWSSBits(RegisterBase);
  return 0;
}

///
/// AvpSetMacrovisionMode
///
///  Sets up the macrovision stuff for each encoder/gpu
///
///  Should RETURN NONE/NOTHING
///
VOID AvpSetMacrovisionMode(
		in PVOID RegisterBase,
		IN ULONG MacrovisionMode)


{
  unsigned int v2; // eax
  int v3; // ecx
  unsigned __int8 *v4; // eax
  unsigned __int8 *v5; // esi
  unsigned __int8 *v6; // esi
  unsigned __int8 *v7; // edi
  int v8; // esi
  int v9; // ebx
  unsigned __int8 *v10; // edi
  int v11; // esi
  int v12; // ebx
  unsigned int v13; // edi
  __int16 v14; // ax
  unsigned __int16 v15; // ax
  int v16; // ebx
  int v17; // eax
  int v18; // ecx
  unsigned __int8 *v19; // edi
  unsigned __int8 *v20; // edi
  unsigned __int8 *v21; // esi
  int v22; // edi
  int v23; // ebx
  unsigned __int8 *v24; // esi
  int v25; // edi
  int v26; // ebx
  unsigned int v27; // esi
  int v28; // edi
  unsigned int DataValue; // [esp+0h] [ebp-4h] BYREF

  v2 = AvpCurrentMode & 0xC0000000;
  if ( (AvpCurrentMode & 0xC0000000) != 0 && v2 != 0x40000000 )
    return;
  if ( TVEncoderSMBusID == 0xE0 && !MacrovisionMode )
  {
    WriteMVReg(RegisterBase, 0xFu, 0);
    return;
  }
  if ( v2 )
  {
    v18 = AvpTableRowSize[4];
    v19 = &AV_TABLE_MACROVISIONPAL_DATA[2 * v18];
    if ( !MacrovisionMode )
      v19 = &AV_TABLE_MACROVISIONPAL_DATA[v18];
    if ( TVEncoderSMBusID == 0xE0 )
    {
      v20 = &v19[-v18];
      if ( AvpTableRowSize[4] )
      {
        v21 = AV_TABLE_MACROVISIONPAL_DATA;
        v22 = v20 - AV_TABLE_MACROVISIONPAL_DATA;
        v23 = AvpTableRowSize[4];
        do
        {
          WriteMVReg(RegisterBase, *v21, v21[v22]);
          ++v21;
          --v23;
        }
        while ( v23 );
      }
    }
    else if ( AvpTableRowSize[4] )
    {
      v24 = AV_TABLE_MACROVISIONPAL_DATA;
      v25 = v19 - AV_TABLE_MACROVISIONPAL_DATA;
      v26 = AvpTableRowSize[4];
      do
      {
        SMB_WR(RegisterBase, *v24, v24[v25]);
        ++v24;
        --v26;
      }
      while ( v26 );
    }
    v17 = 3;
    goto LABEL_40;
  }
  if ( MacrovisionMode )
  {
    if ( MacrovisionMode == 1 )
    {
      v3 = AvpTableRowSize[3];
      v4 = AV_TABLE_MACROVISION_DATA;
      v5 = &AV_TABLE_MACROVISION_DATA[2 * v3];
    }
    else
    {
      v3 = AvpTableRowSize[3];
      v4 = AV_TABLE_MACROVISION_DATA;
      if ( MacrovisionMode == 2 )
        v5 = &AV_TABLE_MACROVISION_DATA[2 * v3 + v3];
      else
        v5 = &AV_TABLE_MACROVISION_DATA[4 * v3];
    }
  }
  else
  {
    v3 = AvpTableRowSize[3];
    v4 = AV_TABLE_MACROVISION_DATA;
    v5 = &AV_TABLE_MACROVISION_DATA[v3];
  }
  if ( TVEncoderSMBusID != 0xE0 )
  {
    if ( v3 )
    {
      v10 = v4;
      v11 = v5 - v4;
      v12 = v3;
      do
      {
        SMB_WR(RegisterBase, *v10, v10[v11]);
        ++v10;
        --v12;
      }
      while ( v12 );
    }
    goto LABEL_22;
  }
  v6 = &v5[-v3];
  if ( v3 )
  {
    v7 = v4;
    v8 = v6 - v4;
    v9 = v3;
    do
    {
      WriteMVReg(RegisterBase, *v7, v7[v8]);
      ++v7;
      --v9;
    }
    while ( v9 );
LABEL_22:
    if ( TVEncoderSMBusID == 0xD4 )
    {
      v13 = AvpDump;
      AvpDump = 0;
      HalReadSMBusValue(0xD4u, 0xB2u, 1u, &DataValue);
      v14 = DataValue & 0xC000;
      v15 = MacrovisionMode ? v14 | 0x34D : v14 | 0x555;
      AvpDump = 0;
      v16 = v15;
      HalWriteSMBusValue(TVEncoderSMBusID, 0xB2u, 1u, v15);
      AvpDump = v13;
      if ( v13 )
        DbgPrint("TV%04X = %04X\n", 178, v16);
    }
  }
  v17 = (XboxGameRegion == 2) + 1;
LABEL_40:
  if ( TVEncoderSMBusID == 0xD4 )
  {
    v27 = 0;
    if ( !MacrovisionMode )
      v17 += 3;
    v28 = 6 * v17 - 2147379332 - (_DWORD)"NOPQ^_8";
    do
    {
      SMB_WR(RegisterBase, Focus_MacrovisionHamp[0][v27], Focus_MacrovisionHamp[0][v27 + v28]);
      ++v27;
    }
    while ( v27 < 6 );
  }
}