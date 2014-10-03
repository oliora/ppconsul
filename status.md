# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. Note that all implemented features are tested with automated integration tests.

### kv

Get value(s), put value, delete value(s) and get keys operation supported. CAS and flags supported. The following is **NOT** supported:
* Consistency modes
* Blocking queries
* Acquire/release locks
 
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

## Documentation
TBD
