///
/// NTOS Initilization components
///
/// empyreal96
///

#include "ntos.h"
#include <ntimage.h>
#include <ntddcdrm.h>
#include <ntdddisk.h>
#include <ntverp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ldr.h>
#include <xbeimage.h>
#include <dm.h>
#include <xlaunch.h>
#include <ani.h>
#include <xprofp.h>
#include <bldr.h>
#include <rc4.h>
#include <xcrypt.h>
#include <smcdef.h>

#include <wtypes.h>
#include <xconfig.h>
#include <xbox.h>
#include <xboxp.h>



//
// Initilization routine of the kernel
// 

VOID
ExpInitializeExecutive(
	VOID
	)
{
  unsigned __int8 result; // al   TODO

  if (KeHasQuickBooted) 
  {
    HalInitSystemPhase0Quick();
  }
  else
  {
    HalInitSystemPhase0();
  }
  
  HalPulseHardwareMonitorPin();
  MmInitSystem();
  
  if (!KeHasQuickBooted)
  {
    ExpInitializeHardwareInfo();
  }
  
  if (!ObInitSystem())
  {
    KeBugCheck(OBJECT_INITIALIZATION_FAILED); // 0x5Eu
  }
  
  result = PsInitSystem();
  
  if (!result)  // TODO
  {
    KeBugCheck(PROCESS_INITIALIZATION_FAILED); // 0x60u
  }
  return result; // TODO
}