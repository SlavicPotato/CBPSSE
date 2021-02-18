#include "pch.h"

namespace CBP
{
    IBoneCast IBoneCast::m_Instance;

    IBoneCast::IBoneCast() :
        m_cache(m_iio, 1024 * 1024 * 64)
    {
    }

    bool IBoneCast::ExtractGeometry(
        Actor* a_actor,
        const BSFixedString& a_nodeName,
        NiAVObject* a_armorNode,
        ColliderDataStorage& a_out)
    {
        auto geometry = a_armorNode->GetAsBSGeometry();
        if (!geometry)
            return false;

        if (geometry->shapeType != BSGeometry::Type::kTriShape)
            return false;

        auto vflags = NiSkinPartition::GetVertexFlags(geometry->vertexDesc);

        if ((vflags & VertexFlags::VF_VERTEX) != VertexFlags::VF_VERTEX)
            return false;

        auto triShape = RTTI<BSTriShape>()(geometry);
        if (!triShape)
            return false;

        auto& skinInstance = geometry->m_spSkinInstance;
        if (skinInstance == nullptr)
            return false;

        IScopedCriticalSectionEx _(&skinInstance->lock);

        auto& skinPartition = skinInstance->m_spSkinPartition;
        if (skinPartition == nullptr)
            return false;

        auto& skinData = skinInstance->m_spSkinData;
        if (skinData == nullptr)
            return false;

        if (!skinPartition->m_uiPartitions)
            return false;

        if (!skinPartition->m_pkPartitions[0].m_usTriangles)
            return false;

        UInt16 numTriangles = triShape->triangleCount ? triShape->triangleCount : skinPartition->m_pkPartitions[0].m_usTriangles;
        UInt32 numVertices = triShape->numVertices ? triShape->numVertices : skinPartition->vertexCount;

        if (numTriangles == 0 || numVertices == 0)
            return false;

        for (UInt32 i = 0; i < skinData->m_uiBones; i++)
        {
            auto bone = skinInstance->m_ppkBones[i];
            if (!bone)
                continue;

            //_DMESSAGE(">>>> %s", bone->m_name);

            if (a_nodeName.data != bone->m_name)
                continue;

            auto& boneData = skinData->m_pkBoneData[i];

            if (boneData.m_usVerts == 0)
                return false;

            //UInt32 numIndices = numTriangles * 3;

            UInt32 vertexSize = NiSkinPartition::GetVertexSize(geometry->vertexDesc);

            UInt8* vertices = skinPartition->m_pkPartitions[0].shapeData->m_RawVertexData;
            UInt16* trilist = skinPartition->m_pkPartitions[0].m_pusTriList;

            /*PerfTimer pt;
            pt.Start();*/

            auto tris = std::make_unique<Triangle[]>(numTriangles);
            auto vertmap = std::make_unique<Vertex[]>(numVertices);

            for (UInt16 i = 0, j = 0; i < numTriangles; i++, j += 3)
            {
                auto tri = std::addressof(tris[i]);

                tri->m_isBoneTri = false;

                auto i1 = tri->m_indices[0] = trilist[j];
                auto i2 = tri->m_indices[1] = trilist[j + 1];
                auto i3 = tri->m_indices[2] = trilist[j + 2];

                if (i1 >= numVertices || i2 >= numVertices || i3 >= numVertices) {
                    m_Instance.Debug("%s: [m_pusTriList] index >= numVertices", a_nodeName.c_str());
                    return false;
                }

                vertmap[i1].m_triangles.emplace_back(tri);
                vertmap[i2].m_triangles.emplace_back(tri);
                vertmap[i3].m_triangles.emplace_back(tri);

            }

            for (UInt16 i = 0; i < boneData.m_usVerts; i++)
            {
                auto& v = boneData.m_pkBoneVertData[i];

                if (v.m_usVert >= numVertices) {
                    m_Instance.Debug("%s: [m_pkBoneVertData] index >= numVertices", a_nodeName.c_str());
                    return false;
                }

                auto& d = vertmap[v.m_usVert];

                d.m_weight = v.m_fWeight;

                for (auto& e : d.m_triangles)
                    e->m_isBoneTri = true;

            }

            size_t c = 0;
            uint32_t vi = 0;

            for (UInt16 i = 0; i < numTriangles; i++)
            {
                auto& tri = tris[i];

                if (tri.m_isBoneTri) {

                    for (int j = 0; j < 3; j++) {
                        auto index = tri.m_indices[j];

                        auto& vme = vertmap[index];

                        if (!vme.m_hasVertex) {
                            auto vtx = reinterpret_cast<DirectX::XMVECTOR*>(&vertices[index * vertexSize]);

                            auto p = boneData.m_kSkinToBone * NiPoint3(
                                vtx->m128_f32[0], vtx->m128_f32[1], vtx->m128_f32[2]);

                            tri.m_indices[j] = vi;

                            vme.m_vertex = MeshPoint{ p.x, p.y, p.z };
                            vme.m_index = vi;
                            vme.m_hasVertex = true;

                            vi++;
                        }
                        else {
                            tri.m_indices[j] = vme.m_index;
                        }

                        if (vme.m_weight < 0.0f)
                            vme.m_weight = 0.0f;
                    }

                    c++;
                }
            }

            if (c == 0 || vi == 0)
                return false;

            size_t rnIndices = c * 3;
            size_t rnVertices = static_cast<size_t>(vi);

            a_out.m_vertices.resize(rnVertices);
            a_out.m_weights.resize(rnVertices);
            a_out.m_indices.resize(rnIndices);

            for (UInt32 i = 0; i < numVertices; i++) {

                auto& vme = vertmap[i];

                if (vme.m_hasVertex)
                {
                    auto index = vme.m_index;

                    a_out.m_vertices[index].x = vme.m_vertex.x;
                    a_out.m_vertices[index].y = vme.m_vertex.y;
                    a_out.m_vertices[index].z = vme.m_vertex.z;

                    a_out.m_weights[index] = vme.m_weight;
                }
            }

            for (UInt16 i = 0, c = 0; i < numTriangles; i++)
            {
                auto& tri = tris[i];

                if (tri.m_isBoneTri) {

                    for (int j = 0; j < 3; j++) {
                        auto index = tri.m_indices[j];

                        a_out.m_indices[c] = index;

                        c++;
                    }

                }
            }

            a_out.m_numTriangles = static_cast<int>(rnIndices / 3);

            return true;

        }

        return false;
    }

    bool IBoneCast::UpdateGeometry(
        BoneCastCache::CacheEntry::data_t& a_in,
        float a_weightThreshold,
        float a_simplifyTarget,
        float a_simplifyTargetError)
    {
        auto numIndices = a_in.first.m_indices.size();

        auto indices = decltype(a_in.second.m_indices)();

        indices.reserve(numIndices);

        for (size_t i = 0; i < numIndices; i += 3)
        {
            bool skip(false);

            size_t m = i + 3;

            for (size_t j = i; j < m; j++)
            {
                int index = a_in.first.m_indices[j];

                if (a_in.first.m_weights[index] < a_weightThreshold)
                {
                    skip = true;
                    break;
                }
            }

            if (skip)
                continue;

            for (size_t j = i; j < m; j++)
            {
                indices.emplace_back(a_in.first.m_indices[j]);
            }
        }

        numIndices = indices.size();

        if (numIndices < 3) {
            a_in.second.Clear();
            return false;
        }

        auto targetIndices = std::clamp<size_t>(static_cast<size_t>(static_cast<double>(numIndices) * a_simplifyTarget), 3, numIndices);

        if (targetIndices < numIndices)
        {
            auto tmp = std::make_unique<unsigned int[]>(numIndices);

            auto newIndices = meshopt_simplify(
                tmp.get(),
                reinterpret_cast<unsigned int*>(indices.data()),
                numIndices,
                reinterpret_cast<const float*>(a_in.first.m_vertices.data()),
                a_in.first.m_vertices.size(),
                sizeof(MeshPoint),
                targetIndices,
                a_simplifyTargetError);

            if (newIndices % 3 != 0) {
                a_in.second.Clear();
                return false;
            }

            a_in.second.m_indices = decltype(a_in.second.m_indices)(newIndices);
            a_in.second.m_hullPoints = decltype(a_in.second.m_hullPoints)(newIndices);

            for (decltype(newIndices) i = 0; i < newIndices; i++)
            {
                auto index = static_cast<int>(tmp[i]);

                a_in.second.m_indices[i] = index;
                a_in.second.m_hullPoints[i] = a_in.first.m_vertices[index];
            }

            a_in.second.m_numTriangles = static_cast<int>(newIndices / 3);
        }
        else
        {
            a_in.second.m_indices = std::move(indices);
            a_in.second.m_indices.shrink_to_fit();

            a_in.second.m_hullPoints = decltype(a_in.second.m_hullPoints)(numIndices);

            for (decltype(numIndices) i = 0; i < numIndices; i++)
                a_in.second.m_hullPoints[i] = a_in.first.m_vertices[a_in.second.m_indices[i]];

            a_in.second.m_numTriangles = static_cast<int>(numIndices / 3);
        }

        return true;

    }

    bool IBoneCast::GetGeometry(
        Actor* a_actor,
        const std::string& a_nodeName,
        const std::string& a_shape,
        ColliderDataStorage& a_result)
    {
        if (a_shape.empty() || a_nodeName.empty())
            return false;

        BSFixedString shape(a_shape.c_str());
        BSFixedString nodeName(a_nodeName.c_str());

        //_DMESSAGE(">>> %s", a_shape.c_str());

        bool found = IArmor::VisitEquippedNodes(a_actor, [&](TESObjectARMO*, TESObjectARMA*, NiAVObject* a_object, bool a_firstPerson)
            {
                //_DMESSAGE("%s: equip (%hhu): %s", a_nodeName.c_str(), a_firstPerson, a_object->m_name);

                /*if (a_firstPerson)
                    return false;*/

                if (a_object->m_name != shape.data)
                    return false;

                return ExtractGeometry(a_actor, nodeName, a_object, a_result);
            });

        if (!found) {
            auto skin = Game::GetActorSkin(a_actor);
            if (skin) {
                found = IArmor::VisitArmor(a_actor, skin, false,
                    [&](TESObjectARMO*, TESObjectARMA*, NiAVObject* a_object, bool a_firstPerson)
                    {
                        //_DMESSAGE("%s: skin (%hhu): %s", a_object->m_name, a_firstPerson, a_object->m_name);

                        return ExtractGeometry(a_actor, nodeName, a_object, a_result);
                    });
            }
        }

        //_DMESSAGE(">>> %d", found);

        return found;
    }

    void BoneCastCreateTask1::Run()
    {
        auto actor = m_handle.Resolve<Actor>();
        if (!actor)
            return;

        IScopedCriticalSection _(DCBP::GetLock());

        auto& nodeConfig = CBP::IConfig::GetActorNode(m_handle);
        auto itn = nodeConfig.find(m_nodeName);
        if (itn == nodeConfig.end())
            return;

        if (!IBoneCast::Update(m_handle, m_nodeName, itn->second))
            return;

        DCBP::DispatchActorTask(
            m_handle, ControllerInstruction::Action::UpdateConfig);
    }

    BoneCastCache::BoneCastCache(
        IBoneCastIO& a_iio,
        size_t a_maxSize)
        :
        m_maxSize(a_maxSize),
        m_totalSize(0),
        m_iio(a_iio)
    {
    }

    template <class T, BoneCastCache::is_data_type<T>>
    auto BoneCastCache::Add(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        T&& a_data) -> iterator
    {
        EvictOverflow();

        key_t key(a_handle, a_nodeName);

        iterator it = m_data.find(key);

        if (it != m_data.end()) {
            m_totalSize -= it->second.m_size;
            it->second.m_data = std::forward<T>(a_data);
            it->second.m_lastAccess = PerfCounter::Query();
        }
        else {
            it = m_data.try_emplace(std::move(key), std::forward<T>(a_data)).first;
        }

        it->second.m_size = it->second.m_data.GetSize();

        m_totalSize += it->second.m_size;

        return it;
    }

    void BoneCastCache::UpdateSize(CacheEntry& a_in)
    {
        m_totalSize -= a_in.m_size;
        a_in.m_data.UpdateSize();
        a_in.m_size = a_in.m_data.GetSize();
        m_totalSize += a_in.m_size;
    }

    bool BoneCastCache::Remove(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName)
    {
        auto it = m_data.find(key_t(a_handle, a_nodeName));
        if (it == m_data.end())
            return false;

        Remove(it);

        return true;
    }

    template <class T, BoneCastCache::is_iterator_type<T>>
    void BoneCastCache::Remove(
        const T& a_it)
    {
        size_t size = a_it->second.m_size;
        m_data.erase(a_it);

        m_totalSize -= size;
    }

    void BoneCastCache::EvictOverflow()
    {
        while (m_data.size() > 1 && m_totalSize > m_maxSize)
        {
            auto it = std::min_element(m_data.begin(), m_data.end(),
                [](auto& a, auto& b) {
                    return a.second.m_lastAccess < b.second.m_lastAccess;
                });

            Remove(it);
        }
    }

    template <class T, BoneCastCache::is_iterator_type<T>>
    bool BoneCastCache::Get(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        bool a_read,
        T& a_result)
    {
        auto it = m_data.find(key_t(a_handle, a_nodeName));
        if (it != m_data.end())
        {
            it->second.m_lastAccess = PerfCounter::Query();
            a_result = std::move(it);
            return true;
        }

        if (!a_read)
            return false;

        data_t tmp;
        //{
            //IScopedCriticalSection _(m_iio.GetLock());
        if (!m_iio.Read(a_handle, a_nodeName, tmp))
            return false;
        //}

        tmp.UpdateSize();

        a_result = Add(a_handle, a_nodeName, std::move(tmp));

        return true;
    }

    bool IBoneCast::Get(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        bool a_read,
        BoneCastCache::iterator& a_result)
    {
        return GetCache().Get(a_handle, a_nodeName, a_read, a_result);
    }

    bool IBoneCast::Get(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        bool a_read,
        BoneCastCache::const_iterator& a_result)
    {
        return GetCache().Get(a_handle, a_nodeName, a_read, a_result);
    }

    bool IBoneCast::Get(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        const configNode_t& a_nodeConfig,
        BoneResult& a_out)
    {
        auto& cache = GetCache();

        BoneCastCache::iterator result;

        if (!cache.Get(a_handle, a_nodeName, true, result))
            return false;

        bool evict(false);

        if (result->second.m_data.second != a_nodeConfig) {

            bool res = UpdateGeometry(
                result->second.m_data,
                a_nodeConfig.fp.f32.bcWeightThreshold,
                a_nodeConfig.fp.f32.bcSimplifyTarget,
                a_nodeConfig.fp.f32.bcSimplifyTargetError);

            cache.UpdateSize(result->second);

            if (!res)
                return false;

            evict = true;

            result->second.m_updateID.Update(); // not necessary atm
            result->second.m_data.second = a_nodeConfig;
        }

        if (result->second.m_data.second.m_indices.empty())
            return false;

        a_out.data = std::make_unique<ColliderData>(result->second.m_data);
        a_out.updateID = result->second.m_updateID;

        //_DMESSAGE("cache usage: %zu", cache.GetSize());

        if (evict)
            cache.EvictOverflow();

        return true;

    }

    bool IBoneCast::Update(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        const configNode_t& a_nodeConfig)
    {
        auto actor = a_handle.Resolve<Actor>();
        if (!actor)
            return false;

        return Update(a_handle, actor, a_nodeName, a_nodeConfig);
    }

    bool IBoneCast::Update(
        Game::ObjectHandle a_handle,
        Actor* a_actor,
        const std::string& a_nodeName,
        const configNode_t& a_nodeConfig)
    {
        BoneCastCache::data_t cacheEntry;

        if (!IBoneCast::GetGeometry(
            a_actor,
            a_nodeName,
            a_nodeConfig.ex.bcShape,
            cacheEntry.first))
        {
            return false;
        }

        cacheEntry.UpdateSize();

        auto& cache = GetCache();
        auto r = cache.Add(a_handle, a_nodeName, std::move(cacheEntry));

        /*auto& wq = DCBP::GetWriterQueue();
        wq.Create<IBoneCastIO::TaskObjectWriter>(m_Instance.m_iio, a_handle, a_nodeName, r->second.m_data);*/

        if (!m_Instance.m_iio.Write(a_handle, a_nodeName, r->second.m_data))
        {
            m_Instance.m_iio.Error("[%X] write failed [%s]: %s",
                a_handle, a_nodeName.c_str(), m_Instance.m_iio.GetLastException().what());
        }

        //_DMESSAGE("cache usage: %zu", cache.GetSize());

        return true;
    }

    bool IBoneCastIO::Read(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        BoneCastCache::data_t& a_out)
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            auto path = driverConf.paths.boneCastData / MakeKey(a_handle, a_nodeName);

            std::ifstream ifs;

            ifs.open(path, std::ifstream::in | std::ifstream::binary);
            if (!ifs.is_open())
                throw std::system_error(errno, std::system_category(), path.string());

            using namespace boost::iostreams;
            using namespace boost::archive;

            filtering_streambuf<input> in;
            in.push(gzip_decompressor(zlib::default_window_bits, 1024 * 512));
            in.push(ifs);

            binary_iarchive ia(in);

            ia >> a_out;

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            return false;
        }
    }

    bool IBoneCastIO::Write(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        const BoneCastCache::data_t& a_in)
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            auto path = driverConf.paths.boneCastData / MakeKey(a_handle, a_nodeName);

            Serialization::CreateRootPath(path);

            auto tmpPath(path);
            tmpPath += ".tmp";

            try
            {
                {
                    std::ofstream ofs;
                    ofs.open(tmpPath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

                    if (!ofs.is_open())
                        throw std::system_error(errno, std::system_category(), tmpPath.string());

                    using namespace boost::iostreams;
                    using namespace boost::archive;

                    filtering_streambuf<output> out;
                    out.push(gzip_compressor(gzip_params(zlib::best_speed), 1024 * 512));
                    out.push(ofs);

                    binary_oarchive oa(out);

                    oa << a_in;
                }

                fs::rename(tmpPath, path);
            }
            catch (const std::exception& e)
            {
                Serialization::SafeCleanup(tmpPath);
                throw e;
            }

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            return false;
        }
    }

    const pluginInfo_t* IBoneCastIO::GetPluginInfo(
        Game::FormID a_formid)
    {
        if (DData::HasPluginList())
        {
            UInt32 modID;
            if (a_formid.GetPluginPartialIndex(modID))
            {
                auto& modData = DData::GetPluginData();
                return modData.Lookup(modID);
            }
        }

        return nullptr;
    }

    std::string IBoneCastIO::MakeKey(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName)
    {
        auto formID = a_handle.GetFormID();

        std::ostringstream ss;

        auto modInfo = GetPluginInfo(formID);
        if (modInfo) {
            std::string mn(modInfo->name);
            std::transform(mn.begin(), mn.end(), mn.begin(), ::tolower);
            ss << modInfo->GetFormIDLower(formID) << "." << mn;
        }
        else {
            ss << formID;
        }

        ss << "." << a_nodeName;

        return Crypto::SHA1(ss.str());
    }

    /*void IBoneCastIO::TaskObjectWriter::Run()
    {
        bool res;
        {
            IScopedCriticalSection _(m_parent.GetLock());
            res = m_parent.Write(m_handle, m_nodeName, m_data);
        }

        if (!res) {
            m_parent.Error("[%X] write failed [%s]: %s", m_handle, m_nodeName.c_str(), m_parent.m_lastException.what());
        }
    }*/

}