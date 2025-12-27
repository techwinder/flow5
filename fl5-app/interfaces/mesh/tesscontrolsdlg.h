/****************************************************************************

    flow5 application
    Copyright © 2025 André Deperrois
    
    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/


#pragma once

#include <QRadioButton>
#include <QStackedWidget>

#include <interfaces/widgets/customdlg/xfldialog.h>
#include <api/occmeshparams.h>
#include <api/gmshparams.h>
#include <interfaces/mesh/gmshctrlswt.h>
#include <interfaces/mesh/occtessctrlswt.h>

class TessControlsDlg : public XflDialog
{
    Q_OBJECT

    public:
        TessControlsDlg(QWidget *pParent);

        void initDialog(OccMeshParams const &occparams, GmshParams const &gmshparams, bool bShowBtnBox, bool bOcc);
        bool isChanged() const {return m_bChanged || m_pOccWt->isChanged() || m_pGmshWt->isChanged();}

        bool bOcc() const {return m_prbOcc->isChecked();}

        OccMeshParams const &occParameters() const {return m_pOccWt->params();}
        GmshParams gmshParameters() const {return m_pGmshWt->params();}


    private:
        void setupLayout();


    private slots:
        void onTessellatorChanged();

    private:
        bool m_bChanged;

        bool m_bOcc;

        QRadioButton *m_prbOcc, *m_prbGmsh;
        QStackedWidget *m_pStackedWt;

        GmshCtrlsWt *m_pGmshWt;
        OccTessCtrlsWt *m_pOccWt;


};

