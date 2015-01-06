#include <stdio.h>
#include <stdlib.h>
#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "event.h"
#include "channel.h"
#include "tc2_cmds.h"

/* Do nothing fcn module, used as default in new_event. */
int tc2_nop(struct tcds *t, struct event *e) { return(TRUE) ; }

/*** EVENTS ***/
struct event_l *new_event_l() {
  struct event_l *new ;

  if (NULL == (new = (struct event_l *)malloc(sizeof(struct event_l)))) {
    log_err("new_event_l", "malloc failed", TRUE) ;
    return(EVENT_L_NULL) ;
  }
  new->e = EVENT_NULL ;
  return(new) ;
}

int rem_event_l(struct tcds *t) {
  if (TCDS_NULL == t) { return(FALSE) ; }
  if (EVENT_L_NULL == t->el) { return(TRUE) ; }
  while (EVENT_NULL != t->el->e) {
    if (FALSE == rem_event(t->el, t->el->e)) {
      return(FALSE) ;
    }
  }
  free(t->el) ;
  t->el = EVENT_L_NULL ;
  return(TRUE) ;
}

struct event *new_event() {
  struct event *new ;

  if (NULL == (new = (struct event *)malloc(sizeof(struct event)))) {
    log_err("new_event", "malloc of new event failed", TRUE) ;
    return(EVENT_NULL) ;
  }
  new->f = tc2_nop ;
  new->next = EVENT_NULL ;
  return(new) ;
}

int rem_event(struct event_l *l, struct event *e) {
  struct event *tmp, *prev ;

  if (EVENT_L_NULL == l) {
    log_err("rem_event", "Given empty event list", FALSE) ;
    return(FALSE) ;
  }
  if (EVENT_NULL == e) {
    log_err("rem_event", "Given empty event", FALSE) ;
    return(FALSE) ;
  }
  if (EVENT_NULL == l->e) {
    log_err("rem_event", "Event list empty", FALSE) ;
    return(FALSE) ;
  }

  tmp = prev = l->e ;
  while (EVENT_NULL != tmp && tmp != e) { prev = tmp ; tmp = tmp->next ; }

  if (EVENT_NULL == tmp) {
    log_err("rem_event", "Failed to locate event in list", FALSE) ;
    return(FALSE) ;
  }
  if (tmp == l->e) {
    l->e = l->e->next ;
  } else {
    prev->next = tmp->next ;
  }

  free(tmp) ;
  return(TRUE) ;
}

struct event *get_event(struct tcds *t) {
  struct event *tmp ;

  tmp = t->el->e ;
  if (EVENT_NULL == tmp) return(EVENT_NULL) ;

  t->el->e = t->el->e->next ;
  return(tmp) ;
}

int add_to_el(struct tcds *t, struct event *e) {
  struct event *tmp, *prev ;

  if (TCDS_NULL == t) return(FALSE) ;

  tmp = prev = t->el->e ;
  if (EVENT_NULL == tmp) { t->el->e = e ; return(TRUE) ; }
  while (EVENT_NULL != tmp) { prev = tmp ; tmp = tmp->next ; }
  prev->next = e ;

  return(TRUE) ;
}
