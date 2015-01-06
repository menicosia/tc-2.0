#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "event.h"
#include "channel.h"
#include "util.h"
#include "announce.h"
#include "tc2_cmds.h"

#include "libipc2.h"

extern int genesis ;

int do_help(struct tcds *t, struct event *e) {
  int i ;
  char *str ;
  char *lines[15] = {
    " "PROG_LABEL" Help\r\n",
    "-------------------------------------------------------------\r\n",
    " /h              : This help message.\r\n",
    " /e              : Toggle echo of own lines.\r\n",
    " /n name         : Change name.\r\n",
    " /. text         : Print text without handle.\r\n",
    " /m name message : Send message to another user.\r\n",
    " /r message      : Send message to same user as last /m.\r\n",
    " /a channel      : Set active channel.\r\n",
    " /j channel      : Join a channel, create if necessary.\r\n",
    " /l channel      : Leave a channel.\r\n",
    " /w [channel]    : List of users [in a particular channel].\r\n",
    " /c              : List of channels.\r\n",
    " /q              : Quit.\r\n",
    "-------------------------------------------------------------\r\n",
  } ;

  if (FALSE == send_file(t, e->a.c->u, HELP_FILE)) {
    for (i = 0 ; i < 15 ; i++) {
      if (NULL == (str = (char *)malloc(80*sizeof(char)))) {
	log_err("do_help", "malloc failed", TRUE) ;
	return(FALSE) ;
      }
      strcpy(str, lines[i]) ;
      write_sock(t->ss, e->a.c->u->sock, str) ;
    }
  }
  return(TRUE) ;
}

int toggle_echo(struct tcds *t, struct event *e) {
  char *str ;

  if (NULL == (str = (char *)malloc(sizeof(char)*80))) {
    log_err("toggle_echo", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  if (e->a.c->u->echo == TRUE) {
    e->a.c->u->echo = FALSE ;
    strcpy(str, "Echo is now off.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    return(TRUE) ;
  }
  if (e->a.c->u->echo == FALSE) {
    e->a.c->u->echo = TRUE ;
    strcpy(str, "Echo is now on.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    return(TRUE) ;
  }
  
  return(FALSE) ;
}

int do_mesg(struct tcds *t, struct event *e) {
  struct user *target ;
  char *str, *msg, *t_name, *p ;
  int len ;

  str = e->a.c->str ;
  if (NULL == str) { mesg_usage(t, e) ; return(FALSE) ; }

  /* Find next whitespace */
  p = str ;
  while ('\0' != *p && !isspace(*p)) { p++ ; }
  if ('\0' == *p) { mesg_usage(t, e) ; return(FALSE) ; }

  len = (p - str) + 1 ;
  if (NULL == (t_name = (char *)malloc(sizeof(char)*len))) {
    log_err("do_mesg", "malloc failed on target name", TRUE) ;
    return(FALSE) ;
  }
  strncpy(t_name, str, p-str) ;
  t_name[p-str] = '\0' ; /* strncpy won't null terminate for us. */
  if (USER_NULL == (target = user_by_name(t, t_name))) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
      log_err("do_mesg", "malloc failed error message", TRUE) ;
      free(t_name) ;
      return(FALSE) ;
    }
    sprintf(msg, "Unable to locate user: %s\r\n", t_name) ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    free(t_name) ;
    return(FALSE) ;
  }
  free(t_name) ;

  p++ ;
  while ('\0' != *p && isspace(*p)) { p++ ; }
  if ('\0' == *p) { mesg_usage(t, e) ; return(FALSE) ; }

  if (FALSE == mesg(t, e->a.c->u, target, p)) { return(FALSE) ; }
  e->a.c->u->last = target ;
  return(TRUE) ;
}

int mesg_usage(struct tcds *t, struct event *e) {
  char *str ;

  if (NULL == (str = (char *)malloc(sizeof(char)*80))) {
    log_err("mesg_usage", "malloc failed on usage message", TRUE) ;
    return(FALSE) ;
  }
  strcpy(str, "Usage: /m user message\r\n") ;
  write_sock(t->ss, e->a.c->u->sock, str) ;
  return(TRUE) ;
}

int mesg(struct tcds *t, struct user *from, struct user *to, char *str) {
  char *msg ;
  int len ;

  if (NULL == from->name) { len = strlen(str) + strlen(to->name) + 20 ; }
  else { len = strlen(from->name) + strlen(str) + strlen(to->name) + 20 ; }

  if (NULL == (msg = (char *)malloc(sizeof(char)*len))) {
    log_err("mesg", "malloc failed for message", TRUE) ;
    return(FALSE) ;
  }
  if (NULL == from->name) { sprintf(msg, "** %s\r\n", str) ; }
  else { sprintf(msg, "*%s* %s\r\n", from->name, str) ; }
  write_sock(t->ss, to->sock, msg) ;

  if (TRUE == from->echo) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*len))) {
      log_err("mesg", "malloc failed for message", TRUE) ;
      return(FALSE) ;
    }
    if (NULL == from->name) {
      sprintf(msg, "** -> *%s* %s\r\n", to->name, str) ;
    }
    else {
      sprintf(msg, "*%s* -> *%s* %s\r\n", from->name, to->name, str) ;
    }
    write_sock(t->ss, from->sock, msg) ;
  }
  return(TRUE) ;
}

int do_rept(struct tcds *t, struct event *e) {
  char *str ;

  if (USER_NULL == e->a.c->u->last) {
    if (NULL == (str = (char *)malloc(sizeof(char)*80))) {
      log_err("do_rept", "malloc failed for error", TRUE) ;
      return(FALSE) ;
    }
    strcpy(str, "No previous messagee.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    return(FALSE) ;
  }

  if (FALSE == mesg(t, e->a.c->u, e->a.c->u->last, e->a.c->str)) {
    return(FALSE) ;
  }
  return(TRUE) ;
}

int change_name(struct tcds *t, struct event *e) {
  char *msg, *str ;

  if (NULL == e->a.c->u->name) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(80)))) {
      log_err("change_name", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "You must select a name before changing it.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  str = e->a.c->str ;
  if (NULL == str) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
      log_err("change_name", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    strcpy(msg, "Usage: /n name\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  if (NULL == (msg = (char *)malloc(sizeof(char)*(40+strlen(str))))) {
    log_err("change_name", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "%s is now known as: ", e->a.c->u->name) ;

  if (FALSE == new_name(t, e->a.c->u, e->a.c->str)) {
    free(msg) ;
    return(FALSE) ;
  }

  strcat(msg, e->a.c->u->name) ;
  announce_all(t, e->a.c->u, msg) ;
  free(msg) ;
  return(TRUE) ;
}

int do_emote(struct tcds *t, struct event *e) {
  char *msg, *str ;

  str = e->a.c->str ;
  if (NULL == str) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
      log_err("do_emote", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    strcpy(msg, "Usage: /e text\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }
  if (NULL == (msg = (char *)malloc(sizeof(char)*(30+strlen(str))))) {
    log_err("do_emote", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "] %s", str) ;
  if (FALSE == announce(t, e->a.c->u->chan, e->a.c->u, msg)) {return(FALSE) ;}
  return(TRUE) ;
}

int do_who(struct tcds *t, struct event *e) {
  char *str ;
  struct user *tmp ;
  struct chan *c ;
  struct member *m ;
  int len, count = 0 ;

  /* Do a listing for all users in the TCDS */
  if (NULL == e->a.c->str) {
    if (NULL == (str = (char *)malloc(sizeof(char)*80))) {
      log_err("do_who", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    sprintf(str, "%-*s %-*s %-*s %-*s\r\n", 
	    MAX_NAME_LEN, "User:", MAX_CHAN_LEN, "Channel:",
	    7, "Idle:", 7, "On For:") ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    tmp = t->ul->u ;
    while (USER_NULL != tmp) {
      if (NULL != tmp->name) {
	count++ ;
	len = MAX_NAME_LEN + MAX_CHAN_LEN + 30 ;
	if (NULL == (str = (char *) malloc(sizeof(char)*len))) {
	  log_err("do_who", "malloc failed", TRUE) ;
	  return(FALSE) ;
	}
	sprintf(str, "%-*s %-*s %-*s %-*s\r\n",
		MAX_NAME_LEN, tmp->name, MAX_CHAN_LEN, tmp->chan->title,
		7, make_idle(time(0) - tmp->last_time),
		7, make_idle(time(0) - tmp->conn_time)) ;
	write_sock(t->ss, e->a.c->u->sock, str) ;
      }
      tmp = tmp->next ;
    }
    if (NULL == (str = (char *) malloc(sizeof(char)*40))) {
      log_err("do_who", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    sprintf(str, "** %d user(s) connected. (Up for %s)\r\n", count,
	    make_idle(time(0) - genesis)) ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    return(TRUE) ;
  }

  /* Do a listing for just the specific users of a channel */
  if (CHAN_NULL == (c = chan_by_name(t, e->a.c->str))) {
    if (NULL == (str = (char *)malloc(sizeof(char)*(40+strlen(e->a.c->str))))){
      log_err("do_who", "malloc failed on error", TRUE) ;
      return(FALSE) ;
    }
    sprintf(str, "No such channel: %s\r\n", e->a.c->str) ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    return(FALSE) ;
  }

  m = c->ml->m ;
  len = MAX_NAME_LEN + 30 ;
  if (NULL == (str = (char *)malloc(sizeof(char)*len))) {
    log_err("do_who", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(str, "%-*s %-*s %-*s\r\n", MAX_NAME_LEN, "User:",
	  7, "Idle:", 7, "On For:") ;
  write_sock(t->ss, e->a.c->u->sock, str) ;
  while (MEMBER_NULL != m) {
    count++ ;
    if (NULL == (str = (char *)malloc(sizeof(char)*len))) {
      log_err("do_who", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    sprintf(str, "%-*s %-*s %-*s\r\n", MAX_NAME_LEN, m->u->name,
	    7, make_idle(time(0) - m->u->last_time),
	    7, make_idle(time(0) - m->u->conn_time)) ;
    write_sock(t->ss, e->a.c->u->sock, str) ;
    m = m->next ;
  }
  if (NULL == (str = (char *)malloc(sizeof(char)*80))) {
    log_err("do_who", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(str, "** %d user(s) in channel %s.\r\n", count, c->title) ;
  write_sock(t->ss, e->a.c->u->sock, str) ;

  return(TRUE) ;
}

int do_chanl(struct tcds *t, struct event *e) {
  struct chan *tmp ;
  char *msg ;
  int count = 0, n_m, len ;

  tmp = t->cl->c ;
  if (CHAN_NULL == tmp) { /* This should never happen, but... */
    if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
      log_err("do_chanl", "malloc failed on error", TRUE) ;
      return(FALSE) ;
    }
    strcpy(msg, "No channels.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(TRUE) ;
  }
  len = MAX_CHAN_LEN+30 ;
  if (NULL == (msg = (char *)malloc(sizeof(char)*len))) {
    log_err("do_chanl", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "%-*s Users:\r\n", MAX_CHAN_LEN, "Channel:") ;
  write_sock(t->ss, e->a.c->u->sock, msg) ;
  while (CHAN_NULL != tmp) {
    count++ ;
    n_m = num_members(t, tmp->title) ;
    if (NULL == (msg = (char *)malloc(sizeof(char)*len))) {
      log_err("do_chanl", "malloc failed", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "%-*s %d\r\n", MAX_CHAN_LEN, tmp->title, n_m) ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    tmp = tmp->next ;
  }
  if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
    log_err("do_chanl", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "** %d channel(s).\r\n", count) ;
  write_sock(t->ss, e->a.c->u->sock, msg) ;
  return(TRUE) ;
}

int do_active(struct tcds *t, struct event *e) {
  struct chan *c ;
  char *msg ;

  if (NULL == e->a.c->u->name) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(80)))) {
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "You must select a name before using channel commands.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  if (CHAN_NULL == (c = chan_by_name(t, e->a.c->str))) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(40+strlen(e->a.c->str))))){
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "No such channel: %s\r\n", e->a.c->str) ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  if (!is_member(t, e->a.c->str, e->a.c->u)) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(40+strlen(e->a.c->str))))){
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "Not a member of: %s\r\n", e->a.c->str) ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  e->a.c->u->chan = c ;
  if (NULL == (msg = (char *)malloc(sizeof(char)*(40+strlen(e->a.c->str))))){
    log_err("do_active", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "Active channel set to: %s\r\n", c->title) ;
  write_sock(t->ss, e->a.c->u->sock, msg) ;

  return(TRUE) ;
}
int do_join(struct tcds *t, struct event *e) {
  int len ;
  char *msg, *str ;

  if (NULL == e->a.c->u->name) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "You must select a name before using channel commands.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  str = e->a.c->str ;
  len = strlen(str) ;
  new_member(t, e->a.c->str, e->a.c->u) ;
  
  return(TRUE) ;
}

int do_leave(struct tcds *t, struct event *e) {
  int len ;
  char *str, *msg ;
  struct chan *c ;

  if (NULL == e->a.c->u->name) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(80)))) {
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "You must select a name before using channel commands.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  if (NULL == e->a.c->str) {
    c = e->a.c->u->chan ;
    str = c->title ;
    len = strlen(str) ;
  } else {
    str = e->a.c->str ;
    len = strlen(str) ;
    
    if (FALSE == (c = chan_by_name(t, str))) {
      if (NULL == (msg = (char *)malloc(sizeof(char)*(40+len)))) {
	log_err("do_leave", "malloc failed on error message", TRUE) ;
	return(FALSE) ;
      }
      sprintf(msg, "No such channel: %s\r\n", str) ;
      write_sock(t->ss, e->a.c->u->sock, msg) ;
      return(FALSE) ;
    }
  }
  if (FALSE == rem_member(t, str, e->a.c->u)) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(40+len)))) {
      log_err("do_leave", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "Unable to leave channel: %s\r\n", str) ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  /* If leaving current channel, dump them in the default channel. */
  if (c == e->a.c->u->chan) {
    if (!is_member(t, DEFAULT_CHANNEL, e->a.c->u)) {
      new_member(t, DEFAULT_CHANNEL, e->a.c->u) ;
    } else {
      e->a.c->u->chan = chan_by_name(t, DEFAULT_CHANNEL) ;
    }
  }

  len = strlen(e->a.c->u->chan->title) ;
  if (NULL == (msg = (char *)malloc(sizeof(char)*(40+len)))) {
    log_err("do_leave", "malloc failed on active channel message", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "Active channel is now: %s\r\n", e->a.c->u->chan->title) ;
  write_sock(t->ss, e->a.c->u->sock, msg) ;
  return(TRUE) ;
}

int do_quit(struct tcds *t, struct event *e) {
  announce_quit(t, e->a.c->u) ;
  rem_user(t, e->a.c->u) ;

  return(GONE) ; /* Special exit code to signify user is GONE. */
}

int do_shutdown(struct tcds *t, struct event *e) {
  char *msg, *who ;
  int len = 0 ;

  if (NULL == e->a.c->u->name) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(80)))) {
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "You must select a name before shutting down.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  who = e->a.c->u->name ;
  len = strlen(who) ;
  if (NULL != e->a.c->str) { len += strlen(e->a.c->str) ; }
  if (NULL == (msg = (char *)malloc(sizeof(char)*(40+strlen(who))))) {
    log_err("do_shut", "malloc failed", TRUE) ;
    return(FALSE) ;
  }
  if (NULL != e->a.c->str) {
    sprintf(msg, "%s is being shut down by %s: %s", PROG_LABEL,
	    who, e->a.c->str) ;
  } else {
    sprintf(msg, "%s is being shut down by %s, bye!", PROG_LABEL, who) ;
  }
  announce_all(t, e->a.c->u, msg) ;

  exit(0) ;
}

int do_boot(struct tcds *t, struct event *e) {
  char *msg ;
  struct user *u ;
  int len ;

  if (NULL == e->a.c->u->name) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*(80)))) {
      log_err("do_active", "malloc failed on error message", TRUE) ;
      return(FALSE) ;
    }
    sprintf(msg, "You must select a name before booting users.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  if (USER_NULL == (u = user_by_name(t, e->a.c->str))) {
    if (NULL == (msg = (char *)malloc(sizeof(char)*80))) {
      log_err("do_boot", "malloc failed on error", TRUE) ;
      return(FALSE) ;
    }
    strcpy(msg, "No such user.\r\n") ;
    write_sock(t->ss, e->a.c->u->sock, msg) ;
    return(FALSE) ;
  }

  len = strlen(e->a.c->u->name) + strlen(u->name) + 40 ;
  if (NULL == (msg = (char *)malloc(sizeof(char)*len))) {
    log_err("do_boot", "malloc failed on error", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "You have been booted by %s, bye!\r\n", e->a.c->u->name) ;
  write_sock(t->ss, u->sock, msg) ;
  rem_user(t, u) ;

  if (NULL == (msg = (char *)malloc(sizeof(char)*len))) {
    log_err("do_boot", "malloc failed on error", TRUE) ;
    return(FALSE) ;
  }
  sprintf(msg, "%s has been booted by %s, see yuh!", u->name, e->a.c->u->name);
  announce_all(t, e->a.c->u, msg) ;

  return(TRUE) ;
}
