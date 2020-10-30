#pragma once

namespace CBP
{
    class SimComponent;
    class Collider;

#ifdef _CBP_ENABLE_DEBUG
    struct SimDebugInfo
    {
        NiTransform worldTransform;
        NiTransform localTransform;

        NiTransform worldTransformParent;
        NiTransform localTransformParent;

        std::string parentNodeName;
    };
#endif

    struct MeshPoint
    {
        btScalar x;
        btScalar y;
        btScalar z;
    };

    struct ColliderData
    {
        std::shared_ptr<MeshPoint[]> m_vertices;
        std::shared_ptr<MeshPoint[]> m_hullPoints;
        std::shared_ptr<int[]> m_indices;

        int numVertices;
        int numTriangles;
        int numIndices;

        btTriangleIndexVertexArray *m_triVertexArray;
    };

    class CollisionShape
    {
    public:
        virtual void UpdateShape() = 0;

        virtual void SetRadius(float a_radius);
        virtual void SetHeight(float a_height);
        virtual void SetExtent(const btVector3& a_extent);
        virtual void SetNodeScale(float a_scale);

        virtual btCollisionShape* GetBTShape() = 0;

        virtual ~CollisionShape() noexcept = default;

    protected:

        CollisionShape(float a_nodeScale);

        float m_nodeScale;
    };

    template <class T>
    class CollisionShapeBase :
        public CollisionShape
    {

    public:

        [[nodiscard]] virtual btCollisionShape* GetBTShape();

    protected:

        virtual ~CollisionShapeBase() noexcept;

        template <typename... Args>
        CollisionShapeBase(btCollisionObject* a_collider, Args&&... a_args);

        template <typename... Args>
        __forceinline void RecreateShape(Args&&... a_args);
        __forceinline void PostUpdateShape();

        union
        {
            T* m_shape;
            btCollisionShape* m_baseShape;
        };

        btCollisionObject* m_collider;
    };

    template <class T>
    class CollisionShapeTemplRH :
        public CollisionShapeBase<T>
    {
    protected:

        template <typename... Args>
        CollisionShapeTemplRH(btCollisionObject* a_collider, Args&&... a_args);

    public:

        virtual void DoRecreateShape(float a_radius, float a_height) = 0;

        virtual void UpdateShape();
        virtual void SetRadius(float a_radius);
        virtual void SetHeight(float a_height);

    protected:

        float m_radius;
        float m_height;

        float m_currentRadius;
        float m_currentHeight;
    };

    template <class T>
    class __declspec(align(16)) CollisionShapeTemplExtent :
        public CollisionShapeBase<T>
    {
    public:

        template <typename... Args>
        CollisionShapeTemplExtent(btCollisionObject* a_collider, Args&&... a_args);

        virtual void DoRecreateShape(const btVector3& a_extent) = 0;
        virtual void SetShapeProperties(const btVector3& a_extent);
        virtual void UpdateShape();
        virtual void SetExtent(const btVector3& a_extent);

    protected:
        btVector3 m_extent;
        btVector3 m_currentExtent;
    };

    class CollisionShapeSphere :
        public CollisionShapeBase<btSphereShape>
    {
    public:

        CollisionShapeSphere(btCollisionObject* a_collider, float a_radius);

        virtual void UpdateShape();
        virtual void SetRadius(float a_radius);

    private:
        float m_radius;
        float m_currentRadius;
    };


    class CollisionShapeCapsule :
        public CollisionShapeTemplRH<btCapsuleShape>
    {
    public:
        CollisionShapeCapsule(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class CollisionShapeCone :
        public CollisionShapeTemplRH<btConeShape>
    {
    public:
        CollisionShapeCone(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class CollisionShapeBox :
        public CollisionShapeTemplExtent<btBoxShape>
    {
    public:
        CollisionShapeBox(btCollisionObject* a_collider, const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
    };

    class CollisionShapeCylinder :
        public CollisionShapeTemplRH<btCylinderShape>
    {
    public:
        CollisionShapeCylinder(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class CollisionShapeTetrahedron :
        public CollisionShapeTemplExtent<btTetrahedronShapeEx>
    {
    public:
        CollisionShapeTetrahedron(btCollisionObject* a_collider, const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
        virtual void SetShapeProperties(const btVector3& a_extent);

    private:
        static const btVector3 m_vertices[4];
    };

    class CollisionShapeMesh :
        public CollisionShapeTemplExtent<btGImpactMeshShape>
    {
    public:
        CollisionShapeMesh(
            btCollisionObject* a_collider,
            btTriangleIndexVertexArray* a_data,
            const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
        virtual void SetShapeProperties(const btVector3& a_extent);

    private:

        btTriangleIndexVertexArray *m_triVertexArray;
    };

    class CollisionShapeConvexHull :
        public CollisionShapeTemplExtent<btConvexHullShape>
    {
    public:

        CollisionShapeConvexHull(
            btCollisionObject* a_collider,
            const std::shared_ptr<MeshPoint[]>& a_data,
            int a_numVertices,
            const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
        virtual void SetShapeProperties(const btVector3& a_extent);

    private:

        std::shared_ptr<MeshPoint[]> m_convexHullPoints;
        int m_convexHullNumVertices;
    };

    __declspec(align(16)) class Collider :
        ILog
    {
        static constexpr float crdrmul = std::numbers::pi_v<float> / 180.0f;

    public:
        Collider(SimComponent& a_parent);
        virtual ~Collider() noexcept;

        Collider() = delete;
        Collider(const Collider& a_rhs) = delete;
        Collider(Collider&& a_rhs) = delete;

        bool Create(ColliderShapeType a_shape);
        bool Destroy();
        __forceinline void Update();

        inline void SetColliderRotation(float a_x, float a_y, float a_z)
        {
            m_colRot.setEulerZYX(
                a_x * crdrmul,
                a_y * crdrmul,
                a_z * crdrmul
            );
        }

        inline void SetRadius(float a_val) {
            if (m_created)
                m_colshape->SetRadius(a_val);
        }

        inline void SetHeight(float a_val) {
            if (m_created)
                m_colshape->SetHeight(a_val);
        }

        inline void SetExtent(const btVector3& a_extent)
        {
            if (m_created)
                m_colshape->SetExtent(a_extent);
        }

        inline void SetOffset(const NiPoint3& a_offset) {
            m_bodyOffset = a_offset;
        }

        inline void SetOffset(float a_x, float a_y, float a_z) {
            m_bodyOffset.x = a_x;
            m_bodyOffset.y = a_y;
            m_bodyOffset.z = a_z;
        }

        [[nodiscard]] inline bool IsActive() const {
            return m_created && m_active;
        }

        [[nodiscard]] inline const auto& GetSphereOffset() const {
            return m_bodyOffset;
        }

        inline void SetMotionScale(float a_scale) {
            m_motionScale = a_scale;
        }

        void SetShouldProcess(bool a_switch);

    private:

        void Activate();
        void Deactivate();

        btCollisionObject* m_collider;
        CollisionShape* m_colshape;

        btMatrix3x3 m_colRot;
        NiPoint3 m_bodyOffset;

        ColliderShapeType m_shape;
        std::string m_meshShape;

        float m_nodeScale;

        bool m_created;
        bool m_active;
        bool m_colliderActivated;
        bool m_process;
        bool m_rotation;

        float m_motionScale;

        PerfTimer pt;

        SimComponent& m_parent;
    };

    __declspec(align(16)) class SimComponent
    {
        struct Force
        {
            uint32_t steps;
            NiPoint3 force;
            float mag;
        };

        friend class Collider;

    private:

        bool ColUpdateWeightData(
            Actor* a_actor,
            const configComponent_t& a_config,
            const configNode_t& a_nodeConf);

        NiPoint3 m_npCogOffset;
        NiPoint3 m_npGravityCorrection;

        NiPoint3 m_oldWorldPos;
        NiPoint3 m_virtld;
        NiPoint3 m_velocity;

        NiTransform m_initialTransform;
        bool m_hasScaleOverride;

        Collider m_collider;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t m_conf;

        bool m_collisions;
        bool m_movement;

        btVector3 m_extent;
        float m_colRad = 1.0f;
        float m_colHeight = 0.001f;
        float m_colCapsuleHeight = 0.001f;
        float m_colOffsetX = 0.0f;
        float m_colOffsetY = 0.0f;
        float m_colOffsetZ = 0.0f;
        float m_nodeScale = 1.0f;
        float m_massInv = 1.0f;

        uint64_t m_groupId;
        uint64_t m_parentId;

        bool m_resistanceOn;
        bool m_rotScaleOn;

        NiPointer<NiAVObject> m_obj;
        NiPointer<NiAVObject> m_objParent;

        NiAVObject::ControllerUpdateContext m_updateCtx;

#ifdef _CBP_ENABLE_DEBUG
        SimDebugInfo m_debugInfo;
#endif

        Game::FormID m_formid;

        __forceinline void ClampVelocity()
        {
            float len = m_velocity.Length();
            if (len <= m_conf.phys.maxVelocity)
                return;

            m_velocity.x /= len;
            m_velocity.y /= len;
            m_velocity.z /= len;
            m_velocity *= m_conf.phys.maxVelocity;
        }

        __forceinline void ConstrainMotion(
            const NiMatrix33& a_invRot,
            const NiPoint3& a_target,
            float a_timeStep
        );

    public:
        SimComponent(
            Actor* a_actor,
            NiAVObject* a_obj,
            const std::string& a_configBoneName,
            const configComponent_t& config,
            const configNode_t& a_nodeConf,
            uint64_t a_parentId,
            uint64_t a_groupId,
            bool a_collisions,
            bool a_movement
        );

        virtual ~SimComponent() noexcept;

        SimComponent() = delete;
        SimComponent(const SimComponent& a_rhs) = delete;
        SimComponent(SimComponent&& a_rhs) = delete;

        void UpdateConfig(
            Actor* a_actor,
            const configComponent_t* a_physConf,
            const configNode_t& a_nodeConf,
            bool a_collisions,
            bool a_movement) noexcept;

        __forceinline void UpdateMovement(float timeStep);
        __forceinline void UpdateVelocity();
        void Reset();
        bool ValidateNodes(NiAVObject* a_obj);

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        __forceinline void SetVelocity(const btVector3& a_vel) {
            m_velocity.x = a_vel.x();
            m_velocity.y = a_vel.y();
            m_velocity.z = a_vel.z();
            ClampVelocity();
        }

        __forceinline void SetVelocity(const NiPoint3& a_vel)
        {
            m_velocity = a_vel;
            ClampVelocity();
        }

        __forceinline void SetVelocity(NiPoint3&& a_vel)
        {
            m_velocity = std::move(a_vel);
            ClampVelocity();
        }

        __forceinline void AddVelocity(const NiPoint3& a_vel) {
            m_velocity += a_vel;
        }

        [[nodiscard]] inline const auto& GetVelocity() const {
            return m_velocity;
        }

        [[nodiscard]] inline const auto& GetConfig() const {
            return m_conf;
        }

        [[nodiscard]] inline const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }

        [[nodiscard]] inline bool IsSameGroup(const SimComponent& a_rhs) const {
            return a_rhs.m_groupId != 0 && m_groupId != 0 &&
                a_rhs.m_parentId == m_parentId &&
                a_rhs.m_groupId == m_groupId;
        }

        inline void UpdateGroupInfo(uint64_t a_groupId) {
            m_groupId = a_groupId;
        };

        [[nodiscard]] inline bool HasMovement() const {
            return m_movement;
        }

        [[nodiscard]] inline bool HasActiveCollider() const {
            return m_collider.IsActive();
        }

        [[nodiscard]] inline const auto& GetPos() const {
            return m_obj->m_worldTransform.pos;
        }

        [[nodiscard]] inline const auto& GetWorldTransform() const {
            return m_obj->m_worldTransform;
        }

        [[nodiscard]] inline const auto& GetParentWorldTransform() const {
            return m_objParent->m_worldTransform;
        }

        [[nodiscard]] inline const auto& GetVirtualPos() const {
            return m_virtld;
        }

        [[nodiscard]] inline const auto& GetCenterOfMass() const {
            return m_npCogOffset;
        }

        [[nodiscard]] inline float GetNodeScale() const {
            return m_obj->m_worldTransform.scale;
        }

        [[nodiscard]] inline auto& GetCollider() {
            return m_collider;
        }

        [[nodiscard]] inline const auto& GetCollider() const {
            return m_collider;
        }

        [[nodiscard]] inline const float GetMassInverse() const {
            return m_massInv;
        }

        [[nodiscard]] inline auto GetNode() {
            return m_obj.m_pObject;
        }

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] inline const auto& GetDebugInfo() const {
            return m_debugInfo;
        }
#endif
    };

    namespace Math
    {
        __forceinline float norm(float a_val, float a_min, float a_max) {
            return (a_val - a_min) / (a_max - a_min);
        }

        __forceinline float normc(float a_val, float a_min, float a_max) {
            return std::clamp(norm(a_val, a_min, a_max), 0.0f, 1.0f);
        }

        __forceinline constexpr float sgn(float a_val) {
            return a_val < 0.0f ? -1.0f : 1.0f;
        }
    }

    void Collider::Update()
    {
        if (!m_created)
            return;

        auto nodeScale = m_parent.m_obj->m_worldTransform.scale;

        if (!m_active)
        {
            if (nodeScale > 0.0f)
            {
                m_active = true;
                if (m_process)
                    Activate();
            }
            else
                return;
        }
        else
        {
            if (nodeScale <= 0.0f)
            {
                m_active = false;
                Deactivate();
                return;
            }
        }

        auto& transform = m_collider->getWorldTransform();

        if (!m_parent.m_movement || m_motionScale == 1.0f)
        {
            auto pos = m_parent.m_obj->m_worldTransform * m_bodyOffset;
            transform.setOrigin({ pos.x, pos.y, pos.z });
        }
        else {
            auto tf = m_parent.m_obj->m_localTransform;
            tf.pos = m_parent.m_initialTransform.pos + ((tf.pos - m_parent.m_initialTransform.pos) * m_motionScale);
            auto pos = (m_parent.m_objParent->m_worldTransform * tf) * m_bodyOffset;
            transform.setOrigin({ pos.x, pos.y, pos.z });
        }

        if (m_rotation)
        {
            auto& m = m_parent.m_obj->m_worldTransform.rot;
            auto& b = transform.getBasis();

#if defined(BT_USE_SIMD_VECTOR3) && defined(BT_USE_SSE_IN_API) && defined(BT_USE_SSE)

            // kinda fucky but much faster

            static_assert(sizeof(b) == sizeof(btVector3) * 3);

            auto el = reinterpret_cast<btVector3*>(&b);

            el[0].mVec128 = _mm_and_ps(_mm_loadu_ps(m.data[0]), btvFFF0fMask);
            el[1].mVec128 = _mm_and_ps(_mm_loadu_ps(m.data[1]), btvFFF0fMask);
            el[2].mVec128 = _mm_and_ps(_mm_loadu_ps(m.data[2]), btvFFF0fMask); // should be safe since there's stuff after matrix

#else
            b.setValue(
                m.arr[0], m.arr[1], m.arr[2],
                m.arr[3], m.arr[4], m.arr[5],
                m.arr[6], m.arr[7], m.arr[8]);
#endif

            b *= m_colRot;
        }

        if (nodeScale != m_nodeScale)
        {
            m_nodeScale = nodeScale;
            m_colshape->SetNodeScale(nodeScale);
        }
    }

    void SimComponent::ConstrainMotion(
        const NiMatrix33& a_invRot,
        const NiPoint3& a_target,
        float a_timeStep
    )
    {
        NiPoint3 diff;
        NiPoint3 depth;
        bool constrain(false);

        float x(m_virtld.x);

        if (x > m_conf.phys.maxOffsetP[0])
        {
            diff.x = x;
            depth.x = x - m_conf.phys.maxOffsetP[0];
            constrain = true;
        }
        else if (x < m_conf.phys.maxOffsetN[0])
        {
            diff.x = x;
            depth.x = x - m_conf.phys.maxOffsetN[0];
            constrain = true;
        }

        float y(m_virtld.y);

        if (y > m_conf.phys.maxOffsetP[1])
        {
            diff.y = y;
            depth.y = y - m_conf.phys.maxOffsetP[1];
            constrain = true;
        }
        else if (y < m_conf.phys.maxOffsetN[1])
        {
            diff.y = y;
            depth.y = y - m_conf.phys.maxOffsetN[1];
            constrain = true;
        }

        float z(m_virtld.z);

        if (z > m_conf.phys.maxOffsetP[2])
        {
            diff.z = z;
            depth.z = z - m_conf.phys.maxOffsetP[2];
            constrain = true;
        }
        else if (z < m_conf.phys.maxOffsetN[2])
        {
            diff.z = z;
            depth.z = z - m_conf.phys.maxOffsetN[2];
            constrain = true;
        }

        if (!constrain)
            return;

        auto n = m_objParent->m_worldTransform.rot * diff;
        n.Normalize();

        auto mag = depth.Length();

        float bias = mag > 0.1f ?
            (a_timeStep * 2880.0f) * std::clamp(mag - 0.1f, 0.0f, m_conf.phys.maxOffsetMaxBiasMag) : 0.0f;

        float vdotn = m_velocity.Dot(n);
        float impulse = std::max(vdotn + bias, 0.0f);
        float J = (1.0f + m_conf.phys.maxOffsetRestitutionCoefficient) * impulse;

        m_velocity -= n * (J * m_conf.phys.maxOffsetVelResponseScale);

        m_virtld = a_invRot * ((m_oldWorldPos + m_velocity * a_timeStep) - a_target);
    }

    void SimComponent::UpdateMovement(float a_timeStep)
    {
        if (m_movement)
        {
            //m_objParent->UpdateWorldData(&m_updateCtx);

            //Offset to move Center of Mass make rotational motion more significant  
            NiPoint3 target(m_objParent->m_worldTransform * m_npCogOffset);

            NiPoint3 diff(target - m_oldWorldPos);

            float ax(std::fabs(diff.x));
            float ay(std::fabs(diff.y));
            float az(std::fabs(diff.z));

            if (ax > 256.0f || ay > 256.0f || az > 256.0f) {
                Reset();
                return;
            }

            // Compute the "Spring" Force
            NiPoint3 diff2(diff.x * ax, diff.y * ay, diff.z * az);
            NiPoint3 force((diff * m_conf.phys.stiffness) + (diff2 * m_conf.phys.stiffness2));

            force.z -= m_conf.phys.gravityBias * m_conf.phys.mass;

            if (!m_applyForceQueue.empty())
            {
                auto& current = m_applyForceQueue.front();

                auto vDir = m_objParent->m_worldTransform.rot * current.force;
                vDir.Normalize();

                force += (vDir * current.mag) / a_timeStep;

                if (!current.steps--)
                    m_applyForceQueue.pop();
            }

            float res = m_resistanceOn ?
                (1.0f - 1.0f / (m_velocity.Length() * 0.0075f + 1.0f)) * m_conf.phys.resistance + 1.0f : 1.0f;

            SetVelocity((m_velocity + (force / m_conf.phys.mass * a_timeStep)) -
                (m_velocity * ((m_conf.phys.damping * res) * a_timeStep)));

            auto invRot = m_objParent->m_worldTransform.rot.Transpose();
            m_virtld = invRot * ((m_oldWorldPos + m_velocity * a_timeStep) - target);

            ConstrainMotion(invRot, target, a_timeStep);

            m_oldWorldPos = m_objParent->m_worldTransform.rot * m_virtld + target;

            float ldx(m_virtld.x);
            float ldy(m_virtld.y);
            float ldz(m_virtld.z);

            m_obj->m_localTransform.pos.x = m_initialTransform.pos.x + ldx * m_conf.phys.linear[0];
            m_obj->m_localTransform.pos.y = m_initialTransform.pos.y + ldy * m_conf.phys.linear[1];
            m_obj->m_localTransform.pos.z = m_initialTransform.pos.z + ldz * m_conf.phys.linear[2];

            /*if (m_formid == 0x14 && std::string(m_obj->m_name) == "NPC L Breast")
                gLog.Debug(">> %f %f %f", dir.x, dir.y, dir.z);*/

            m_obj->m_localTransform.pos += (invRot * m_npGravityCorrection) * m_obj->m_localTransform.scale;

            if (m_rotScaleOn)
            {
                m_obj->m_localTransform.rot.SetEulerAngles(
                    ldx * m_conf.phys.rotational[0],
                    ldy * m_conf.phys.rotational[1],
                    (ldz + m_conf.phys.rotGravityCorrection) * m_conf.phys.rotational[2]);
            }

            m_obj->UpdateWorldData(&m_updateCtx);
        }

        m_collider.Update();
    }

    void SimComponent::UpdateVelocity()
    {
        if (m_movement)
            return;

        m_velocity = m_obj->m_worldTransform.pos - m_oldWorldPos;
        m_oldWorldPos = m_obj->m_worldTransform.pos;
    }

}