#include "pch.h"

#include <boost/uuid/detail/sha1.hpp>

namespace Crypto
{
    std::string SHA1(const std::string& a_in)
    {
        boost::uuids::detail::sha1 sha1;
        sha1.process_bytes(a_in.data(), a_in.size());
        std::uint32_t hash[5]{ 0 };
        sha1.get_digest(hash);

        std::ostringstream ss;

        for (int i = 0; i < 5; i++)
        {
            ss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << hash[i];
        }

        return ss.str();
    }
}