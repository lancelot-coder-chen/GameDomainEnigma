﻿/*********************************************************************
 * \file   CreateNewWorldDlg.h
 * \brief  
 * 
 * \author Lancelot 'Robin' Chen
 * \date   July 2023
 *********************************************************************/
#ifndef CREATE_NEW_WORLD_DLG_H
#define CREATE_NEW_WORLD_DLG_H

#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/label.hpp>

namespace LevelEditor
{
    class WorldEditConsole;
    class CreateNewWorldDlg : public nana::form
    {
    public:
        CreateNewWorldDlg(nana::window owner, const std::shared_ptr<WorldEditConsole>& world_editor);
        virtual ~CreateNewWorldDlg();

        void OnOkButton(const nana::arg_click& arg);
        void OnCancelButton(const nana::arg_click& arg);
    private:
        nana::button* m_okButton;
        nana::button* m_cancelButton;
        nana::label* m_namePrompt;
        nana::textbox* m_nameInputBox;
        nana::label* m_folderNamePrompt;
        nana::textbox* m_folderNameInputBox;

        std::weak_ptr<WorldEditConsole> m_worldEditor;
    };
}

#endif // CREATE_NEW_WORLD_DLG_H
