﻿/*********************************************************************
 * \file   ShadowCasterBoundFilter.h
 * \brief  過濾掉不是 Shadow Caster
 * 
 * \author Lancelot 'Robin' Chen
 * \date   June 2023
 *********************************************************************/
#ifndef _SHADOW_CASTER_BOUND_FILTER_H
#define _SHADOW_CASTER_BOUND_FILTER_H

#include "Renderer/RenderableBoundFilter.h"

namespace Enigma::ShadowMap
{
    /** Shadow caster Bound Filter */
    class ShadowCasterBoundFilter : public Renderer::RenderableBoundFilter
    {
    public:
        ShadowCasterBoundFilter();
        virtual ~ShadowCasterBoundFilter();

        virtual bool FilterOutSpatial(const std::shared_ptr<SceneGraph::Spatial>& spatial) override;
    };
};


#endif // _SHADOW_CASTER_BOUND_FILTER_H