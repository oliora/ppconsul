Notes about the project rationals
=================================

Just some notes for future me about the project decisions was made.


Registration interface rationale
--------------------------------

Checks:
All checks has "name", "id" = name, "notes" = "", "name" = "service:<ServiceId>" for a service checks.

* TTL: "ttl"
* Script: "script" + "interval" [+ "timeout"]
* HTTP: "http" + "interval" [+ "timeout"]
* Docker: "docker_container_id" + "shell" + "script" + "interval"


```code=cpp
// New style 1: all optional params are keywords
agent.registerCheck("check1", params::ttl=std::chrono::seconds(10));
agent.registerCheck("check1", params::notes="", params::ttl=std::chrono::seconds(10));
agent.registerCheck("check1", params::notes="", params::script="bla-bla.py", params::interval=std::chrono::seconds(10));
agent.registerService("service1", params::tags={"udp", "service1", "service2"}, params::ttl=std::chrono::seconds(10));
agent.registerService("service1", params::tags={"udp", "service1", "service2"}, params::script="bla-bla.py", params::interval=std::chrono::seconds(10));

// New style 2: structures with keywords
agent.registerCheck({"check1", params::notes="bla-bla"}, HttpCheck("http://localhost/test", std::chrono::seconds(10), params::tiemout=std::chrono::seconds(2)));
agent.registerService({"service1", params::tags={"udp", "service1", "service2"}});
// ...

// New Style 3: higher granulated keywords
agent.registerCheck("check1", params::notes="", params::ttl_check={std::chrono::seconds(10)});
agent.registerCheck("check1", params::notes="", params::script_check={params::script="bla-bla.py", params::interval=std::chrono::seconds(10)});
agent.registerService("service1", params::tags={"udp", "service1", "service2"});
agent.registerService("service1", params::tags={"udp", "service1", "service2"}, params::ttl_check={std::chrono::seconds(10)});
agent.registerService("service1", params::tags={"udp", "service1", "service2"}, params::script_check={"bla-bla.py", std::chrono::seconds(10), params::timeout=std::chrono::seconds(2)});

// New Style 4: higher granulated keywords using variant
agent.registerCheck("check1", TtlCheck{std::chrono::seconds(10)});
agent.registerCheck("check1", ScriptCheck{"bla-bla.py", std::chrono::seconds(10), params::timeout=std::chrono::seconds(2)}, params::notes="");
agent.registerService("service1", params::tags={"udp", "service1", "service2"});
agent.registerService("service1", params::tags={"udp", "service1", "service2"}, params::check=TtlCheck{std::chrono::seconds(10)});
agent.registerService("service1", params::tags={"udp", "service1", "service2"}, params::check=ScriptCheck{"bla-bla.py", std::chrono::seconds(10), params::timeout=std::chrono::seconds(2)});
```

Option 4 seems the most descriptive, clear and intuitive.