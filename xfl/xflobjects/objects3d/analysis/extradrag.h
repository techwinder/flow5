/****************************************************************************

    flow5 application
    Copyright (C) Andre Deperrois 
    All rights reserved.

*****************************************************************************/


#pragma once

#include <QString>

struct ExtraDrag
{
	ExtraDrag()
	{
		reset();
	}

	ExtraDrag(QString tag, double area, double coef)
	{
		m_Tag = tag;
		m_Area = area;
		m_Coef = coef;
	}

	void reset()
	{
		m_Tag.clear();
		m_Area = 0.0;
		m_Coef = 0.0;
	}

	void set(QString tag, double area, double coef)
	{
		m_Tag = tag;
		m_Area = area;
		m_Coef = coef;
	}

	QString tag()   const {return m_Tag;}
	QString name() const {return m_Tag;}
	double area()  const {return m_Area;}
	double coef()  const {return m_Coef;}

    void setTag(QString tag)   {m_Tag=tag;}
    void setName(QString name) {m_Tag=name;}
    void setArea(double area)  {m_Area=area;}
	void setCoef(double coef)  {m_Coef=coef;}


	QString m_Tag; /** The array of extra drag tags */
	double m_Area; /** The array of extra drag area */
	double m_Coef; /** The array of extra drag coefficient */
};

