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
        ColliderData() :
            m_triVertexArray(nullptr)
        {
        }

        std::shared_ptr<MeshPoint[]> m_vertices;
        std::shared_ptr<MeshPoint[]> m_hullPoints;
        std::shared_ptr<int[]> m_indices;

        int numVertices;
        int numTriangles;
        int numIndices;

        btTriangleIndexVertexArray* m_triVertexArray;
    };

    class SKMP_ALIGN(CollisionShape, 16)
    {
    public:
        BT_DECLARE_ALIGNED_ALLOCATOR();

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
    class SKMP_ALIGN(CollisionShapeBase, 16) :
        public CollisionShape
    {

    public:

        [[nodiscard]] virtual btCollisionShape* GetBTShape();

    protected:

        virtual ~CollisionShapeBase() noexcept;

        template <typename... Args>
        CollisionShapeBase(btCollisionObject* a_collider, Args&&... a_args);

        template <typename... Args>
        SKMP_FORCEINLINE void RecreateShape(Args&&... a_args);
        SKMP_FORCEINLINE void PostUpdateShape();

        union
        {
            T* m_shape;
            btCollisionShape* m_baseShape;
        };

        btCollisionObject* m_collider;
    };

    template <class T>
    class SKMP_ALIGN(CollisionShapeTemplRH, 16) :
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
    class SKMP_ALIGN(CollisionShapeTemplExtent, 16) :
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

    class __declspec(align(16)) CollisionShapeSphere :
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


    class SKMP_ALIGN(CollisionShapeCapsule, 16) :
        public CollisionShapeTemplRH<btCapsuleShape>
    {
    public:
        CollisionShapeCapsule(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class SKMP_ALIGN(CollisionShapeCone, 16) :
        public CollisionShapeTemplRH<btConeShape>
    {
    public:
        CollisionShapeCone(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class SKMP_ALIGN(CollisionShapeBox, 16) :
        public CollisionShapeTemplExtent<btBoxShape>
    {
    public:
        CollisionShapeBox(btCollisionObject* a_collider, const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
    };

    class SKMP_ALIGN(CollisionShapeCylinder, 16) :
        public CollisionShapeTemplRH<btCylinderShape>
    {
    public:
        CollisionShapeCylinder(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class __declspec(align(16)) CollisionShapeTetrahedron :
        public CollisionShapeTemplExtent<btTetrahedronShapeEx>
    {
    public:
        CollisionShapeTetrahedron(btCollisionObject* a_collider, const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
        virtual void SetShapeProperties(const btVector3& a_extent);

    private:
        static const btVector3 m_vertices[4];
    };

    class SKMP_ALIGN(CollisionShapeMesh, 16) :
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

        btTriangleIndexVertexArray* m_triVertexArray;
    };

    class SKMP_ALIGN(CollisionShapeConvexHull, 16) :
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

    class SKMP_ALIGN(Collider,16) :
        ILog
    {
        static constexpr float crdrmul = float(MATH_PI) / 180.0f;

    public:
        Collider(SimComponent& a_parent);
        virtual ~Collider() noexcept;

        Collider() = delete;
        Collider(const Collider& a_rhs) = delete;
        Collider(Collider&& a_rhs) = delete;

        bool Create(ColliderShapeType a_shape);
        bool Destroy();
        SKMP_FORCEINLINE void Update();

        SKMP_FORCEINLINE void SetColliderRotation(float a_x, float a_y, float a_z)
        {
            m_colRot.setEulerZYX(
                a_x * crdrmul,
                a_y * crdrmul,
                a_z * crdrmul
            );
        }
        
        SKMP_FORCEINLINE void SetRadius(float a_val) {
            if (m_created)
                m_colshape->SetRadius(a_val);
        }

        SKMP_FORCEINLINE void SetHeight(float a_val) {
            if (m_created)
                m_colshape->SetHeight(a_val);
        }

        SKMP_FORCEINLINE void SetExtent(const btVector3& a_extent)
        {
            if (m_created)
                m_colshape->SetExtent(a_extent);
        }

        SKMP_FORCEINLINE void SetOffset(const btVector3& a_offset, const btVector3& a_initial) {
            m_bodyOffset = a_offset;
            m_bodyOffsetPlusInitial = a_offset + a_initial;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool IsActive() const {
            return m_created && m_active;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetSphereOffset() const {
            return m_bodyOffset;
        }

        SKMP_FORCEINLINE void SetPositionScale(float a_scale) {
            m_positionScale = a_scale;
            m_doPositionScaling = a_scale != 1.0f;
        }

        void SetShouldProcess(bool a_switch);

        SKMP_FORCEINLINE void SetOffsetParent(bool a_switch) {
            m_offsetParent = a_switch;
        }

    private:

        void Activate();
        void Deactivate();

        btMatrix3x3 m_colRot;
        btVector3 m_bodyOffset;
        btVector3 m_bodyOffsetPlusInitial;

        btCollisionObject* m_collider;
        CollisionShape* m_colshape;

        ColliderShapeType m_shape;
        std::string m_meshShape;

        float m_nodeScale;

        bool m_created;
        bool m_active;
        bool m_colliderActivated;
        bool m_process;
        bool m_rotation;

        float m_positionScale;
        bool m_doPositionScaling;
        bool m_offsetParent;

        SimComponent& m_parent;
    };

    class SKMP_ALIGN(SimComponent, 16)
    {
        struct SKMP_ALIGN(Force, 16)
        {
            Force(
                uint32_t a_steps,
                const btVector3& a_norm)
                :
                m_steps(a_steps),
                m_force(a_norm)
            {
            }

            btVector3 m_force;
            uint32_t m_steps;
        };

        friend class Collider;

    private:

        btMatrix3x3 m_itrInitialMat;
        btMatrix3x3 m_itrMatObj;
        btMatrix3x3 m_itrMatParent;
        btVector3 m_itrInitialPos;
        btVector3 m_itrPosObj;
        btVector3 m_itrPosParent;

        btVector3 m_cogOffset;
        btVector3 m_gravityCorrection;

        btVector3 m_oldWorldPos;
        btVector3 m_virtld;
        btVector3 m_ld;
        btVector3 m_velocity;

        btVector3 m_colExtent;
        btVector3 m_colOffset;
        btVector3 m_linearScale;

        NiTransform m_initialTransform;
        NiMatrix33 m_tempLocalRot;

        std::string m_nodeName;
        std::string m_configGroupName;

        configComponent16_t m_conf;

        bool m_collisions;
        bool m_movement;

        float m_colRad;
        float m_colHeight;
        float m_nodeScale;
        float m_invMass;
        float m_maxVelocity2;
        float m_gravForce;
        uint64_t m_groupId;
        uint64_t m_parentId;

        bool m_resistanceOn;
        bool m_rotScaleOn;
        bool m_hasScaleOverride;

        NiPointer<NiAVObject> m_obj;
        NiPointer<NiAVObject> m_objParent;

        NiAVObject::ControllerUpdateContext m_updateCtx;

        std::queue<Force, std::deque<Force, mem::aligned_allocator<Force, 16>>> m_applyForceQueue;

#ifdef _CBP_ENABLE_DEBUG
        SimDebugInfo m_debugInfo;
#endif

        Game::FormID m_formid;

        Collider m_collider;

        void ColUpdateWeightData(
            Actor* a_actor,
            const configComponent16_t& a_config,
            const configNode_t& a_nodeConf);

        SKMP_FORCEINLINE void ClampVelocity()
        {
            float len2 = m_velocity.length2();
            if (len2 < m_maxVelocity2)
                return;

            m_velocity /= std::sqrtf(len2);
            m_velocity *= m_conf.fp.f32.maxVelocity;
        }

        SKMP_FORCEINLINE void ConstrainMotion(
            const btMatrix3x3& a_invRot,
            const btVector3& a_target,
            float a_timeStep
        );

        SKMP_FORCEINLINE void SIMDFillObj();
        SKMP_FORCEINLINE void SIMDFillParent();

    public:
        BT_DECLARE_ALIGNED_ALLOCATOR();

        SimComponent(
            Actor* a_actor,
            NiAVObject* a_obj,
            const std::string& a_nodeName,
            const std::string& a_configBoneName,
            const configComponent32_t& config,
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
            const configComponent32_t* a_physConf,
            const configNode_t& a_nodeConf,
            bool a_collisions,
            bool a_movement) noexcept;

        void UpdateMotion(float timeStep);
        SKMP_FORCEINLINE void UpdateVelocity();
        void Reset();
        bool ValidateNodes(NiAVObject* a_obj);

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        SKMP_FORCEINLINE void AddVelocity(const btVector3& a_vel) {
            m_velocity += a_vel;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetVelocity() const {
            return m_velocity;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfig() const {
            return m_conf;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE const auto& GetNodeName() const {
            return m_nodeName;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool IsSameGroup(const SimComponent& a_rhs) const {
            return a_rhs.m_groupId != 0 && m_groupId != 0 &&
                a_rhs.m_parentId == m_parentId &&
                a_rhs.m_groupId == m_groupId;
        }

        SKMP_FORCEINLINE void UpdateGroupInfo(uint64_t a_groupId) {
            m_groupId = a_groupId;
        };

        [[nodiscard]] SKMP_FORCEINLINE bool HasMotion() const {
            return m_movement;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasActiveCollider() const {
            return m_collider.IsActive();
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetPos() const {
            return m_obj->m_worldTransform.pos;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetWorldTransform() const {
            return m_obj->m_worldTransform;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetParentWorldTransform() const {
            return m_objParent->m_worldTransform;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetVirtualPos() const {
            return m_virtld;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetCenterOfGravity() const {
            return m_cogOffset;
        }

        [[nodiscard]] SKMP_FORCEINLINE float GetNodeScale() const {
            return m_obj->m_worldTransform.scale;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto& GetCollider() {
            return m_collider;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetCollider() const {
            return m_collider;
        }

        [[nodiscard]] SKMP_FORCEINLINE const float GetMassInverse() const {
            return m_invMass;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto GetNode() {
            return m_obj.m_pObject;
        }

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] SKMP_FORCEINLINE const auto& GetDebugInfo() const {
            return m_debugInfo;
        }
#endif
    };

    namespace Math
    {
        SKMP_FORCEINLINE float norm(float a_val, float a_min, float a_max) {
            return (a_val - a_min) / (a_max - a_min);
        }

        SKMP_FORCEINLINE float normc(float a_val, float a_min, float a_max) {
            return std::clamp(norm(a_val, a_min, a_max), 0.0f, 1.0f);
        }

        SKMP_FORCEINLINE constexpr float sgn(float a_val) {
            return a_val < 0.0f ? -1.0f : 1.0f;
        }
    }


    void SimComponent::UpdateVelocity()
    {
        if (m_movement)
            return;

        btVector3 pos(
            m_obj->m_worldTransform.pos.x,
            m_obj->m_worldTransform.pos.y,
            m_obj->m_worldTransform.pos.z);

        m_velocity = pos - m_oldWorldPos;
        m_oldWorldPos = pos;
    }

}