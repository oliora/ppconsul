# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. Note that all implemented features are tested with automated integration tests.

### Key/Value Store (KV)

Everything is supported except the following:
- ACL tokens
- Lock acquiring / release. Will be implemented after Session endpoint.
- Support of blocking queries is limited [[1]].

At the moment it's only possible to work with values as strings, but there is a plan to add a typed interface, something like `get<int>(key)`, `put(key, some_double)`, etc. 

### Agent

Everything is supported except the following:
- Set a check's initial status
- ACL tokens
- Endpoint [`/v1/agent/maintenance`](https://www.consul.io/docs/agent/http/agent.html#agent_maintenance)
- Endpoint [`/v1/agent/service/maintenance`](https://www.consul.io/docs/agent/http/agent.html#agent_service_maintenance)
- Accessing to `"Config"` object received from `/v1/agent/self` endpoint. There is no plan to support it any time soon unless requested by users.

### Catalog

Everything is supported except the following:
- ACL tokens
- Support of blocking queries is limited [[1]].
- Registration/derigistration. Please use the agent endpoint instead. There is no plan to support it any time soon unless requested by users.

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
