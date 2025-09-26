/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QSettings>

class ExtraDragWt;
class WPolar;


class ExtraDragDlg : public QDialog
{
    Q_OBJECT
    public:
        ExtraDragDlg(QWidget *pParent);

        void initDialog(const WPolar *pWPolar);
        void setExtraDragData(WPolar *pWPolar);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        QSize sizeHint() const override {return QSize(750,550);}

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    private:
        void setupLayout();

    private slots:
        void onButton(QAbstractButton*pButton);

    private:
        QDialogButtonBox *m_pButtonBox;

        ExtraDragWt *m_pExtraDragWt;

        static QByteArray s_Geometry;

};


