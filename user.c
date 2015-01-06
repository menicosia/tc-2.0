#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "channel.h"

#include "libipc2.h"

struct user_l *new_user_l() {
  struct user_l *new ;

  if (USER_L_NULL == (new = (struct user_l *)malloc(sizeof(struct user_l)))) {
    log_err("new_user_l", "malloc failed", TRUE) ;
    return(USER_L_NULL) ;
  }

  new->u = USER_NULL ;
  return(new) ;
}

int rem_user_l(struct tcds *t) {
  if (TCDS_NULL == t) { return(FALSE) ; }
  if (USER_L_NULL == t->ul) { return(TRUE) ; }
  while (USER_NULL != t->ul->u) {
    if (FALSE == rem_user(t, t->ul->u)) {
      return(FALSE) ;
    }
  }
  free(t->ul) ;
  t->ul = USER_L_NULL ;
  return(TRUE) ;
}

struct user *new_user(struct tcds *t, int sock, int conn_time, char *name) {
  struct user *new ;
  struct user_l *l ;

  if (TCDS_NULL == t) { return(FALSE) ; }
  l = t->ul ;

  if (USER_L_NULL == l) {
    log_err("new_user", "Unable to add user, given null user list", FALSE) ;
    return(USER_NULL) ;
  }
  if (NULL == (new = (struct user *)malloc(sizeof(struct user)))) {
    log_err("new_user", "malloc failed", TRUE) ;
    return(USER_NULL) ;
  }
  new->sock = sock ;
  new->echo = TRUE ; /* Default is to echo lines back to the user */
  new->conn_time = new->last_time = time(0) ;
  new->name = name ;
  new->last = USER_NULL ;
  new->chan = t->cl->c ;

  if (USER_NULL == l->u) {
    new->next = USER_NULL ;
    l->u = new ;
    return(new) ;
  } else {
    new->next = l->u ;
    l->u = new ;
    return(new) ;
  }
}

int rem_user(struct tcds *t, struct user *u) {
  struct user *tmp, *prev ;
  struct user_l *l ;

  if (TCDS_NULL == t) { return(FALSE) ; }
  l = t->ul ;

  if (USER_L_NULL == l) {
    log_err("rem_user", "Given empty user list", FALSE) ;
    return(FALSE) ;
  }
  if (USER_NULL == u) {
    log_err("rem_user", "Given empty user", FALSE) ;
    return(FALSE) ;
  }
  if (USER_NULL == l->u) {
    log_err("rem_user", "no users in list to remove", FALSE) ;
    return(FALSE) ;
  }

  rem_user_from_all(t, u) ;

  tmp = prev = l->u ;
  while (tmp != u && USER_NULL != tmp) { prev = tmp ; tmp = tmp->next ; }

  if (USER_NULL == tmp) {
    log_err("rem_user", "Failed to locate user in list", FALSE) ;
    return(FALSE) ;
  }
  if (tmp == l->u) {
    l->u = l->u->next ;
  } else {
    prev->next = tmp->next ;
  }

  close_sock(t->ss, tmp->sock) ;
  free(tmp->name) ;
  free(tmp) ;
  return(TRUE) ;
}

struct user *user_by_sock(struct tcds *t, int s) {
  struct user *tmp ;
  struct user_l *l ;

  if (TCDS_NULL == t) {
    log_err("user_by_sock", "given empty TCDS", FALSE) ;
    return(FALSE) ;
  }

  l = t->ul ;
  if (USER_L_NULL == l) {
    log_err("user_by_sock", "uninitialized user list", FALSE) ;
    return(FALSE) ;
  }

  tmp = l->u ;
  while (USER_NULL != tmp) {
    if (s == tmp->sock) { return(tmp) ; }
    else { tmp = tmp->next ; }
  }
  return(USER_NULL) ;
}

struct user *user_by_name(struct tcds *t, char *n) {
  struct user *tmp ;
  struct user_l *l ;

  if (TCDS_NULL == t) {
    log_err("user_by_sock", "given empty TCDS", FALSE) ;
    return(FALSE) ;
  }

  l = t->ul ;
  if (USER_L_NULL == l) {
    log_err("user_by_sock", "uninitialized user list", FALSE) ;
    return(FALSE) ;
  }

  tmp = l->u ;
  while (USER_NULL != tmp) {
    if (NULL != tmp->name && 0 == strcasecmp(n, tmp->name)) { return(tmp) ; }
    else { tmp = tmp->next ; }
  }
  return(USER_NULL) ;
}
