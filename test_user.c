#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "defaults.h"
#include "tcds.h"
#include "user.h"

extern int errno ;

int main() {
  struct user_l *l1, *l2 ;
  struct user *u ;
  char *n ;

  printf("*** Testing new_user_l()...\n") ;
  if (USER_L_NULL == (l1 = new_user_l())) {
    printf("Failed to create l1.\n") ;
    exit(1) ;
  }
  if (USER_L_NULL == (l2 = new_user_l())) {
    printf("Failed to create l2.\n") ;
    exit(1) ; 
  }

  printf("*** Testing new_user()...\n") ;
  if (NULL == (n = (char *)malloc(10*sizeof(char)))) {
    fprintf(stderr, "Malloc failed, %s\n", strerror(errno)) ;
    exit(1) ;
  }
  strcpy(n, "marco") ;
  if (USER_NULL == new_user(l1, 1, 12, n)) {
    printf("Failed to add marco to l1.\n") ;
    exit(1) ;
  }

  if (NULL == (n = (char *)malloc(10*sizeof(char)))) {
    fprintf(stderr, "Malloc failed, %s\n", strerror(errno)) ;
    exit(1) ;
  }
  strcpy(n, "norby") ;
  if (USER_NULL == new_user(l1, 2, 10, n)) {
    printf("Failed to add norby to l1.\n") ;
    exit(1) ;
  }

  if (NULL == (n = (char *)malloc(10*sizeof(char)))) {
    fprintf(stderr, "Malloc failed, %s\n", strerror(errno)) ;
    exit(1) ;
  }
  strcpy(n, "flopez") ;
  if (USER_NULL == new_user(l2, 1, 5, n)) {
    printf("Failed to add flopez to l2.\n") ;
    exit(1) ;
  }

  if (NULL == (n = (char *)malloc(10*sizeof(char)))) {
    fprintf(stderr, "Malloc failed, %s\n", strerror(errno)) ;
    exit(1) ;
  }
  strcpy(n, "ryanh") ;
  if (USER_NULL != new_user(USER_L_NULL, 1, 5, n)) {
    printf("Failure: added ryanh to null list.\n") ;
    exit(1) ;
  }

  printf("*** Testing user_by_port()...\n") ; 
  strcpy(n, "marco") ;
  if (USER_NULL == (u = user_by_port(l1, 1))) {
    printf("Failed to find %s by port 1 in l1.\n", n) ;
    exit(1) ;
  } else if (1 != u->port && !strcmp(n, u->name)) {
    printf("Failure: user_by_port didn't return user on port 1 (%s).\n", n) ;
    exit(1) ;
  }

  strcpy(n, "norby") ;
  if (USER_NULL == (u = user_by_port(l1, 2))) {
    printf("Failed to find %s by port 2 in l1.\n", n) ;
    exit(1) ;
  } else if (2 != u->port && !strcmp(n, u->name)) {
    printf("Failure: user_by_port didn't return user on port 2 (%s).\n", n) ;
    exit(1) ;
  }

  strcpy(n, "flopez") ;
  if (USER_NULL == (u = user_by_port(l2, 1))) {
    printf("Failed to find %s by port 1 in l2.\n", n) ;
  } else if (1 != u->port && !strcmp(n, u->name)) {
    printf("Failure: user_by_port didn't return user on port 1 (%s).\n", n) ;
    exit(1) ;
  }

  if (USER_NULL != (u = user_by_port(l1, 3))) {
    printf("Failure: found bogus port 3 in l1.\n") ;
    exit(1) ;
  }

  printf("*** Testing user_by_name()...\n") ; 
  strcpy(n, "marco") ;
  if (USER_NULL == (u = user_by_name(l1, n))) {
    printf("Failed to find %s by name in l1.\n", n) ;
    exit(1) ;
  } else if (1 != u->port && !strcmp(n, u->name)) {
    printf("Failure: user_by_name didn't return %s on port 1.\n", n) ;
    exit(1) ;
  }

  strcpy(n, "norby") ;
  if (USER_NULL == (u = user_by_name(l1, n))) {
    printf("Failed to find %s by name in l1.\n", n) ;
    exit(1) ;
  } else if (2 != u->port && !strcmp(n, u->name)) {
    printf("Failure: user_by_name didn't return %s on port 2.\n", n) ;
    exit(1) ;
  }

  strcpy(n, "flopez") ;
  if (USER_NULL == (u = user_by_name(l2, n))) {
    printf("Failed to find %s by name in l2.\n", n) ;
    exit(1) ;
  } else if (1 != u->port && !strcmp(n, u->name)) {
    printf("Failure: user_by_name didn't return %s on port 1.\n", n) ;
    exit(1) ;
  }

  strcpy(n, "ryanh") ;
  if (USER_NULL != (u = user_by_name(l1, n))) {
    printf("Failure: user_by_name found bogus ryanh in l1.\n") ;
    exit(1) ;
  }

  printf("*** Testing rem_user()...\n") ; 
  u = user_by_port(l1, 2) ;
  if (TRUE != rem_user(l1, u)) {
    printf("Failure: error from rem_user on port 2 in l1.\n") ;
    exit(1) ;
  }
  if (USER_NULL != (u = user_by_port(l1, 2))) {
    printf("Failure: found bogus port 2 in l1 after deletion.\n") ;
    exit(1) ;
  }

  if (USER_NULL == (u = user_by_port(l1, 1))) {
    printf("Failure: failed to find port 1 in l1 after deletion of port 2.\n");
    exit(1) ;
  }

  exit(0) ;
}
