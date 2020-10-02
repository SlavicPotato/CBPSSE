#include "pch.h"

namespace CBP
{
    IData::raceList_t IData::raceList;
    IData::actorRefMap_t IData::actorNpcMap;
    IData::actorCache_t IData::actorCache;
    Game::ObjectHandle IData::crosshairRef = 0;
    uint64_t IData::actorCacheUpdateId = 1;
    armorCache_t IData::armorCache;

    std::map<UInt32, modData_t> IData::modList;

    except::descriptor IData::lastException;

    std::unordered_set<Game::FormID> IData::ignoredRaces = {
        0x0002C65C,
        0x00108272,
        0x0002C659,
        0x0002C65B,
        0x0002C65A
    };

    void IData::UpdateActorMaps(Game::ObjectHandle a_handle, const Actor* a_actor)
    {
        actorRefData_t tmp;

        auto npc = DYNAMIC_CAST(a_actor->baseForm, TESForm, TESNPC);
        if (npc != nullptr)
        {
            if (a_actor->race != nullptr)
            {
                tmp.race.first = true;
                tmp.race.second = a_actor->race->formID;
            }
            else
                tmp.race.first = false;

            tmp.npc = npc->formID;
            tmp.sex = CALL_MEMBER_FN(npc, GetSex)();
            tmp.baseflags = npc->flags;

            actorNpcMap.insert_or_assign(
                a_handle, std::move(tmp));
        }
    }

    void IData::UpdateActorMaps(Game::ObjectHandle a_handle)
    {
        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (actor == nullptr)
            return;

        UpdateActorMaps(a_handle, actor);
    }

    void IData::FillActorCacheEntry(Game::ObjectHandle a_handle, actorCacheEntry_t& a_out)
    {
        std::ostringstream ss;

        ss << "[" << std::uppercase << std::setfill('0') <<
            std::setw(8) << std::hex << (a_handle & 0xFFFFFFFF) << "]";

        auto it = actorNpcMap.find(a_handle);
        if (it != actorNpcMap.end())
        {
            a_out.base = it->second.npc;
            a_out.race = it->second.race.first ?
                it->second.race.second :
                Game::FormID(0);
            a_out.female = it->second.sex == 1;
            a_out.baseflags = it->second.baseflags;
        }
        else {
            a_out.race = Game::FormID(0);
            a_out.base = Game::FormID(0);
            a_out.female = false;
            a_out.baseflags = 0;
        }

        a_out.name = ss.str();
    }

    void IData::FillActorCacheEntry(Actor* a_actor, actorCacheEntry_t& a_out)
    {
        std::ostringstream ss;

        ss << "[" << std::uppercase << std::setfill('0') <<
            std::setw(8) << std::hex << a_actor->formID << "] " <<
            CALL_MEMBER_FN(a_actor, GetReferenceName)();

        if (a_actor->race != nullptr)
            a_out.race = a_actor->race->formID;
        else
            a_out.race = Game::FormID(0);

        auto npc = DYNAMIC_CAST(a_actor->baseForm, TESForm, TESNPC);
        if (npc != nullptr)
        {
            a_out.base = npc->formID;
            a_out.female = CALL_MEMBER_FN(npc, GetSex)() == 1;
            a_out.baseflags = npc->flags;
        }
        else {
            a_out.base = Game::FormID(0);
            a_out.female = false;
            a_out.baseflags = 0;
        }

        a_out.name = ss.str();
    }

    void IData::AddExtraActorEntry(
        Game::ObjectHandle a_handle)
    {
        if (actorCache.find(a_handle) != actorCache.end())
            return;

        actorCacheEntry_t tmp;
        tmp.active = false;

        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);

        if (actor)
            FillActorCacheEntry(actor, tmp);
        else
            FillActorCacheEntry(a_handle, tmp);

        actorCache.emplace(a_handle, std::move(tmp));
    }

    void IData::UpdateActorCache(const simActorList_t& a_list)
    {
        actorCache.clear();
        crosshairRef = 0;

        for (const auto& e : a_list)
        {
            auto actor = Game::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (actor == nullptr)
                continue;

            actorCacheEntry_t tmp;
            tmp.active = true;

            FillActorCacheEntry(actor, tmp);

            actorCache.emplace(e.first, std::move(tmp));
        }

        for (const auto& e : IConfig::GetActorPhysicsConfigHolder())
            AddExtraActorEntry(e.first);

        for (const auto& e : IConfig::GetActorNodeConfigHolder())
            AddExtraActorEntry(e.first);

        auto refHolder = CrosshairRefHandleHolder::GetSingleton();
        if (refHolder) {
            auto handle = refHolder->CrosshairRefHandle();

            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(handle, ref);
            if (ref != nullptr) {
                if (ref->formType == Actor::kTypeID) {
                    Game::ObjectHandle handle;
                    if (Game::GetHandle(ref, ref->formType, handle)) {
                        auto it = actorCache.find(handle);
                        if (it != actorCache.end()) {
                            crosshairRef = it->first;
                        }
                    }
                }
            }
        }

        actorCacheUpdateId++;
    }

    bool IData::PopulateRaceList()
    {
        raceList.clear();

        auto dh = DataHandler::GetSingleton();
        if (!dh)
            return false;

        for (UInt32 i = 0; i < dh->races.count; i++)
        {
            auto race = dh->races[i];
            if (race == nullptr)
                continue;

            if (race->formID == 0)
                continue;

            if (race->data.raceFlags & TESRace::kRace_Child)
                continue;

            if (IsIgnoredRace(race->formID))
                continue;

            const char* fullName = race->fullName.GetName();
            if (!fullName)
                fullName = "";

            const char* edid = race->editorId.c_str();
            if (!edid)
                edid = "";

            bool playable = (race->data.raceFlags & TESRace::kRace_Playable) != 0;

            raceList.emplace(race->formID, 
                raceCacheEntry_t{ playable, fullName, edid, race->data.raceFlags });
        }

        return true;
    }

    bool IData::PopulateModList()
    {
        auto dh = DataHandler::GetSingleton();
        if (!dh)
            return false;

        for (auto it = dh->modList.modInfoList.Begin(); !it.End(); ++it)
        {
            auto modInfo = it.Get();
            if (!modInfo)
                continue;

            if (!modInfo->IsActive())
                continue;

            modList.try_emplace(it->GetPartialIndex(),
                modInfo->fileFlags,
                modInfo->modIndex,
                modInfo->lightIndex,
                modInfo->name);
        }

        return true;
    }

    bool IData::GetActorName(Game::ObjectHandle a_handle, std::string& a_out)
    {
        auto it = actorCache.find(a_handle);
        if (it != actorCache.end()) {
            a_out = it->second.name;
            return true;
        }
        return false;
    }

    bool IData::HasArmorCacheEntry(const std::string& a_path)
    {
        return armorCache.find(a_path) != armorCache.end();
    }

    const armorCacheEntry_t* IData::GetArmorCacheEntry(const std::string& a_path)
    {
        auto it = armorCache.find(a_path);
        if (it != armorCache.end())
            return std::addressof(it->second);

        armorCacheEntry_t* ptr = nullptr;
        if (UpdateArmorCache(a_path, &ptr))
            return ptr;

        return nullptr;
    }

    bool IData::UpdateArmorCache(const std::string& a_path, armorCacheEntry_t** a_out)
    {
        try
        {
            const fs::path path(a_path);

            if (!fs::exists(path) || !fs::is_regular_file(path))
                throw std::exception("Invalid or non-existent path");

            std::ifstream ifs;

            ifs.open(path, std::ifstream::in | std::ifstream::binary);
            if (!ifs.is_open())
                throw std::exception("Couldn't open file for reading");

            Json::Value root;
            ifs >> root;

            if (!root.isObject())
                throw std::exception("Root not an object");

            armorCacheEntry_t entry;

            for (auto it1 = root.begin(); it1 != root.end(); ++it1)
            {
                if (!it1->isObject())
                    throw std::exception("Unexpected data");

                std::string configGroup(it1.key().asString());
                transform(configGroup.begin(), configGroup.end(), configGroup.begin(), ::tolower);

                if (!IConfig::IsValidConfigGroup(configGroup))
                    continue;

                auto& e = entry[configGroup];

                for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                {
                    if (!it2->isArray())
                        throw std::exception("Unexpected data");

                    if (it2->size() != 2)
                        throw std::exception("Value array size must be 2");

                    auto& v = *it2;

                    auto& type = v[0];

                    if (!type.isNumeric())
                        throw std::exception("Value type not numeric");

                    auto& value = v[1];

                    if (!value.isNumeric())
                        throw std::exception("Value not numeric");

                    uint32_t m = type.asUInt();

                    if (m > 1)
                        throw std::exception("Value type out of range");

                    std::string valName(it2.key().asString());
                    transform(valName.begin(), valName.end(), valName.begin(), ::tolower);

                    if (!configComponent_t::descMap.contains(valName)) {
                        gLog.Warning("%s: Unknown value name: %s", __FUNCTION__, valName.c_str());
                        continue;
                    }

                    auto& r = e[valName];

                    r.first = m;
                    r.second = value.asFloat();
                }
            }

            auto res = armorCache.insert_or_assign(a_path, std::move(entry));
            *a_out = std::addressof(res.first->second);

            return true;
        }
        catch (const std::exception& e)
        {
            lastException = e;
            return false;
        }
    }
}