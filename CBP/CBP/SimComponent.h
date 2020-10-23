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

    __declspec(align(16)) struct MeshPoint
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

        std::shared_ptr<btTriangleIndexVertexArray> m_data;
    };

    class CollisionShape
    {
    public:
        virtual void UpdateShape() = 0;

        virtual void SetRadius(float a_radius)
        {
        }

        virtual void SetHeight(float a_height)
        {
        }

        virtual void SetExtent(const btVector3& a_extent)
        {
        }

        virtual void SetNodeScale(float a_scale)
        {
            m_nodeScale = a_scale;
            UpdateShape();
        }

        virtual btCollisionShape* GetBTShape() = 0;

        virtual ~CollisionShape() noexcept = default;

    protected:

        CollisionShape(float a_nodeScale) :
            m_nodeScale(a_nodeScale)
        {
        }

        float m_nodeScale;
    };

    template <class T>
    class CollisionShapeBase :
        public CollisionShape
    {
        friend class Collider;

    public:

        virtual btCollisionShape* GetBTShape()
        {
            return m_baseShape;
        }

        virtual ~CollisionShapeBase() noexcept
        {
            delete m_shape;
        }

    protected:

        template <typename... Args>
        CollisionShapeBase(btCollisionObject* a_collider, Args&&... a_args) :
            CollisionShape(1.0f),
            m_collider(a_collider)
        {
            m_shape = new T(std::forward<Args>(a_args)...);
        }

        template <typename... Args>
        __forceinline void RecreateShape(Args&&... a_args)
        {
            delete m_shape;
            m_shape = new T(std::forward<Args>(a_args)...);
        }

        __forceinline void PostUpdateShape()
        {
            m_collider->setCollisionShape(m_shape);
            ICollision::CleanProxyFromPairs(m_collider);
        }

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
        CollisionShapeTemplRH(btCollisionObject* a_collider, Args&&... a_args) :
            CollisionShapeBase<T>(a_collider, std::forward<Args>(a_args)...)
        {
        }

    public:

        virtual void DoRecreateShape(float a_radius, float a_height) = 0;

        virtual void UpdateShape()
        {
            auto rad = m_radius * m_nodeScale;
            if (rad <= 0.0f)
                return;

            auto height = std::max(m_height * m_nodeScale, 0.001f);
            if (rad == m_currentRadius && height == m_currentHeight)
                return;

            DoRecreateShape(rad, height);
            PostUpdateShape();

            m_currentRadius = rad;
            m_currentHeight = height;
        }

        virtual void SetRadius(float a_radius)
        {
            m_radius = a_radius;
            UpdateShape();
        }

        virtual void SetHeight(float a_height)
        {
            m_height = a_height;
            UpdateShape();
        }

    protected:

        float m_radius;
        float m_height;

        float m_currentRadius;
        float m_currentHeight;
    };

    template <class T>
    class CollisionShapeTemplExtent :
        public CollisionShapeBase<T>
    {
    public:

        template <typename... Args>
        CollisionShapeTemplExtent(btCollisionObject* a_collider, Args&&... a_args) :
            CollisionShapeBase<T>(a_collider, std::forward<Args>(a_args)...)
        {
        }

        virtual void DoRecreateShape(const btVector3& a_extent) = 0;
        virtual void SetShapeProperties(const btVector3& a_extent) {};

        virtual void UpdateShape()
        {
            auto extent(m_extent * m_nodeScale);
            if (extent == m_currentExtent)
                return;

            DoRecreateShape(extent);
            SetShapeProperties(extent);
            PostUpdateShape();

            m_currentExtent = extent;
        }

        virtual void SetExtent(const btVector3& a_extent)
        {
            m_extent = a_extent;
            UpdateShape();
        }

    protected:
        btVector3 m_extent;
        btVector3 m_currentExtent;
    };

    class CollisionShapeSphere :
        public CollisionShapeBase<btSphereShape>
    {
    public:
        CollisionShapeSphere(btCollisionObject* a_collider, float a_radius) :
            CollisionShapeBase<btSphereShape>(a_collider, a_radius)
        {
            m_radius = m_currentRadius = a_radius;
        }

        virtual void UpdateShape()
        {
            auto rad = m_radius * m_nodeScale;
            if (rad <= 0.0f || rad == m_currentRadius)
                return;

            RecreateShape(rad);
            PostUpdateShape();

            m_currentRadius = rad;
        }

        virtual void SetRadius(float a_radius)
        {
            m_radius = a_radius;
            UpdateShape();
        }

    private:
        float m_radius;

        float m_currentRadius;
    };


    class CollisionShapeCapsule :
        public CollisionShapeTemplRH<btCapsuleShape>
    {
    public:
        CollisionShapeCapsule(btCollisionObject* a_collider, float a_radius, float a_height) :
            CollisionShapeTemplRH<btCapsuleShape>(a_collider, a_radius, a_height)
        {
            m_radius = m_currentRadius = a_radius;
            m_height = m_currentHeight = a_height;
        }

        virtual void DoRecreateShape(float a_radius, float a_height)
        {
            RecreateShape(a_radius, a_height);
        }
    };

    class CollisionShapeCone :
        public CollisionShapeTemplRH<btConeShape>
    {
    public:
        CollisionShapeCone(btCollisionObject* a_collider, float a_radius, float a_height) :
            CollisionShapeTemplRH<btConeShape>(a_collider, a_radius, a_height)
        {
            m_radius = m_currentRadius = a_radius;
            m_height = m_currentHeight = a_height;
        }

        virtual void DoRecreateShape(float a_radius, float a_height)
        {
            RecreateShape(a_radius, a_height);
        }
    };

    class CollisionShapeBox :
        public CollisionShapeTemplExtent<btBoxShape>
    {
    public:
        CollisionShapeBox(btCollisionObject* a_collider, const btVector3& a_extent) :
            CollisionShapeTemplExtent<btBoxShape>(a_collider, a_extent)
        {
            m_extent = m_currentExtent = a_extent;
        }

        virtual void DoRecreateShape(const btVector3& a_extent)
        {
            RecreateShape(a_extent);
        }
    };

    class CollisionShapeCylinder :
        public CollisionShapeTemplRH<btCylinderShape>
    {
    public:
        CollisionShapeCylinder(btCollisionObject* a_collider, float a_radius, float a_height) :
            CollisionShapeTemplRH<btCylinderShape>(a_collider, btVector3(a_radius, a_height, 1.0f))
        {
            m_radius = m_currentRadius = a_radius;
            m_height = m_currentHeight = a_height;
        }

        virtual void DoRecreateShape(float a_radius, float a_height)
        {
            RecreateShape(btVector3(a_radius, a_height, 1.0f));
        }
    };

    class CollisionShapeTetrahedron :
        public CollisionShapeTemplExtent<btTetrahedronShapeEx>
    {
    public:
        CollisionShapeTetrahedron(btCollisionObject* a_collider, const btVector3& a_extent) :
            CollisionShapeTemplExtent<btTetrahedronShapeEx>(a_collider)
        {
            m_extent = m_currentExtent = a_extent;

            SetShapeProperties(a_extent);
        }

        virtual void DoRecreateShape(const btVector3& a_extent)
        {
            RecreateShape();
        }

        virtual void SetShapeProperties(const btVector3& a_extent)
        {
            m_shape->setVertices(
                m_vertices[0] * a_extent,
                m_vertices[1] * a_extent,
                m_vertices[2] * a_extent,
                m_vertices[3] * a_extent
            );
        }

    private:
        static const btVector3 m_vertices[4];
    };

    class CollisionShapeMesh :
        public CollisionShapeTemplExtent<btGImpactMeshShape>
    {
    public:
        CollisionShapeMesh(
            btCollisionObject* a_collider,
            const std::shared_ptr<btTriangleIndexVertexArray>& a_data,
            const btVector3& a_extent)
            :
            CollisionShapeTemplExtent<btGImpactMeshShape>(a_collider, a_data.get()),
            m_triVertexArray(a_data)
        {
            m_extent = m_currentExtent = a_extent;

            SetShapeProperties(a_extent);
        }

        virtual void DoRecreateShape(const btVector3& a_extent)
        {
            RecreateShape(m_triVertexArray.get());
        }

        virtual void SetShapeProperties(const btVector3& a_extent)
        {
            m_shape->setLocalScaling(a_extent);
            m_shape->updateBound();
        }

    private:

        std::shared_ptr<btTriangleIndexVertexArray> m_triVertexArray;
    };

    class CollisionShapeConvexHull :
        public CollisionShapeTemplExtent<btConvexHullShape>
    {
    public:

        CollisionShapeConvexHull(
            btCollisionObject* a_collider,
            const std::shared_ptr<MeshPoint[]>& a_data,
            int a_numVertices,
            const btVector3& a_extent)
            :
            CollisionShapeTemplExtent<btConvexHullShape>(a_collider, reinterpret_cast<const btScalar*>(a_data.get()), a_numVertices, sizeof(MeshPoint)),
            m_convexHullPoints(a_data),
            m_convexHullNumVertices(a_numVertices)
        {
            m_extent = m_currentExtent = a_extent;

            SetShapeProperties(a_extent);
        }

        virtual void DoRecreateShape(const btVector3& a_extent)
        {
            RecreateShape(reinterpret_cast<const btScalar*>(m_convexHullPoints.get()), m_convexHullNumVertices);
        }

        virtual void SetShapeProperties(const btVector3& a_extent)
        {
            m_shape->setLocalScaling(a_extent);
            m_shape->recalcLocalAabb();
        }

    private:

        std::shared_ptr<MeshPoint[]> m_convexHullPoints;
        int m_convexHullNumVertices;
    };

    class Collider :
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
        inline void Update();

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

        void SetShouldProcess(bool a_switch);

    private:

        void Activate();
        void Deactivate();

        static const btVector3 m_tetrahedronVertices[4];

        __forceinline btTetrahedronShapeEx* CreateTetrahedronShape(const btVector3& a_scale)
        {
            auto tmp = new btTetrahedronShapeEx();
            tmp->setVertices(
                m_tetrahedronVertices[0] * a_scale,
                m_tetrahedronVertices[1] * a_scale,
                m_tetrahedronVertices[2] * a_scale,
                m_tetrahedronVertices[3] * a_scale
            );
            return tmp;
        }

        std::unique_ptr<btCollisionObject> m_collider;
        std::unique_ptr<CollisionShape> m_colshape;

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

        PerfTimer pt;

        SimComponent& m_parent;
    };

    class SimComponent
    {

        static constexpr uint32_t CONSTRAIN_X = 0x1;
        static constexpr uint32_t CONSTRAIN_Y = 0x2;
        static constexpr uint32_t CONSTRAIN_Z = 0x4;

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
        NiPoint3 m_velocity;

        NiTransform m_initialTransform;
        bool m_hasScaleOverride;

        Collider m_collider;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t m_conf;

        bool m_collisions;
        bool m_movement;

        float m_colRad = 1.0f;
        float m_colHeight = 0.001f;
        float m_colCapsuleHeight = 0.0f;
        float m_colOffsetX = 0.0f;
        float m_colOffsetY = 0.0f;
        float m_colOffsetZ = 0.0f;
        float m_nodeScale = 1.0f;
        btVector3 m_extent;

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
            float a_timeStep,
            NiPoint3& a_ldiff
        );

    public:
        SimComponent(
            Actor* a_actor,
            NiAVObject* m_obj,
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

        void UpdateMovement(float timeStep);
        void UpdateVelocity();
        void Reset();

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

        [[nodiscard]] inline auto GetCenterOfMass() const {
            return m_objParent->m_worldTransform * m_npCogOffset;
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

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] inline const auto& GetDebugInfo() const {
            return m_debugInfo;
        }
#endif
    };
}