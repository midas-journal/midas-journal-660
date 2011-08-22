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

#ifndef __vtkSlicerSecondaryViewerWindow_h
#define __vtkSlicerSecondaryViewerWindow_h



// for getting display device information
#ifdef WIN32
#include "Windows.h"
#endif

#include "vtkSecondaryWindowWithOpenCVWin32Header.h"


#include "vtkSmartPointer.h"
//#include "vtkSlicerFiducialListWidget.h"
//#include "vtkSlicerROIViewerWidget.h"
#include "vtkSlicerSecondaryViewerWidget.h"

#include "vtkKWTopLevel.h"
#include "vtkSmartPointer.h"
#include "vtkKWRenderWidget.h"//10.01.12-komura
class VTK_SecondaryWindowWithOpenCV_EXPORT vtkSlicerSecondaryViewerWindow : 
  public vtkKWTopLevel
{
public:
  static vtkSlicerSecondaryViewerWindow *New();  
  vtkTypeRevisionMacro(vtkSlicerSecondaryViewerWindow,vtkKWTopLevel);
  void PrintSelf(ostream& os, vtkIndent indent);

  void DisplayOnSecondaryMonitor();

  //void SetCurrentTarget(vtkTRProstateBiopsyTargetDescriptor* target);

  vtkKWRenderWidget *rw;//10.01.12-komura
  vtkKWRenderWidget *rwLeft;//10.01.25 ayamada


  // vtkKWRenderWidget slcass reference website
  //http://www.kwwidgets.org/doc/nightly/html/classvtkKWRenderWidget.html

protected:

  void UpdateSecondaryMonitorPoisition();
  virtual void CreateWidget();

  vtkSlicerSecondaryViewerWindow();
  ~vtkSlicerSecondaryViewerWindow();  

  vtkKWFrame* MainFrame;
  vtkKWFrame* MainGrid; //10.01.15-komura
//  vtkKWFrame* SubFrame; //10.01.25 ayamada

  vtkSlicerSecondaryViewerWidget* ViewerWidget;
  //vtkSlicerFiducialListWidget* FiducialListWidget;  
  //vtkSlicerROIViewerWidget* ROIViewerWidget;
  //vtkTRProstateBiopsyRobotWidget* RobotViewerWidget;

  bool MultipleMonitorsAvailable; 
  int  SecondaryMonitorPosition[2]; // virtual screen position in pixels
  int  SecondaryMonitorSize[2]; // virtual screen size in pixels

  //BTX
  //vtkTRProstateBiopsyTargetDescriptor* CurrentTarget;
  //ETX

private:
  vtkSlicerSecondaryViewerWindow(const vtkSlicerSecondaryViewerWindow&);
  void operator=(const vtkSlicerSecondaryViewerWindow&);
};

#endif
