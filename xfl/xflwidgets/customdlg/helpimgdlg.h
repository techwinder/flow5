/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>

class HelpImgDlg : public QDialog
{
    Q_OBJECT
    public:
        HelpImgDlg(QString const &imagepath, QWidget *pParent);
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;

//        QSize sizeHint() const override; // ignored?

    private:
        QLabel *m_pHelpLab;
        QPixmap m_Image;
        static QByteArray s_Geometry;
};

