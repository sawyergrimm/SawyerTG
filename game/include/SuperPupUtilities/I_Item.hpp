#pragma once
#include <string>

namespace SuperPupUtilities
{
    class I_Item
    {
    public:
        virtual ~I_Item() = default;
        virtual std::string GetName() = 0;
    };
}