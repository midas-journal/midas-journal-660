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

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSecondaryWindowWithOpenCVGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"

#include "vtkKWPushButton.h"

#include "vtkCornerAnnotation.h"

// 10.01.25 ayamada
#include "vtkProperty.h"


int first=0;//10.01.12-komura
//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSecondaryWindowWithOpenCVGUI );
vtkCxxRevisionMacro ( vtkSecondaryWindowWithOpenCVGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkSecondaryWindowWithOpenCVGUI::vtkSecondaryWindowWithOpenCVGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkSecondaryWindowWithOpenCVGUI::DataCallback);
  
  //----------------------------------------------------------------
  // GUI widgets
  this->ShowSecondaryWindowWithOpenCVButton = NULL;
  this->HideSecondaryWindowWithOpenCVButton = NULL;

  this->SecondaryViewerWindow = NULL;

  //----------------------------------------------------------------
  // Locator  (MRML)
  this->TimerFlag = 0;

//----10.01.12-komura
  this->threadLock = 0;
  this->ThreadID = -1;
  this->Mutex = vtkMutexLock::New();
  this->Thread = vtkMultiThreader::New();

// 10.01.24 ayamada
  this->updateView = 0;
  this->updateViewTriger = 0;
  this->camNumber = 0;


    captureImage = NULL;
    RGBImage = NULL;
    captureImageTmp = NULL;
    idata = NULL;
    capture = NULL;
    importer = vtkImageImport::New();
    atext = vtkTexture::New();
    planeSource = vtkPlaneSource::New();
    planeMapper = vtkPolyDataMapper::New();
    actor = vtkActor::New();

    // 10.01.25 ayamada
    planeSourceLeftPane = vtkPlaneSource::New();
    planeMapperLeftPane = vtkPolyDataMapper::New();
    actorLeftPane = vtkActor::New();

    runThread = 0;//10.01.21-komura
//----
}

//---------------------------------------------------------------------------
vtkSecondaryWindowWithOpenCVGUI::~vtkSecondaryWindowWithOpenCVGUI ( )
{

  //----------------------------------------------------------------
  // Remove Callbacks

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  //----------------------------------------------------------------
  // Remove Observers

  this->RemoveGUIObservers();

  //----------------------------------------------------------------
  // Remove GUI widgets

  if (this->SecondaryViewerWindow)
    {
    this->SecondaryViewerWindow->Withdraw();
    this->SecondaryViewerWindow->SetApplication(NULL);
    this->SecondaryViewerWindow->Delete();
    this->SecondaryViewerWindow = NULL;
    }

  if (this->ShowSecondaryWindowWithOpenCVButton)
    {
    this->ShowSecondaryWindowWithOpenCVButton->SetParent(NULL);
    this->ShowSecondaryWindowWithOpenCVButton->Delete();
    }

  if (this->HideSecondaryWindowWithOpenCVButton)
    {
    this->HideSecondaryWindowWithOpenCVButton->SetParent(NULL);
    this->HideSecondaryWindowWithOpenCVButton->Delete();
    }

  //----------------------------------------------------------------
  // Unregister Logic class

  this->SetModuleLogic ( NULL );

//----10.01.12-komura
  Mutex->Delete();
  Thread->Delete();

    cvReleaseCapture(&capture);
    cvReleaseImage(&RGBImage);
    cvReleaseImage(&captureImage);
    importer->Delete();
    planeSource->Delete();
    planeSource = NULL;
    planeMapper->Delete();
    planeMapper = NULL;
    atext->Delete(); 
    atext=NULL;
    actor->Delete();
    actor=NULL;

	// 10.01.25 ayamada
    planeSourceLeftPane->Delete();
    planeSourceLeftPane = NULL;
    planeMapperLeftPane->Delete();
    planeMapperLeftPane = NULL;
    actorLeftPane->Delete();
    actorLeftPane=NULL;

//----
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
//    this->TimerInterval = 100;  // 100 ms
    this->TimerInterval = 1;  // 100 ms		// 10.01.23 ayamada
    ProcessTimerEvents();
    }

}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::Exit ( )
{
  // Fill in
}

//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::TearDownGUI() 
{
  /*
  this->PrimaryMonitorRobotViewerWidget->SetApplication(NULL);
  this->PrimaryMonitorRobotViewerWidget->SetMainViewerWidget(NULL);
  this->PrimaryMonitorRobotViewerWidget->SetMRMLScene(NULL);
  */
}



//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "SecondaryWindowWithOpenCVGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (this->ShowSecondaryWindowWithOpenCVButton)
    {
    this->ShowSecondaryWindowWithOpenCVButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->HideSecondaryWindowWithOpenCVButton)
    {
    this->HideSecondaryWindowWithOpenCVButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }


  this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  //----------------------------------------------------------------
  // GUI Observers

  this->ShowSecondaryWindowWithOpenCVButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->HideSecondaryWindowWithOpenCVButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}




//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkSecondaryWindowWithOpenCVLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  
  if (this->ShowSecondaryWindowWithOpenCVButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent)
    {
    if (this->SecondaryViewerWindow)
      {  
	  //this->makeCameraThread();//10.01.12-komura //10.01.21-komura
      this->SecondaryViewerWindow->DisplayOnSecondaryMonitor();
      }
    }
  else if (this->HideSecondaryWindowWithOpenCVButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    if (this->SecondaryViewerWindow)
      {  
      this->SecondaryViewerWindow->Withdraw();
      }
    }
} 


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkSecondaryWindowWithOpenCVGUI *self = reinterpret_cast<vtkSecondaryWindowWithOpenCVGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkSecondaryWindowWithOpenCVGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkSecondaryWindowWithOpenCVLogic::SafeDownCast(caller))
    {
    if (event == vtkSecondaryWindowWithOpenCVLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in

  if (event == vtkMRMLScene::SceneCloseEvent)
    {
    }
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
	
//	if(SecondaryViewerWindow->rw->IsMapped() == 1){ //----10.01.21-komura
	if(this->updateViewTriger==1){ // 10.01.25 ayamada
	    if(this->runThread == 0){                  
		this->makeCameraThread();              
		runThread = 1;                         
	    }
	    else{
		if(updateView==1){	// 10.01.25 ayamada
			this->SecondaryViewerWindow->rw->Render();//10.01.12-komura
		}	
		this->SecondaryViewerWindow->rwLeft->Render();
    
	    }
	}
	//----
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "SecondaryWindowWithOpenCV", "SecondaryWindowWithOpenCV", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForWindowConfigurationFrame();

  this->SecondaryViewerWindow = vtkSlicerSecondaryViewerWindow::New();
  this->SecondaryViewerWindow->SetApplication(this->GetApplication());
  this->SecondaryViewerWindow->Create();
  
  // 10.01.25 ayamada
  this->updateViewTriger=1;

}


void vtkSecondaryWindowWithOpenCVGUI::BuildGUIForHelpFrame ()
{
  // Define your help text here.
  const char *help = 
    "See "
    "<a>http://www.slicer.org/slicerWiki/index.php/Modules:SecondaryWindowWithOpenCV</a> for details.";
  const char *about =
    "This work is supported by NCIGT, NA-MIC.";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "SecondaryWindowWithOpenCV" );
  this->BuildHelpAndAboutFrame (page, help, about);
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::BuildGUIForWindowConfigurationFrame()
{

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("SecondaryWindowWithOpenCV");
  
  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Secondary Window Configuration");
  //conBrowsFrame->CollapseFrame();
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Secondary Window child frame

  vtkKWFrame *switchframe = vtkKWFrame::New();
  switchframe->SetParent(conBrowsFrame->GetFrame());
  switchframe->Create();
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 switchframe->GetWidgetName() );

  // -----------------------------------------
  // Push buttons

  this->ShowSecondaryWindowWithOpenCVButton = vtkKWPushButton::New ( );
  this->ShowSecondaryWindowWithOpenCVButton->SetParent ( switchframe );
  this->ShowSecondaryWindowWithOpenCVButton->Create ( );
  this->ShowSecondaryWindowWithOpenCVButton->SetText ("ON");
  this->ShowSecondaryWindowWithOpenCVButton->SetWidth (12);

  this->HideSecondaryWindowWithOpenCVButton = vtkKWPushButton::New ( );
  this->HideSecondaryWindowWithOpenCVButton->SetParent ( switchframe );
  this->HideSecondaryWindowWithOpenCVButton->Create ( );
  this->HideSecondaryWindowWithOpenCVButton->SetText ("OFF");
  this->HideSecondaryWindowWithOpenCVButton->SetWidth (12);

  this->Script("pack %s %s -side left -padx 2 -pady 2", 
               this->ShowSecondaryWindowWithOpenCVButton->GetWidgetName(),
               this->HideSecondaryWindowWithOpenCVButton->GetWidgetName());

  conBrowsFrame->Delete();
  switchframe->Delete();

}


//----------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVGUI::UpdateAll()
{
}


//----10.01.12-komura

int vtkSecondaryWindowWithOpenCVGUI::makeCameraThread(void)
{
    this->ThreadID = this->Thread->SpawnThread((vtkThreadFunctionType) &vtkSecondaryWindowWithOpenCVGUI::thread_CameraThread, this);
    sleep(1);	// 10.01.23 ayamada
    return 1;
}
void *vtkSecondaryWindowWithOpenCVGUI::thread_CameraThread(void* t)
{
    int i=0;
    vtkMultiThreader::ThreadInfo* vinfo = 
	static_cast<vtkMultiThreader::ThreadInfo*>(t);
    vtkSecondaryWindowWithOpenCVGUI* pWindowGUI = static_cast<vtkSecondaryWindowWithOpenCVGUI*>(vinfo->UserData);
    if(first == 0){
	sleep(1);	// 10.01.23 ayamada
	while(1){//10.01.20-komura
//	    if( (NULL==(pWindowGUI->capture = cvCaptureFromCAM(0))) )
	    if( (NULL==(pWindowGUI->capture = cvCaptureFromCAM(i))))	// 10.01.25 ayamada
		{
		    fprintf(stdout, "\n\ncannot find a camera\n\n");	// 10.01.25 ayamada
		    i++;	// 10.01.25 ayamada

		    // 10.01.25 ayamada
		    if(i==10){
			i = 0;
		    }

		    continue;
		}
	    break;
	}
	while(1){//10.01.20-komura

	    if(NULL == (pWindowGUI->captureImageTmp = cvQueryFrame( pWindowGUI->capture ))){
		fprintf(stdout, "\n\ncannot take a picture\n\n");
		sleep(1);
		continue;
	    }		

	    // pWindowGUI->Mutex->Lock();

	    pWindowGUI->imageSize = cvGetSize( pWindowGUI->captureImageTmp );
	    pWindowGUI->captureImage = cvCreateImage(pWindowGUI->imageSize, IPL_DEPTH_8U,3);	
	    // 10.01.25 ayamada
	    pWindowGUI->atext->SetInputConnection(pWindowGUI->importer->GetOutputPort());
	    pWindowGUI->atext->InterpolateOn();

	    pWindowGUI->RGBImage = cvCreateImage( pWindowGUI->imageSize, IPL_DEPTH_8U, 3);	
	    pWindowGUI->captureImageTmp = cvQueryFrame( pWindowGUI->capture );
	    cvFlip(pWindowGUI->captureImageTmp, pWindowGUI->captureImage, 0);
	    cvCvtColor( pWindowGUI->captureImage, pWindowGUI->RGBImage, CV_BGR2RGB);
	    pWindowGUI->idata = (unsigned char*) pWindowGUI->RGBImage->imageData;

	    pWindowGUI->importer->SetWholeExtent(0,pWindowGUI->imageSize.width-1,0,pWindowGUI->imageSize.height-1,0,0);
	    pWindowGUI->importer->SetDataExtentToWholeExtent();
	    pWindowGUI->importer->SetDataScalarTypeToUnsignedChar();
	    pWindowGUI->importer->SetNumberOfScalarComponents(3);
	    pWindowGUI->importer->SetImportVoidPointer(pWindowGUI->idata);

	    pWindowGUI->importer->Update();

            // 10.01.24 ayamada
	    double planeX = -1.0;
	    double planeY = -1.0;
	    double planeLength = 1.0;
	
	    pWindowGUI->planeSource->SetOrigin(planeX,planeY,0.0);	
	    pWindowGUI->planeSource->SetCenter(0.0,0.0,0.0);	
	    pWindowGUI->planeSource->SetResolution(1,1);
	    pWindowGUI->planeSource->SetPoint1(planeLength,planeY,0.0);
	    pWindowGUI->planeSource->SetPoint2(planeX,planeLength,0.0);

/*
	    // 10.01.25 ayamada
	    pWindowGUI->planeSourceLeftPane->SetOrigin(planeX,planeY,0.0);	
	    pWindowGUI->planeSourceLeftPane->SetCenter(0.0,0.0,0.0);	
	    pWindowGUI->planeSourceLeftPane->SetResolution(1,1);
	    pWindowGUI->planeSourceLeftPane->SetPoint1(planeLength,planeY,0.0);
	    pWindowGUI->planeSourceLeftPane->SetPoint2(planeX,planeLength,0.0);

	    pWindowGUI->planeMapperLeftPane->SetInputConnection(pWindowGUI->planeSourceLeftPane->GetOutputPort());
	    pWindowGUI->actorLeftPane->SetMapper(pWindowGUI->planeMapperLeftPane);   // plane source mapper
	    pWindowGUI->actorLeftPane->GetProperty()->SetColor(255,0,0);

	    pWindowGUI->SecondaryViewerWindow->rwLeft->AddViewProp(pWindowGUI->actorLeftPane);
*/

	    pWindowGUI->planeMapper->SetInputConnection(pWindowGUI->planeSource->GetOutputPort());
	    pWindowGUI->actor->SetMapper(pWindowGUI->planeMapper);   // plane source mapper
	    pWindowGUI->actor->SetTexture(pWindowGUI->atext);        // texture mapper
	    pWindowGUI->actor->SetMapper(pWindowGUI->planeMapper);
	    //actor->SetMapper(mapper); //10.01.16-komura
	    pWindowGUI->SecondaryViewerWindow->rw->AddViewProp(pWindowGUI->actor);

	    // 10.01.25 ayamada
//	    pWindowGUI->Mutex->Lock();
//	    pWindowGUI->SecondaryViewerWindow->rw->Render();
//	    pWindowGUI->Mutex->Unlock();
	    pWindowGUI->updateView=1;

	    sleep(1);	// 10.01.25 ayamada

	    pWindowGUI->SecondaryViewerWindow->rw->ResetCamera();
	    //    pWindowGUI->SecondaryViewerWindow->rw->SetCameraPosition(0,0,0);

	    // pWindowGUI->Mutex->Unlock();
	    first = 1;
	    fprintf(stdout, "\nget camera handle\n");//10.01.20-komura
	    break;//10.01.20-komura
	}
    }
    
    
    while(1){

	    pWindowGUI->captureImageTmp = cvQueryFrame( pWindowGUI->capture );	// 10.01.23 ayamada

	    pWindowGUI->imageSize = cvGetSize( pWindowGUI->captureImageTmp );
	    cvFlip(pWindowGUI->captureImageTmp, pWindowGUI->captureImage, 0);
	    cvCvtColor( pWindowGUI->captureImage, pWindowGUI->RGBImage, CV_BGR2RGB);
	    pWindowGUI->idata = (unsigned char*) pWindowGUI->RGBImage->imageData;
//	    pWindowGUI->Mutex->Lock();
	    pWindowGUI->importer->Modified();
//	    pWindowGUI->Mutex->Unlock();

    }
    pWindowGUI->runThread = 0;//10.01.21-komura
    return NULL;
}


