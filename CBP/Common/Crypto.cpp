#include "pch.h"

namespace Crypto
{
    void SHA1(const std::string& a_in, std::string& a_out)
    {
        boost::uuids::detail::sha1 sha1;
        sha1.process_bytes(a_in.data(), a_in.size());
        std::uint32_t hash[5]{ 0 };
        sha1.get_digest(hash);

        char buf[41];

        _snprintf_s(buf, _TRUNCATE, "%.8X%.8X%.8X%.8X%.8X", hash[0], hash[1], hash[2], hash[3], hash[4]);

        a_out = buf;
    }
}