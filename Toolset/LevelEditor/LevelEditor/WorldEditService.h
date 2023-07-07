﻿/*********************************************************************
 * \file   WorldEditService.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   July 2023
 *********************************************************************/
#ifndef WORLD_EDIT_SERVICE_H
#define WORLD_EDIT_SERVICE_H

#include "Frameworks/ServiceManager.h"
#include <filesystem>

namespace LevelEditor
{
    class WorldEditService : public Enigma::Frameworks::ISystemService
    {
        DECLARE_EN_RTTI;
    public:
        WorldEditService(Enigma::Frameworks::ServiceManager* srv_mngr);
        virtual ~WorldEditService() override;

        virtual Enigma::Frameworks::ServiceResult OnInit() override;
        virtual Enigma::Frameworks::ServiceResult OnTerm() override;

        void SetWorldMapRootFolder(const std::filesystem::path& folder);

        // check world files
        //bool CheckWorldMapFiles(const std::string& world_name);
        // delete world files
        //void DeleteWorldMapFiles(const std::string& world_name);
        // create world map directory & files
        //void CreateWorldMapFiles(const std::string& world_name) const;

        //void AddNewTerrain(const std::string& terrain_name, const Enigma::Matrix4& mxLocal,
        //    const TerrainCreationSetting& terrain_creation_setting);

        /** picking terrain
        @param clip_pos position on clipping space
        @return picked terrain and picked position on terrain */
        //std::tuple<Enigma::TerrainEntityPtr, Enigma::Vector3> PickingTerrainOnWorldMap(const Enigma::Vector2& clip_pos);

    private:
        //void OnAsyncKeyPressed(const Enigma::IMessagePtr& m);
        //void OnAsyncKeyReleased(const Enigma::IMessagePtr& m);

        //void OnEditorModeSelected(EditorModeSelectedMessage* msg);
    private:
        std::filesystem::path m_mapFileRootPath;

        //Enigma::MessageSubscriberPtr m_onAsyncKeyPressed;
        //Enigma::MessageSubscriberPtr m_onAsyncKeyReleased;

        //unsigned int m_bookedModeSelectedHandler;
    };
}

#endif // WORLD_EDIT_SERVICE_H
