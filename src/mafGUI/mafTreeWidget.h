/*
 *  mafTreeWidget.h
 *  mafGUI
 *
 *  Created by Daniele Giunchi on 26/10/10.
 *  Copyright 2010 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFTREEWIDGET_H
#define MAFTREEWIDGET_H

// Includes list
#include "mafGUIDefinitions.h"
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QAction>
#include <QContextMenuEvent>

namespace mafGUI {
/**
Class Name: mafTreeWidget
It represents the view in model-view pattern, and it visualize che model
stored in mafTreeModel.
*/
class MAFGUISHARED_EXPORT mafTreeWidget : public QTreeView {
    Q_OBJECT

public Q_SLOTS:
    //void refresh(void);

public:
    /// Object Constructor
    mafTreeWidget(QWidget *parent = 0);

protected:
    /// handle the drag & move event
    /*virtual*/ void dragMoveEvent(QDragMoveEvent *event);
    /*virtual*/ bool event(QEvent *event);
private:
    void contextMenuEvent(QContextMenuEvent *e);

    QAction *m_ActionShow;
    QAction *m_ActionDelete;

};

} //end namespace

#endif // MAFTREEWIDGET_H
