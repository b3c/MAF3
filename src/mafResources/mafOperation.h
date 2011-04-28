/*
 *  mafOperation.h
 *  mafResources
 *
 *  Created by Paolo Quadrani on 30/12/09.
 *  Copyright 2009 B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFOPERATION_H
#define MAFOPERATION_H

#include "mafResource.h"

namespace mafResources {

// Class forwarding list

/// This class provides basic API for building elaboration algorithms called mafOperation.
/**
Class name: mafOperation
This class provides basic API for building elaboration algorithms for MAF framework.
An operation takes as input one or more mafVMEs and generate as output a mafVME. The algorithm executed on the input data
is provided by a mafPipeData. The mafOperation should be implemented to be able to perform an undo and be able to abort its execution. Operation's execution is managed by the mafOperationManager and is run on a separate thread.
 
 @sa mafOperationManager.
*/
class MAFRESOURCESSHARED_EXPORT mafOperation : public mafResource {
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning)
    Q_PROPERTY(bool canAbort READ canAbort)

    /// typedef macro.
    mafSuperclassMacro(mafResources::mafResource);

signals:
    /// Trigger the undo execution.
    void undoExecution();

public slots:
    
    /// Set parameters of operation.
    virtual void setParameters(QVariantList parameters);

    /// Allows to call the piece of algorithm that is needed to restore the previous state of the operation's execution.
    virtual void unDo();

    /// Allows to call the piece of algorithm that is needed to apply the operation again.
    virtual void reDo();

    /// Terminate the execution by resetting the m_IsRunning at false.
    void terminate();

private slots:
    /// Terminate the execution.
    void abort();
    
public:
    /// Object constructor.
    mafOperation(const QString code_location = "");
    
    /// Return true or false according to the unDo ability of the operation.
    bool canUnDo() const;
    
    /// check if the operation is running.
    bool isRunning() const;
    
    /// Initialize the operation. Put here the initialization of operation's parameters.
    virtual bool initialize();
    
    /// Return the abort capability of the operation.
    bool canAbort() const;
    
    /// Return the status of the input preserve flag.
    bool isInputPreserve() const;
    
protected:
    /// Virtual method to implement the cleanup of the operation when it ends.
    virtual void terminated() = 0;
    
    /// Object destructor.
    /* virtual */ ~mafOperation();

    volatile mafOperationStatus m_Status; ///< Operation status flag.
    bool m_CanUnDo; ///< Flag that store the unDo capability of the operation.
    bool m_CanAbort;         ///< Flag indicating that the operation can abort its execution or no (default true).
    bool m_InputPreserve;  ///< Flag represnting the behavior of the operationabout the input data. True value means that the input data is not modified (default true).

};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////
inline bool mafOperation::isRunning() const {
    return m_Status == EXECUTING;
}

inline bool mafOperation::canUnDo() const {
    return m_CanUnDo;
}

inline bool mafOperation::canAbort() const {
    return m_CanAbort;
}

inline bool mafOperation::isInputPreserve() const {
    return m_InputPreserve;
}

} // namespace mafResources

#endif // MAFOPERATION_H
