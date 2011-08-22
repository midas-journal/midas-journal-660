/*==========================================================================
 
 Portions (c) Copyright 2010 Atsushi Yamada (Fujimoto Lab, Nagoya Institute of Technology (NIT)) 
			     and M. Komura (NIT) All Rights Reserved.
 
 Acknowledgement: K. Chinzei (AIST), Y. Hayashi (Nagoya Univ.), T. Takeuchi (SFC Corp.), H. Liu (BWH), J. Tokuda (BWH), N. Hata (BWH), and H. Fujimoto (NIT). 
 CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake are contributions of K. Chinzei(AIST) and T. Takeuchi (SFC Corp.).

 !+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!
 This module is based on the "Secondary Window" module by J. Tokuda (BWH).
 !+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!+!
 
 See README.txt
 or http://www.slicer.org/copyright/copyright.txt for details.
 
 Program:   SecondaryWindowWithOpenCV
 Module:    $HeadURL: $
 Date:      $Date:01/25/2010 $
 Version:   $Revision: $
 
 ==========================================================================*/

// .NAME vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerSecondaryViewerWidget_h
#define __vtkSlicerSecondaryViewerWidget_h

#include "vtkSecondaryWindowWithOpenCVWin32Header.h"

#include "vtkSlicerWidget.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerModelHierarchyLogic.h"

#include "vtkSlicerRenderWidget.h"
#include "vtkKWFrame.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLViewNode.h"

#include "vtkSlicerViewerWidget.h"

class vtkMRMLDisplayableNode;
class vtkMRMLDisplayNode;
class vtkMRMLModelHierarchyNode;
class vtkSlicerModelHierarchyLogic;
class vtkPolyData;
class vtkCellArray;
class vtkProp3D;
class vtkActor;
class vtkActorText;
class vtkFollower;
class vtkImplicitBoolean;
class vtkPlane;
class vtkWorldPointPicker;
class vtkPropPicker;
class vtkCellPicker;
class vtkPointPicker;
class vtkSlicerBoxWidget2;
class vtkSlicerBoxRepresentation;


class VTK_SecondaryWindowWithOpenCV_EXPORT vtkSlicerSecondaryViewerWidget : public vtkSlicerViewerWidget
{

public:
  static vtkSlicerSecondaryViewerWidget* New();
  vtkTypeRevisionMacro(vtkSlicerSecondaryViewerWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Register(vtkObject *o) { Superclass::Register(o); };
  virtual void UnRegister(vtkObject *o) { Superclass::UnRegister(o); };

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // Updates Actors based on models in the scene
  void UpdateFromMRML();

    // Description: 
  // Actually do a render (don't wait for idle)
  void Render();

protected:

  vtkSlicerSecondaryViewerWidget();
  virtual ~vtkSlicerSecondaryViewerWidget();

  void UpdateCameraNode();
  void UpdateViewNode();

  void UpdateModelsFromMRML();

  int GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model);

  // Description:
  // Create the widget.
  virtual void CreateWidget();



private:
  
  vtkSlicerSecondaryViewerWidget(const vtkSlicerSecondaryViewerWidget&); // Not implemented
  void operator=(const vtkSlicerSecondaryViewerWidget&); // Not Implemented
};

#endif

