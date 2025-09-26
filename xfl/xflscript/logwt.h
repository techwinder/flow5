/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QPushButton>
#include <QSettings>


class PlainTextOutput;

class LogWt : public QDialog
{
    Q_OBJECT

    public:
        LogWt(QWidget *pParent=nullptr);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void customEvent(QEvent *pEvent) override;

        QPushButton *ctrlButton() {return m_ppbButton;}

        QSize sizeHint() const override {return QSize(900,550);}

        void setCancelButton(bool bCancel);
        void setFinished(bool bFinished);

        static void loadSettings(QSettings &settings);
        static void saveSettings(QSettings &settings);

    public slots:
        void onOutputMessage(const QString &msg);

    private slots:
        void onCancelClose();

    private:
        void setupLayout();

        bool m_bFinished;

        PlainTextOutput *m_pptoLogView;
        QPushButton *m_ppbButton;

        static QByteArray s_Geometry;
};


