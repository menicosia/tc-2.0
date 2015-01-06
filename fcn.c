#include <stdio.h>
#include <stdlib.h>

extern int errno ;

struct args_a {
  int a ;
} ;

struct args_b {
  int b ;
} ;

union args {
  struct args_a *a ;
  struct args_b *b ;
} ;

int main() {
  union args *foo ;

  if (NULL == (foo = (union args *)malloc(sizeof(union args)))) {
    fprintf(stderr, "Error in malloc: %s\n", strerror(errno)) ;
    return -1 ;
  }

  if (NULL == (foo->a = (struct args_a *)malloc(sizeof(struct args_a)))) {
    fprintf(stderr, "Error in malloc: %s\n", strerror(errno)) ;
    return -1 ;
  }

  foo->a->a = 12 ;

  return 0 ;
}
