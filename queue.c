#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#include "defaults.h"
#include "tcds.h"
#include "event.h"
#include "queue.h"
#include "mods.h"

int load_q(struct tcds *t, time_t tic_start) {
  int i ;
  struct event *(*mod_q_fcns[MOD_Q_COUNT])(struct tcds *t) = { MOD_Q_FCNS } ;

  for (i = 0 ; i < MOD_Q_COUNT ; i++) {
    /* If out of time, come back to this later. */
    if ((tic_start+TIC_SECS) <= time(0)) break ;
    /* Else, call each queing function defined. */
    if (FALSE == add_to_el(t, mod_q_fcns[i](t))) { return(FALSE) ; }
  }
  return(TRUE) ;
}

int exec_q(struct tcds *t, time_t tic_start) {
  struct event *e ;

  /* Process as much of or all of the queue before the tic ends. */
  while ((tic_start+TIC_SECS) >= time(0)) {
    if (FALSE == (e = get_event(t))) break ;
    e->f(t, e) ;
  }

  return(TRUE) ;
}
