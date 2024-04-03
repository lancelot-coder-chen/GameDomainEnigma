﻿#include "ViewerRenderablesFileStoreMapper.h"
#include "Renderables/ModelPrimitive.h"
#include "ViewerQueries.h"
#include "Frameworks/QueryDispatcher.h"

using namespace EnigmaViewer;

ViewerRenderablesFileStoreMapper::ViewerRenderablesFileStoreMapper(const std::string& mapper_filename, const std::shared_ptr<Enigma::Gateways::IDtoGateway>& gateway) : Enigma::FileStorage::PrimitiveFileStoreMapper(mapper_filename, gateway)
{
}

ViewerRenderablesFileStoreMapper::~ViewerRenderablesFileStoreMapper()
{
}

void ViewerRenderablesFileStoreMapper::subscribeHandlers()
{
    m_requestModelNames = std::make_shared<Enigma::Frameworks::QuerySubscriber>([=](const Enigma::Frameworks::IQueryPtr& q) { requestModelNames(q); });
    Enigma::Frameworks::QueryDispatcher::subscribe(typeid(RequestModelNames), m_requestModelNames);
}

void ViewerRenderablesFileStoreMapper::unsubscribeHandlers()
{
    Enigma::Frameworks::QueryDispatcher::unsubscribe(typeid(RequestModelNames), m_requestModelNames);
    m_requestModelNames = nullptr;
}

std::vector<std::string> ViewerRenderablesFileStoreMapper::modelNames() const
{
    std::vector<std::string> model_names;
    for (const auto& [id, filename] : m_filename_map)
    {
        if (id.rtti().isDerived(Enigma::Renderables::ModelPrimitive::TYPE_RTTI)) model_names.push_back(id.name());
    }
    return model_names;
}

std::optional<Enigma::Primitives::PrimitiveId> ViewerRenderablesFileStoreMapper::modelId(const std::string& model_name) const
{
    for (const auto& [id, filename] : m_filename_map)
    {
        if (id.name() == model_name) return id;
    }
    return std::nullopt;
}

void ViewerRenderablesFileStoreMapper::requestModelNames(const Enigma::Frameworks::IQueryPtr& q)
{
    const auto query = std::dynamic_pointer_cast<RequestModelNames>(q);
    if (query == nullptr) return;
    query->setResult(modelNames());
}