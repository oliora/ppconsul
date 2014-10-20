# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. Note that all implemented features are tested with automated integration tests.

### /v1/kv/

Most features supported. Note that:
- Lock acquiring / release is **not supported**.
- Support of blocking queries is limited [[1]].
- It's only possible to work with values as strings, but there is a plan to add typed interface, something like `get<int>(key)`, `put(key, some_double)` etc. 

### /v1/agent/

Most features supported. Note that:
- Retriving of `"Config"` from `/v1/agent/self` endpoint is **not supported**.

### /v1/catalog/
In progress

### /v1/health/
TBD

### /v1/session/
TBD

### /v1/acl/
TBD

### /v1/event/
TBD

### /v1/status/
TBD

[1]: #blocking-queries-note "Blocking Queries Note"

#### Blocking Queries Note
Blocking queries are supported in very primitive way: if query is called with extra parameter specifying wait timeout and index then the call blocks.
Background polling, async non-block handling and other smart things to work with blocking queries in a more effective way are in the far future plan.
If you have any ideas/suggerstions of a convenient interface to work with blocking queries, you are more than welcome.
