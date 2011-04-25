#ifndef SCHEDULER_INCLUDED
#define SCHEDULER_INCLUDED

/* Copyright (C) 2007 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/*
  Classes for the thread scheduler
*/

#ifdef USE_PRAGMA_INTERFACE
#pragma interface
#endif

class THD;

/* Functions used when manipulating threads */

struct scheduler_functions
{
  uint max_threads, *connection_count;
  ulong *max_connections;
  bool (*init)(void);
  bool (*init_new_connection_thread)(void);
  void (*add_connection)(THD *thd);
  void (*thd_wait_begin)(THD *thd, int wait_type);
  void (*thd_wait_end)(THD *thd);
  void (*post_kill_notification)(THD *thd);
  bool (*end_thread)(THD *thd, bool cache_thread);
  void (*end)(void);
};


/**
  Scheduler types enumeration.

  The default of --thread-handling is the first one in the
  thread_handling_names array, this array has to be consistent with
  the order in this array, so to change default one has to change the
  first entry in this enum and the first entry in the
  thread_handling_names array.

  @note The last entry of the enumeration is also used to mark the
  thread handling as dynamic. In this case the name of the thread
  handling is fetched from the name of the plugin that implements it.
*/
enum scheduler_types
{
  SCHEDULER_ONE_THREAD_PER_CONNECTION=0,
  SCHEDULER_NO_THREADS,
  SCHEDULER_TYPES_COUNT
};

void one_thread_per_connection_scheduler(scheduler_functions *func,
    ulong *arg_max_connections, uint *arg_connection_count);
void one_thread_scheduler(scheduler_functions *func);

enum pool_command_op
{
  NOT_IN_USE_OP= 0, NORMAL_OP= 1, CONNECT_OP, KILL_OP, DIE_OP
};

#if defined(HAVE_LIBEVENT) && !defined(EMBEDDED_LIBRARY)

#define HAVE_POOL_OF_THREADS 1

struct event;
 
class thd_scheduler
{
public:
  /*
    Thread instrumentation for the user job.
    This member holds the instrumentation while the user job is not run
    by a thread.

    Note that this member is not conditionally declared
    (ifdef HAVE_PSI_INTERFACE), because doing so will change the binary
    layout of THD, which is exposed to plugin code that may be compiled
    differently.
  */
  PSI_thread *m_psi;

  bool logged_in;
  struct event* io_event;
  LIST list;
  bool thread_attached;  /* Indicates if THD is attached to the OS thread */

#  ifndef DBUG_OFF
  char dbug_explain[512];
  bool set_explain;
#  endif

  thd_scheduler();
  ~thd_scheduler();
  bool init(THD* parent_thd);
  bool thread_attach();
  void thread_detach();
};

void pool_of_threads_scheduler(scheduler_functions* func);
#else

#define pool_of_threads_scheduler(A) \
  one_thread_per_connection_scheduler(A, &max_connections, \
                                      &connection_count)

class thd_scheduler
{};

#endif

#endif
