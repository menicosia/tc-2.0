#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "event.h"
#include "channel.h"
#include "announce.h"
#include "util.h"

#include "libipc2.h"

struct chan_l *new_chan_l() {
  struct chan_l *new ;

  if (NULL == (new = (struct chan_l *)malloc(sizeof(struct chan_l)))) {
    log_err("new_chan_l", "malloc failed", TRUE) ;
    return(CHAN_L_NULL) ;
  }
  new->c = CHAN_NULL ;
  return(new) ;
}

int rem_chan_l(struct tcds *t) {
  struct chan *tmp ;

  if (TCDS_NULL == t) {
    log_err("rem_chan_l", "given empty tcds", FALSE) ;
    return(FALSE) ;
  }
  if (CHAN_L_NULL == t->cl) {
    log_err("rem_chan_l", "given empty channel list (OK)", FALSE) ;
    return(TRUE) ;
  }

  /* Remove all channels from the list */
  while (CHAN_NULL != t->cl->c) {
    tmp = t->cl->c ;
    t->cl->c = tmp->next ;
    if (FALSE == rem_member_l(tmp)) {
      log_err("rem_chan_l", "failed to remove member list", FALSE) ;
      return(FALSE) ;
    }
    free(tmp->title) ;
    free(tmp) ;
  }
  return(FALSE) ;
}

int new_chan(struct tcds *t, char *title) {
  struct chan *new, *c ;
  int len ;

  if (TCDS_NULL == t) {
    log_err("new_chan", "given empty TCDS", FALSE) ;
    return(FALSE) ;
  }
  if (CHAN_L_NULL == t->cl) {
    log_err("new_chan", "TCDS channel list unitialized", FALSE) ;
    return(FALSE) ;
  }
  if (NULL == title) {
    log_err("new_chan", "given empty title", FALSE) ;
    return(FALSE) ;
  }

  if (CHAN_NULL != (c = chan_by_name(t, title))) { return(FALSE) ; }

  if (NULL == (new = (struct chan *)malloc(sizeof(struct chan)))) {
    log_err("new_chan", "malloc failed", TRUE) ;
    return(FALSE) ;
  }

  len = strlen(title)+1 ;
  if (MAX_CHAN_LEN+1 < len) { len = MAX_CHAN_LEN+1 ; }
  if (NULL == (new->title = (char *)malloc(sizeof(char)*len))) {
    log_err("new_chan", "malloc of new title failed", FALSE) ;
    free(new) ;
    return(FALSE) ;
  }
  if (len < MAX_CHAN_LEN) {
    strncpy(new->title, title, len) ;
    new->title[len] = '\0' ;
  } else {
    strncpy(new->title, title, MAX_CHAN_LEN) ;
    new->title[MAX_CHAN_LEN] = '\0' ;
  }
  under_space(new->title) ;

  /* Put this channel at the head of the list */
  new->next = t->cl->c ;
  t->cl->c = new ;
  if (FALSE == (new->ml = new_member_l())) {
    log_err("new_chan", "failed to make new member list", FALSE) ;
    return(FALSE) ;
  }

  return(TRUE) ;
}

int rem_chan(struct tcds *t, char *title) {
  struct chan *def_c, *tmp, *prev ;

  if (TCDS_NULL == t) {
    log_err("rem_chan", "given empty TCDS", FALSE) ;
    return(FALSE) ;
  }
  if (NULL == title) {
    log_err("rem_chan", "given empty title", FALSE) ;
    return(FALSE) ;
  }

  def_c = chan_by_name(t, DEFAULT_CHANNEL) ;

  prev = tmp = t->cl->c ;
  while (CHAN_NULL != tmp && Strcmp(tmp->title, title)) {
    prev = tmp ;
    tmp = tmp->next ;
  }
  if (CHAN_NULL == tmp) { return(FALSE) ; }
  if (!Strcmp(tmp->title, title)) {
    if (tmp == t->cl->c) { t->cl->c = tmp->next ; }
    else { prev->next = tmp->next ; }
    if (NULL == rem_member_l(tmp)) {
      log_err("rem_chan", "failed to remove member list", FALSE) ;
      return(FALSE) ;
    }
    if (tmp != def_c) { free(tmp->title) ; } /* Don't free constant! */
    free(tmp) ;
    return(TRUE) ;
  }
  return(FALSE) ;
}

struct member_l *new_member_l() {
  struct member_l *new ;

  if (NULL == (new = (struct member_l *)malloc(sizeof(struct member_l)))) {
    log_err("new_member_l", "malloc failed", FALSE) ;
    return(FALSE) ;
  }
  new->m = MEMBER_NULL ;
  return(new) ;
}

int rem_member_l(struct chan *c) {
  struct member *tmp ;

  if (CHAN_NULL == c) {
    log_err("rem_member_l", "given null channel", FALSE) ;
    return(FALSE) ;
  }

  while (MEMBER_NULL != c->ml->m) {
    tmp = c->ml->m ;
    c->ml->m = tmp->next ;
    free(tmp) ;
  }
  return(TRUE) ;
}

int new_member(struct tcds *t, char *title, struct user *u) {
  struct member *new ;
  struct chan *c ;
  char *str, *date, *nl ;
  int len ;
  time_t now ;

  if (NULL == title) {
    log_err("new_member", "given null channel", FALSE) ;
    return(FALSE) ;
  }

  if (CHAN_NULL == (c = chan_by_name(t, title))) {
    if (FALSE == new_chan(t, title)) { return(FALSE) ; }
    c = chan_by_name(t, title) ;
  }

  if (MEMBER_L_NULL == c->ml) {
    log_err("new_member", "channel member list uninitialize", FALSE) ;
    return(FALSE) ;
  }

  if (NULL == (new = (struct member *)malloc(sizeof(struct member)))) {
    log_err("new_member", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  new->u = u ;
  /* Place this member at the head of the list */
  new->next = c->ml->m ;
  c->ml->m = new ;
  /* Set this to be the user's current channel */
  u->chan = c ;

  now = time(0) ;
  date = ctime(&now) ;
  if (NULL != (nl = index(date, '\n'))) { *nl = '\0' ; }
  len = 21 + strlen(u->name) + strlen(title) + strlen(date) ;
  if (NULL == (str = (char *)malloc(len))) {
    log_err("new_member", "malloc of announce string failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(str, "%s has joined %s. (%s)", u->name, title, date) ;
  announce(t, c, u, str) ;
  return(TRUE) ;
}

int rem_member(struct tcds *t, char *title, struct user *u) {
  struct member *prev, *tmp ;
  struct chan *c ;
  char *str, *date, *nl ;
  int len ;
  time_t now ;

  if (NULL == title) {
    log_err("rem_member", "given null channel", FALSE) ;
    return(FALSE) ;
  }

  c = t->cl->c ;
  while (CHAN_NULL != c && Strcmp(c->title, title)) { c = c->next ; }
  if (CHAN_NULL == c) { return(FALSE) ; }

  if (MEMBER_L_NULL == c->ml) {
    log_err("rem_member", "channel member list uninitialize", FALSE) ;
    return(FALSE) ;
  }

  prev = tmp = c->ml->m ;
  while (MEMBER_NULL != tmp && tmp->u != u) { prev = tmp ; tmp = tmp->next ; }
  if (MEMBER_NULL == tmp) { return(FALSE) ; }
  if (tmp == c->ml->m) { c->ml->m = tmp->next ; }
  else { prev->next = tmp->next ; }

  now = time(0) ;
  date = ctime(&now) ;
  if (NULL != (nl = index(date, '\n'))) { *nl = '\0' ; }
  len = 21 + strlen(u->name) + strlen(title) + strlen(date) ;
  if (NULL == (str = (char *)malloc(len))) {
    log_err("rem_member", "malloc of announce string failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(str, "%s has left %s. (%s)", u->name, title, date) ;
  announce(t, c, u, str) ;
  free(tmp) ;

  if (MEMBER_NULL == c->ml->m) { rem_chan(t, title) ; }
  return(TRUE) ;
}

int rem_user_from_all(struct tcds *t, struct user *u) {
  struct chan *tmp ;

  if (TCDS_NULL == t) {
    log_err("rem_user_from_all", "given empty TCDS", FALSE) ;
    return(FALSE) ;
  }
  if (CHAN_L_NULL == t->cl) {
    log_err("rem_user_from_all", "TCDS channel list unitialized", FALSE) ;
    return(FALSE) ;
  }

  tmp = t->cl->c ;
  while (CHAN_NULL != tmp) {
    rem_member(t, tmp->title, u) ;
    tmp = tmp->next ;
  }
  return(TRUE) ;
}

struct chan *chan_by_name(struct tcds *t, char *n) {
  struct chan *tmp ;

  if (MAX_CHAN_LEN < strlen(n)) { n[MAX_CHAN_LEN] = '\0' ; }
  under_space(n) ;
  if (CHAN_L_NULL == t->cl) {
    log_err("chan_by_sock", "uninitialized channel list", FALSE) ;
    return(FALSE) ;
  }

  tmp = t->cl->c ;
  while (CHAN_NULL != tmp) {
    if (0 == strcasecmp(n, tmp->title)) { return(tmp) ; }
    else { tmp = tmp->next ; }
  }
  return(CHAN_NULL) ;
}

int num_members(struct tcds *t, char *title) {
  struct chan *c ;
  struct member *tmp ;
  int count = 0 ;

  if (NULL == (c = chan_by_name(t, title))) { return(FALSE) ; }
  tmp = c->ml->m ;
  while (MEMBER_NULL != tmp) { count++ ; tmp = tmp->next ; }
  return(count) ;
}

int is_member(struct tcds *t, char *title, struct user *u) {
  struct chan *c ;
  struct member *tmp ;

  if (NULL == (c = chan_by_name(t, title))) { return(FALSE) ; }
  tmp = c->ml->m ;
  while (MEMBER_NULL != tmp) {
    if (! Strcmp(tmp->u->name, u->name)) { return(TRUE) ; }
    tmp = tmp->next ;
  }
  return(FALSE) ;
}
