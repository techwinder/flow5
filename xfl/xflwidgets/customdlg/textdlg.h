/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QDialog>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QKeyEvent>


class TextDlg : public QDialog
{
    Q_OBJECT

    public:
        TextDlg(QString const &text, QWidget *pParent=nullptr);
        void setQuestion(QString const &quest) {m_plabQuestion->setText(quest);}
        QString const &newText() const {return m_NewText;}

    private:
        void setupLayout();

        void keyPressEvent(QKeyEvent *pEvent) override;
        QSize sizeHint() const override {return  QSize(1000,750);}
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private slots:
        void accept() override;
        void onButton(QAbstractButton*pButton);

    private:
        QDialogButtonBox *m_pButtonBox;
        QLabel *m_plabQuestion;
        QPlainTextEdit *m_ppteText;
        QString m_NewText;
        static QByteArray s_WindowGeometry;
};


