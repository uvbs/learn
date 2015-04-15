#pragma once

#include <string>

namespace DwUtility
{
    namespace encryption
    {
        std::string AESEncrypt(std::string const& in,std::string const& key);
        std::string AESDecrypt(std::string const& in,std::string const& key);
    }
}
