#pragma once

namespace CBP
{
    class ColliderProfile :
        public ProfileBase<ColliderData>,
        ILog
    {
        static constexpr int IMPORT_RVC_FLAGS = 
            aiComponent_COLORS |
            aiComponent_NORMALS |
            aiComponent_TANGENTS_AND_BITANGENTS |
            aiComponent_TEXCOORDS |
            aiComponent_BONEWEIGHTS |
            aiComponent_ANIMATIONS |
            aiComponent_TEXTURES |
            aiComponent_LIGHTS |
            aiComponent_CAMERAS |
            aiComponent_MATERIALS;

        static constexpr int IMPORT_FLAGS =
            aiProcess_FindInvalidData |
            aiProcess_RemoveComponent |
           // aiProcess_Triangulate |
            aiProcess_ValidateDataStructure |
            aiProcess_JoinIdenticalVertices;
    public:

        template <typename... Args>
        ColliderProfile(Args&&... a_args) :
            ProfileBase<ColliderData>(std::forward<Args>(a_args)...)
        {
        }

        virtual ~ColliderProfile() noexcept = default;

        virtual bool Load();
        virtual bool Save(const ColliderData& a_data, bool a_store);
        virtual void SetDefaults() noexcept;
       
        FN_NAMEPROC("ColliderProfile");
    };

    class ProfileManagerCollider :
        public ProfileManager<ColliderProfile>
    {
    public:

        template<typename... Args>
        ProfileManagerCollider(Args&&... a_args) :
            ProfileManager<ColliderProfile>(std::forward<Args>(a_args)...)
        {
        }

        inline static ProfileManagerCollider& GetSingleton() {
            return m_Instance;
        }

        FN_NAMEPROC("CBP::ProfileManagerCollider");

    private:
        static ProfileManagerCollider m_Instance;
    };

    class ICollision :
        public r3d::EventListener
    {
    public:

        [[nodiscard]] inline static auto& GetSingleton() {
            return m_Instance;
        }

        inline static void SetTimeStep(float a_timeStep) {
            m_Instance.m_timeStep = a_timeStep;
        }

        static void Initialize(r3d::PhysicsWorld* a_world);

        ICollision(const ICollision&) = delete;
        ICollision(ICollision&&) = delete;
        ICollision& operator=(const ICollision&) = delete;
        void operator=(ICollision&&) = delete;

    private:
        ICollision() = default;

        virtual void onContact(const CollisionCallback::CallbackData& callbackData) override;

        static bool collisionCheckFunc(r3d::Collider* a_lhs, r3d::Collider* a_rhs);

        float m_timeStep = 1.0f / 60.0f;

        static ICollision m_Instance;
    };
}

