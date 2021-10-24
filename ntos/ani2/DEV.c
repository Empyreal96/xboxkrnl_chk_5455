///
/// definitions for specific devices
///
/// empyreal96

#define _base_

extern  struct DSPpatch *Patches[];
extern  unsigned short  Noise8192[];

#include    <dsound.h>
#include "sosdsp.h"
#include "sos.h"
#include "bootsnd.h"
#include    <xtl.h>
#include "dsptables.h"

#define MAX_BUFFERS 16