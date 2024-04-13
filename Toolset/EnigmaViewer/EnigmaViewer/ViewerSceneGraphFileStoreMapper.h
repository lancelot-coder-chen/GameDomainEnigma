﻿/*****************************************************************
 * \file   ViewerSceneGraphFileStoreMapper.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   April 2024
 ******************************************************************/
#ifndef VIEWER_SCENE_GRAPH_FILE_STORE_MAPPER_H
#define VIEWER_SCENE_GRAPH_FILE_STORE_MAPPER_H

#include "FileStorage/SceneGraphFileStoreMapper.h"
#include "Frameworks/QuerySubscriber.h"

namespace EnigmaViewer
{
    class ViewerSceneGraphFileStoreMapper : public Enigma::FileStorage::SceneGraphFileStoreMapper
    {
    public:
        ViewerSceneGraphFileStoreMapper(const std::string& mapper_filename, const std::string& lazied_mapper_filename, const std::shared_ptr<Enigma::Gateways::IDtoGateway>& gateway);
        virtual ~ViewerSceneGraphFileStoreMapper() override;

        void subscribeHandlers();
        void unsubscribeHandlers();

        bool hasAnimatedPawn(const std::string& name);

        std::vector<std::string> pawnNames() const;
        std::optional<Enigma::SceneGraph::SpatialId> pawnId(const std::string& pawn_name) const;

    protected:
        void hasAnimatedPawn(const Enigma::Frameworks::IQueryPtr& q);
        void requestPawnNames(const Enigma::Frameworks::IQueryPtr& q);
        void resolvePawnId(const Enigma::Frameworks::IQueryPtr& q);

    protected:
        Enigma::Frameworks::QuerySubscriberPtr m_hasAnimatedPawn;
        Enigma::Frameworks::QuerySubscriberPtr m_requestPawnNames;
        Enigma::Frameworks::QuerySubscriberPtr m_resolvePawnId;
    };
}

#endif // VIEWER_SCENE_GRAPH_FILE_STORE_MAPPER_H
