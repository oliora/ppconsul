# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. Note that all implemented features are tested with automated integration tests.

### kv

Implemented, except:
- acquire/release locks

Notes:
- Blocking queries are supported <sup>[1]</sup>.
- It's only possible to work with values as strings, but there is a plan to add typed interface, something like `get<int>(key)`, `put(some_double)` etc. 

### agent

Implemented, except:
- Retriving of config from /v1/agent/self

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

[1]: #Blocking-Query-Note "Blocking Query Note"

#### Blocking Query Note
Blocking queries are supported in very primitive way: if query is called with extra parameter specifying wait timeout and index then the call blocks.
Background polling, async non-block handling and other smart things to work with blocking queries in a more effective way are in the far future plan.
If you have any ideas/suggerstions of a convenient interface to work with blocking queries, you are more than welcome.
