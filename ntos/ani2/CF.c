///
/// sound call pocessing
///
/// empyreal96
///

#include "sos.h"
#include "protos.h"
#include "externs.h"
#include <dsound.h>

extern	unsigned char	sound_call;	
extern struct timer_music music_timers[];
extern	unsigned char	n_timers;


///
/// Music calls 
///
/// TODO

void call_silence()
{
  process *pp; // eax   v0
  process *tpp; // esi    v1
  unsigned __int8 i; // [esp+4h] [ebp-4h]

  pp = process_queue->next;
  if ( process_queue->next )
  {
    do
    {
      tpp = pp->next;
      kill_process(pp);
      pp = tpp;
    }
    while ( tpp );
  }
  current_timer_priority = 0;
  for ( i = 0; i < max_tracks; ++i )
  {
    channel_level[i] = 0;
    init_track_status(LEVEL_MUSIC, i); //0
    init_track_status(LEVEL_EFFECT, i);  //  1u
    nosound(i);
  }
  return call_user_function();
}

///
unsigned __int8 __stdcall call_music()
{
  unsigned __int8 result; // al
  ushort const **track_ptr; // edi  v1
  unsigned __int8 *v2; // esi
  ushort track_map; // [esp+8h] [ebp-Ch]
  ushort mask; // [esp+Ch] [ebp-8h]
  uchar i; // [esp+10h] [ebp-4h]

  marker = 0;
  last_music_call = sound_call;
  
  remove_processes_by_level(LEVEL_MUSIC);// 0
  result = (unsigned __int8)current_call; // 
  track_ptr = current_call->tbl_ptr; // v1
  track_map = current_call->track_map;
  i = 0;
  for ( mask = 1; i < max_tracks; mask *= 2 )
  {
    if ( (track_map & mask) != 0 )
    {
      init_track_status(LEVEL_MUSIC, i); // 0
      create_process((ushort*)*(track_ptr++),LEVEL_MUSIC,i); //0 /v1
      v2 = &channel_level[i];
      track_ptr++;
      if ( !*v2 )
      {
        nosound(i);
        *v2 = 0;
      }
    }
    result = ++i;
  }
  return result;
}









void call_effect()
{
  unsigned __int8 result; // al
  ushort const **track_ptr; // esi  v1
  ushort track_map; // di   v2
  ushort mask; // [esp+Ch] [ebp-8h]
  uchar i; // [esp+10h] [ebp-4h]

  result = (unsigned __int8)current_call;
  track_ptr = current_call->tbl_ptr;  // v1
  track_map = current_call->track_map; // v2
  i = 0;
  for ( mask = 1; i < max_tracks; mask *= 2 )
  {
    if ( (track_map & mask) != 0 ) // v2
    {
      remove_processes_by_level_and_channel(LEVEL_EFFECT, i); //1u
      channel_level[i] = LEVEL_EFFECT; //1
      init_track_status(LEVEL_EFFECT, i); // 1u
      create_process((ushort *)*(track_ptr++), LEVEL_EFFECT, i); // v1  1u
    }
    result = ++i;
  }
  return result;
}




///
/// Mark sound calls
///
void call_mark()
{
  unsigned __int8 result; // al

  result = current_call->priority;
  marker = result;
  return result;
}


init_track_status(uchar level, uchar channel)
{
  struct track_info	*ti; // eax

  ti = get_track_info_block2(level, channel);
  ti->patch = 0;
  ti->loop_level = 0;
  ti->mux_level = 0;
  ti->transpose = 0;
  ti->filtercutoff = 0;
  ti->volume = 127;
  ti->pan = 0;
  return ti;
}


///
/// stop sound on channel
///
void nosound(uchar chan)
{
  if ( chan < max_tracks )
    silence_functions[chan](chan);
}


///
/// Remove all processes of 'level'
///
remove_processes_by_level(uchar level)
{
  process *pp; // eax  v1
  process *tpp; // esi v2

  pp = process_queue->next;
  if ( process_queue->next )
  {
    do
    {
      tpp = pp->next;
      if ( pp->level == level )
        kill_process(pp);
      pp = tpp;
    }
    while ( tpp );
  }
  return pp;
}


///
///
///

remove_processes_by_level_and_channel(uchar level, uchar channel)
{
  process *pp; // eax v2
  process *tpp; // esi  v3

  pp = process_queue->next;
  if ( process_queue->next )
  {
    do
    {
      tpp = pp->next;
      if ( pp->level == level && pp->hard_channel == channel )
        kill_process(pp);
      pp = tpp;
    }
    while ( tpp );
  }
}


