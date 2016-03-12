Notes about the project rationals
=================================

Just some notes for future me about the project decisions was made.

HTTP Clients
------------

http::Client - interface for http/https client
^      ^
|      |
|      curl::HttpClient
|                     ^
|                     |
|                     curl::HttpsClient
netlib::HttpClient
^
|
netlib::HttpsClient (?)





Keywords naming and namespace rationale
---------------------------------------

How to name parameters namespace?
a1) ppconsul::keywords::service_id   !!   too detailed
* a2) ppconsul::keywords::id           !!!
b1) ppconsul::_service_id            !    not clear that it's about keywords
b2) ppconsul::_id                    !

Registration interface rationale 2
----------------------------------

1) Registration object + extra keywords vs flat set of keywords

agent.registerCheck(
	keywords::name = "",
	keywords::check*** = agent::ScriptCheck{"ccc.sh", std::chrono::seconds(20)},
	keywords::notes = "",
	keywords::id = "",
	keywords::token = ""
);

agent.registerCheck(
	{
		keywords::name = "",
		keywords::check*** = agent::ScriptCheck{"ccc.sh", std::chrono::seconds(20)},
		keywords::notes = "",
		keywords::id = ""
	},
	keywords::token = ""
);

2) Use keyword arguments for all field (a) s vs only for optional ones (b)

a)
agent.registerCheck(
	{
		keywords::name = "",
		keywords::check*** = agent::ScriptCheck{"ccc.sh", std::chrono::seconds(20)},
		keywords::notes = "",
		keywords::id = ""
	},
	keywords::token = ""
);
or even
agent.registerCheck(
	{
		keywords::name = "",
		keywords::script_check = {"ccc.sh", std::chrono::seconds(20)},
		keywords::notes = "",
		keywords::id = ""
	},
	keywords::token = ""
);

b)
agent.registerCheck(
	{
		"name",
		agent::ScriptCheck{"ccc.sh", std::chrono::seconds(20)},
		keywords::notes = "",
		keywords::id = ""
	},
	keywords::token = ""
);


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




            Qs:
            1) Where to put data types - under the root namespace or under the endpoint's namespace?
                - some types are shared between endpoints, some types are different (with the same name) in different endpoints
                - probably better unique names should be found, so all types can be placed to the root namespace?
            2) Why are endpoint namespaces needed at all?
                - To avoid conflicts between entities related to different endpoints

            3) Typed parameters for check params, yes or no?

                ppconsul::agent::params::check_params = ppconsul::agent::TtlCheckParams{std::chrono::minutes(5)}
                ppconsul::agent::params::check_params = ppconsul::agent::ScriptCheckParams{"run.sh", std::chrono::minutes(5)}
         
                vs

                ppconsul::agent::params::ttl_check = {std::chrono::minutes(5)}
                ppconsul::agent::params::scipt_check = {"run.sh", std::chrono::minutes(5), "bla-bla"}
         

            5) Has user to provide a RegistrationData object?
                 a) agent.registerService()



            Ex:
            1) current approach
                ppconsul::agent::Agent agent(consul);

                agent.registerService({
                    name,
                    ppconsul::kw::check = ppconsul::agent::TtlCheckParams{std::chrono::minutes(5)},
                    ppconsul::kw::params::id = "id"
                });
             
                Long, collision between structures related to a one endpoint is possible

            2) remove endpoint namespace
                ppconsul::Agent agent(consul);

                agent.registerService({
                    name,
                    ppconsul::params::check = ppconsul::agent::TtlCheckParams{std::chrono::minutes(5)},
                    ppconsul::params::id = "id"
                });
         
                Still noisy, collisions between endpoints
         
            3) instead of endpoint's, use a type's namespace
                 ppconsul::agent::Agent agent(consul);

                 agent.registerService({
                    name,
                    ppconsul::service::check = ppconsul::agent::TtlCheckParams{std::chrono::minutes(5)},
                    ppconsul::service::id = "id"
                 });
         
                 Name is not obvious at all. There is confusion between different approach to name endpoints

             4)
                 ppconsul::agent::Agent agent(consul);

                 agent.registerService({
                    name,
                    ppconsul::agent::service_check = ppconsul::agent::TtlCheckParams{std::chrono::minutes(5)},
                    ppconsul::agent::service_id = "id"
                 });

