/*
 *  mafResourcesDefinitions.h
 *  mafResources
 *
 *  Created by Roberto Mucci on 30/12/09.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFRESOURCESDEFINITIONS_H
#define MAFRESOURCESDEFINITIONS_H

#define degreesToRadiant(x) (M_PI * (x) / (double)180.0)
#define radiantToDegrees(x) ((x) * (double)180.0/M_PI)

// Includes list
#include "mafResources_global.h"
#include <qmath.h>
#include <QUrl>
#include <mafCoreDefinitions.h>
#include <mafEventBusManager.h>
#include <mafTopicRegistry.h>
#include <mafCoreSingletons.h>
#include <mafObject.h>
#include <mafHierarchy.h>
#include <mafProxyInterface.h>

#include "mafMatrix4x4.h"

namespace mafResources {

class mafDataSet;
class mafDataSetCollection;
class mafResource;
class mafVME;

/// Type definition for a list of mafResource.
typedef QList<mafResource *> mafResourceList;

/// Time varying data set map.
typedef QMap<double, mafDataSet *> mafDataSetMap;

/// List of VME used as input list for mafPipe.
typedef QList<mafVME *> mafVMEList;

/** Structure representing the information about a plug-in library.*/
struct mafPluginInfo {
    float m_Version;                ///< Plug-in Version
    QString m_PluginName;    ///< Plug-in name
    QString m_Author;           ///< Plug-in author
    QString m_Vendor;           ///< Plug-in company name
    QString m_VendorHomepage; ///< Plug-in company homepage
    QString m_Description;    ///< Plug-in general description.
};

///< Enumeration that define the Cartesian Axis.
enum {
    X_AXIS = 0,
    Y_AXIS,
    Z_AXIS
};

///< Enumeration that define the local/global behaviour of a pipe visual.
enum {
    LOCAL = 0,
    GLOBAL
};

/// Typedef to define the operation's status
typedef enum {
    mafOperationStatusIdle,
    mafOperationStatusStarted,
    mafOperationStatusExecuting,
    mafOperationStatusCanceled,
    mafOperationStatusAborted,
} mafOperationStatus;
    
/// Typedef to define the policy for creating/destroying the visual pipes
typedef enum {
    mafVisibilityPolicyKeepAlive,
    mafVisibilityPolicyDestroyOnHide,
    mafVisibilityPolicySmartMemory,
} mafVisibilityPolicy;

    
/// Typedef to define the status of a scenenode when used inside a view    
typedef enum {
    mafVisualizationStatusVisible,
    mafVisualizationStatusNotVisible,
    mafVisualizationStatusMutex,
} mafVisualizationStatus;

/// typedef to define the data type of the volume
typedef enum {
    mafVolUndefined,
    mafVolUnsignedChar,
    mafVolChar,
    mafVolUnsignedShort,
    mafVolShort,
    mafVolUnsignedInt,
    mafVolInt,
    mafVolFloat,
    mafVolDouble    
} mafDataType;

inline int getByteNum(mafDataType dataType) {
    int byteNum = 0;
    switch(dataType) {
        case mafVolUnsignedChar:
        case mafVolChar:
            byteNum = 1;
            break;
        case mafVolUnsignedShort:
        case mafVolShort:
            byteNum = 2;
            break;
        case mafVolUnsignedInt:
        case mafVolInt:
        case mafVolFloat:
            byteNum = 4;
            break;
        case mafVolDouble:
            byteNum = sizeof(double);
            break;
        default:
            qCritical("Unknown data type.\n");
    }
    return byteNum;
}
    
} // namespace mafResources

/// Vector of Operation Worker.
typedef QList<QPointer<QThread> > mafExecutionPool;
typedef mafExecutionPool * mafExecutionPoolPointer;

Q_DECLARE_METATYPE(mafExecutionPoolPointer);

#endif // MAFRESOURCESDEFINITIONS_H
