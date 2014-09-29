#pragma once

#include <type_traits>
#include <string>
#include <map>


namespace ppconsul {

    namespace detail {

        template<class T> struct is_widechar_: public std::false_type {};
        // If your compiler does not distinguish wchar_t and unsigned short, comment this line
        // and report an issue.
        template<> struct is_widechar_<wchar_t>: public std::true_type{};
        // MSVS 2013 does not distinguish charXX_t and unsigned short/long types
        // TODO: check for other compilers and probably disable only for MS
        //template<> struct is_widechar_<char16_t>: public std::true_type{};
        //template<> struct is_widechar_<char32_t>: public std::true_type{};

        template<class T>
        struct is_widechar: public is_widechar_<
            typename std::remove_cv<T>::type>
        {};


        template<class T,
        class Enabler = typename std::enable_if<
            std::is_arithmetic<typename std::remove_reference<T>::type>::value>
            ::type
        >
        inline std::string to_string_param(T&& t)
        {
            static_assert(!is_widechar<typename std::remove_reference<T>::type>::value,
                "Wide character types are not supported");

            return std::to_string(std::forward<T>(t));
        }

        inline std::string to_string_param(char c)
        {
            return std::string(1, c);
        }

        inline std::string to_string_param(std::string s)
        {
            return std::move(s);
        }

        class Parameter
        {
        public:
            template<class V>
            Parameter(std::string name, V&& value)
                : m_name(std::move(name))
                , m_value(to_string_param(std::forward<V>(value)))
            {}

            std::string m_name, m_value;
        };

    } // namespace detail

    // TODO: add comparison operators
    class Parameters
    {
    public:
        Parameters()
        {}

        Parameters(std::initializer_list<detail::Parameter> values)
        {
            for (const auto& p : values)
                doUpdate(p.m_name, p.m_value);
        }

        template<class V>
        Parameters(std::string name, V&& value)
        {
            doUpdate(std::move(name), detail::to_string_param(std::forward<V>(value)));
        }

        Parameters& update(const Parameters& values)
        {
            for (const auto& p : values.m_values)
                doUpdate(p.first, p.second);
            return *this;
        }

        Parameters& update(Parameters&& values)
        {
            for (auto& p : values.m_values)
                doUpdate(std::move(p.first), std::move(p.second));
            return *this;
        }

        template<class V>
        Parameters& update(std::string name, V&& value)
        {
            doUpdate(std::move(name), detail::to_string_param(std::forward<V>(value)));
            return *this;
        }

        bool empty() const { return m_values.empty(); }
        void clear() { m_values.clear(); }

        std::string query() const;

    private:
        void doUpdate(std::string name, std::string value)
        {
            m_values[std::move(name)] = std::move(value);
        }

        typedef std::map<std::string, std::string> Values;
        Values m_values;
    };

}
