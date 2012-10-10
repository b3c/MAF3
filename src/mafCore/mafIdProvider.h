/*
 *  mafIdProvider.h
 *  mafCore
 *
 *  Created by Paolo Quadrani on 27/03/09.
 *  Copyright 2009 SCS-B3C. All rights reserved.
 *  
 *  See Licence at: http://tiny.cc/QXJ4D
 *  
 */

#ifndef MAFIDPROVIDER_H
#define MAFIDPROVIDER_H

// Includes list
#include "mafCore_global.h"

namespace mafCore {

/**
 Class name: mafIdProvider
 This singletone provides the generation of uniques ID used for objects or events.
 */
class MAFCORESHARED_EXPORT mafIdProvider {
public:
    /// Return an instance of the provider
    static mafIdProvider *instance();

    /// Create a new ID to be used for object or events.
    /** Return next valid Id to assign to an object or to use for an event.
    By default, if no name is given to the ID, a OBJECT_ID name is assigned.
    If you try to create a new ID by using an existing name, the previous defined ID is returned and no new ID is created.*/
    mafId createNewId(const QString id_name = "");

    /// Allow to remove a previously created Id.
    bool removeId(const QString id_name);

    /// Change (or allows to customize) the name associated to the ID. Return true on success, otherwise false.
    bool setIdName(const mafId id, const QString id_name);

    /// Return the ID name corresponding to the value.
    /** The function returns the string associated with the numeric ID. If the passed value doesn't exist, an empty string is returned.*/
    const QString idName(const mafId id_value) const;

    /// Return the ID value given to the id_name
    /** The function returns the numeric ID associated with the string passed as argument. If the string name is not present, -1 is returned.*/
    mafId idValue(const QString id_name) const;

    /// Destroy the singleton instance. To be called at the end of the application.
    void shutdown();

private:
    /// Object constructor.
    mafIdProvider();

    /// Object destructor.
    ~mafIdProvider();

    /// Types definitions (to be more readable).
    typedef QHash<mafId, QString> mafIdHashType;

    mafIdHashType m_IdHash; ///< Hash that store the associations between IDs and corresponding strings.

    mafId m_Id; ///< Current ID generated by the ID Provider.
};

} // mafCore

#endif // MAFIDPROVIDER_H
