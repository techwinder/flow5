/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/

#pragma once

#include <QPainter>




void clearLayout(QLayout *pLayout);
void removeLayout(QWidget* pWidget);

void drawCheckBox(QPainter *painter, bool bChecked, QRect const & r, int side, bool bBackground, bool bContour, const QColor &crossclr, const QColor &backclr);
void drawCheckBox(QPainter *painter, Qt::CheckState state, QRect const & theRect, int side, bool bBackground, bool bContour, const QColor &crossclr,
                  const QColor &backclr, const QColor &ContourClr);

QString formatDouble(double d, int decimaldigits, bool bLocalize);

void setLayoutStyle(QLayout *pLayout, const QPalette &palette);
void setWidgetStyle(QWidget *pWidget, const QPalette &palette);
