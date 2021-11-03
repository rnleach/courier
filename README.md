# courier
A thread-safe multi-producer / multi-consumer queue in C.

Courier is a sinlge-header source library in C for a threadsafe queue. 

It is intended for use cases where there will not be high contention for the queue because the 
processing time for each item in the queue will be much, much longer than pushing or popping an 
item from the queue. Thread safety is the primary goal.

It is also expected that this queue will only pass pointers since the items are expected to take a
large amount of memory.
