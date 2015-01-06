/* Data structures */
struct user_l {
  struct user *u ;
} ;
#define USER_L_NULL (struct user_l *)0

struct user {
  struct user *next ;
  struct chan *chan ;
  struct user *last ;
  int sock, echo, conn_time, last_time ;
  char *name ;
} ;
#define USER_NULL (struct user *)0

/* Function declarations */
struct user_l *new_user_l() ;
int rem_user_l(struct tcds *t) ;
struct user *new_user(struct tcds *t, int sock, int conn_time, char *name) ;
int rem_user(struct tcds *t, struct user *u) ;
struct user *user_by_sock(struct tcds *t, int s) ;
struct user *user_by_name(struct tcds *t, char *n) ;

