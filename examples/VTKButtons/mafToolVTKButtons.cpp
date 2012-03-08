/*
 *  mafToolVTKButtons.cpp
 *  VTKButtons
 *
 *  Created by Roberto Mucci on 13/01/12.
 *  Copyright 2012 B3C. All rights reserved.
 *
 *  See License at: http://tiny.cc/QXJ4D
 *
 */

#include "mafToolVTKButtons.h"
#include <mafSceneNodeVTK.h>
#include <QImage>
#include <QDir>
#include <mafVME.h>
#include <mafPoint.h>

#include "mafAnimateVTK.h"

#include <vtkSmartPointer.h>
#include <vtkAlgorithmOutput.h>
#include <vtkQImageToImageSource.h>
#include <mafDataSet.h>
#include <vtkTextProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

#include <vtkButtonWidget.h>
#include <vtkEllipticalButtonSource.h>
#include <vtkTexturedButtonRepresentation.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkBalloonRepresentation.h>

#include <vtkCommand.h>

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;
using namespace mafPluginVTK;


// Callback for the interaction
class vtkButtonCallback : public vtkCommand {
public:
    static vtkButtonCallback *New() { 
        return new vtkButtonCallback; 
    }

    virtual void Execute(vtkObject *caller, unsigned long, void*) {
        mafVTKWidget *widget = qobject_cast<mafPluginVTK::mafVTKWidget *>(this->graphicObject);
        mafAnimateVTK *animateCamera = mafNEW(mafPluginVTK::mafAnimateVTK);
        if (flyTo) {
            animateCamera->flyTo(widget, bounds, 200);
        } else {
            widget->renderer()->ResetCamera(bounds);
        }
        
        mafDEL(animateCamera);
    }

    void setBounds(mafBounds *b) {
        bounds[0] = b->xMin(); 
        bounds[1] = b->xMax();
        bounds[2] = b->yMin();
        bounds[3] = b->yMax();
        bounds[4] = b->zMin();
        bounds[5] = b->zMax();
    }

    void setFlyTo(bool fly) {
        flyTo = fly;
    }

    vtkButtonCallback():graphicObject(0), flyTo(true) {}
    QObject *graphicObject;
    double bounds[6];
    bool flyTo;
};

mafToolVTKButtons::mafToolVTKButtons(const QString code_location) : mafPluginVTK::mafToolVTK(code_location), m_ShowLabel(true), m_FlyTo(true), m_OnCenter(false) {

    bool loaded = false;
    VTK_CREATE(vtkTexturedButtonRepresentation2D, rep);
    rep->SetNumberOfStates(1);

    // Decomment this code to use a standard circle icon 
    // Create an image for the button
//     QImage image;
//     
//     QString pathStr = QDir::currentPath();
//     loaded = image.load(pathStr + "/" + "buttonIcon.png");
// 
//     if (!loaded) {
//         image.load(":/images/buttonIcon.png");
//     }
// 
//     VTK_CREATE(vtkQImageToImageSource, imageToVTK2);
//     imageToVTK2->SetQImage(&image);
//     imageToVTK2->Update();


    //rep->SetButtonTexture(0, imageToVTK2->GetOutput());
    myCallback = vtkButtonCallback::New();

    m_ButtonWidget = vtkButtonWidget::New();
    m_ButtonWidget->SetRepresentation(rep);
    m_ButtonWidget->AddObserver(vtkCommand::StateChangedEvent,myCallback);
}

mafToolVTKButtons::~mafToolVTKButtons() {
    m_ButtonWidget->Delete();
}

void mafToolVTKButtons::resetTool() {
    removeWidget(m_ButtonWidget);
}

void mafToolVTKButtons::graphicObjectInitialized() {
    // Graphic widget (render window, interactor...) has been created and initialized.
    // now can add the widget.
    addWidget(m_ButtonWidget);
}

void mafToolVTKButtons::updatePipe(double t) {
    mafVME *vme = input();
    if(vme == NULL) {
        return;
    }
    QString vmeName = vme->property("objectName").toString();

    double b[6];
    vme->bounds(b, t);

    mafObjectBase *obj = vme;
    //Get absolute pose matrix from mafVMEManager
    mafMatrixPointer absMatrix = NULL;
    mafEventArgumentsList argList;
    argList.append(mafEventArgument(mafCore::mafObjectBase *, obj));
    QGenericReturnArgument ret_val = mafEventReturnArgument(mafResources::mafMatrixPointer, absMatrix);
    mafEventBusManager::instance()->notifyEvent("maf.local.resources.vme.absolutePoseMatrix", mafEventTypeLocal, &argList, &ret_val);

    mafBounds *newBounds = new mafBounds(b);
    newBounds->transformBounds(absMatrix);

    vtkTexturedButtonRepresentation2D *rep = reinterpret_cast<vtkTexturedButtonRepresentation2D*>(m_ButtonWidget->GetRepresentation());

    QImage image;
    QString iconType = vme->property("iconType").toString();
    QString iconFileName = mafIconFromObjectType(iconType);
    image.load(iconFileName);
    VTK_CREATE(vtkQImageToImageSource, imageToVTK);
    imageToVTK->SetQImage(&image);
    imageToVTK->Update();
    rep->SetButtonTexture(0, imageToVTK->GetOutput());

    int size[2]; size[0] = 16; size[1] = 16;
    rep->GetBalloon()->SetImageSize(size);

    if (m_ShowLabel) {
        //Add a label to the button and change its text property
        rep->GetBalloon()->SetBalloonText(vmeName.toAscii());
        vtkTextProperty *textProp = rep->GetBalloon()->GetTextProperty();
        rep->GetBalloon()->SetPadding(2);
        textProp->SetFontSize(13);
        textProp->BoldOff();
        //textProp->SetColor(0.9,0.9,0.9);

        //Set label position
        rep->GetBalloon()->SetBalloonLayoutToImageLeft();
        

        //This method allows to set the label's background opacity
        rep->GetBalloon()->GetFrameProperty()->SetOpacity(0.65);
    } else {
        rep->GetBalloon()->SetBalloonText("");
    }

    //modify position of the vtkButton 
    double bds[3];
    if (m_OnCenter) {
        newBounds->center(bds);
    } else {
        //on the corner of the bounding box of the VME.
        bds[0] = newBounds->xMin();
        bds[1] = newBounds->yMin(); 
        bds[2] = newBounds->zMin();
    }

    rep->PlaceWidget(bds, size);
    rep->Modified();
    m_ButtonWidget->SetRepresentation(rep);

    myCallback->graphicObject = this->m_GraphicObject;
    myCallback->setBounds(newBounds);
    myCallback->setFlyTo(m_FlyTo);
    updatedGraphicObject();
}


