#pragma once

namespace CBP
{

    class SimComponent
    {
        struct Force
        {
            uint32_t steps;
            NiPoint3 force;
        };

    private:
        BSFixedString boneName;
        NiPoint3 oldWorldPos;
        NiPoint3 velocity;
        long long time;

        NiPoint3 npCogOffset;
        NiPoint3 npGravityCorrection;
        NiPoint3 npZero;

        configComponent_t conf;

        float diffMult;
        int numSteps;

        std::queue<Force> m_applyForceQueue;

        std::string m_configBoneName;
    public:
        SimComponent(
            NiAVObject* obj,
            const BSFixedString& name,
            const std::string& a_configBoneName,
            const configComponent_t& config
        ) noexcept;

        void updateConfig(const configComponent_t& centry) noexcept;

        void update(Actor* actor);
        void reset(Actor* actor);

        void applyForce(uint32_t a_steps, const NiPoint3& a_force);

        inline const std::string& GetConfigBoneName() const {
            return m_configBoneName;
        }
    };
}