/* Data structures */

/* This structure contains the invoking user and the actual cmd string */
struct cmd {
  struct user *u ;
  char *str ;
} ;
#define CMDS_NULL (struct cmds *)0 

/* Function declarations */
int do_help(struct tcds *t, struct event *e) ;
int toggle_echo(struct tcds *t, struct event *e) ;
int do_mesg(struct tcds *t, struct event *e) ;
int mesg(struct tcds *t, struct user *from, struct user *to, char *str) ;
int mesg_usage(struct tcds *t, struct event *e) ;
int do_rept(struct tcds *t, struct event *e) ;
int change_name(struct tcds *t, struct event *e) ;
int do_emote(struct tcds *t, struct event *e) ;
int do_who(struct tcds *t, struct event *e) ;
int do_active(struct tcds *t, struct event *e) ;
int do_chanl(struct tcds *t, struct event *e) ;
int do_join(struct tcds *t, struct event *e) ;
int do_leave(struct tcds *t, struct event *e) ;
int do_quit(struct tcds *t, struct event *e) ;
int do_shutdown(struct tcds *t, struct event *e) ;
int do_boot(struct tcds *t, struct event *e) ;
