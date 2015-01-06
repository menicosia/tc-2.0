#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "defaults.h"
#include "log.h"
#include "tcds.h"
#include "user.h"
#include "util.h"

#include "libipc2.h"

int send_file(struct tcds *t, struct user *u, char *f) {
  char *str ;
  FILE *fd ;

  if (NULL == f) {
    log_err("send_file", "given null file name", FALSE) ;
    return(FALSE) ;
  }

  if (NULL == (fd = fopen(f, "r"))) {
    if (0 < VERBOSE) {
      if (NULL == (str = (char *)malloc((20+strlen(f))*sizeof(char)))) {
	log_err("send_file", "malloc failed on error message", TRUE) ;
	return(FALSE) ;
      }
      sprintf(str, "unable to open %s", f) ;
      log_err("send_file", str, TRUE) ;
      free(str) ;
    }
    return(FALSE) ;
  }

  if (NULL == (str = (char *)malloc(83 * sizeof(char)))) {
    log_err("send_file", "malloc failed on greet string", TRUE) ;
    return(FALSE) ;
  }
  while (NULL != fgets(str, 80, fd)) {
    strcat(str, "\r\n") ;
    write_sock(t->ss, u->sock, str) ;
    if (NULL == (str = (char *)malloc(83 * sizeof(char)))) {
      log_err("send_file", "malloc failed on greet string", TRUE) ;
      return(FALSE) ;
    }
  }
  return(TRUE) ;
}

int new_name(struct tcds *t, struct user *u, char *name) {
  char *msg, *new ;
  int len ;
  struct user *tmp ;

  if (USER_NULL == u) {
    log_err("new_name", "given null user", FALSE) ;
    return(FALSE) ;
  }

  if (NULL == name) {
    log_err("new_name", "given null name", FALSE) ;
    return(FALSE) ;
  }

  name = strip_crs(name) ;
  name = under_space(name) ;
  len = strlen(name) ;

  tmp = t->ul->u ;
  while(USER_NULL != tmp) {
    if (NULL != tmp->name) {
      if (!strcasecmp(tmp->name, name)) {
	if (NULL == (msg = (char *)malloc((len+55)*sizeof(char)))) {
	  log_err("new_name", "malloc for error failed", TRUE) ;
	  return(FALSE) ;
	}
	sprintf(msg, "Sorry, ``%s'' is already in use, try another name.\r\n",
		name) ;
	write_sock(t->ss, u->sock, msg) ;
	return(FALSE) ;
      } 
    }
    tmp = tmp->next ;
  }

  len = strlen(name) + 1 ;
  if (MAX_NAME_LEN+1 < len) { len = MAX_NAME_LEN+1 ; }
  if (NULL == (new = (char *)malloc(sizeof(char)+len))) {
    log_err("new_name", "malloc for new name failed", TRUE) ;
    return(FALSE) ;
  }
  if (len < MAX_NAME_LEN) {
    strncpy(new, name, len) ;
    new[len] = '\0' ;
  } else {
    strncpy(new, name, MAX_NAME_LEN) ;
    new[MAX_NAME_LEN] = '\0' ;
  }
  u->name = new ;
  return(TRUE) ;
}

char *strip_crs(char *str) {
  int len, i ;

  len = strlen(str) ;
  for (i = 0 ; i < len ; i++) {
    switch(str[i]) {
    case '\r':
      str[i] = '\0' ;
      break ;
    case '\n':
      str[i] = '\0' ;
      break ;
    }
  }
  return(str) ;
}

char *under_space(char *str) {
  int len, i ;

  len = strlen(str) ;
  for (i = 0 ; i < len ; i++) {
    if (isspace(str[i])) { str[i] = '_' ; }
  }
  return(str) ;
}

char *make_idle(int diff) {
  char *idle = (char *) malloc(20 * sizeof(char)) ;
  
  if (diff == 0) idle[0] = '\0' ;
  else if (diff < 60) sprintf(idle, "%ds", diff) ;
  else if (diff < 3600) sprintf(idle, "%dm", diff / 60) ;
  else if (diff < 86400)
    sprintf(idle, "%d:%.2d", diff / 3600, (diff % 3600) / 60) ;
  else sprintf(idle, "%dd", diff / 86400) ;
  return(idle) ;
}
