/* Data structures */

struct event {
  struct event *next ;
  int (*f)(struct tcds *t, struct event *e) ;
  union {
    int i ;
    char *s ;
    struct user *u ;
    struct cmd *c ;
  } a ;
} ;
#define EVENT_NULL (struct event *)0

struct event_l {
  struct event *e ;
} ;
#define EVENT_L_NULL (struct event_l *)0

/* Fcn module to do nothing */
int tc2_nop(struct tcds *t, struct event *e) ;

/* Function declarations */
struct event_l *new_event_l() ;
int rem_event_l(struct tcds *t) ;
struct event *new_event() ;
int rem_event(struct event_l *l, struct event *e) ;
struct event *get_event(struct tcds *t) ;
int add_to_el(struct tcds *t, struct event *e) ;
