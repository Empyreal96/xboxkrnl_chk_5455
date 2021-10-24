///
/// process creation and control
///
///  empyreal96

#include "error.h"
#include "sos.h"
#include "protos.h"
#include "externs.h"

struct process *pp

int create_process(ushort *mem_ptr, 
					uchar level, 
					uchar channel)
{
  //__int16 v3; // cx
  struct process *tpp; // eax // v4
  bool tpp; // zf  v5

  pp = get_process_packet();
  pp->mem_ptr.c = mem_ptr;
  pp->level = level;
  pp->timer = 0;
  pp->hard_channel = channel;
  pp->function = *mem_ptr;
  pp->type = 0;
  pp->prev = process_queue;
  //v3 = system_clock_music;
  
  if ( level == 0)
		pp->prev_timer = system_clock_music; 
	else
		pp->prev_timer = system_clock_fx;
  tpp = process_queue->next;  // v4
  tpp = process_queue->next == 0;
  pp->next = process_queue->next;
  if (tpp != NULL ){
    tpp->prev = pp;  // v4
  }
  process_queue->next = pp;
  return 1;
}



void kill_process(process *pp)
{
  pp->prev->next = pp->next;
  if ( pp->next != NULL)
    pp->next->prev = pp->prev;

  pp->next = queue_list[0].next;
  pp->prev = queue_list;
  queue_list[0].next = pp;
  if ( pp->next == NULL)
    pp->next->prev = pp;
}

process *get_process_packet()
{
  process *pp; // eax

  pp = queue_list[0].next;
  queue_list[0].next = queue_list[0].next->next;
  if ( pp->next == (struct process *)NULL)
    pp->next->prev = &(queue_list[0]);
  return pp;
}
