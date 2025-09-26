/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once



#include <QDialog>



class PlainTextOutput;

class ObjectPropsDlg : public QDialog
{
    Q_OBJECT

    public:
        ObjectPropsDlg(QWidget *pParent);
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




