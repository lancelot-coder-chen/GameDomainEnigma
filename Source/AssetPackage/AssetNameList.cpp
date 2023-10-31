﻿#include "AssetNameList.h"
#include "AssetErrors.h"
#include <string>
#include <cassert>
#include <vector>

using namespace Enigma::AssetPackage;

using string = std::string;
const string empty_string = { "" };

AssetNameList::AssetNameList()
{
    m_assetNames.clear();
}

AssetNameList::~AssetNameList()
{
    m_assetNames.clear();
}

bool AssetNameList::hasAssetName(const string& name)
{
    if (name.empty()) return false;
    return m_assetNames.find(name) != m_assetNames.end();
}

error AssetNameList::appendAssetName(const string& name)
{
    if (name.empty()) return ErrorCode::emptyKey;
    if (hasAssetName(name)) return ErrorCode::duplicatedKey;
    m_assetNames.emplace(name);
    return ErrorCode::ok;
}

error AssetNameList::removeAssetName(const std::string& name)
{
    if (name.empty()) return ErrorCode::emptyKey;
    if (!hasAssetName(name)) return ErrorCode::notExistedKey;
    m_assetNames.erase(name);
    return ErrorCode::ok;
}

size_t AssetNameList::calcNameListDataBytes() const
{
    size_t sum = 0;
    for (const string& name : m_assetNames)
    {
        sum += (name.length() + 1);
    }
    return sum;
}

std::vector<char> AssetNameList::exportToByteBuffer() const
{
    size_t size = calcNameListDataBytes();
    if (size == 0) return std::vector<char>();

    std::vector<char> buff;
    buff.resize(size, 0);

    size_t index = 0;
    for (const string& name : m_assetNames)
    {
        assert(index + name.length() + 1 <= size);
        memcpy(&buff[index], name.c_str(), name.length());
        index += (name.length() + 1);
    }
    return buff;
}

error AssetNameList::importFromByteBuffer(const std::vector<char>& buff)
{
    if (buff.empty()) return ErrorCode::emptyBuffer;
    m_assetNames.clear();
    size_t index = 0;
    error er;
    while ((index < buff.size()) && (!er))
    {
        string name{ &buff[index] };
        index += (name.length() + 1);
        er = appendAssetName(name);
    }
    return er;
}
