//  Copyright (c) 2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ppconsul/consul.h"
#include "ppconsul/helpers.h"
#include "ppconsul/types.h"

namespace ppconsul { namespace status {

	namespace impl {
		std::string parseLeader(const std::string&);

		std::vector<std::string> parsePeers(const std::string&);
	}

	class Status 
	{
		public:
		explicit Status(Consul &consul)
		: m_consul(consul)
		{}

		std::string leader() const
		{
			return impl::parseLeader(m_consul.get("/v1/status/leader"));
		}

		std::vector<std::string> peers() const
		{
			return impl::parsePeers(m_consul.get("/v1/status/peers"));
		}

		private:
		Consul &m_consul;
	};



}}