#include "pch.h"

namespace UIData
{
    void UICollapsibleStates::Parse(const Json::Value& a_in)
    {
        if (!a_in.isObject())
            return;

        for (auto it = a_in.begin(); it != a_in.end(); ++it)
        {
            if (!it->isBool())
                continue;

            m_data[it.key().asString()] = it->asBool();
        }
    }

    void UICollapsibleStates::Create(Json::Value& a_out) const
    {
        for (const auto& e : m_data)
            a_out[e.first] = e.second;
    }
}