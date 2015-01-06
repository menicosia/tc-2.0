#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "event.h"
#include "channel.h"
#include "announce.h"
#include "util.h"
#include "tc2_read.h"
#include "tc2_cmds.h"

#include "libipc2.h"

struct event *tc2_q_read(struct tcds *t) {
  struct event *end = EVENT_NULL, *rd = EVENT_NULL ;
  struct user *tmp ;
  struct timeval z ;
  fd_set socks ;

  z.tv_sec = 0 ;
  z.tv_usec = 0 ;

  FD_ZERO(&socks) ;

  tmp = t->ul->u ;
  while (USER_NULL != tmp) {
    FD_SET(tmp->sock, &socks) ;
    tmp = tmp->next ;
  }

  if (-1 == select(getdtablesize(), &socks, NULL, NULL, &z)) {
    log_err("tc2_q_accept", "select failed", TRUE) ;
    return(EVENT_NULL) ;
  }

  tmp = t->ul->u ;
  while (USER_NULL != tmp) {
    if (FD_ISSET(tmp->sock, &socks)) {
      if (EVENT_NULL == rd) {
	if (EVENT_NULL == (rd = new_event())) { return(rd) ; }
	rd->f = tc2_read ;
	rd->a.u = tmp ;
	end = rd ; /* Now the end is the first (and only) element. */
      } else {
	if (EVENT_NULL == (end->next = new_event())) { return(rd) ; }
	end->next->f = tc2_read ;
	end->next->a.u = tmp ;
	end = end->next ;
      }
    }
    tmp = tmp->next ;
  }
  return(rd) ;
}

int tc2_read(struct tcds *t, struct event *e) {
  int ret ;

  ret = read_sock(t->ss, e->a.u->sock) ;
  e->a.u->last_time = time(0) ;
  switch(ret) {
  case BAD_READ:
    if (-1 != pop_all(t, e->a.u)) { /* If pop_all doesn't close. */
      announce_lost(t, e->a.u) ;
      rem_user(t, e->a.u) ;
    }
    break ;
  case CLOSED:
    if (-1 != pop_all(t, e->a.u)) { /* If pop_all doesn't close. */
      announce_lost(t, e->a.u) ;
      rem_user(t, e->a.u) ;
    }
    break ;
  default:
    pop_all(t, e->a.u) ;
  }

  free(e) ;
  return(TRUE) ;
}

/* Returns -1 if socket is closed, TRUE if still open, FALSE if error. */
int pop_all(struct tcds *t, struct user *u) {
  char *msg ;
  int ret ;

  if (USER_NULL == u) {
    log_err("pop_all", "given empty user", FALSE) ;
    return(FALSE) ;
  }

  if (SOCK_SET_NULL == t->ss) {
    log_err("pop_all", "TCDS has empty sock_set", FALSE) ;
    return(FALSE) ;
  }

  while (NULL != (msg = pop_sock(t->ss, u->sock))) {
    msg = strip_crs(msg) ;
    ret = proc_line(t, u, msg) ;
    free(msg) ;
    if (-1 == ret) { printf("returning -1\n") ; return(-1) ; }
  }
  return(TRUE) ;
}

/* Returns -1 if connection has been closed, TRUE or FALSE on error. */
int proc_line(struct tcds *t, struct user *u, char *str) {
  int len, ret ;
  char *out, *p ;

  /* Users without names can execute commands. */

  /* If this is a command, handle it. */
  ret = proc_cmd(t, u, str) ;
  if (GONE == ret) { return(-1) ; } /* do_quit and do_boot will return GONE. */
  if (TRUE == ret) { return(TRUE) ; }
  if (FALSE == ret) { return(FALSE) ; }

  /* If they haven't chosen a name yet, try to use this as the name. */
  if (NULL == u->name) {
    p = str ;
    while ('\0' != *p && isspace(*p)) { p++ ; }
    if ('\0' == *p) { return(FALSE) ; }
    if (FALSE == new_name(t, u, str)) { return(FALSE) ; }
    else {
      new_member(t, DEFAULT_CHANNEL, u) ;
      announce_new(t, u) ;
      return(TRUE) ;
    }
  }

  /* Else, broadcast it normally. */
  len = strlen(u->name) + strlen(str) + 3 ;
  if (NULL == (out = (char *)malloc(sizeof(char)*len))) {
    log_err("proc_line", "malloc failed on out", TRUE) ;
    return(FALSE) ;
  }
  sprintf(out, "%s> %s", u->name, str) ;

  if (FALSE == announce(t, u->chan, u, out)) { free(out) ; return(FALSE) ; }
  free(out) ;
  return(TRUE) ;
}

/* Return TRUE if it is a command and an event was added. Return FALSE if  */
/* it was a command, but an error occured, and return -1 if NOT a command. */
int proc_cmd(struct tcds *t, struct user *u, char *str) {
  char *sp, *ch ;
  int i, len, x ;
  struct event *e ;

  /* These lists are WAY too fragile. I should probably build a table. */
  char *cmds[NUM_CMDS + NUM_LONG] =
  { HELP, ECHO, NAME, EMOTE, MESG, REPT, ACTV, CHANL, JOIN, LEAVE, WHO, QUIT,
    LONG_HELP, LONG_ECHO, LONG_NAME, LONG_EMOTE, LONG_MESG, LONG_REPT,
    LONG_ACTV, LONG_CHANL, LONG_JOIN, LONG_LEAVE, LONG_WHO, LONG_QUIT,
    LONG_SHUT, LONG_BOOT
  } ;
  int (*cmd_fcns[NUM_CMDS+NUM_LONG])(struct tcds *t, struct event *e) = {
    do_help, toggle_echo, change_name, do_emote, do_mesg, do_rept, do_active,
    do_chanl, do_join, do_leave, do_who, do_quit,
    do_help, toggle_echo, change_name, do_emote, do_mesg, do_rept,
    do_active, do_chanl, do_join, do_leave, do_who, do_quit,
    do_shutdown, do_boot
  } ;

  if (CMD_CHAR != str[0]) { return(-1) ; }
  sp = str ;
  while (*sp != '\0' && !isspace(*sp)) { sp++ ; }
  len = sp - str+1 ;
  for (i = 0 ; i < NUM_CMDS+NUM_LONG ; i++ ) {
    /* Shortest string compare */
    if (strlen(cmds[i]) < len) { x = strlen(cmds[i]) ; }
    else { x = len ; }
    if (!strncasecmp(cmds[i], str+1, x)) {
      if (EVENT_NULL == (e = new_event())) { return(FALSE) ; }
      if (NULL == (e->a.c = (struct cmd *)malloc(sizeof(struct cmd)))) {
	log_err("proc_cmd", "malloc failed on cmd", TRUE) ;
	free(e) ;
	return(FALSE) ;
      }
      /* Make sure the arguments to the command are not solely whitespace */
      ch = sp+1 ;
      while ('\0' != *ch && isspace(*ch)) { ch++ ; }
      if ('\0' == *ch) { e->a.c->str = NULL ; }
      else {
	if (NULL ==
	    (e->a.c->str = (char *) malloc(sizeof(char)*(strlen(sp+1)+1)))) {
	  free(e) ;
	  return(FALSE) ;
	}
	strcpy(e->a.c->str, sp+1) ;
      }
      e->f = cmd_fcns[i] ;
      e->a.c->u = u ;
      if (FALSE == add_to_el(t, e)) {
	if (NULL != e->a.c->str) { free(e->a.c->str) ; }
	free(e) ;
	return(FALSE) ;
      }
      return(TRUE) ;
    }
  }
  return(-1) ;
}
