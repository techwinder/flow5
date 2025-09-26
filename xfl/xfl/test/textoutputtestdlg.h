/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QRadioButton>
#include <QDialogButtonBox>

class PlainTextOutput;

class TextOutputTestDlg : public QDialog
{
    Q_OBJECT
    public:
        TextOutputTestDlg(QWidget *pParent);


    private slots:
        void onButton(QAbstractButton *pButton);
        void toPlainText();
        void toMarkDown();
        void toHTML();

        void onOutputFont();

    private:
        void setupLayout();
        QSize sizeHint() const override {return QSize(700,900);}

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;

    private:
        QDialogButtonBox *m_pButtonBox;
        PlainTextOutput *m_pptInput;
        QTextEdit* m_pteOutput;

        QPushButton *m_ppbPlain, *m_ppbMarkDown, *m_ppbHtml;

        QRadioButton *m_prbTextFnt, *m_prbTableFnt, *m_prbTreeFnt;

        static QByteArray s_Geometry;
};

