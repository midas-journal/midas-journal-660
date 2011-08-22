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

// 4/23/2010 ayamada
#include "vtkMRMLLinearTransformNode.h"


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
    
    // 4/25/2010 ayamada
    // for text overlay
    textActor = vtkTextActor::New();
    textActor1 = vtkTextActor::New();
    textActor2 = vtkTextActor::New();
    textActor3 = vtkTextActor::New();
    //textActor4 = vtkTextActor::New();
    //textActor5 = vtkTextActor::New();
    //textActor6 = vtkTextActor::New();
    //testActor = vtkActor2D::New();    
    

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
    this->TimerInterval = 1;  // 1 ms 4/25/2010 ayamada
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
          
          // read MRML; 4/23/2010 ayamada
          vtkMRMLScene* scene = this->GetMRMLScene();
          if (scene)
          {
              /*
              // test network connection check (baloon sensor)
              vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLLinearTransformNode4");
              vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::SafeDownCast(node);
              
              // test baloon sensor connection check (suction tube)
              vtkMRMLNode* node2 = scene->GetNodeByID("vtkMRMLLinearTransformNode5");
              vtkMRMLLinearTransformNode* tnode2 = vtkMRMLLinearTransformNode::SafeDownCast(node2);
              
              vtkMRMLNode *nnode = NULL; // TODO: is this OK?
              vtkIntArray* nodeEvents = vtkIntArray::New();
              nodeEvents->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
              vtkSetAndObserveMRMLNodeEventsMacro(nnode,tnode,nodeEvents);
              
              vtkMRMLNode *nnode2 = NULL; // TODO: is this OK?
              vtkIntArray* nodeEvents2 = vtkIntArray::New();
              nodeEvents2->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
              vtkSetAndObserveMRMLNodeEventsMacro(nnode2,tnode2,nodeEvents2);
              
              nodeEvents->Delete();
              nodeEvents2->Delete();
              */
          }                
          
      }
    }
  else if (this->HideSecondaryWindowWithOpenCVButton == vtkKWPushButton::SafeDownCast(caller)
           && event == vtkKWPushButton::InvokedEvent)
    {
    if (this->SecondaryViewerWindow)
      {  
      //this->SecondaryViewerWindow->Withdraw();
          
          // read MRML; 4/25/2010 ayamada
          vtkMRMLScene* scene = this->GetMRMLScene();
          if (scene)
          {
              // test network connection check (baloon sensor)
              vtkMRMLNode* node = scene->GetNodeByID("vtkMRMLLinearTransformNode4");
              vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::SafeDownCast(node);
              
              // test baloon sensor connection check (suction tube)
              vtkMRMLNode* node2 = scene->GetNodeByID("vtkMRMLLinearTransformNode5");
              vtkMRMLLinearTransformNode* tnode2 = vtkMRMLLinearTransformNode::SafeDownCast(node2);

              vtkMRMLNode *nnode = NULL; // TODO: is this OK?
              vtkIntArray* nodeEvents = vtkIntArray::New();
              nodeEvents->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
              vtkSetAndObserveMRMLNodeEventsMacro(nnode,tnode,nodeEvents);
              
              vtkMRMLNode *nnode2 = NULL; // TODO: is this OK?
              vtkIntArray* nodeEvents2 = vtkIntArray::New();
              nodeEvents2->InsertNextValue(vtkMRMLLinearTransformNode::TransformModifiedEvent);
              vtkSetAndObserveMRMLNodeEventsMacro(nnode2,tnode2,nodeEvents2);
              
              nodeEvents->Delete();
              nodeEvents2->Delete();              
              
          }
          
          
          
          
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
    // 4/23/2010 ayamada
    if (event == vtkMRMLLinearTransformNode::TransformModifiedEvent)
    {
		std::cerr << "TransformModifiedEvent is invoked." << std::endl;
        
		
		vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast(caller);
		vtkMatrix4x4* transformToParent = node->GetMatrixTransformToParent();
        
		vtkMRMLLinearTransformNode* node2 = vtkMRMLLinearTransformNode::SafeDownCast(caller);
		vtkMatrix4x4* transformToParent2 = node2->GetMatrixTransformToParent();
        
		tx = transformToParent->GetElement(0, 0);
		ty = transformToParent->GetElement(1, 0);
		tz = transformToParent->GetElement(2, 0);
		t0 = transformToParent->GetElement(3, 0);
		sx = transformToParent->GetElement(0, 1);
		sy = transformToParent->GetElement(1, 1);
		sz = transformToParent->GetElement(2, 1);
		s0 = transformToParent->GetElement(3, 1);
		nx = transformToParent->GetElement(0, 2);
		ny = transformToParent->GetElement(1, 2);
		nz = transformToParent->GetElement(2, 2);
		n0 = transformToParent->GetElement(3, 2);
		px = transformToParent->GetElement(0, 3);
		py = transformToParent->GetElement(1, 3);
		pz = transformToParent->GetElement(2, 3);
		p0 = transformToParent->GetElement(3, 3);
        
		tx2 = transformToParent2->GetElement(0, 0);
		ty2 = transformToParent2->GetElement(1, 0);
		tz2 = transformToParent2->GetElement(2, 0);
		t02 = transformToParent2->GetElement(3, 0);
		sx2 = transformToParent2->GetElement(0, 1);
		sy2 = transformToParent2->GetElement(1, 1);
		sz2 = transformToParent2->GetElement(2, 1);
		s02 = transformToParent2->GetElement(3, 1);
		nx2 = transformToParent2->GetElement(0, 2);
		ny2 = transformToParent2->GetElement(1, 2);
		nz2 = transformToParent2->GetElement(2, 2);
		n02 = transformToParent2->GetElement(3, 2);
		px2 = transformToParent2->GetElement(0, 3);
		py2 = transformToParent2->GetElement(1, 3);
		pz2 = transformToParent2->GetElement(2, 3);
		p02 = transformToParent2->GetElement(3, 3);

		/*
		std::cerr << "tx  = "  << tx << std::endl;
		std::cerr << "ty  = "  << ty << std::endl;
		std::cerr << "tz  = "  << tz << std::endl;
		std::cerr << "t0  = "  << t0 << std::endl;
		std::cerr << "sx  = "  << sx << std::endl;
		std::cerr << "sy  = "  << sy << std::endl;
		std::cerr << "sz  = "  << sz << std::endl;
		std::cerr << "s0  = "  << s0 << std::endl;
		std::cerr << "nx  = "  << nx << std::endl;
		std::cerr << "ny  = "  << ny << std::endl;
		std::cerr << "nz  = "  << nz << std::endl;
		std::cerr << "n0  = "  << n0 << std::endl;
		std::cerr << "px  = "  << px << std::endl;
		std::cerr << "py  = "  << py << std::endl;
		std::cerr << "pz  = "  << pz << std::endl;
		std::cerr << "p0  = "  << p0 << std::endl;
        
		std::cerr << "tx2  = "  << tx2 << std::endl;
		std::cerr << "ty2  = "  << ty2 << std::endl;
		std::cerr << "tz2  = "  << tz2 << std::endl;
		std::cerr << "t02  = "  << t02 << std::endl;
		std::cerr << "sx2  = "  << sx2 << std::endl;
		std::cerr << "sy2  = "  << sy2 << std::endl;
		std::cerr << "sz2  = "  << sz2 << std::endl;
		std::cerr << "s02  = "  << s02 << std::endl;
		std::cerr << "nx2  = "  << nx2 << std::endl;
		std::cerr << "ny2  = "  << ny2 << std::endl;
		std::cerr << "nz2  = "  << nz2 << std::endl;
		std::cerr << "n02  = "  << n02 << std::endl;
		std::cerr << "px2  = "  << px2 << std::endl;
		std::cerr << "py2  = "  << py2 << std::endl;
		std::cerr << "pz2  = "  << pz2 << std::endl;
		std::cerr << "p02  = "  << p02 << std::endl;
        */
        
		// display the status 4/25/2010 ayamada
		char bufA[100],bufB[100],bufC[100];
        
		sprintf(bufA, "Position X:   %f",px);
		textActor1->SetInput(bufA);
		
		sprintf(bufB, "Position Y:   %f ",py);
		textActor2->SetInput(bufB);
		
		sprintf(bufC,"Position Z:    %f ",pz);
		textActor3->SetInput(bufC);
        
        
        
    }        

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
  conBrowsFrame->SetLabelText("Secondary Window with OpenCV Configuration");
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
  //this->HideSecondaryWindowWithOpenCVButton->SetText ("OFF");
  this->HideSecondaryWindowWithOpenCVButton->SetText ("OBSERVE MRML");  // 4/25/2010 ayamada
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
    vtkSecondaryWindowWithOpenCVGUI* pGUI = static_cast<vtkSecondaryWindowWithOpenCVGUI*>(vinfo->UserData);
    if(first == 0){
	sleep(1);	// 10.01.23 ayamada
	while(1){//10.01.20-komura
//	    if( (NULL==(pGUI->capture = cvCaptureFromCAM(0))) )
	    if( (NULL==(pGUI->capture = cvCaptureFromCAM(i))))	// 10.01.25 ayamada
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

	    if(NULL == (pGUI->captureImageTmp = cvQueryFrame( pGUI->capture ))){
		fprintf(stdout, "\n\ncannot take a picture\n\n");
		sleep(1);
		continue;
	    }		

	    // pGUI->Mutex->Lock();

	    pGUI->imageSize = cvGetSize( pGUI->captureImageTmp );
	    pGUI->captureImage = cvCreateImage(pGUI->imageSize, IPL_DEPTH_8U,3);	
	    // 10.01.25 ayamada
	    pGUI->atext->SetInputConnection(pGUI->importer->GetOutputPort());
	    pGUI->atext->InterpolateOn();

	    pGUI->RGBImage = cvCreateImage( pGUI->imageSize, IPL_DEPTH_8U, 3);	
	    pGUI->captureImageTmp = cvQueryFrame( pGUI->capture );
	    cvFlip(pGUI->captureImageTmp, pGUI->captureImage, 0);
	    cvCvtColor( pGUI->captureImage, pGUI->RGBImage, CV_BGR2RGB);
	    pGUI->idata = (unsigned char*) pGUI->RGBImage->imageData;

	    pGUI->importer->SetWholeExtent(0,pGUI->imageSize.width-1,0,pGUI->imageSize.height-1,0,0);
	    pGUI->importer->SetDataExtentToWholeExtent();
	    pGUI->importer->SetDataScalarTypeToUnsignedChar();
	    pGUI->importer->SetNumberOfScalarComponents(3);
	    pGUI->importer->SetImportVoidPointer(pGUI->idata);

	    pGUI->importer->Update();

            // 10.01.24 ayamada
	    double planeX = -1.0;
	    double planeY = -1.0;
	    double planeLength = 1.0;
	
	    pGUI->planeSource->SetOrigin(planeX,planeY,0.0);	
	    pGUI->planeSource->SetCenter(0.0,0.0,0.0);	
	    pGUI->planeSource->SetResolution(1,1);
	    pGUI->planeSource->SetPoint1(planeLength,planeY,0.0);
	    pGUI->planeSource->SetPoint2(planeX,planeLength,0.0);

/*
	    // 10.01.25 ayamada
	    pGUI->planeSourceLeftPane->SetOrigin(planeX,planeY,0.0);	
	    pGUI->planeSourceLeftPane->SetCenter(0.0,0.0,0.0);	
	    pGUI->planeSourceLeftPane->SetResolution(1,1);
	    pGUI->planeSourceLeftPane->SetPoint1(planeLength,planeY,0.0);
	    pGUI->planeSourceLeftPane->SetPoint2(planeX,planeLength,0.0);

	    pGUI->planeMapperLeftPane->SetInputConnection(pGUI->planeSourceLeftPane->GetOutputPort());
	    pGUI->actorLeftPane->SetMapper(pGUI->planeMapperLeftPane);   // plane source mapper
	    pGUI->actorLeftPane->GetProperty()->SetColor(255,0,0);

	    pGUI->SecondaryViewerWindow->rwLeft->AddViewProp(pGUI->actorLeftPane);
*/

	    pGUI->planeMapper->SetInputConnection(pGUI->planeSource->GetOutputPort());
	    pGUI->actor->SetMapper(pGUI->planeMapper);   // plane source mapper
	    pGUI->actor->SetTexture(pGUI->atext);        // texture mapper
	    pGUI->actor->SetMapper(pGUI->planeMapper);
	    //actor->SetMapper(mapper); //10.01.16-komura
	    pGUI->SecondaryViewerWindow->rw->AddViewProp(pGUI->actor);

	    // 10.01.25 ayamada
//	    pGUI->Mutex->Lock();
//	    pGUI->SecondaryViewerWindow->rw->Render();
//	    pGUI->Mutex->Unlock();
	    pGUI->updateView=1;

	    sleep(1);	// 10.01.25 ayamada

	    //pGUI->SecondaryViewerWindow->rw->ResetCamera(); //10.01.27-komura
	    //    pGUI->SecondaryViewerWindow->rw->SetCameraPosition(0,0,0);

	    // pGUI->Mutex->Unlock();
	    first = 1;
	    fprintf(stdout, "\nget camera handle\n");//10.01.20-komura
	    break;//10.01.20-komura
	}
    }

    // 4/25/2010 ayamada
    pGUI->textActor->SetInput("Marker Position");
    pGUI->textActor->GetTextProperty()->SetFontSize(14);
    pGUI->textActor->GetTextProperty()->BoldOn();
    pGUI->textActor->SetPosition(10,70);    
    pGUI->SecondaryViewerWindow->rwLeft->AddViewProp(pGUI->textActor);
    
    pGUI->textActor1->SetInput("Position X:");
    pGUI->textActor1->GetTextProperty()->SetFontSize(14);
    pGUI->textActor1->SetPosition(10,50);
    
    pGUI->textActor2->SetInput("Position Y:");
    pGUI->textActor2->GetTextProperty()->SetFontSize(14);
    pGUI->textActor2->SetPosition(10,30);
    
    pGUI->textActor3->SetInput("Position Z:");
    pGUI->textActor3->GetTextProperty()->SetFontSize(14);
    pGUI->textActor3->SetPosition(10,10);
        
    while(1){

        // 4/25/2010 ayamaada
        // ren->AddActor(baloonActor);
	    pGUI->SecondaryViewerWindow->rwLeft->AddViewProp(pGUI->textActor1);
	    pGUI->SecondaryViewerWindow->rwLeft->AddViewProp(pGUI->textActor2);
	    pGUI->SecondaryViewerWindow->rwLeft->AddViewProp(pGUI->textActor3);
        
	    pGUI->captureImageTmp = cvQueryFrame( pGUI->capture );	// 10.01.23 ayamada

	    pGUI->imageSize = cvGetSize( pGUI->captureImageTmp );
	    cvFlip(pGUI->captureImageTmp, pGUI->captureImage, 0);
	    cvCvtColor( pGUI->captureImage, pGUI->RGBImage, CV_BGR2RGB);
	    pGUI->idata = (unsigned char*) pGUI->RGBImage->imageData;
//	    pGUI->Mutex->Lock();
	    pGUI->importer->Modified();
//	    pGUI->Mutex->Unlock();

/* 10.01.26-komura

	    if(pGUI->SecondaryViewerWindow->rw->GetApplication()->EvaluateBooleanExpression(
		"expr {[winfo exists %s] && [winfo ismapped %s]}", 
		pGUI->SecondaryViewerWindow->rw->GetWidgetName(), pGUI->SecondaryViewerWindow->rw->GetWidgetName())
		== 0){
	
		fprintf(stdout,"\nbreak\n");//10.01.20-komura
		break;
	    }
*/

/* 10.01.26-komura

//IsMapped function returns 0 when "rw" is disappeared.
//But this function isn't stability, so I stoped using this.

	    if(pGUI->SecondaryViewerWindow->rw->IsMapped() == 0){//10.01.21-komura
		//if(pGUI->SecondaryViewerWindow->rw->IsAlive() == 0){//10.01.21-komura
		//if(pGUI->SecondaryViewerWindow->rw->IsCreated() == 0){//10.01.21-komura
		fprintf(stdout,"\nbreak\n");//10.01.20-komura
		break;
	    }
*/
    }
    pGUI->runThread = 0;//10.01.21-komura
    return NULL;
}


