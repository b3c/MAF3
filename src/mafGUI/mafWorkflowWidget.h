/*
 *  mafWorkflowWidget.h
 *  mafGUI
 *
 *  Created by Roberto Mucci on 07/07/11.
 *  Copyright 2011 B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFFINDWIDGET_H
#define MAFFINDWIDGET_H

// Includes list
#include "mafGUIDefinitions.h"
#include <QAction>
#include <QListWidget>
#include <QPushButton>

namespace mafGUI {
/**
Class Name: mafWorkflowWidget
It represents a widget to search words inside a text.
*/
class MAFGUISHARED_EXPORT mafWorkflowWidget : public QWidget {
    Q_OBJECT
public:
    /// Object Constructor
    mafWorkflowWidget(QWidget *parent = 0);
    QPushButton *m_OpenWorkbenchButton; ///< Open external workbench (Taverna).
    QPushButton *m_SubmitButton; ///< Submit workflow.
    QPushButton *m_RemoveButton; ///< Remove workflow.
    QListWidget *m_WorkflowList; ///< List of available workflow.


    QAction *m_ActionCase; ///< Match case menu.
    QAction *m_ActionWhole; ///< Match whole word menu.

};

} //end namespace

#endif // MAFFINDWIDGET_H
