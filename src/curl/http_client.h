//  Copyright (c)  2014-2017 Andrey Upadyshev <oliora@gmail.com>
//
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <ppconsul/http_client.h>
#include <curl/curl.h>


namespace ppconsul { namespace curl {

    class HttpClient: public ppconsul::http::impl::Client
    {
    public:
        using GetResponse = std::tuple<http::Status, ResponseHeaders, std::string>;

        HttpClient();

        virtual ~HttpClient() override;

        virtual GetResponse get(const std::string& url) override;
        virtual std::pair<http::Status, std::string> put(const std::string& url, const std::string& data) override;
        virtual std::pair<http::Status, std::string> del(const std::string& url) override;

        HttpClient(const HttpClient&) = delete;
        HttpClient(HttpClient&&) = delete;
        HttpClient& operator= (const HttpClient&) = delete;
        HttpClient& operator= (HttpClient&&) = delete;

    private:
        template<class Opt, class T>
        void setopt(Opt opt, const T& t);

        void perform();

        CURL *m_handle; // TODO: use unique_ptr<CURL, ...>
        char m_errBuffer[CURL_ERROR_SIZE]; // TODO: replace with unique_ptr<char[]> to allow move
    };

}}
