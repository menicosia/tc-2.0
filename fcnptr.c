#include <stdio.h>
#include <stdlib.h>

struct f {
  int (*fptr)() ;
} ;

int foo();  

int main() {
  struct f *fcn ;

  fcn = (struct f *)malloc(sizeof(struct f)) ;
  fcn->fptr = foo ;
  fcn->fptr() ;

  exit(0) ;
}

int foo() {
  printf("Found my inner self!\n") ;
  return(0) ;
}


/*  int (*q_fcns[2])() = { foo, foo } ; */

/*    q_fcns[0]() ; */
/*    q_fcns[1]() ; */
