/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>


class XflDialog : public QDialog
{
    Q_OBJECT
    public:
        XflDialog(QWidget *pParent);

        bool bChanged() const {return m_bChanged;}
        bool bDescriptionChanged() const {return m_bDescriptionChanged;}

    protected slots:
        void reject() override;
        virtual void keyPressEvent(QKeyEvent *pEvent) override;
        virtual void onButton(QAbstractButton*pButton);
        virtual void onApply() {} // base class method does nothing
        virtual void onReset() {} // base class method does nothing
        virtual void onDefaults() {} // base class method does nothing


    protected:
        void setButtons(QDialogButtonBox::StandardButtons buttons);

    protected:
        QDialogButtonBox *m_pButtonBox;

        bool m_bChanged;
        bool m_bDescriptionChanged;
};


