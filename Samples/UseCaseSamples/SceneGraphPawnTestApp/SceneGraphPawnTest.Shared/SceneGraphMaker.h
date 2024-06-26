﻿/*********************************************************************
 * \file   SceneGraphMaker.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   February 2023
 *********************************************************************/
#ifndef _SCENE_GRAPH_MAKER_H
#define _SCENE_GRAPH_MAKER_H

#include "GameEngine/GenericDto.h"
#include <vector>
#include "SceneGraph/SpatialId.h"
#include "Primitives/PrimitiveId.h"

class SceneGraphMaker
{
public:
    static Enigma::Engine::GenericDto makeSceneGraph(const Enigma::SceneGraph::SpatialId& id, const Enigma::Primitives::PrimitiveId& primitive_id, const Enigma::SceneGraph::SpatialId& pawn_id, const Enigma::SceneGraph::SpatialId& stillpawn_id);
};

#endif // _SCENE_GRAPH_MAKER_H
