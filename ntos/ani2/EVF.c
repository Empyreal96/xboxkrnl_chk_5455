///
/// Event functions
///
/// empyreal96
///
#include	"sos.h"
#include	"protos.h"
#include	"externs.h"
#define	c_p 	current_process		


ushort	* ptr;			
unsigned short	un_int;
uchar			un_char;

int	f_MidiNoteOn()
{
	return(1);
}
int	f_MidiNoteOff()
{
	return(1);
}
int	f_MidiProgram()
{
	return(1);
}
int	f_MidiVolume()
{
	return(1);
}
int	f_MidiTempo()
{
	return(1);
}
int	f_MidiControl()
{
	return(1);
}
int	f_MidiEOT()
{
	return(1);
}



int f_note()
{
  char v0; // dl
  unsigned __int16 *ptr; // eax  v1
  bool v2; // sf
  unsigned __int16 v3; // dx

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  v0 = *(_BYTE *)ptr;
  ptr = ptr + 1;   v1
  v2 = *(char *)ptr < 0;
  b_value = *(_BYTE *)ptr++;
  if ( v2 )
  {
    dp_word1 = v1;  // v1
    b_value = v0 & 0x7F;
  }
  else
  {
    v3 = *(unsigned short *)ptr++;  // v1
    dp_word1 = v3;
    ptr = ptr;  //v1
  }
  current_process->function = *(ptr);  //v1
  current_process->mem_ptr.c = ptr;
  current_process->timer += dp_word1;
  value_16_bit = ti->transpose + (b_value << 8);
  ti->pitch = value_16_bit;
  if ( current_level == 1 )
    ti->pitch += global_fxpose;
  value_16_bit = ti->pitch;
  if ( current_level >= channel_level[current_channel] )
    note_on_functions[current_channel]();
  return 1;
}


/// TODO
int __stdcall f_rest()
{
  unsigned __int16 v0; // dx

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  v0 = *ptr++;
  un_int = v0;
  current_process->function = *ptr;
  current_process->timer += un_int;
  current_process->mem_ptr.c = ptr;
  if ( current_level >= channel_level[current_channel] )
    note_off_functions[current_channel]();
  return 1;
}


int f_jumpto()
{
  current_process->mem_ptr.c += 2;
  ptr = *(ushort **)current_process->mem_ptr.c;
  current_process->mem_ptr.c += 2;
  current_process->mem_ptr.c = ptr;
  current_process->function = *ptr;
  return 1;
}

int f_loop()
{
  current_process->mem_ptr.c += 2;
  ti->loop_counter[ti->loop_level] = *current_process->mem_ptr.i;
  current_process->mem_ptr.c += 2;
  ti->loop_addr[ti->loop_level++] = current_process->mem_ptr.i;
  current_process->function = *current_process->mem_ptr.i;
  return 1;
}

/// TODO
int f_endloop()
{
  unsigned __int16 *v0; // eax
  int v1; // ecx

  v0 = &ti->pitch + ti->loop_level;
  --*v0;
  v1 = ti->loop_level;
  if ( *(&ti->pitch + v1) )
  {
    current_process->mem_ptr.c = *(unsigned __int8 **)&ti->loop_counter[2 * v1 + 2];
  }
  else
  {
    current_process->mem_ptr.c += 2;
    --ti->loop_level;
  }
  current_process->function = *current_process->mem_ptr.i;
  return 1;
}



int f_patch()
{
  ushort pat; // dx  v0

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  pat = *ptr++; //v0
  current_process->function = *(ptr);
  current_process->mem_ptr.c = ptr;
  ti->patch = pat; //v0
  ti->volume = 0;
  ti->pan = 0;
  if ( current_level >= channel_level[current_channel] )
    patch_functions[current_channel])(pat); //v0
  return 1;
}


int f_pan()
{
  unsigned __int8 a_value; // dl

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  a_value = *(uchar *)ptr++;
  
  current_process->function = *ptr;
  current_process->mem_ptr.c = (unsigned __int8 *)ptr;
  ti->pan = a_value;
  if ( current_level >= channel_level[current_channel] )
    pan_functions[current_channel])(
      current_channel,
      a_value,
      ti->patch);
  return 1;
}


int __stdcall f_paninc()
{
  unsigned __int8 a_value; // dl

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  a_value = *(char *)ptr++;
  // = v0;
  current_process->function = *ptr;
  current_process->mem_ptr.c = ptr;
  ti->pan += (signed char) a_value;
  if ( current_level >= channel_level[current_channel] )
    pan_functions[current_channel])(
      current_channel,
      ti->pan,
      ti->patch);
  return 1;
}


int f_mux()
{
  ti->mux_addr[ti->mux_level++] = (ushort *)(++current_process->mem_ptr.c + 2);
  current_process->mem_ptr.c = (uchar *)*(ushort *)(current_process->mem_ptr.i);
  current_process->function = *current_process->mem_ptr.c;
  return 1;
}

int  f_demux()
{
  un_char = --(ti->mux_level);
  current_process->mem_ptr.c = (uchar *)ti->mux_addr[un_char];
  current_process->function = *current_process->mem_ptr.c;
  return 1;
}


int  f_volume()
{
  current_process->mem_ptr.c += 2;
  un_char = *current_process->mem_ptr.c + ti->volume;
  current_process->mem_ptr.c += 2;
  current_process->function = *current_process->mem_ptr.i;
  ti->volume = un_char;
  if ( current_level >= channel_level[current_channel] )
    volume_functions[current_channel])(
      un_char,
      current_level,
      ti->patch);
  return 1;
}


int  f_fxset()
{
  global_fxpose = *++current_process->mem_ptr.c;
  current_process->mem_ptr.c += 2;
  current_process->function = *current_process->mem_ptr.c;
  return 1;
}


int f_xpose()
{
  current_process->mem_ptr.c += 2;
  ti->transpose += *current_process->mem_ptr.i;
  current_process->mem_ptr.c += 2;
  current_process->function = *current_process->mem_ptr.i;
  return 1;
}


int f_xset()
{
  current_process->mem_ptr.c += 2;
  ti->transpose = *current_process->mem_ptr.i;
  current_process->mem_ptr.c += 2;
  current_process->function = *current_process->mem_ptr.i;
  return 1;
}




////
////
//// int f_filterinc()




///
///
/// int f_filterset()


intf_slur()
{
  char v0; // dl
  unsigned __int16 *ptr; // eax v1
  bool v2; // sf
  unsigned __int16 dp_word1; // dx  v3

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  v0 = *(_BYTE *)ptr;  //
  ptr = ptr + 1;  // v1
  v2 = *(char *)ptr < 0;
  b_value = (uchar) *(ushort *)ptr++;  
  if ( v2 )
  {
    dp_word1 = *(uchar *)ptr;  //v1
    b_value = v0 & 0x7F;
  }
  else
  {
    dp_word1 = *(unsigned short *)ptr++;   //v1  v3
    
    ptr = ptr;  //v1
  }
  current_process->function = *ptr; //v1
  current_process->mem_ptr.c = ptr;
  current_process->timer += dp_word1;
  value_16_bit = ti->transpose + (b_value << 8);
  ti->pitch = value_16_bit;
  if ( current_level == 1 )
    ti->pitch += global_fxpose;
  value_16_bit = ti->pitch;
  if ( current_level >= channel_level[current_channel] )
    slur_functions[current_channel]();
  return 1;
}


int __stdcall f_ring()
{
  unsigned __int16 un_int; // dx

  current_process->mem_ptr.c += 2;
  ptr = current_process->mem_ptr.i;
  un_int = *ptr++;
  // = v0;
  current_process->function = *ptr;
  current_process->timer += un_int;
  current_process->mem_ptr.c = (unsigned __int8 *)ptr;
  return 1;
}


/// TODO
int  f_clockset()
{
  process *v0; // eax

  ++current_process->mem_ptr.c;
  v0 = current_process;
  clock_cntr_value[current_process->level] = *current_process->mem_ptr.c;
  ++v0->mem_ptr.c;
  current_process->function = *current_process->mem_ptr.c;
  return 1;
}




int f_end()
{
  return 0;
}


int f_mark()
{
  return 1;
}


int f_sound_call()
{
  unsigned __int8 *dest; // ecx
  //unsigned __int8 v1; // dl

  dest = ++current_process->mem_ptr.c;
  
  current_process->mem_ptr.c = dest + 1;
  current_process->function = *current_process->mem_ptr.c;
  put_fifo(dest);
  return 1;
}


// IDA NOTE: positive sp value has been detected, the output may be wrong!
int f_user_1_var_evf()
{
  unsigned __int8 v1; // [esp-Ch] [ebp-Ch]
  char v2; // [esp-8h] [ebp-8h]
  unsigned __int8 *v3; // [esp-4h] [ebp-4h]

  a_value = *(++current_process->mem_ptr.c);
  current_process->function = *++current_process->mem_ptr.c;
  user_1_var_evf_functions[current_channel]()/*(v1, v2, v3)*/;
  return (1);
}



// IDA NOTE: positive sp value has been detected, the output may be wrong!
int  f_user_2_var_evf()
{
  unsigned __int8 v1; // [esp-Ch] [ebp-Ch]
  char v2; // [esp-8h] [ebp-8h]
  unsigned __int8 *v3; // [esp-4h] [ebp-4h]

  a_value = *(++current_process->mem_ptr.c);
  b_value = *(++current_process->mem_ptr.c);
  current_process->function = *++current_process->mem_ptr.c;
  user_2_var_evf_functions[current_channel])(/*v1, v2, v3*/);
  return 1;
}



int  f_sig()
{
  a_value = *(++current_process->mem_ptr.c);
  current_process->function = *++current_process->mem_ptr.c;
  return 1;
}