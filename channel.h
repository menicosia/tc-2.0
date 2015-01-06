/* Data structures */

struct chan {
  struct chan *next ;
  char *title ;
  struct member_l *ml ;
} ;
#define CHAN_NULL (struct chan *)0

struct chan_l {
  struct chan *c ;
} ;
#define CHAN_L_NULL (struct chan_l *)0

struct member {
  struct member *next ;
  struct user *u ;
} ;
#define MEMBER_NULL (struct member *)0 

struct member_l {
  struct member *m ;
} ;
#define MEMBER_L_NULL (struct member_l *)0

/* Functions */
struct chan_l *new_chan_l() ;
int rem_chan_l(struct tcds *t) ;
int new_chan(struct tcds *t, char *title) ;
int rem_chan(struct tcds *t, char *title) ;
struct member_l *new_member_l() ;
int rem_member_l(struct chan *c) ;
int new_member(struct tcds *t, char *title, struct user *u) ;
int rem_member(struct tcds *t, char *title, struct user *u) ;
int rem_user_from_all(struct tcds *t, struct user *u) ;
struct chan *chan_by_name(struct tcds *t, char *n) ;
int num_members(struct tcds *t, char *title) ;
int is_member(struct tcds *t, char *title, struct user *u) ;
