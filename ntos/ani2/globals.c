#include "sos.h"
#include "error.h"
#include "protos.h"


struct	process	*  process_queue;		
struct sound	*  current_call;		
struct process *  current_process;	
struct	fifo fifo;			
struct	track_info	*  ti;   	
ushort		value_16_bit;	
short 		system_clock_music ;
uchar		system_clock_mlow;  
short 		system_clock_fx; 
uchar		system_clock_fxlow; 
uchar 		clock_cntr[2];	
uchar 		clock_cntr_value[2];	
uchar		clock_cntr_save;	
ushort 		global_music_xpose;
ushort		global_effect_xpose;	
uchar		a_value;		
uchar		b_value;		
uchar		master_music_volume;	
uchar		master_effect_volume;	
uchar		music_atten;		
uchar		marker;				
uchar		current_level;		
uchar		current_channel;	
ushort		dp_word1;			
uchar		gtifbtmp;			
uchar		last_music_call;
uchar		current_timer_priority;
uchar		sos_only;			
uchar		sound_call_table;	
ushort		global_fxpose;		
