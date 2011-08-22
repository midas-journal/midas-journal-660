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

#ifndef __vtkSecondaryWindowWithOpenCVGUI_h
#define __vtkSecondaryWindowWithOpenCVGUI_h

#ifdef WIN32
#include "vtkSecondaryWindowWithOpenCVWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include "vtkSecondaryWindowWithOpenCVLogic.h"
#include "vtkSlicerSecondaryViewerWindow.h"

//----10.01.12-komura
#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <math.h>
#include <stdlib.h>
#include "vtkImageImport.h"
#include "vtkTexture.h"
#include "vtkPlaneSource.h"
#include "vtkImageMapper.h"
#include "vtkImageImport.h"
#include "vtkActor2D.h"
#include "vtkPolyDataMapper.h"
#include "vtkKWRenderWidget.h"

// 10.01.25 ayamada
#include "vtkTextActor3D.h"

// 10.04.23 ayamada
//#include "igtlOSUtil.h"
//#include "igtlImageMessage.h"
//#include "igtlServerSocket.h"
#include <vtkTextProperty.h>
#include <vtkTextSource.h>
#include <vtkTextActor.h>
#include <vtkTextMapper.h>

//----
class vtkKWPushButton;

class VTK_SecondaryWindowWithOpenCV_EXPORT vtkSecondaryWindowWithOpenCVGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkSecondaryWindowWithOpenCVGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkSecondaryWindowWithOpenCVLogic );
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
  }

 protected:
  //----------------------------------------------------------------
  // Constructor / Destructor (proctected/private) 
  //----------------------------------------------------------------

  vtkSecondaryWindowWithOpenCVGUI ( );
  virtual ~vtkSecondaryWindowWithOpenCVGUI ( );

 private:
  vtkSecondaryWindowWithOpenCVGUI ( const vtkSecondaryWindowWithOpenCVGUI& ); // Not implemented.
  void operator = ( const vtkSecondaryWindowWithOpenCVGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkSecondaryWindowWithOpenCVGUI* New ();
  void Init();
  virtual void Enter ( );
  virtual void Exit ( );

  virtual void TearDownGUI();

  void PrintSelf (ostream& os, vtkIndent indent );

  //----------------------------------------------------------------
  // Observer Management
  //----------------------------------------------------------------

  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  void AddLogicObservers ( );
  void RemoveLogicObservers ( );

  //----------------------------------------------------------------
  // Event Handlers
  //----------------------------------------------------------------

  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  void ProcessTimerEvents();
  void HandleMouseEvent(vtkSlicerInteractorStyle *style);
  static void DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);
  
  //----------------------------------------------------------------
  // Build Frames
  //----------------------------------------------------------------

  virtual void BuildGUI ( );
  void BuildGUIForHelpFrame();
  void BuildGUIForWindowConfigurationFrame();

  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();


//----10.01.12-komura
  int threadLock;
  int               ThreadID;
  vtkMutexLock*     Mutex;
  vtkMultiThreader* Thread;
  static void *thread_CameraThread(void*);
  int makeCameraThread(void);


    IplImage*	captureImage;
    IplImage*	RGBImage;
    IplImage*	captureImageTmp;
    CvSize		imageSize;
    unsigned char* idata;
    CvCapture* capture;
    vtkImageImport *importer;
    vtkTexture *atext;
    vtkPlaneSource *planeSource;
    vtkPolyDataMapper *planeMapper;
    vtkActor *actor;

	// 10.01.25 ayamada
    vtkPlaneSource *planeSourceLeftPane;
    vtkPolyDataMapper *planeMapperLeftPane;
    vtkActor *actorLeftPane;

    int runThread;//10.01.21-komura

    // 10.01.24 ayamada
    int updateView;
    int updateViewTriger;
    int camNumber;
    
    // 4/25/2010 ayamada
    // for text overlay
    vtkTextActor *textActor;
    vtkTextActor *textActor1;
    vtkTextActor *textActor2;
    vtkTextActor *textActor3;
    //vtkTextActor *textActor4;
    //vtkTextActor *textActor5;
    //vtkTextActor *textActor6;
    //vtkActor2D *testActor;
    
    // 4/23/2010 ayamada
    float tx;
    float ty;
    float tz;
    float t0;
    float sx;
    float sy;
    float sz;
    float s0;
    float nx;
    float ny;
    float nz;
    float n0;
    float px;
    float py;
    float pz;
    float p0;
    
    float tx2;
    float ty2;
    float tz2;
    float t02;
    float sx2;
    float sy2;
    float sz2;
    float s02;
    float nx2;
    float ny2;
    float nz2;
    float n02;
    float px2;
    float py2;
    float pz2;
    float p02;
    
    
    

//----
 protected:
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkKWPushButton* ShowSecondaryWindowWithOpenCVButton;
  vtkKWPushButton* HideSecondaryWindowWithOpenCVButton;

  vtkSlicerSecondaryViewerWindow* SecondaryViewerWindow;

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkSecondaryWindowWithOpenCVLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int                        CloseScene;

};



#endif
