/*
 *  mafDataSetCollection.h
 *  mafResources
 *
 *  Created by Paolo Quadrani on 30/12/09.
 *  Copyright 2009 B3C. All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#ifndef MAFDATASETCOLLECTION_H
#define MAFDATASETCOLLECTION_H

// Includes list
#include "mafResourcesDefinitions.h"

namespace mafResources {

// Class forwarding list
class mafInterpolator;

/**
 Class name: mafDataSetCollection
 This is the class representing the time varying data for MAF3.
 */
class MAFRESOURCESSHARED_EXPORT mafDataSetCollection : public mafCore::mafObject {
    Q_OBJECT
    /// typedef macro.
    mafSuperclassMacro(mafCore::mafObjectBase);

public:
     /// Object constructor.
    mafDataSetCollection(const mafString code_location = "");

    /// Set the current timestamp for the collection.
    void setTimestamp(double t);

    /// Return the current timestamp assigned to the collection.
    double timeStamp();

    /// Set the pose for the current data.
    /** The resulting matrix is obtained from the Matrix multiplication of the following 3 matrices (based on Yaw-Pitch-Roll representation):
    Pose = Rz * Ry * Rx where the latters are the rotation matrices generated by the angles (degrees) of rotation around the axes Z, Y and X.*/
    void setPose(double rx, double ry, double rz, double x = 0.0, double y = 0.0, double z = 0.0, double t = -1);

    /// Assign the pose matrix to the current data.
    void setPose(const mafPoseMatrix matrix, double t = -1);

    /// Return the currend data pose matrix.
    mafPoseMatrix *poseMatrix(double t = -1);

    /// Set the orientation for the data at the timestamp 't'
    /** The input angle are expressed in degrees and are the rotation around the main axis (Yaw-Pitch_Roll convention).*/
    void setOrientation(double rx, double ry, double rz, double t = -1);

    /// Return the orientation angles (degrees) of rotations around X, Y and Z axes.
    void orientations(double ori[3], double t = -1);

    /// Set the position for the current data.
    void setPosition(double pos[3], double t = -1);

    /// Set the position for the current data.
    void setPosition(double x, double y, double z, double t = -1);

    /// Return the position of the current data
    void position(double pos[3], double t = -1);

    /// Add a new item and the time 't' to the collection of the time varying Posed Items, return true on success.
    /** The memory associated to new item added is removed by the VME itself. The user has to remove the allocated
    memory for the dataset only if the insert operation fails.*/
    bool insertItem(mafDataSet *item, double t = -1);

    /// Remove the given item from the collection.
    /** This method remove the reference to the mafDataSet item from the mafDataSetMap and delete its allocated memory.
    If 'keep_alive' flag is true, the collection doesn't destroy the item; this can occour when the method is called from
    the 'itemDestroyed' slot where the item has already been destroted from someone else outside the collection.*/
    bool removeItem(mafDataSet *item, bool keep_alive = false);

    /// Return the data item at the given time based on the current interpolation mechanism assigned.
    mafDataSet *itemAt(double t);

    /// Return the item at the current time.
    mafDataSet *itemAtCurrentTime();

    /// Set the active interpolator type to be instantiated when needed by the VME itself.
    void setInterpolator(const mafString &interpolator_type);

    /// Set the active interpolator by passing an instance of it.
    /** The instance of the interpolator will be destroyed by the collection itself when
    another interpolator is assigned or th collection destruction.*/
    void setInterpolator(mafInterpolator *interpolator);

    /// Set the dataset at the given timestamp. Return true on success.
    /** If the timestamp 't' is given and the collection does not contain such timestamp, the
    mafDataSet will be assigned to that one returned by the interpolation mechanism. The memory allocated
    for the dataset is removed from the VME when is destructed. The user has to remove the dataset memory only
    if the operation fails. Anyway, if you want to insert new data items at new timestamp please refear to the 'insertItem' method.*/
    bool setDataSet(mafDataSet *data, double t = -1);

    /// Return the collection map.
    const mafDataSetMap *collectionMap() const;

private slots:
    /// Method callen when an item has been destroyed
    void itemDestroyed();

protected:
    /// Object destructor.
    /* virtual */ ~mafDataSetCollection();

    /// Define the checks to accept incoming mafDataSets.
    /** This method allows to check that input mafDataSet is of the same type of that one previously inserted.
    If the given dataset if the first one, then it initialize the data 'type' and future data accepted.*/
    bool acceptData(mafDataSet *data);

    /// Write into the given matrix the given orientation.
    void writeOrientation(double rx, double ry, double rz, mafPoseMatrix *m);

    /// Write into the given matrix the given position.
    void writePosition(double x, double y, double z, mafPoseMatrix *m);

private:
    mafDataSetMap *m_CollectionMap; ///< Collection of data.
    mafInterpolator *m_Interpolator; ///< Interpolator used to find items given the timestamp given the type of interponation mechanism used.
    mafString m_DataTypeAccepted; ///< Variable that store the type of incoming mafDataSet.
    double m_CurrentTimestamp; ///< Variable that store the current timestamp.
};

/////////////////////////////////////////////////////////////
// Inline methods
/////////////////////////////////////////////////////////////

inline double mafDataSetCollection::timeStamp() {
    return m_CurrentTimestamp;
}

inline const mafDataSetMap *mafDataSetCollection::collectionMap() const {
    return m_CollectionMap;
}

} // namespace mafResources

#endif // MAFDATASETCOLLECTION_H
