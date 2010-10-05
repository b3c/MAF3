/*
 *  mafLoggerFile.h
 *  mafCore
 *
 *  Created by Paolo Quadrani on 17/09/09.
 *  Copyright 2009 B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFLOGGERFILE_H
#define MAFLOGGERFILE_H

// Includes list
#include "mafLogger.h"
#include <QTemporaryFile>

namespace mafCore {

// Class forwarding list

/**
 Class name: mafLoggerFile
 This class defines the MAF3 logging class that will store messages into a file on filesystem.
 @sa maf Logger mafLoggerConsole mafLoggerBuffer mafMessageHandler
 */
class MAFCORESHARED_EXPORT mafLoggerFile : public mafLogger {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafCore::mafLogger);

public:
    /// Object constructor.
    mafLoggerFile(const mafString code_location = "");

    /// Object destructor.
    /*virtual*/ ~mafLoggerFile();

    /// Check if the object is equal to that passed as argument.
    /* virtual */ bool isEqual(const mafObjectBase *obj) const;

    /// Clear all the logged messages until now.
    /** This methods close the connection with the last temporary file and remove it, then open a new log file.
    This will update also the m_LastLogFile member variable.*/
    /*virtual*/ void clearLogHistory();

    /// Return the filename associated to the last session of logs.
    const mafString lastLogFile() const {return m_LastLogFile;}

protected:
    /// Method used to log the given message to the filesystem.
    /*virtual*/ void loggedMessage(const mafMsgType type, const mafString &msg);

    /// Clear history logs from the last temporary file.
    /** This method is used to close the connection with the last opened temporary file.
    It is invoked by the destructor and cleanLogHistory methods.*/
    void closeLastTempFile();

    /// Create a new temporary file reference.
    /** This method is called by the constructor and by the clearLogHistory methods
    to initialize a new temporary file.*/
    void initializeNewTemporaryFile();

private:
    QTemporaryFile *m_TempFileLog; ///< Temporary File containing all the logged messages for a specific session.
    mafString m_LastLogFile; ///< Filename of last logged file. Useful to retrieve information of file log when application cresh.
};

}

#endif // MAFLOGGERFILE_H
