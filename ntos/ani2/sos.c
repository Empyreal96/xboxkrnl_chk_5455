///
/// Sound Operating System
///
/// empyreal96


#include "sos.h"
#include "protos.h"
#include "ftables.h"		
#include "externs.h"
#include <xtl.h>
#include <stdio.h>


unsigned char	sound_call;
extern	uchar	max_sound_call;
extern	ushort	default_clock_value;
extern	const ushort	max_processes;
extern	_base_	uchar	gtifbtmp;



///
///
/// TODO LOTS
void sos_main(void)
{
  process *v0; // eax
  bool i; // zf
  process *v2; // esi
  unsigned __int8 v3; // dl
  unsigned __int8 v4; // cl

  do
  {
    if ( fifo.fifo_read != fifo.fifo_write )
    {
      LOBYTE(v0) = get_fifo();
      sound_call = (unsigned __int8)v0;
      if ( (unsigned __int8)v0 > max_sound_call )
        continue;
      current_call = (sound *)(8 * (unsigned __int8)v0 - 2146388448);
      call_fcns[current_call->type]();
    }
    v0 = process_queue->next;
    for ( i = process_queue->next == 0; ; i = v2 == 0 )
    {
      current_process = v0;
      if ( i )
        break;
      v2 = v0->next;
      v0->timer += v0->prev_timer - system_clock_music;
      current_process->prev_timer = system_clock_music;
      if ( current_process->timer < 0 )
      {
        v3 = current_process->hard_channel;
        current_channel = v3;
        current_level = current_process->level;
        v4 = current_level ? v3 + max_tracks : v3;
        gtifbtmp = v4;
        ti = &track_status[v4];
        if ( !event_fcns[current_process->function]() )
        {
          v0 = process_queue->next;
          current_process = process_queue->next;
          break;
        }
      }
      v0 = v2;
    }
  }
  while ( sos_only );
  return (char)v0;
}



void do_sos_init_return()
{
  int result; // eax

  sos_only = 0;
  fifo.fifo_read = 0;
  fifo.fifo_write = 0;
  dev_init();
  result = init_queuelist();
  system_clock_music = 1;
  sound_call_table = 0;
  return result;
}



void put_fifo(uchar value)
{
  fifo.data[fifo.fifo_write++] = value;
  if ( fifo.fifo_write >= FIFO_SIZE )  //0x10u
    fifo.fifo_write = 0;
}

uchar get_fifo(void)
{
  uchar retval; // al

  retval = fifo.data[fifo.fifo_read];
  if ( ++fifo.fifo_read >= 0x10u )
    fifo.fifo_read = 0;
  return (retval);
}

///
///
/// TODO LOTS
init_queuelist()
{
  int v0; // edx
  unsigned __int8 v1; // bl
  int v2; // esi
  int v3; // eax
  unsigned int v4; // eax
  int v5; // eax
  process *v6; // eax

  v0 = max_processes;
  dword_80114BCC[0] = 0;
  v1 = 1;
  v2 = max_processes - 1;
  queue_list[0].next = (process *)dword_80114BE4;
  if ( v2 > 1 )
  {
    v3 = 1;
    do
    {
      v4 = 28 * v3;
      queue_list[v4 / 0x1C].next = (process *)&dword_80114BE4[v4 / 4];
      ++v1;
      dword_80114BCC[v4 / 4] = (int)&queue_list[v4 / 0x1C - 1];
      v3 = v1;
    }
    while ( v1 < v2 );
  }
  v5 = 28 * v0;
  *(_LIST_ENTRY **)((char *)&g_BootSoundDpc.DpcListEntry.Flink + v5) = 0;
  *(_LIST_ENTRY **)((char *)&g_BootSoundDpc.DpcListEntry.Blink + v5) = (_LIST_ENTRY *)&queue_list[v0 - 2];
  v6 = get_process_packet();
  process_queue = v6;
  if ( v6 )
  {
    v6->next = 0;
    process_queue->prev = 0;
    if ( max_tracks )
      memset(channel_level, 0, max_tracks);
  }
  return 0;
}

struct track_info *get_track_info_block2(uchar level, uchar chan)
{
  gtifbtmp = chan + (level)*max_tracks;
  return (&(track_status[gtifbtmp]));
}

