#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "libipc2.h"

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "event.h"
#include "channel.h"

/*** TCDS ***/
struct tcds_l *new_tcds_l() {
  struct tcds_l *new ;

  if (NULL == (new = (struct tcds_l *)malloc(sizeof(struct tcds_l)))) {
    log_err("new_tcds_l", "malloc failed", TRUE) ;
    return(TCDS_L_NULL) ;
  }
  new->t = TCDS_NULL ;
  return(new) ;
}

int rem_tcds_l(struct tcds_l *l) {
  while (TCDS_NULL != l->t) {
    if (FALSE == rem_tcds(l, l->t)) { return(FALSE) ; }
  }
  free(l) ;
  return(TRUE) ;
}

struct tcds *new_tcds(struct tcds_l *l, int p) {
  struct tcds *new ;

  if (NULL == (new = (struct tcds *)malloc(sizeof(struct tcds)))) {
    log_err("new_tcds", "malloc failed", TRUE) ;
    return(FALSE) ;
  }

  new->ss = new_sock_set() ;
  new->master_sock = new_read_sock(new->ss, p) ;
  if (0 != listen(new->master_sock, 8)) {
    log_err("new_tcds", "listen failed", TRUE) ;
    close_sock_set(new->ss) ;
    free(new) ;
    return(TCDS_NULL) ;
  }
  new->ul = new_user_l() ;
  new->el = new_event_l() ;
  new->cl = new_chan_l() ;

  if (TCDS_NULL == l->t) { l->t = new ; return(TCDS_NULL) ; }
  new->next = l->t ;
  l->t = new ;
  return(new) ;
}

int rem_tcds(struct tcds_l *l, struct tcds *t) {
  struct tcds *tmp, *prev ;

  if (TCDS_L_NULL == l) {
    log_err("rem_tcds", "Given empty TCDS list", FALSE) ;
    return(FALSE) ;
  }
  if (TCDS_NULL == t) {
    log_err("rem_tcds", "Given empty TCSD", FALSE) ;
    return(FALSE) ;
  }
  if (TCDS_NULL == l->t) {
    log_err("rem_tcds", "TCDS list empty", FALSE) ;
    return(FALSE) ;
  }

  tmp = prev = l->t ;
  while (TCDS_NULL != tmp && tmp != t) { prev = tmp ; tmp = tmp->next ; }

  if (TCDS_NULL == tmp) {
    log_err("rem_tcds", "Unable to locate TCDS in list", FALSE) ;
    return(FALSE) ;
  }
  if (tmp == l->t) {
    l->t = l->t->next ;
  } else {
    prev = tmp->next ;
  }
  rem_user_l(tmp) ;
  rem_event_l(tmp) ;
  if (0 != close(tmp->master_sock)) {
    log_err("rem_tcds", "close failed", TRUE) ;
    return(FALSE) ;
  }
  free(tmp) ;
  return(TRUE) ;
}

struct tcds *tcds_by_sock(struct tcds_l *l, int p) {
  struct tcds *tmp ;

  tmp = l->t ;
  while (TCDS_NULL != tmp) {
    if (p == tmp->master_sock) { return(tmp) ; }
    else { tmp = tmp->next ; }
  }
  return(TCDS_NULL) ;
}
