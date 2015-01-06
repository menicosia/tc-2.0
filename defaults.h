/* Configuration information */

#define VERBOSE 0 /* How much to blab on stderr */

#define PROG_LABEL "TC2.0"
#define DEFAULT_PORT 3555
#define TIC_SECS 1
#define DEFAULT_CHANNEL "tc2"

#define MAX_CHAN_LEN 20
#define MAX_NAME_LEN 20

#define GREET_FILE "tc2.motd"
#define HELP_FILE "tc2.help"

/* User-level commands */
#define CMD_CHAR '/'

#define NUM_CMDS 12
#define HELP  "h"
#define ECHO  "e"
#define NAME  "n"
#define EMOTE "."
#define MESG  "m"
#define REPT  "r"
#define ACTV  "a"
#define CHANL "c"
#define JOIN  "j"
#define LEAVE "l"
#define WHO   "w"
#define QUIT  "q"

#define NUM_LONG 14
#define LONG_HELP  "help"
#define LONG_ECHO  "echo"
#define LONG_NAME  "name"
#define LONG_EMOTE "emote"
#define LONG_MESG  "mesg"
#define LONG_REPT  "repeat"
#define LONG_ACTV  "active"
#define LONG_CHANL "channels"
#define LONG_JOIN  "join"
#define LONG_LEAVE "leave"
#define LONG_WHO   "who"
#define LONG_QUIT  "quit"
#define LONG_SHUT  "SHUTDOWN"
#define LONG_BOOT  "BOOT"

/* Module information */

/* Tell the TC2 queing system how many and what the names of the module */
/* functions that add events to the queue each tic. The count and the   */
/* number of names must agree, else mayhem.                             */
#define MOD_Q_COUNT 2
#define MOD_Q_FCNS tc2_q_accept, tc2_q_read

/***                                                   ***/
/*** NO MORE CONFIGURATION DEFINITIONS BELOW THIS LINE ***/
/***                                                   ***/

#define TRUE 1
#define FALSE 0
#define GONE -5

#define Strcmp(a,b) ((NULL!=a && NULL!=b && a[0] != b[0]) ? -1 : strcmp(a,b))

