/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois 
    
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



#include <QDialog>



class PlainTextOutput;

class ObjectPropsDlg : public QDialog
{
    Q_OBJECT

    public:
        ObjectPropsDlg(QWidget *pParent);
        void initDialog(const std::string &title, const std::string &props);
        void initDialog(const QString &title, const QString &props);

        QSize sizeHint() const override {return QSize(700,500);}

        static void setWindowGeometry(QByteArray geom) {s_Geometry=geom;}
        static QByteArray windowGeometry() {return s_Geometry;}

    protected:
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:
        void setupLayout();

        PlainTextOutput *m_ppto;

        static QByteArray s_Geometry;
};




