﻿#include "GeometryCreationHelper.h"
#include "Geometries/StandardGeometryDtoHelper.h"
#include "Geometries/TriangleList.h"
#include "Frameworks/CommandBus.h"
#include "Geometries/GeometryCommands.h"

using namespace EnigmaViewer;
using namespace Enigma::Geometries;
using namespace Enigma::MathLib;
using namespace Enigma::Engine;

std::shared_ptr<GeometryData> GeometryCreationHelper::createSquareXZQuad(const GeometryId& id)
{
    auto geometry = GeometryData::queryGeometryData(id);
    if (geometry) return geometry;

    SquareQuadDtoHelper helper(id);
    helper.xzQuad(Vector3(-5.0f, 0.0f, -5.0f), Vector3(5.0f, 0.0f, 5.0f)).normal().textureCoord(Vector2(0.0f, 1.0f), Vector2(1.0f, 0.0f));
    auto dto = helper.toGenericDto();
    dto.addRtti(FactoryDesc(TriangleList::TYPE_RTTI.getName()).ClaimAsResourceAsset(id.name(), id.name() + ".geo", "APK_PATH"));
    Enigma::Frameworks::CommandBus::post(std::make_shared<ConstituteGeometry>(id, dto));
    return geometry;
}
