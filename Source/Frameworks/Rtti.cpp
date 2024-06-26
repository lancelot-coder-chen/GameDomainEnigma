﻿#include "Rtti.h"
#include <cassert>

using namespace Enigma::Frameworks;

std::unique_ptr<std::unordered_map<std::string, const Rtti*>> Rtti::m_valueMap;

Rtti::Rtti(const std::string& name)
{
    m_name = name;
    m_base = nullptr;
    if (!m_valueMap) m_valueMap = std::make_unique<std::unordered_map<std::string, const Rtti*>>();
    assert(m_valueMap->find(name) == m_valueMap->end());
    m_valueMap->insert_or_assign(name, this);
}

Rtti::Rtti(const std::string& name, const Rtti* base_rtti)
{
    m_name = name;
    m_base = base_rtti;
    if (!m_valueMap) m_valueMap = std::make_unique<std::unordered_map<std::string, const Rtti*>>();
    assert(m_valueMap->find(name) == m_valueMap->end());
    m_valueMap->insert_or_assign(name, this);
}

bool Rtti::operator==(const Rtti& rhs) const
{
    return isExactly(rhs);
}

const Rtti& Rtti::fromName(const std::string& name)
{
    auto iter = m_valueMap->find(name);
    assert(iter != m_valueMap->end());
    return *iter->second;
}

bool Rtti::isExactly(const Rtti& type) const
{
    return &type == this;
}

bool Rtti::isDerived(const Rtti& type) const
{
    const Rtti* search = this;
    while (search)
    {
        if (search == &type)
            return true;
        search = search->m_base;
    }
    return false;
}

const std::string& Rtti::getName() const
{
    return m_name;
}

bool Rtti::isDerivedFrom(const std::string& type_token, const std::string& base_rtti_token)
{
    auto iter_type = m_valueMap->find(type_token);
    auto iter_base = m_valueMap->find(base_rtti_token);
    if (iter_type == m_valueMap->end() || iter_base == m_valueMap->end()) return false;
    return (*iter_type->second).isDerived(*iter_base->second);
}

bool Rtti::isExactlyOrDerivedFrom(const std::string& type_token, const std::string& base_rtti_token)
{
    return type_token == base_rtti_token || isDerivedFrom(type_token, base_rtti_token);
}

