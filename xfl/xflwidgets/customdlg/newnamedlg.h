/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>

#include <QKeyEvent>



class NewNameDlg : public QDialog
{
    Q_OBJECT

    public:
        NewNameDlg(QString const &name, QWidget *pParent=nullptr);


        void setQuestion(QString const &quest) {m_plabQuestion->setText(quest);}
        QString const &newName() const {return m_NewName;}

    private:
        void setupLayout();
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private slots:
        void accept() override;
        void onButton(QAbstractButton*pButton);

    private:
        QDialogButtonBox *m_pButtonBox;
        QLabel *m_plabQuestion;
        QLineEdit *m_pleName;
        QString m_NewName;

        static QByteArray s_WindowGeometry;
};



