#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "libipc2.h"

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "event.h"
#include "util.h"
#include "tc2_accept.h"

struct event *tc2_q_accept(struct tcds *t) {
  struct event *acc ;
  struct timeval z ;
  fd_set *socks ;

  z.tv_sec = 0 ;
  z.tv_usec = 0 ;

  if (NULL == (socks = (fd_set *)malloc(sizeof(fd_set)))) {
    log_err("tc2_q_accept", "malloc failed fd_set", TRUE) ;
    return(EVENT_NULL) ;
  }

  FD_ZERO(socks) ;
  FD_SET(t->master_sock, socks) ;
  if (-1 == select(getdtablesize(), socks, NULL, NULL, &z)) {
    log_err("tc2_q_accept", "select failed", TRUE) ;
    free(socks) ;
    return(EVENT_NULL) ;
  }

  if (FD_ISSET(t->master_sock, socks)) {
    if (EVENT_NULL == (acc = new_event())) { free(socks); return(EVENT_NULL); }
    acc->f = tc2_accept ;
    acc->a.i = 1 ;
    free(socks) ;
    return(acc) ;
  }
  free(socks) ;
  return(EVENT_NULL) ;
}

int tc2_accept(struct tcds *t, struct event *e) {
  struct user *u ;
  int sock ;

  if (FALSE == (sock = new_sock(t->ss, t->master_sock))) {
    log_err("tc2_accept", "new_sock() failed", FALSE) ;
    return(FALSE) ;
  }

  if (USER_NULL == (u = new_user(t, sock, time(0), NULL))) {
    return(FALSE) ;
  }

  tc2_greet(t, u) ; /* Ignore return code, what would we do if false? =) */
  return(TRUE) ;
}

int tc2_greet(struct tcds *t, struct user *u) {
  char *str ;

  if (TCDS_NULL == t) { return(FALSE) ; }
  if (USER_NULL == u) { return(FALSE) ; }
  
  if (FALSE == send_file(t, u, GREET_FILE)) {
    if (NULL == (str = (char *)malloc(83 * sizeof(char)))) {
      log_err("tc2_greet", "malloc failed on greet string", TRUE) ;
      return(FALSE) ;
    }
    strcpy(str, "*** Welcome to TC2.0!           (/h for help.)\r\n") ;
    write_sock(t->ss, u->sock, str) ;
    if (NULL == (str = (char *)malloc(83 * sizeof(char)))) {
      log_err("tc2_greet", "malloc failed on greet string", TRUE) ;
      return(FALSE) ;
    }
    strcpy(str, "Enter your name (20 chars max):\r\n") ;
    write_sock(t->ss, u->sock, str) ;
  }
  return(TRUE) ;
}

