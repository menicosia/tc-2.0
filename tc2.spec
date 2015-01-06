General Structure of the TC2.0 system:

Instead of the old TC-1.0 system, which busy-waited on each of the
clients' sockets, the TC-2.0 system operates over time-units (tics)
performing a series of tasks each tic.

TC2.0 is just this side of object oriented. If I knew C++, I might
have written it in a fully object-oriented fashion.

How TC 2.0 handles arbitrary load:
TC 2.0 uses the time alotted in each tic to accomplish as much as it
can. Actually, tics are not strictly enforced. Instead, TC 2.0 tries
to get as many things done as possible per tic, analyzes how much time
it has spent, and forecasts from there. The function which runs the
queue checks after every completed operation if the tic limit has been
exceeded.

How TC 2.0 decides what work to do:
TC 2.0 uses a modular system, where different modules are given a
chance to insert operations into the queue each tic. Whether or not
these operations are actually accomplished in that tic depends on
on the load TC 2.0 is currently handling, and the speed it can process
that load. Such modules can be reading data in from the users, writing
data out to the users, general housekeeping, etc. There is no
specified limit on the amount of time a single operation can
take. (Ie, it is legal to enqueue a 5 minute operation!)

How the TC 2.0 operation queue is implemented:
The operation queue is simply a list of module functions. Module
functions are distinguished from typical functions in that they take a
specific structure (constructed by the queing function) as an
argument.

How TC 2.0 handles multiple channels:
All data regarding a certain set of chat information will be passed
via a TCDS -- a Tiny Chat Data Structure. This could also be thought
of a scope in which to perform operations.

