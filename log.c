#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defaults.h"
#include "log.h"

/* Usage: log_err("new_userl", "malloc failed", TRUE) ; */
int log_err(char *fcn, char *str, int errno_set) {
  char *out ;
  char *err_msg = NULL ;
  int len = 0 ;

  /* Grab errno before anything happens. */
  if (TRUE == errno_set) {
    err_msg = strerror(errno) ;
    len = strlen(err_msg) ;
  }
  len += strlen(fcn) + strlen(str) + strlen(PROG_LABEL) + 10 ; /* 10 extra */
  if (NULL == (out = (char *)malloc(sizeof(char)*len))) {
    fprintf(stderr, "log_err: Error in malloc, cannot log error.\n") ;
    return (FALSE) ;
  }
  if (TRUE == errno_set) {
    sprintf(out, "[%s] %s: %s, %s.\n", PROG_LABEL, fcn, str, err_msg) ;
  } else {
    sprintf(out, "[%s] %s: %s.\n", PROG_LABEL, fcn, str) ;
  }

  /* For now, just print to stderr. Later, possibly log to file, etc. */
  fputs(out, stderr) ;
  free(out) ;
  return(TRUE) ;
}

