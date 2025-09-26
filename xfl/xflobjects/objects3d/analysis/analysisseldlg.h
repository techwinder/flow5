/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>

#include <xflcore/fontstruct.h>
class Foil;
class Plane;
class Boat;
class Polar;
class Polar3d;

class ExpandableTreeView;
class ObjectTreeDelegate;
class ObjectTreeModel;

class AnalysisSelDlg : public QDialog
{
    Q_OBJECT
    public:
        enum enumObject {FOIL, PLANE, BOAT};

    public:
        AnalysisSelDlg(QWidget *parent = nullptr);
        void initDialog(const Foil *pCurFoil, const Plane *pCurPlane, const Boat *pCurBoat);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void accept() override;
        QSize sizeHint() const override {return QSize(450, 550);}

        QVector<Polar const*> selected2dPolars() {return m_Selected2dPolars;}
        Polar const* selected2dPolar(int ip) {if(ip<0||ip>=m_Selected2dPolars.size()) return nullptr; else return m_Selected2dPolars.at(ip);}

        QVector<Polar3d const*> selected3dPolars() {return m_Selected3dPolars;}
        Polar3d const* selectedPolar3d(int ip) {if(ip<0||ip>=m_Selected3dPolars.size()) return nullptr; else return m_Selected3dPolars.at(ip);}

    private:
        void setupLayout();

    private slots:
        void onButton(QAbstractButton *pButton);

    private:
        QDialogButtonBox *m_pButtonBox;

        QLabel *m_plabTitle;

        ExpandableTreeView *m_pStruct;
        ObjectTreeModel *m_pModel;
        ObjectTreeDelegate *m_pDelegate;

        QVector<Polar const*> m_Selected2dPolars;
        QVector<Polar3d const*> m_Selected3dPolars;

        enumObject m_Object;

        static QByteArray s_Geometry;

};


