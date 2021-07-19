#pragma once

#include "ext/magic_enum.hpp"

namespace ConfigCommon
{
    enum class ConfigGender : char
    {
        Male = 0,
        Female
    };

    template <class T>
    class configGenderRoot_t
    {
        friend class boost::serialization::access;

    public:

        using config_type = T;
        using value_type = typename T::value_type;
        using mapped_type = typename T::mapped_type;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

        configGenderRoot_t() = default;

        [[nodiscard]] SKMP_FORCEINLINE auto& operator()() noexcept {
            return m_configs;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& operator()() const noexcept {
            return m_configs;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto& operator()(ConfigGender a_gender) noexcept {
            return m_configs[Enum::Underlying(a_gender)];
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& operator()(ConfigGender a_gender) const noexcept {
            return m_configs[Enum::Underlying(a_gender)];
        }

        SKMP_FORCEINLINE void clear() {
            for (auto& e : m_configs) {
                e.clear();
            }
        }

    private:
        T m_configs[magic_enum::enum_count<ConfigGender>()];

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            for (auto& e : m_configs) {
                ar& e;
            }
        }
    };

}

