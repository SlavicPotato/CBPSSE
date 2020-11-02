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

        btTriangleIndexVertexArray* m_triVertexArray;
    };

    class __declspec(align(16)) CollisionShape
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
    class __declspec(align(16)) CollisionShapeBase :
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
    class __declspec(align(16)) CollisionShapeTemplRH :
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


    class __declspec(align(16)) CollisionShapeCapsule :
        public CollisionShapeTemplRH<btCapsuleShape>
    {
    public:
        CollisionShapeCapsule(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class __declspec(align(16)) CollisionShapeCone :
        public CollisionShapeTemplRH<btConeShape>
    {
    public:
        CollisionShapeCone(btCollisionObject* a_collider, float a_radius, float a_height);

        virtual void DoRecreateShape(float a_radius, float a_height);
    };

    class __declspec(align(16)) CollisionShapeBox :
        public CollisionShapeTemplExtent<btBoxShape>
    {
    public:
        CollisionShapeBox(btCollisionObject* a_collider, const btVector3& a_extent);

        virtual void DoRecreateShape(const btVector3& a_extent);
    };

    class __declspec(align(16)) CollisionShapeCylinder :
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

    class __declspec(align(16)) CollisionShapeMesh :
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

    class __declspec(align(16)) CollisionShapeConvexHull :
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

        inline void SetOffset(const btVector3& a_offset, const btVector3& a_initial) {
            m_bodyOffset = a_offset;
            m_bodyOffsetPlusInitial = a_offset + a_initial;
        }

        [[nodiscard]] inline bool IsActive() const {
            return m_created && m_active;
        }

        [[nodiscard]] inline const auto& GetSphereOffset() const {
            return m_bodyOffset;
        }

        inline void SetPositionScale(float a_scale) {
            m_positionScale = a_scale;
            m_doPositionScaling = a_scale != 1.0f;
        }

        void SetShouldProcess(bool a_switch);

    private:

        void Activate();
        void Deactivate();

        btCollisionObject* m_collider;
        CollisionShape* m_colshape;

        btMatrix3x3 m_colRot;
        btVector3 m_bodyOffset;
        btVector3 m_bodyOffsetPlusInitial;

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

        PerfTimer pt;

        SimComponent& m_parent;
    };

    __declspec(align(16)) class SimComponent
    {
        struct Force
        {
            Force(
                uint32_t a_steps,
                const btVector3& a_norm,
                float a_mag)
                :
                m_steps(a_steps),
                m_norm(a_norm),
                m_mag(a_mag)
            {
            }

            uint32_t m_steps;
            btVector3 m_norm;
            float m_mag;
        };

        friend class Collider;

    private:

        bool ColUpdateWeightData(
            Actor* a_actor,
            const configComponent_t& a_config,
            const configNode_t& a_nodeConf);

        btVector3 m_cogOffset;
        btVector3 m_gravityCorrection;

        btVector3 m_oldWorldPos;
        btVector3 m_virtld;
        btVector3 m_ld;
        btVector3 m_velocity;

        NiTransform m_initialTransform;
        
        bool m_hasScaleOverride;

        btMatrix3x3 m_itrInitialMat;
        btMatrix3x3 m_itrMatObj;
        btMatrix3x3 m_itrMatParent;
        btVector3 m_itrInitialPos;
        btVector3 m_itrPosObj;
        btVector3 m_itrPosParent;

        std::string m_nodeName;
        std::string m_configGroupName;

        configComponent_t m_conf;

        bool m_collisions;
        bool m_movement;

        float m_colRad;
        float m_colHeight;
        float m_nodeScale;
        float m_massInv;
        btVector3 m_extent;
        btVector3 m_colOffset;
        btVector3 m_linearScale;

        uint64_t m_groupId;
        uint64_t m_parentId;

        bool m_resistanceOn;
        bool m_rotScaleOn;

        NiPointer<NiAVObject> m_obj;
        NiPointer<NiAVObject> m_objParent;

        NiAVObject::ControllerUpdateContext m_updateCtx;

        std::queue<Force> m_applyForceQueue;

#ifdef _CBP_ENABLE_DEBUG
        SimDebugInfo m_debugInfo;
#endif

        Game::FormID m_formid;

        Collider m_collider;

        __forceinline void ClampVelocity()
        {
            float len = m_velocity.length();
            if (len < m_conf.phys.data.maxVelocity)
                return;

            m_velocity /= len;
            m_velocity *= m_conf.phys.data.maxVelocity;
        }

        __forceinline void ConstrainMotion(
            const btMatrix3x3& a_invRot,
            const btVector3& a_target,
            float a_timeStep
        );

        __forceinline void SIMDFillObj();
        __forceinline void SIMDFillParent();

    public:
        BT_DECLARE_ALIGNED_ALLOCATOR();

        SimComponent(
            Actor* a_actor,
            NiAVObject* a_obj,
            const std::string& a_nodeName,
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

        void UpdateMotion(float timeStep);
        __forceinline void UpdateVelocity();
        void Reset();
        bool ValidateNodes(NiAVObject* a_obj);

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        __forceinline void AddVelocity(const btVector3& a_vel) {
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
        
        [[nodiscard]] inline const auto& GetNodeName() const {
            return m_nodeName;
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
            return m_cogOffset;
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