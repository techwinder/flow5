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
        void onOutputStdMessage(const std::string &msg) {onOutputMessage(QString::fromStdString(msg));}
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


