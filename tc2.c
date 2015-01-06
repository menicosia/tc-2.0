#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "libipc2.h"

/* TC2 specific includes */
#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "event.h"
#include "queue.h"
#include "channel.h"

/* TC2 global variables */
time_t genesis ;
struct tcds_l *top ;

int main(int argc, char **argv) {
  struct tcds *tmp ;
  time_t tic_start ;
  struct timeval z ;

  genesis = time(0) ;
  top = new_tcds_l() ;
  new_tcds(top, DEFAULT_PORT) ;

  while(1) {
    tic_start = time(0) ;
    tmp = top->t ;
    while (TCDS_NULL != tmp) {
      /* Load the queue */
      /* Note: Under heavy conditions, TC *could* spend too much time */
      /* loading the queue and not enough executing it. If thisis the */
      /* case, simply switch the order to execute before loading.     */
      if (FALSE == load_q(tmp, tic_start)) {
	log_err("main", "load_q failed", FALSE) ;
	exit(0) ;
      }
      /* Run anything in the queue */
      if (FALSE == exec_q(tmp, tic_start)) {
	log_err("main", "exec_q failed", FALSE) ;
	exit(0) ;
      }
      tmp = tmp->next ;

      z.tv_sec = 0 ;
      /* Sleep one fifth of a second. */
      z.tv_usec = 200000 ;
      select(0, NULL, NULL, NULL, &z) ;
    }
  }
  return(0) ;
}

