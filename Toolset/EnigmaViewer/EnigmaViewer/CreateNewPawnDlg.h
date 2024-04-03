﻿/*****************************************************************
 * \file   CreateNewPawnDlg.h
 * \brief
 *
 * \author Lancelot 'Robin' Chen
 * \date   March 2024
 ******************************************************************/
#ifndef CREATE_NEW_PAWN_DLG_H
#define CREATE_NEW_PAWN_DLG_H

#include <nana/gui/widgets/form.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>

namespace EnigmaViewer
{
    class CreateNewPawnDlg : public nana::form
    {
    public:
        CreateNewPawnDlg(nana::window owner);
        virtual ~CreateNewPawnDlg();

        void onOkButton(const nana::arg_click& arg);
        void onCancelButton(const nana::arg_click& arg);

    private:
        nana::button* m_okButton;
        nana::button* m_cancelButton;
        nana::label* m_namePrompt;
        nana::textbox* m_nameInputBox;
        nana::label* m_modelSelectPrompt;
        nana::combox* m_modelSelectBox;
    };
}

#endif // CREATE_NEW_PAWN_DLG_H