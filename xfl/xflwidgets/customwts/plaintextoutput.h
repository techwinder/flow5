/****************************************************************************

    flow5
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


class PlainTextOutput : public QPlainTextEdit
{
    Q_OBJECT
    public:
        PlainTextOutput(QWidget *pWidget=nullptr);
        void setCharDimensions(int nHChar, int nVChar);

        void updateColors(bool bOpaque);


    public slots:
        void onAppendThisPlainText(QString const &sometext);

    protected:
        QSize sizeHint() const override;
        void showEvent(QShowEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

    private:
        int m_nHChar;
        int m_nVChar;

};

