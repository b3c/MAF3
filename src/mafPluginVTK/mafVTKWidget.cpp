/*
 *  mafVTKWidget.cpp
 *  mafPluginVTK
 *
 *  Created by Roberto Mucci on 12/10/10.
 *  Copyright 2009 B3C.s All rights reserved.
 *
 *  See Licence at: http://tiny.cc/QXJ4D
 *
 */

#include "mafVTKWidget.h"
#include "mafAxes.h"

#include <QInputEvent>
#include <mafProxyInterface.h>
#include <mafInteractionManager.h>

#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkAssemblyPath.h>
#include <vtkCellPicker.h>
#include <vtkSmartPointer.h>
#include <vtkProp.h>

using namespace mafCore;
using namespace mafEventBus;
using namespace mafResources;
using namespace mafPluginVTK;

mafVTKWidget::mafVTKWidget(QWidget* parent, Qt::WFlags f) : QVTKWidget(parent, f), m_Axes(NULL), m_RendererBase(NULL), m_RendererTool(NULL) {
    initializeConnections();
    vtkRenderWindow *renWin = GetRenderWindow();
    // Set the number of layers for the render window.
    renWin->SetNumberOfLayers(2);

    // Layer in which draw the 3D objects
    m_RendererBase = createLayer("base");
    m_RendererTool = createLayer("tool");
}

mafVTKWidget::~mafVTKWidget() {
    QHash<QString, vtkRenderer*>::iterator iter;
    for (iter = m_LayerHash.begin(); iter != m_LayerHash.end(); iter++) {
        iter.value()->Delete();
    }
    if (m_Axes != NULL) {
        delete m_Axes;
        m_Axes = NULL;
    }
}

void mafVTKWidget::initializeConnections() {
    bool result(false);
    result = connect(this, SIGNAL(mousePressSignal(double *, unsigned long, mafCore::mafProxyInterface *, QEvent *)), mafInteractionManager::instance(), SLOT(mousePress(double *, unsigned long, mafCore::mafProxyInterface *, QEvent *)));
    result = connect(this, SIGNAL(mouseReleaseSignal(double *, unsigned long, mafCore::mafProxyInterface *, QEvent *)), mafInteractionManager::instance(), SLOT(mouseRelease(double *, unsigned long, mafCore::mafProxyInterface *, QEvent *)));
    result= connect(this, SIGNAL(mouseMoveSignal(double *, unsigned long, mafCore::mafProxyInterface *, QEvent *)), mafInteractionManager::instance(), SLOT(mouseMove(double *, unsigned long, mafCore::mafProxyInterface *, QEvent *)));
}

vtkRenderer *mafVTKWidget::createLayer(const QString layerName) {
    vtkRenderer *renderer = m_LayerHash.value(layerName, NULL);
    if (m_LayerHash.keys().indexOf(layerName) == -1) {
        // New layer
        // Update the number of layers of the render window.
        unsigned int numLayers = m_LayerHash.size();
        vtkRenderWindow *renWin = GetRenderWindow();
        renWin->SetNumberOfLayers(numLayers + 1);
        // Create the renderer associated with the given layer's name
        renderer = vtkRenderer::New();
        renderer->SetLayer(numLayers);
        renderer->SetInteractive(1);
        // Link the camera to that one present into the base renderer (if available)
        if (m_RendererBase != NULL) {
            renderer->SetActiveCamera(m_RendererBase->GetActiveCamera());
        }
        // Add the new renderer to the render window
        renWin->AddRenderer(renderer);
        // ... and to the layer hash.
        m_LayerHash.insert(layerName, renderer);
    }
    return renderer;
}

bool mafVTKWidget::deleteLayer(const QString layerName) {
    if (layerName == "base" || layerName == "tool") {
        return false;
    }
    
    // Retrieve the renderer associated to the layer. Remove all the view props and delete the renderer.
    vtkRenderer *renderer = m_LayerHash.value(layerName);
    renderer->RemoveAllViewProps();
    renderer->Delete();
    // Remove the layer from the hash
    int n = m_LayerHash.remove(layerName);
    // and update the number of layers.
    GetRenderWindow()->SetNumberOfLayers(m_LayerHash.size());
    return n == 1;
}

void mafVTKWidget::showLayer(const QString layerName, bool show) {
    vtkRenderer *renderer = m_LayerHash.value(layerName, NULL);
    if (renderer) {
        vtkPropCollection *propCollection = renderer->GetViewProps();
        int n = propCollection->GetNumberOfItems();
        int i = 0;
        for (; i < n; ++i) {
            vtkProp3D *prop = vtkProp3D::SafeDownCast(propCollection->GetItemAsObject(i));
            if (prop && prop->GetVisibility() != show) {
                prop->SetVisibility(show ? 1 : 0);
            }
        }
    }
}

void mafVTKWidget::moveLayerTo(const QString layerName, unsigned int layerLevel) {
    vtkRenderer *renderer = m_LayerHash.value(layerName, NULL);
    if (renderer) {
        int n = m_LayerHash.size();
        if (layerLevel >= n) {
            qWarning() << layerLevel << mafTr(" Layer level should be >= 0 and < ") << n;
            return;
        }
        renderer->SetLayer(layerLevel);
    }
}

void mafVTKWidget::showAxes(bool show) {
    m_Axes = new mafAxes(m_RendererTool);
    m_Axes->setVisibility(show);
}

void mafVTKWidget::mousePressEvent(QMouseEvent* e) {
    vtkRenderWindowInteractor* iren = NULL;
    if(this->mRenWin) {
        iren = this->mRenWin->GetInteractor();
    }

    if(!iren || !iren->GetEnabled()) {
        return;
    }

    // give interactor the event information
    iren->SetEventInformationFlipY(e->x(), e->y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0,
                                   0,
                                   e->type() == QEvent::MouseButtonDblClick ? 1 : 0);

    this->getModifiers(iren);

    // Check if a VME has been picked
    this->mousePress(iren, e);

}

void mafVTKWidget::mouseReleaseEvent(QMouseEvent* e) {
    vtkRenderWindowInteractor* iren = NULL;
    if(this->mRenWin) {
        iren = this->mRenWin->GetInteractor();
    }

    if(!iren || !iren->GetEnabled()) {
        return;
    }

    // give vtk event information
    iren->SetEventInformationFlipY(e->x(), e->y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);

    this->getModifiers(iren);

    // Check if a VME has been picked
    this->mouseRelease(iren, e);

}

void mafVTKWidget::mouseMoveEvent(QMouseEvent* e) {
    vtkRenderWindowInteractor* iren = NULL;
    if(this->mRenWin) {
        iren = this->mRenWin->GetInteractor();
    }
    
    if(!iren || !iren->GetEnabled()) {
        return;
    }
    
    // give interactor the event information
    iren->SetEventInformationFlipY(e->x(), e->y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);
    
    this->getModifiers(iren);
    
    // Check if a VME has been picked
    this->mouseMove(iren, e);
}

void mafVTKWidget::wheelEvent(QWheelEvent* e) {
    vtkRenderWindowInteractor* iren = NULL;
    if(this->mRenWin) {
        iren = this->mRenWin->GetInteractor();
    }

    if(!iren || !iren->GetEnabled()) {
        return;
    }

    // VTK supports wheel mouse events only in version 4.5 or greater
    // give event information to interactor
    iren->SetEventInformationFlipY(e->x(), e->y(),
                                   (e->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0,
                                   (e->modifiers() & Qt::ShiftModifier ) > 0 ? 1 : 0);

    this->getModifiers(iren);
    mafEventArgumentsList argList;
    argList.append(mafEventArgument(unsigned long, m_Modifiers));

    //wheel event signal need to be created

    // invoke VTK event
    // if delta is positive, it is a forward wheel event
    if(e->delta() > 0) {
        iren->InvokeEvent(vtkCommand::MouseWheelForwardEvent, e); //Move into InteractorManager?
    } else {
        iren->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, e); //Move into InteractorManager?
    }
}

void mafVTKWidget::getModifiers(vtkRenderWindowInteractor* iren) {
    //check "shift" modifier.
    int flag = 1 << MAF_SHIFT_KEY;
    if (iren->GetShiftKey()) {
        m_Modifiers = m_Modifiers|flag;
    } else {
        m_Modifiers = m_Modifiers&(~flag);
    }

    //check "ctrl" modifier.
    flag = 1 << MAF_CTRL_KEY;
    if (iren->GetControlKey()) {
        m_Modifiers = m_Modifiers|flag;
    } else {
        m_Modifiers = m_Modifiers&(~flag);
    }

    //check "alt" modifier.
    flag = 1 << MAF_ALT_KEY;
    if (iren->GetAltKey()) {
        m_Modifiers = m_Modifiers|flag;
    } else {
        m_Modifiers = m_Modifiers&(~flag);
    }
}

void mafVTKWidget::pickProp(vtkRenderWindowInteractor* iren, QEvent *e, mafCore::mafProxyInterface *proxy, double *posPicked) {
    int mousePosX = 0;
    int mousePosY = 0;
    vtkProp *actor = NULL;
    mafCore::mafProxy<vtkProp> *actorPicked = mafProxyPointerTypeCast(vtkProp, proxy);
    
    iren->GetEventPosition(mousePosX, mousePosY);
    vtkSmartPointer<vtkCellPicker> cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    cellPicker->SetTolerance(0.001);
    vtkRendererCollection *rc = iren->GetRenderWindow()->GetRenderers();
    vtkRenderer *r = NULL;
    rc->InitTraversal();
    while(r = rc->GetNextItem()) {
        int picked = cellPicker->Pick(mousePosX,mousePosY,0,r);
        if(picked) {
            cellPicker->GetPickPosition(posPicked);
            vtkAssemblyPath *path = cellPicker->GetPath();
            actor = path->GetLastNode()->GetViewProp();
            (*actorPicked) = actor;
        }
    }
}

void mafVTKWidget::mousePress(vtkRenderWindowInteractor* iren, QEvent *e) {
    double posPicked[3];
    mafCore::mafProxy<vtkProp> actorPicked;
    
    pickProp(iren, e, &actorPicked, posPicked);
    Q_EMIT mousePressSignal(posPicked, m_Modifiers, &actorPicked, e);
}
void mafVTKWidget::mouseRelease(vtkRenderWindowInteractor* iren, QEvent *e) {
    double posPicked[3];
    mafCore::mafProxy<vtkProp> actorPicked;
    
    pickProp(iren, e, &actorPicked, posPicked);    Q_EMIT mouseReleaseSignal(posPicked, m_Modifiers, &actorPicked, e);
}
void mafVTKWidget::mouseMove(vtkRenderWindowInteractor* iren, QEvent *e) {
    double posPicked[3];
    mafCore::mafProxy<vtkProp> actorPicked;
    
    pickProp(iren, e, &actorPicked, posPicked);    Q_EMIT mouseMoveSignal(posPicked, m_Modifiers, &actorPicked, e);
}

// overloaded resize handler
void mafVTKWidget::resizeEvent(QResizeEvent* event) {
    QVTKWidget::resizeEvent(event);
}
