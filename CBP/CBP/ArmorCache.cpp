#include "pch.h"

#include "ArmorCache.h"
#include "Config.h"
#include "Common/Serialization.h"

namespace CBP
{
    IArmorCache IArmorCache::m_Instance;

    const armorCacheEntry_t* IArmorCache::GetEntry(
        const stl::fixed_string& a_path)
    {
        auto it = m_Instance.m_armorCache.find(a_path);
        if (it != m_Instance.m_armorCache.end())
            return std::addressof(it->second);

        const armorCacheEntry_t* ptr(nullptr);

        Load(a_path, ptr);

        return ptr;
    }

    bool IArmorCache::Load(
        const stl::fixed_string& a_path,
        const armorCacheEntry_t*& a_out)
    {
        try
        {
            Json::Value root;

            Serialization::ReadData(a_path.get(), root);

            if (root.isNull())
                throw std::exception("root == null");

            if (!root.isObject())
                throw std::exception("Root not an object");

            armorCacheEntry_t entry;

            for (auto it1 = root.begin(); it1 != root.end(); ++it1)
            {
                if (!it1->isObject())
                    throw std::exception("Unexpected data");

                /*if (!IConfig::IsValidGroup(configGroup)) {
                    gLog.Warning("%s: Unknown config group '%s', discarding", __FUNCTION__, configGroup.c_str());
                    continue;
                }*/

                auto& e = entry[it1.key().asString()];

                for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                {
                    if (!it2->isArray())
                        throw std::exception("Expected array");

                    if (it2->size() != 2)
                        throw std::exception("Value array size must be 2");

                    auto& v = *it2;

                    auto& type = v[0];

                    if (!type.isNumeric())
                        throw std::exception("Value type not numeric");

                    auto& value = v[1];

                    if (!value.isNumeric())
                        throw std::exception("Value not numeric");

                    std::uint32_t m = type.asUInt();

                    if (m > 2)
                        throw std::exception("Value type out of range");

                    stl::fixed_string valName(it2.key().asString());

                    if (!configComponent_t::descMap.contains(valName)) {
                        gLog.Warning("%s: Unknown value name: %s", __FUNCTION__, valName.c_str());
                        continue;
                    }

                    e.insert_or_assign(valName, std::make_pair(m, value.asFloat()));
                }
            }

            auto res = m_Instance.m_armorCache.insert_or_assign(a_path, std::move(entry));
            a_out = std::addressof(res.first->second);

            return true;
        }
        catch (const std::exception& e)
        {
            m_Instance.m_lastException = e;
            return false;
        }
    }

    bool IArmorCache::Save(
        const stl::fixed_string& a_path,
        const armorCacheEntry_t& a_in)
    {
        try
        {
            SerializeAndWrite(a_path, a_in);

            m_Instance.m_armorCache.insert_or_assign(a_path, a_in);

            return true;
        }
        catch (const std::exception& e)
        {
            m_Instance.m_lastException = e;
            return false;
        }
    }
    
    bool IArmorCache::Save(
        const stl::fixed_string& a_path,
        armorCacheEntry_t&& a_in)
    {
        try
        {
            SerializeAndWrite(a_path, a_in);

            m_Instance.m_armorCache.insert_or_assign(a_path, std::move(a_in));

            return true;
        }
        catch (const std::exception& e)
        {
            m_Instance.m_lastException = e;
            return false;
        }
    }

    void IArmorCache::SerializeAndWrite(
        const std::string& a_path, 
        const armorCacheEntry_t& a_in)
    {
        Json::Value root(Json::objectValue);

        for (auto& e : a_in)
        {
            auto& j = (root[e.first] = Json::Value(Json::objectValue));

            for (auto& f : e.second)
            {
                auto& k = j[f.first];

                k.append(f.second.first);
                if (f.second.second.type == ConfigValueType::kFloat) {
                    k.append(f.second.second.vf);
                }
            }
        }

        Serialization::WriteData(a_path, root);

    }
    
    void IArmorCache::Copy(
        const armorCacheEntry_t& a_lhs, 
        armorCacheEntrySorted_t& a_rhs)
    {
        a_rhs.clear();

        for (auto& e : a_lhs)
        {
            auto& j = a_rhs.try_emplace(e.first);

            for (const auto& f : e.second)
            {
                j.first->second.emplace(f);
            }
        }
    }
    
    void IArmorCache::Copy(
        const armorCacheEntrySorted_t& a_lhs,
        armorCacheEntry_t& a_rhs)
    {
        a_rhs.clear();

        for (auto& e : a_lhs)
        {
            auto& j = a_rhs.try_emplace(e.first);

            for (const auto& f : e.second)
            {
                j.first->second.emplace(f);
            }
        }
    }

}