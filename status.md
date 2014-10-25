# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. Note that all implemented features are tested with automated integration tests.

### KV

Most features supported. Note that:
- Lock acquiring / release is **not supported**.
- Support of blocking queries is limited [[1]].
- It's only possible to work with values as strings, but there is a plan to add typed interface, something like `get<int>(key)`, `put(key, some_double)` etc. 

### Agent

Most features supported. Note that:
- Accessing to `"Config"` object received from `/v1/agent/self` endpoint is **not supported**.

### Catalog

Most features supported. Note that:
- Registration/derigistration is **not supported**. The agent endpoint should be used instead. I have no plans to implement this feature until I get a real use case for it.
- Support of blocking queries is limited [[1]].

### Health

In progress.

### Session
TBD

### ACL
TBD

### Event
TBD

### Status
TBD

[1]: #blocking-queries-note "Blocking Queries Note"

#### Blocking Queries Note
Blocking queries are supported in very primitive way: if query is called with extra parameter specifying wait timeout and index then the call blocks.
Background polling, async non-block handling and other smart things to work with blocking queries in a more effective way are in the far future plan.
If you have any ideas/suggerstions of a convenient interface to work with blocking queries, you are more than welcome.
