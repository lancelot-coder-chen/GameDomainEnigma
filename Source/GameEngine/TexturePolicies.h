﻿/*********************************************************************
 * \file   TexturePolicies.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   September 2022
 *********************************************************************/
#ifndef TEXTURE_POLICIES_H
#define TEXTURE_POLICIES_H

#include "FileSystem/Filename.h"
#include "MathLib/AlgebraBasicTypes.h"
#include <string>

namespace Enigma::Engine
{
    class TexturePolicy
    {
    public:
        enum class JobType
        {
            None,
            Load,
            Create,
        };
    public:
        TexturePolicy() : m_jobType(JobType::None), m_surfaceCount(0) {};
        TexturePolicy(const std::string& name, const std::string& filename, const std::string& path_id)
            : m_jobType(JobType::Load), m_name(name), m_filename(filename), m_pathId(path_id), m_surfaceCount(1) {};
        TexturePolicy(const std::string& name, const FileSystem::Filename& filename) : m_jobType(JobType::Load), m_name(name), m_surfaceCount(1)
        {
            m_filename = filename.GetSubPathFileName();
            m_pathId = filename.GetMountPathID();
        };
        TexturePolicy(const std::string& name, const MathLib::Dimension<unsigned>& dimension, unsigned surface_count)
            : m_jobType(JobType::Create), m_name(name), m_dimension(dimension), m_surfaceCount(surface_count) {};

        const std::string& getName() const { return m_name; };
        const std::string& GetFileName() const { return m_filename; };
        const std::string& GetPathID() const { return m_pathId; };
        const MathLib::Dimension<unsigned>& GetDimension() const { return m_dimension; };
        const JobType& GetJobType() const { return m_jobType; };
        const unsigned& GetSurfaceCount() const { return m_surfaceCount; };

    protected:
        JobType m_jobType;
        std::string m_name;
        std::string m_filename;
        std::string m_pathId;
        MathLib::Dimension<unsigned> m_dimension;
        unsigned m_surfaceCount;
    };
}

#endif // TEXTURE_POLICIES_H
