# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. Note that all implemented features are tested with automated integration tests.

### kv

Getting, putting, deleting of values and getting of keys operation supported. CAS, flags, consistency modes, ACL and blocking queries are supported.
The only thing is NOT supported at the moment is acquire/release locks.
 
It's only possible to work with values as strings, but there is a plan to add typed interface, something like `get<int>(key)`, `put(some_double)` etc.

### agent
**In progress**

### catalog
TBD

### health
TBD

### session
TBD

### acl
TBD

### event
TBD

### status
TBD

### Note for Blocking Queries

Blocking queries are supported in very primitive way: if query is called with extra parameter specifying wait timeout and index then the call blocks.
Background polling, async non-block handling and other smart things to work with blocking queries in a more effective way are in the far future plan.
If you have any ideas/suggerstions of a convenient interface to work with blocking queries, you are more than welcome.
