#include "pch.h"

namespace CBP
{
    bool ColliderDataStorage::operator==(const configNode_t& a_rhs) const
    {
        return
            m_meta.stored &&
            m_meta.weightThreshold == a_rhs.fp.f32.bcWeightThreshold &&
            m_meta.simplifyTarget == a_rhs.fp.f32.bcSimplifyTarget &&
            m_meta.simplifyTargetError == a_rhs.fp.f32.bcSimplifyTargetError;
            //StrHelpers::icompare(m_meta.shape, a_rhs.ex.bcShape) == 0;
    }

    ColliderDataStorage::Meta& ColliderDataStorage::operator=(const configNode_t& a_rhs)
    {
        m_meta.weightThreshold = a_rhs.fp.f32.bcWeightThreshold;
        m_meta.simplifyTarget = a_rhs.fp.f32.bcSimplifyTarget;
        m_meta.simplifyTargetError = a_rhs.fp.f32.bcSimplifyTargetError;
        //m_meta.shape = a_rhs.ex.bcShape;
        m_meta.stored = true;

        return m_meta;
    }
}