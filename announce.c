/* This file contains all of the functions related to passing strings */
/* to multiple users (ie, announcing something in general.            */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "channel.h"
#include "announce.h"

#include "libipc2.h"

int announce_lost(struct tcds *t, struct user *u) {
  char *str ;
  char *date, *nl ;
  time_t now ;

  if (USER_NULL == u) {
    log_err("announce_lost", "given null user", FALSE) ;
    return(FALSE) ;
  }

  /* Only people with names get to be announced. */
  if (NULL != u->name) {
    if (NULL == (str = (char *)malloc(sizeof(char)*(70+strlen(u->name))))) {
      log_err("announce_lost", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    now = time(0) ;
    date = ctime(&now) ;
    if (NULL != (nl = index(date, '\n'))) { *nl = '\0' ; }
    sprintf(str, "%s has lost connection. (%s)", u->name, date) ;
    if (FALSE == announce_all(t, u, str)) { free(str) ; return(FALSE) ; }
    return(TRUE) ;
  }
  return(FALSE) ;
}

int announce_quit(struct tcds *t, struct user *u) {
  char *str ;
  char *date, *nl ;
  time_t now ;

  if (USER_NULL == u) {
    log_err("announce_quit", "given null user", FALSE) ;
    return(FALSE) ;
  }

  /* Only people with names get to be announced. */
  if (NULL != u->name) {
    if (NULL == (str = (char *)malloc(sizeof(char)*(70+strlen(u->name))))) {
      log_err("announce_quit", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    now = time(0) ;
    date = ctime(&now) ;
    if (NULL != (nl = index(date, '\n'))) { *nl = '\0' ; }
    sprintf(str, "%s has disconnected. (%s)", u->name, date) ;
    if (FALSE == announce_all(t, u, str)) { free(str) ; return(FALSE) ; }
    return(TRUE) ;
  }
  return(FALSE) ;
}

int announce_new(struct tcds *t, struct user *u) {
  char *str ;
  char *date, *nl ;
  time_t now ;

  if (USER_NULL == u) {
    log_err("announce_quit", "given null user", FALSE) ;
    return(FALSE) ;
  }

  /* Only people with names get to be announced. */
  if (NULL != u->name) {
    if (NULL == (str = (char *)malloc(sizeof(char)*(70+strlen(u->name))))) {
      log_err("announce_quit", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    now = time(0) ;
    date = ctime(&now) ;
    if (NULL != (nl = index(date, '\n'))) { *nl = '\0' ; }
    sprintf(str, "%s has connected. (%s)", u->name, date) ;
    if (FALSE == announce_all(t, u, str)) { free(str) ; return(FALSE) ; }
    return(TRUE) ;
  }
  return(FALSE) ;
  return(TRUE) ;
}

int announce(struct tcds *t, struct chan *c, struct user *u, char *str) {
  struct member *tmp ;
  char *cpy ;
  int len ;

  if (CHAN_NULL == c) {
    log_err("announce", "given empty channel", FALSE) ;
    return(FALSE) ;
  }
  if (MEMBER_L_NULL == c->ml) {
    log_err("announce", "channel member list unitialized", FALSE) ;
    return(FALSE) ;
  }
  if (USER_NULL == u) {
    log_err("announce", "given empty user", FALSE) ;
    return(FALSE) ;
  }

  if (MEMBER_L_NULL != c->ml) { tmp = c->ml->m ; }
  else { return(FALSE) ; }
  while (MEMBER_NULL != tmp) {
    if (tmp->u != u || (tmp->u == u && FALSE != tmp->u->echo)) {
      len = strlen(str) + 5 ;
      if (CHAN_NULL != tmp->u->chan && c != tmp->u->chan) {
	len += MAX_CHAN_LEN + 2 ;
      }
      if (NULL == (cpy = (char *)malloc(sizeof(char)*len))) {
	log_err("announce", "malloc failed on cpy", TRUE) ;
	return(FALSE) ;
      }
      if (CHAN_NULL != tmp->u->chan && c != tmp->u->chan) {
	sprintf(cpy, "[%s] %s\r\n", c->title, str) ;
      } else { sprintf(cpy, "%s\r\n", str) ; }
      write_sock(t->ss, tmp->u->sock, cpy) ;
    }
    tmp = tmp->next ;
  }

  return(TRUE) ;
}

int announce_all(struct tcds *t, struct user *u, char *str) {
  struct user *tmp ;
  char *cpy ;

  if (TCDS_NULL == t) {
    log_err("announce", "given empty TCDS", FALSE) ;
    return(FALSE) ;
  }
  if (USER_NULL == u) {
    log_err("announce", "given empty user", FALSE) ;
    return(FALSE) ;
  }

  tmp = t->ul->u ;
  while (USER_NULL != tmp) {
    if (NULL == (cpy = (char *)malloc(sizeof(char)*(strlen(str)+5)))) {
      log_err("announce_all", "malloc failed on cpy", TRUE) ;
      return(FALSE) ;
    }
    strcpy(cpy, str) ;
    strcat(cpy, "\r\n") ;
    write_sock(t->ss, tmp->sock, cpy) ;
    tmp = tmp->next ;
  }

  return(TRUE) ;
}
