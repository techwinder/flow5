/****************************************************************************

    flow5 application
    Copyright (C) 2025 André Deperrois

    This file is part of flow5.

    flow5 is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    flow5 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with flow5.
    If not, see <https://www.gnu.org/licenses/>.


*****************************************************************************/

#pragma once

#include <QSettings>
#include <QApplication>
#include <QPlainTextEdit>


class FormatTextOutput : public QTextEdit
{
    Q_OBJECT
    public:
        FormatTextOutput(QWidget *pWidget=nullptr);
        void setCharDimensions(int nHChar, int nVChar);

        void updateColors(bool bOpaque);

        void appendPlainText(QString const &text);
        void appendHtmlText(QString const &text);
        void appendEOL(int n=1);

        void setStdText(std::string const &text) {setPlainText(QString::fromStdString(text));}

    public slots:
        void onAppendStdText(std::string const &sometext) {onAppendQText(QString::fromStdString(sometext));}
        void onAppendQText(QString const &sometext);
        void onCopyAll();

    protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override {return QSize(100,50);}
        void showEvent(QShowEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

    private:
        int m_nHChar;
        int m_nVChar;
};


