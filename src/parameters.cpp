#include "ppconsul/parameters.h"
#include  <numeric>


namespace ppconsul {

    std::string Parameters::query() const
    {
        // TODO: encode parameters

        if (m_values.empty())
            return {};

        std::string r;

        // 2 is for '=' and '&'. -1 is because first param has no '&'
        auto len = std::accumulate(m_values.begin(), m_values.end(), -1, [](int s, const Values::value_type& p) {
            return s + p.first.size() + p.second.size() + 2;
        });

        r.reserve(len - 1);

        auto it = m_values.begin();

        r += it->first;
        r += '=';
        r += it->second;
        ++it;

        for(;it != m_values.end(); ++it)
        {
            r += '&';
            r += it->first;
            r += '=';
            r += it->second;
        }

        return r;
    }

}
