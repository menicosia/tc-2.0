/* Data structures */
struct tcds {
  struct tcds *next ;
  int master_sock ;
  struct sock_set *ss ;
  struct user_l *ul ;
  struct event_l *el ;
  struct chan_l *cl ;
} ;
#define TCDS_NULL (struct tcds *)0

struct tcds_l {
  struct tcds *t ;
} ;
#define TCDS_L_NULL (struct tcds_l *)0

/* Function declarations */
struct tcds_l *new_tcds_l() ;
int rem_tcds_l(struct tcds_l *l) ;
struct tcds *new_tcds(struct tcds_l *l, int p) ;
int rem_tcds(struct tcds_l *l, struct tcds *t) ;
struct tcds *tcds_by_sock(struct tcds_l *l, int p) ;
