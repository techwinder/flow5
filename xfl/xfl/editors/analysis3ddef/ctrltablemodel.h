/****************************************************************************

	flow5 application

	Copyright (C) Andre Deperrois 

	All rights reserved.

*****************************************************************************/

#pragma once


#include <QStandardItemModel>

class CtrlTableModel: public QStandardItemModel
{
public:
    CtrlTableModel(QObject * parent=NULL) : QStandardItemModel(parent)  { }

    Qt::ItemFlags flags(const QModelIndex & index) const
    {
	    if (index.column() == 0)
		    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	    else
		    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
};
