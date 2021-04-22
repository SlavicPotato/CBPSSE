#include "pch.h"

#include "BoneCast.h"

#include "Armor.h"
#include "Config.h"
#include "Common/BulletExtensions.h"
#include "Common/Crypto.h"

#include "Drivers/cbp.h"
#include "Drivers/data.h"

#include "Common/Game.h"
#include "Data/PluginInfo.h"

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
        NiAVObject* a_node,
        ColliderDataStorage& a_out)
    {
        static_assert(sizeof(btVector3) == sizeof(DirectX::XMVECTOR));

        auto geometry = a_node->GetAsBSGeometry();
        if (!geometry)
            return false;

        BSTriShape* triShape(nullptr);
        BSDynamicTriShape* dynamicTriShape(nullptr);

        UInt32 numVertices;
        UInt32 vertexSize;

        std::unique_ptr<SimpleLocker> dynamicTriShapeLocker;

        if (geometry->shapeType == BSGeometry::Type::kTriShape)
        {
            triShape = ni_cast(geometry, BSTriShape);
            if (!triShape)
                return false;

            auto vflags = NiSkinPartition::GetVertexFlags(geometry->vertexDesc);

            if ((vflags & VertexFlags::VF_VERTEX) != VertexFlags::VF_VERTEX)
                return false;

            numVertices = triShape->numVertices;
        }
        else if (geometry->shapeType == BSGeometry::Type::kDynamicTriShape)
        {
            dynamicTriShape = ni_cast(geometry, BSDynamicTriShape);
            if (!dynamicTriShape)
                return false;

            dynamicTriShapeLocker = std::make_unique<SimpleLocker>(std::addressof(dynamicTriShape->lock));

            numVertices = 0;
        }
        else {
            return false;
        }

        if (geometry->m_spSkinInstance == nullptr)
            return false;

        auto& skinInstance = geometry->m_spSkinInstance;

        IScopedCriticalSectionEx _(std::addressof(skinInstance->lock));

        auto& skinPartition = skinInstance->m_spSkinPartition;
        if (skinPartition == nullptr)
            return false;

        auto& skinData = skinInstance->m_spSkinData;
        if (skinData == nullptr)
            return false;

        auto numPartitions = skinPartition->m_uiPartitions;

        if (!numPartitions)
            return false;

        if (numVertices == 0) {
            numVertices = skinPartition->vertexCount;

            if (numVertices == 0)
                return false;
        }

        auto partition = skinPartition->m_pkPartitions;

        if (triShape) {
            vertexSize = NiSkinPartition::GetVertexSize(partition->vertexDesc);
        }

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

            UInt32 numTriangles = 0;

            for (decltype(numPartitions) i = 0; i < numPartitions; i++) {
                numTriangles += skinPartition->m_pkPartitions[i].m_usTriangles;
            }

            if (numTriangles == 0)
                return false;

            Bullet::btTransformEx tnsf(boneData.m_kSkinToBone);

            auto tris = std::make_unique<Triangle[]>(numTriangles);
            auto vertmap = std::make_unique<Vertex[]>(numVertices);

            for (decltype(numPartitions) i = 0, itri = 0; i < numPartitions; i++)
            {
                auto& part = skinPartition->m_pkPartitions[i];

                auto trilist = part.m_pusTriList;
                auto numTris = part.m_usTriangles;

                for (decltype(numTris) j = 0; j < numTris; j++)
                {
                    auto tri = std::addressof(tris[itri]);

                    tri->m_isBoneTri = false;

                    std::uint32_t k = j * 3;

                    auto i1 = tri->m_indices[0] = trilist[k];
                    auto i2 = tri->m_indices[1] = trilist[k + 1];
                    auto i3 = tri->m_indices[2] = trilist[k + 2];

                    if (i1 >= numVertices || i2 >= numVertices || i3 >= numVertices) {
                        m_Instance.Warning("%s: [m_pusTriList] index >= numVertices", a_nodeName.c_str());
                        return false;
                    }

                    vertmap[i1].m_triangles.emplace_back(tri);
                    vertmap[i2].m_triangles.emplace_back(tri);
                    vertmap[i3].m_triangles.emplace_back(tri);

                    itri++;
                }
            }

            for (UInt16 i = 0; i < boneData.m_usVerts; i++)
            {
                auto& v = boneData.m_pkBoneVertData[i];

                if (v.m_usVert >= numVertices) {
                    m_Instance.Warning("%s: [m_pkBoneVertData] index >= numVertices", a_nodeName.c_str());
                    return false;
                }

                auto& d = vertmap[v.m_usVert];

                d.m_weight = v.m_fWeight;

                for (auto& e : d.m_triangles)
                    e->m_isBoneTri = true;

            }

            std::size_t c = 0;
            std::uint32_t vi = 0;

            for (decltype(numTriangles) i = 0; i < numTriangles; i++)
            {
                auto& tri = tris[i];

                if (tri.m_isBoneTri) {

                    for (int j = 0; j < 3; j++)
                    {
                        auto index = tri.m_indices[j];

                        auto& vme = vertmap[index];

                        if (!vme.m_hasVertex)
                        {
                            btVector3 vtx;

                            if (dynamicTriShape) {
                                vtx = reinterpret_cast<DirectX::XMVECTOR*>(dynamicTriShape->pDynamicData)[index];
                            }
                            else {
                                vtx = *reinterpret_cast<DirectX::XMVECTOR*>(&partition->shapeData->m_RawVertexData[index * vertexSize]);
                            }

                            vme.m_vertex.v = tnsf * vtx;
                            vme.m_hasVertex = true;
                            vme.m_index = vi;

                            tri.m_indices[j] = vi;

                            vi++;
                        }
                        else {
                            tri.m_indices[j] = vme.m_index;
                        }

                        /*if (vme.m_weight < 0.0f)
                            vme.m_weight = 0.0f;*/
                    }

                    c++;
                }
            }

            if (c == 0 || vi == 0)
                return false;

            auto rnIndices = c * 3;
            auto rnVertices = static_cast<std::size_t>(vi);

            a_out.m_vertices = std::make_unique_for_overwrite<MeshPoint[]>(rnVertices);
            a_out.m_weights.resize(rnVertices);
            a_out.m_indices.resize(rnIndices);

            for (decltype(numVertices) i = 0; i < numVertices; i++) {

                auto& vme = vertmap[i];

                if (vme.m_hasVertex)
                {
                    auto index = vme.m_index;

                    ASSERT(index < rnVertices);

                    a_out.m_vertices[index].v = vme.m_vertex.v;
                    a_out.m_weights[index] = vme.m_weight;
                }
            }

            a_out.m_numVertices = static_cast<unsigned int>(rnVertices);

            for (decltype(numTriangles) i = 0, c = 0; i < numTriangles; i++)
            {
                auto& tri = tris[i];

                if (tri.m_isBoneTri) {

                    for (int j = 0; j < 3; j++) 
                    {
                        auto index = tri.m_indices[j];

                        ASSERT(c < rnIndices);

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

    void IBoneCast::RemoveDuplicateVertices(
        const MeshPoint* a_vertices,
        Eigen::Index a_numVertices,
        const unsigned int* a_indices,
        Eigen::Index a_numFaces,
        Eigen::MatrixXf& a_verticesOut,
        Eigen::MatrixXi& a_indicesOut)
    {
        Eigen::MatrixXf OV(a_numVertices, 3);
        Eigen::MatrixXi OF(a_numFaces, 3);

        for (decltype(a_numVertices) i = 0; i < a_numVertices; i++)
        {
            auto& v = a_vertices[i];

            OV.row(i) = Eigen::Vector3f(v.v.x(), v.v.y(), v.v.z());
        }

        for (decltype(a_numFaces) i = 0; i < a_numFaces; i++)
        {
            decltype(i) j = i * 3;

            OF.row(i) = Eigen::Vector3i(a_indices[j], a_indices[j + 1], a_indices[j + 2]);
        }

        Eigen::MatrixXi SVI, SVJ;

        igl::remove_duplicate_vertices(OV, OF, 1e-5, a_verticesOut, SVI, SVJ, a_indicesOut);
    }

    void IBoneCast::RemoveUnreferencedVertices(
        const MeshPoint* a_vertices,
        unsigned int a_numVertices,
        const unsigned int* a_indices,
        std::size_t a_numIndices,
        std::shared_ptr<MeshPoint[]>& a_verticesOut,
        unsigned int* a_indicesOut,
        unsigned int& a_newVertexCount
    )
    {
        struct iv_t
        {
            bool set;
            unsigned int first;
            unsigned int second;
        };

        auto im = std::make_unique<iv_t[]>(a_numVertices);

        unsigned int numVertices(0);

        for (decltype(a_numIndices) i = 0; i < a_numIndices; i++)
        {
            auto index = a_indices[i];

            auto& e = im[index];

            if (!e.set)
            {
                e.set = true;
                e.first = numVertices;
                e.second = index;

                a_indicesOut[i] = numVertices;

                numVertices++;
            }
            else
            {
                a_indicesOut[i] = e.first;
            }
        }

        a_verticesOut = std::make_unique_for_overwrite<MeshPoint[]>(numVertices);

        for (decltype(a_numVertices) i = 0; i < a_numVertices; i++)
        {
            auto& e = im[i];
            if (e.set) {
                a_verticesOut[e.first] = a_vertices[e.second];
            }
        }

        a_newVertexCount = numVertices;

    }


    bool IBoneCast::CreateColliderData(
        const ColliderDataStorage& a_cds,
        const unsigned int* a_indices,
        std::size_t a_numIndices,
        ColliderData* a_out,
        bool& a_verticesShared,
        bool a_removeVertices)
    {
        std::shared_ptr<MeshPoint[]> rVertices;
        std::unique_ptr<unsigned int[]> rIndices;
        unsigned int numVertices;

        if (a_removeVertices)
        {
            rIndices = std::make_unique_for_overwrite<unsigned int[]>(a_numIndices);

            RemoveUnreferencedVertices(
                a_cds.m_vertices.get(),
                a_cds.m_numVertices,
                a_indices,
                a_numIndices,
                rVertices,
                rIndices.get(),
                numVertices);

            a_indices = rIndices.get();

            a_verticesShared = false;
        }
        else
        {
            rVertices = a_cds.m_vertices;
            numVertices = a_cds.m_numVertices;
        }

        Eigen::MatrixXf V;
        Eigen::MatrixXi F;

        RemoveDuplicateVertices(
            rVertices.get(),
            static_cast<Eigen::Index>(numVertices),
            a_indices,
            static_cast<Eigen::Index>(a_numIndices / 3),
            V,
            F);

        auto newVertices = V.rows();
        auto newIndices = F.size();

        if (newVertices == 0 || newIndices < 3 || newIndices % 3 != 0) {
            return false;
        }

        if (newVertices == numVertices &&
            newIndices == a_numIndices)
        {
            a_out->m_indices = std::make_unique_for_overwrite<int[]>(a_numIndices);
            a_out->m_hullPoints = std::make_unique_for_overwrite<MeshPoint[]>(a_numIndices);
            a_out->m_vertices = rVertices;

            for (decltype(a_numIndices) i = 0; i < a_numIndices; i++)
            {
                auto index = a_indices[i];

                a_out->m_indices[i] = static_cast<int>(index);
                a_out->m_hullPoints[i] = rVertices[index];
            }

            a_out->m_numTriangles = static_cast<int>(a_numIndices / 3);
            a_out->m_numIndices = static_cast<int>(a_numIndices);
            a_out->m_numVertices = static_cast<int>(numVertices);
        }
        else
        {

            a_out->m_indices = std::make_unique_for_overwrite<int[]>(newIndices);
            a_out->m_hullPoints = std::make_unique_for_overwrite<MeshPoint[]>(newIndices);
            a_out->m_vertices = std::make_unique_for_overwrite<MeshPoint[]>(newVertices);

            auto numRows = V.rows();

            for (decltype(numRows) i = 0; i < numRows; i++)
            {
                auto row = V.row(i);
                a_out->m_vertices[i] = MeshPoint(row.x(), row.y(), row.z());
            }

            numRows = F.rows();

            for (decltype(numRows) i = 0; i < numRows; i++)
            {
                const auto row = F.row(i);

                auto ia = row.x();
                auto ib = row.y();
                auto ic = row.z();

                decltype(i) j = i * 3;

                a_out->m_indices[j] = ia;
                a_out->m_indices[j + 1] = ib;
                a_out->m_indices[j + 2] = ic;

                a_out->m_hullPoints[j] = a_out->m_vertices[ia];
                a_out->m_hullPoints[j + 1] = a_out->m_vertices[ib];
                a_out->m_hullPoints[j + 2] = a_out->m_vertices[ic];
            }

            a_out->m_numTriangles = static_cast<int>(newIndices / 3);
            a_out->m_numIndices = static_cast<int>(newIndices);
            a_out->m_numVertices = static_cast<int>(newVertices);

            a_verticesShared = false;

        }

        a_out->GenerateTriVertexArray();

        return true;
    }

    const auto& IBoneCast::FilterIndicesByWeight(
        ColliderDataStoragePair& a_in,
        decltype(ColliderDataStorage::m_indices)& a_buffer,
        float a_weightThreshold
    )
    {
        if (a_weightThreshold <= 0.0f) {
            return a_in.first.m_indices;
        }

        auto numIndices = a_in.first.m_indices.size();

        a_buffer.reserve(numIndices);

        for (decltype(numIndices) i = 0; i < numIndices; i += 3)
        {
            decltype(i) m = i + 3;

            for (decltype(i) j = i; j < m; j++)
            {
                auto index = a_in.first.m_indices[j];

                if (a_in.first.m_weights[index] < a_weightThreshold) {
                    goto _continue;
                }
            }

            for (decltype(i) j = i; j < m; j++) {
                a_buffer.emplace_back(a_in.first.m_indices[j]);
            }

        _continue:;
        }

        return a_buffer;
    }

    bool IBoneCast::UpdateGeometry(
        ColliderDataStoragePair& a_in,
        float a_weightThreshold,
        float a_simplifyTarget,
        float a_simplifyTargetError)
    {
        if (!a_in.first.m_numVertices) {
            return false;
        }

        auto numIndices = a_in.first.m_indices.size();
        if (numIndices < 3) {
            return false;
        }

        auto startingIndices = numIndices;

        auto ibuffer = decltype(ColliderDataStorage::m_indices)();

        auto& indices = FilterIndicesByWeight(a_in, ibuffer, a_weightThreshold);

        numIndices = indices.size();
        if (numIndices < 3) {
            return false;
        }

        bool verticesShared(true);

        auto data = std::make_unique<ColliderData>();

        auto targetIndices = static_cast<std::size_t>(static_cast<long double>(numIndices) * long double(a_simplifyTarget));

        bool result;

        if (targetIndices < numIndices)
        {
            targetIndices = std::clamp<std::size_t>(targetIndices - (targetIndices % 3), 3, numIndices);

            auto tmp = std::make_unique_for_overwrite<unsigned int[]>(numIndices);

            numIndices = ::meshopt_simplify(
                tmp.get(),
                indices.data(),
                numIndices,
                *a_in.first.m_vertices.get(),
                a_in.first.m_numVertices,
                sizeof(decltype(a_in.first.m_vertices)::element_type::v),
                targetIndices,
                a_simplifyTargetError);

            if (numIndices % 3 != 0) {
                return false;
            }

            result = CreateColliderData(a_in.first, tmp.get(), numIndices, data.get(), verticesShared, numIndices != startingIndices);
        }
        else
        {
            result = CreateColliderData(a_in.first, indices.data(), numIndices, data.get(), verticesShared, numIndices != startingIndices);
        }

        if (!result) {
            return false;
        }

        a_in.second = std::move(data);
        a_in.SetVerticesShared(verticesShared);

        return true;

    }

    bool IBoneCast::GetGeometry(
        Actor* a_actor,
        const std::string& a_nodeName,
        const std::string& a_shape,
        ColliderDataStorage& a_result)
    {
        if (a_nodeName.empty())
            return false;

        BSFixedString nodeName(a_nodeName.c_str());

        //_DMESSAGE(">>> %s", a_shape.c_str());

        bool found(false);

        BSFixedString shape(a_shape.c_str());

        found = IArmor::VisitEquippedNodes(a_actor,
            [&](TESObjectARMO*, TESObjectARMA*, NiAVObject* a_object, bool a_firstPerson)
            {
                //_DMESSAGE("%s: equip (%hhu): %s", a_nodeName.c_str(), a_firstPerson, a_object->m_name);

                /*if (a_firstPerson)
                    return false;*/


                if (!a_shape.empty())
                {
                    if (a_object->m_name != shape.data)
                        return false;
                }

                return ExtractGeometry(a_actor, nodeName, a_object, a_result);
            });

        if (!found)
        {
            auto skin = Game::GetActorSkin(a_actor);
            if (skin)
            {
                found = IArmor::VisitArmor(a_actor, skin, false,
                    [&](TESObjectARMO*, TESObjectARMA*, NiAVObject* a_object, bool a_firstPerson)
                    {
                        //_DMESSAGE("%s: skin (%hhu): %s", a_object->m_name, a_firstPerson, a_object->m_name);

                        return ExtractGeometry(a_actor, nodeName, a_object, a_result);
                    });
            }
        }

        if (!found)
        {
            auto face = a_actor->GetFaceGenNiNode();
            if (face)
            {
                auto animationData = a_actor->GetFaceGenAnimationData();
                if (animationData)
                {
                    FaceGen::GetSingleton()->isReset = 0;

                    for (int t = BSFaceGenAnimationData::kKeyframeType_Expression2; t <= BSFaceGenAnimationData::kKeyframeType_Phoneme2; t++)
                    {
                        auto& keyframe = animationData->keyFrames[t];
                        for (decltype(keyframe.count) i = 0; i < keyframe.count; i++) {
                            keyframe.values[i] = 0.0f;
                        }

                        keyframe.isUpdated = 0;
                    }

                    UpdateModelFace(face);
                }

                found = Game::Node::Traverse2(face, [&](NiAVObject* a_object)
                    {
                        if (!a_shape.empty())
                        {
                            if (a_object->m_name != shape.data)
                                return false;
                        }

                        return ExtractGeometry(a_actor, nodeName, a_object, a_result);
                    });
            }
        }

        return found;
    }

    BoneCastCache::BoneCastCache(
        IBoneCastIO& a_iio,
        std::size_t a_maxSize)
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

        auto key = std::make_pair(a_handle, a_nodeName);

        iterator it = m_data.find(key);

        if (it != m_data.end()) {
            m_totalSize -= it->second.m_size;
            it->second.m_data = std::forward<T>(a_data);
            it->second.m_lastAccess = IPerfCounter::Query();
        }
        else {
            it = m_data.try_emplace(std::move(key), std::forward<T>(a_data)).first;
        }

        it->second.m_size = it->second.m_data.UpdateSize();

        m_totalSize += it->second.m_size;

        return it;
    }

    void BoneCastCache::UpdateSize(CacheEntry& a_in)
    {
        m_totalSize -= a_in.m_size;
        a_in.m_size = a_in.m_data.UpdateSize();
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
        std::size_t size = a_it->second.m_size;
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
        auto it = m_data.find(std::make_pair(a_handle, a_nodeName));
        if (it != m_data.end())
        {
            it->second.m_lastAccess = IPerfCounter::Query();
            a_result = std::move(it);
            return true;
        }

        if (!a_read)
            return false;

        ColliderDataStoragePair cacheEntry;
        if (!m_iio.Read(a_handle, a_nodeName, cacheEntry))
            return false;

        a_result = Add(a_handle, a_nodeName, std::move(cacheEntry));

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

        return ProcessResult(result, a_nodeConfig, a_out);
    }

    bool IBoneCast::ProcessResult(
        const BoneCastCache::iterator& a_result,
        const configNode_t& a_nodeConfig,
        BoneResult& a_out)
    {
        auto& cache = GetCache();

        bool updated(false);

        if (a_result->second.m_data != a_nodeConfig) {

            bool res = UpdateGeometry(
                a_result->second.m_data,
                a_nodeConfig.fp.f32.bcWeightThreshold,
                a_nodeConfig.fp.f32.bcSimplifyTarget,
                a_nodeConfig.fp.f32.bcSimplifyTargetError);

            if (!res) {
                a_result->second.m_data.second =
                    std::make_unique<ColliderData>();
            }

            cache.UpdateSize(a_result->second);

            a_result->second.m_updateID.Update(); // not necessary atm
            a_result->second.m_data = a_nodeConfig;

            if (!res) {
                return false;
            }

            updated = true;
        }

        if (!a_result->second.m_data.second->m_numIndices) {
            return false;
        }

        a_out.data = a_result->second.m_data.second;
        a_out.updateID = a_result->second.m_updateID;

        //_DMESSAGE("cache usage: %zu", cache.GetSize());

        if (updated)
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
        ColliderDataStoragePair cacheEntry;

        if (!IBoneCast::GetGeometry(
            a_actor,
            a_nodeName,
            a_nodeConfig.ex.bcShape,
            cacheEntry.first))
        {
            return false;
        }

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
        ColliderDataStoragePair& a_out)
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            std::string key;
            MakeKey(a_handle, a_nodeName, key);

            auto path = driverConf.paths.boneCastData / key;

            std::ifstream ifs;

            ifs.open(path, std::ifstream::in | std::ifstream::binary);
            if (!ifs.is_open())
                throw std::system_error(errno, std::system_category(), path.string());

            using namespace boost::iostreams;
            using namespace boost::archive;

            filtering_streambuf<input> in;
            in.push(gzip_decompressor(zlib::default_window_bits, 1024 * 256));
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
        const ColliderDataStoragePair& a_in)
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            std::string key;
            MakeKey(a_handle, a_nodeName, key);

            auto path = driverConf.paths.boneCastData / key;

            Serialization::CreateRootPath(path);

            auto tmpPath(path);
            tmpPath += ".tmp";

            try
            {
                {
                    std::ofstream ofs;

                    ofs.open(
                        tmpPath,
                        std::ofstream::out |
                        std::ofstream::binary |
                        std::ofstream::trunc,
                        _SH_DENYWR);

                    if (!ofs.is_open())
                        throw std::system_error(errno, std::system_category(), tmpPath.string());

                    using namespace boost::iostreams;
                    using namespace boost::archive;

                    filtering_streambuf<output> out;
                    out.push(gzip_compressor(gzip_params(zlib::best_speed), 1024 * 256));
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
        Game::FormID a_formid) const
    {
        if (DData::HasPluginList())
        {
            UInt32 modID;
            if (a_formid.GetPluginPartialIndex(modID))
            {
                return DData::GetPluginData().Lookup(modID);
            }
        }

        return nullptr;
    }

    void IBoneCastIO::MakeKey(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        std::string& a_out) const
    {
        auto formID = a_handle.GetFormID();

        std::string s;
        s.reserve(128);

        auto modInfo = GetPluginInfo(formID);
        if (modInfo)
        {
            std::string n(modInfo->name);
            std::transform(n.begin(), n.end(), n.begin(), ::tolower);

            s.append(std::to_string(modInfo->GetFormIDLower(formID)));
            s.append(".");
            s.append(n);

        }
        else {
            s.append(std::to_string(formID));
        }

        s.append(".");
        s.append(a_nodeName);

        Crypto::SHA1(s, a_out);
    }

}