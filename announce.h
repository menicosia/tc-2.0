int announce_lost(struct tcds *t, struct user *u) ;
int announce_quit(struct tcds *t, struct user *u) ;
int announce_new(struct tcds *t, struct user *u) ;
int announce(struct tcds *t, struct chan *c, struct user *u, char *str) ;
int announce_all(struct tcds *t, struct user *u, char *str) ;
