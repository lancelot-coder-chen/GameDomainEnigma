﻿#include "AssetHeaderDataMap.h"
#include <cassert>
#include <vector>

using namespace Enigma::AssetPackage;

AssetHeaderDataMap::AssetHeaderDataMap()
{
    m_headerDataMap.clear();
}

AssetHeaderDataMap::~AssetHeaderDataMap()
{
    m_headerDataMap.clear();
}

error AssetHeaderDataMap::InsertHeaderData(const AssetHeaderData& header)
{
    if (HasAssetKey(header.m_name)) return ErrorCode::DuplicatedKey;
    auto result = m_headerDataMap.emplace(header.m_name, header);
    assert(result.second);
    return ErrorCode::OK;
}

error AssetHeaderDataMap::RemoveHeaderData(const std::string& name)
{
    if (!HasAssetKey(name)) return ErrorCode::NotExistedKey;
    m_headerDataMap.erase(name);
    return ErrorCode::OK;
}

bool AssetHeaderDataMap::HasAssetKey(const std::string& name) const
{
    auto find_iter = m_headerDataMap.find(name);
    return (find_iter != m_headerDataMap.end());
}

void AssetHeaderDataMap::RepackContentOffsets(const unsigned content_size, const unsigned base_offset)
{
    for (auto& kv : m_headerDataMap)
    {
        if (kv.second.m_offset >= base_offset)
        {
            kv.second.m_offset -= content_size;
        }
    }
}

std::optional<AssetHeaderDataMap::AssetHeaderData> AssetHeaderDataMap::TryGetHeaderData(const std::string& name)
{
    auto find_iter = m_headerDataMap.find(name);
    if (find_iter != m_headerDataMap.end()) return find_iter->second;
    return std::nullopt;
}

size_t AssetHeaderDataMap::CalcHeaderDataMapBytes() const
{
    size_t sum = 0;
    for (auto kv : m_headerDataMap)
    {
        sum += (kv.first.length() + 1); // name 的長度加起來
    }
    sum += (GetTotalDataCount() * sizeof(unsigned int) * 5);  // 5個uint * 總數量
    return sum;
}

std::vector<char> AssetHeaderDataMap::ExportToByteBuffer() const
{
    size_t size = CalcHeaderDataMapBytes();
    if (size == 0)
    {
        make_error_code(ErrorCode::EmptyHeader);
        return std::vector<char>();
    }

    std::vector<char> buff;
    buff.resize(size, 0);

    size_t index = 0;
    for (auto kv : m_headerDataMap)
    {
        assert(index + kv.second.m_name.length() + 1 + sizeof(unsigned int) * 5 <= size);
        memcpy(&buff[index], kv.second.m_name.c_str(), kv.second.m_name.length());
        index += (kv.second.m_name.length() + 1);
        memcpy(&buff[index], &(kv.second.m_version), sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&buff[index], &(kv.second.m_size), sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&buff[index], &(kv.second.m_orgSize), sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&buff[index], &(kv.second.m_offset), sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&buff[index], &(kv.second.m_crc), sizeof(unsigned int));
        index += sizeof(unsigned int);
    }
    return buff;
}

std::error_code AssetHeaderDataMap::ImportFromByteBuffer(const std::vector<char>& buff)
{
    if (buff.empty()) return make_error_code(ErrorCode::EmptyBuffer);
    m_headerDataMap.clear();
    const size_t size = buff.size();
    size_t index = 0;
    while (index < size)
    {
        AssetHeaderData header{};
        header.m_name = std::string{ &buff[index] };
        index += (header.m_name.length() + 1);
        memcpy(&header.m_version, &buff[index], sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&header.m_size, &buff[index], sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&header.m_orgSize, &buff[index], sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&header.m_offset, &buff[index], sizeof(unsigned int));
        index += sizeof(unsigned int);
        memcpy(&header.m_crc, &buff[index], sizeof(unsigned int));
        index += sizeof(unsigned int);

        InsertHeaderData(header);
    }
    return ErrorCode::OK;
}
