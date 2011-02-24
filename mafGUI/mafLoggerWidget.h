/*
 *  mafLoggerWidget.h
 *  mafGUI
 *
 *  Created by Paolo Quadrani on 26/10/10.
 *  Copyright 2010 B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFLOGGERWIDGET_H
#define MAFLOGGERWIDGET_H

#include "mafGUIDefinitions.h"
#include "mafTextEditWidget.h"


namespace mafGUI {

//forward class
class mafTextHighlighter;

/**
 Class name: mafLoggerWidget
 This class defines the MAF3 widget for logging.
 @sa mafLogger mafLoggerConsole mafLoggerFile mafMessageHandler
 */
class MAFGUISHARED_EXPORT mafLoggerWidget : public mafCore::mafLogger {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafCore::mafLogger);
public:
    /// Object constructor.
    mafLoggerWidget(const mafString code_location = "");

    /// Check if the object is equal to that passed as argument.
    /* virtual */ bool isEqual(const mafObjectBase *obj) const;

    /// return the text widget log.
    mafTextEditWidget *textWidgetLog();

    /// Clear all the logged messages until now.
    /*virtual*/ void clearLogHistory();

protected:
    /// Object destructor.
    /* virtual */ ~mafLoggerWidget();

    /// Method used to log the given message to the buffer.
    /*virtual*/ void loggedMessage(const mafMsgType type, const mafString &msg);

private:
    mafTextEditWidget *m_TextWidgetLog; ///< String containing all the logged messages for a specific session.
    mafTextHighlighter *m_TextHighlighter;
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline mafTextEditWidget *mafLoggerWidget::textWidgetLog() {
    return m_TextWidgetLog;
}

} // end namespace

#endif // MAFLOGGERWIDGET_H
