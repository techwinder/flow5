/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once


#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>


class XFoilSettings;
class FloatEdit;
class IntEdit;


class Analysis2dSettings : public QDialog
{
    Q_OBJECT

    public:
        Analysis2dSettings(QWidget *parent);

        void keyPressEvent(QKeyEvent *pEvent) override;
        void reject() override;

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

        void initWidget();
        void readData();

    private:
        void setupLayout();

    private slots:
        void onButton(QAbstractButton *pButton);

    private:
        QCheckBox *m_pchInitBL, *m_pchFullReport, *m_pchKeepErrorsOpen;
        IntEdit *m_pieIterLimit;
        FloatEdit * m_pdeVAccel;
        FloatEdit *m_pfeCdError;

        QDialogButtonBox *m_pButtonBox;

        static QByteArray s_WindowGeometry;
};
