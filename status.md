# HTTP API v1 Implementation Status

The status of Consul HTTP API v1 coverage. The API is extending on each Consul release and doesn't have a precise versioning thus I'm trying to keep this document syncronized with the latest Consul documentation (https://www.consul.io/docs/agent/http.html). At the moment, the document reflects **Consul 0.7.2** HTTP API.

Note that all implemented features are tested with automated integration tests.

## General

### Blocking Queries

Blocking query feature is supported in a straitforward way: when API that supports blocking queries is called with keyword `block_for=` then the call blocks. There is no smart things like async non-block handling with calbacks, backgound polling etc. If you have any ideas/suggerstions of a convenient interface to work with blocking queries, you are welcome.

## Endpoints

### ACLs

TBD

### Agent

Endpoint is supported except the following:
- ACL tokens
- Endpoint [`/v1/agent/reload`](https://www.consul.io/docs/agent/http/agent.html#agent_reload)
- Endpoint [`/v1/agent/monitor`](https://www.consul.io/docs/agent/http/agent.html#agent_monitor)
- Endpoint [`/v1/agent/leave`](https://www.consul.io/docs/agent/http/agent.html#agent_leave)
- Endpoint [`/v1/agent/maintenance`](https://www.consul.io/docs/agent/http/agent.html#agent_maintenance)
- Endpoint [`/v1/agent/service/maintenance`](https://www.consul.io/docs/agent/http/agent.html#agent_service_maintenance)
- Specify an initial status for checks (`"Status"` field)
- Multiple checks associated with a single service.
- Accessing to `"Config"` and `"Stats"` objects received from `/v1/agent/self` endpoint. There is no plan to support it any time soon unless requested by users.

### Catalog

Endpoint is supported except the following:
- ACL tokens
- Parameter `?near=` for querying nodes and services
- Registration/deregistration. Please use the agent endpoint instead. There is no plan to support it any time soon unless requested by users.

### Coordinates

TBD

### Events

TBD

### Health

Endpoint support is in progress (code is here but not tests yet)

Not supported:
- ACL tokens

### KV Store

Endpoint is supported except the following:
- ACL tokens

At the moment it's only possible to work with values as strings, but there is a plan to add a typed interface, something like `get<int>(key)`, `put(key, some_double)`, etc. 

### Operator

TBD

### Prepared Query

TBD

### Sessions

TBD

### Snapshots

TBD

### Status

Endpoint is supported

### Transactions

TBD
