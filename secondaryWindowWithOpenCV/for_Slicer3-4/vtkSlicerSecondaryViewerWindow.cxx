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


#include "vtkObjectFactory.h"

#include "vtkSlicerSecondaryViewerWindow.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"

#include "vtkRenderer.h"

#include "vtkCornerAnnotation.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerSecondaryViewerWindow);
vtkCxxRevisionMacro(vtkSlicerSecondaryViewerWindow, "$Revision: 1.0 $");
//----------------------------------------------------------------------------
vtkSlicerSecondaryViewerWindow::vtkSlicerSecondaryViewerWindow()
{
  this->MainFrame = vtkKWFrame::New();
  //this->ViewerWidget=vtkTRProstateBiopsyViewerWidget::New();
  this->ViewerWidget = vtkSlicerSecondaryViewerWidget::New();
  //this->FiducialListWidget=vtkSlicerFiducialListWidget::New();
  //this->ROIViewerWidget=vtkSlicerROIViewerWidget::New();
  //this->RobotViewerWidget=vtkTRProstateBiopsyRobotWidget::New();
  this->MultipleMonitorsAvailable = false; 
  this->SecondaryMonitorPosition[0]=0;
  this->SecondaryMonitorPosition[1]=0;
  this->SecondaryMonitorSize[0]=0;
  this->SecondaryMonitorSize[1]=0;

  //this->CurrentTarget=NULL;

  this->MainGrid = vtkKWFrame::New();//10.01.15-komura
  this->rw = vtkKWRenderWidget::New();//10.01.12-komura
  this->rwLeft = vtkKWRenderWidget::New();//10.01.25 ayamda

}


//----------------------------------------------------------------------------
vtkSlicerSecondaryViewerWindow::~vtkSlicerSecondaryViewerWindow()
{
  //if (this->CurrentTarget)
  //{
  //  this->CurrentTarget->UnRegister(this);
  //  this->CurrentTarget=NULL;
  //}
  this->ViewerWidget->RemoveAllObservers();
  this->ViewerWidget->SetApplicationLogic (NULL);
  this->ViewerWidget->Unpack();  
  //this->FiducialListWidget->RemoveAllObservers();
  //this->FiducialListWidget->SetParent(NULL);
  //this->FiducialListWidget->Delete();
  //this->ROIViewerWidget->SetParent(NULL);
  //this->ROIViewerWidget->Delete();
  //this->RobotViewerWidget->SetParent(NULL);
  //this->RobotViewerWidget->Delete();
  this->ViewerWidget->SetParent(NULL);
  this->ViewerWidget->Delete();   
  this->MainFrame->Delete();
  this->SetApplication(NULL);
  this->rw->Delete();//10.01.12-komura
  this->MainGrid->Delete();//10.01.15-komura
  this->rwLeft->Delete();//10.01.25 ayamada
//  this->SubFrame->Delete();

}

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

#ifdef _WIN32
//----------------------------------------------------------------------------
BOOL CALLBACK StoreNonPrimaryMonitorRectEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT prc, LPARAM dwData) 
{
  MONITORINFO mi;
  mi.cbSize = sizeof(MONITORINFO);
  if (GetMonitorInfo(hMonitor, &mi))
  {
    if (!(mi.dwFlags & MONITORINFOF_PRIMARY))
    {
      RECT *rect = (RECT*)dwData;
      if (rect==NULL)
      {
        return false;
      }
      // store the rect of the non-primary monitor only
      rect->left=mi.rcMonitor.left;
      rect->right=mi.rcMonitor.right;
      rect->top=mi.rcMonitor.top;
      rect->bottom=mi.rcMonitor.bottom;     
    }
  }
  return true;
} 

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::UpdateSecondaryMonitorPoisition()
{
  // Window rect
  RECT rect;    

  // Fill rect by default with primary monitor info
  rect.left=0;
  rect.right=GetSystemMetrics(SM_CXFULLSCREEN);
  rect.top=0;
  rect.bottom=GetSystemMetrics(SM_CYFULLSCREEN);    

  // if a secondary monitor is available then get its virtual screen coordinates
  int nMonitors = GetSystemMetrics(SM_CMONITORS);
  if (nMonitors>1)
  {
    this->MultipleMonitorsAvailable = true; 
    if (!EnumDisplayMonitors(NULL, NULL, StoreNonPrimaryMonitorRectEnumProc, (LPARAM) &rect))
    {
      vtkErrorMacro("DetectMonitors: EnumDisplayMonitors failed");
    }
  }
  else
  {
    this->MultipleMonitorsAvailable = false; 
  }

  this->SecondaryMonitorPosition[0]=rect.left;
  this->SecondaryMonitorPosition[1]=rect.top;
  this->SecondaryMonitorSize[0]=rect.right-rect.left;
  this->SecondaryMonitorSize[1]=rect.bottom-rect.top;
}

#else // _WIN32

void vtkSlicerSecondaryViewerWindow::UpdateSecondaryMonitorPoisition()
{
  // TODO: implement monitor detection for linux
  this->MultipleMonitorsAvailable = false; 
  this->SecondaryMonitorPosition[0]=0;
  this->SecondaryMonitorPosition[1]=0;
  this->SecondaryMonitorSize[0]=960;//1280;	// 4/25/2010 ayamada
  this->SecondaryMonitorSize[1]=600;//800;	// 4/25/2010 ayamada
}

#endif // _WIN32

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::CreateWidget()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app==NULL)
  {
    vtkErrorMacro("CreateWindow: application is invalid");
    return;
  }

  if (this->IsCreated())
  {
    return;
  }

  vtkKWTopLevel::CreateWidget();

  //this->SetMasterWindow (this->GetServerMenuButton() );
  this->SetApplication ( app );
  //this->Create();
  this->SetBorderWidth ( 1 );
  this->SetReliefToFlat();

  //this->SetParent (this->GetApplicationGUI()->GetMainSlicerWindow());

  this->SetTitle ("3D Slicer -- Secondary Window with OpenCV -- ");	// 10.01.23 ayamada
//  this->SetSize (450, 100);
  this->SetSize (640, 480);	// 10.01.23 ayamada
  this->Withdraw();
  //this->SetDeleteWindowProtocolCommand ( this, "DestroySecondaryMonitorWindow");

  this->MainFrame->SetParent ( this );
  this->MainFrame->Create();
  this->MainFrame->SetBorderWidth ( 3 );	// 10.01.23 ayamada

  this->ViewerWidget->SetApplication( app );
  this->ViewerWidget->SetParent(this->MainFrame);

/*10.01.27-komura
  this->Script("place %s -relx 0 -rely 0 -anchor nw -relwidth 0.4 -relheight 1", 
  	       this->MainFrame->GetWidgetName() );//10.01.25 ayamada
*/
  this->Script("place %s -relx 0 -rely 0 -anchor nw -relwidth 0.4 -relheight 0.5", 
  	       this->MainFrame->GetWidgetName() );//10.01.25 ayamada


//----10.01.12-komura
  this->rw->SetParent(this);
//  this->rw->SetParent(this->MainFrame);
  this->rw->Create();

  this->rw->RemoveInteractionBindings();//10.01.22-komura
//  this->rw->IsPickingOff();//10.01.25 ayamada


  this->Script("place %s -relx 0.4 -rely 0 -anchor nw -relwidth 0.6 -relheight 1", 
  	       this->rw->GetWidgetName());//10.01.25 ayamada

//----10.01.27-komura
  this->rwLeft->SetParent(this);
  this->rwLeft->Create();
  this->rwLeft->RemoveInteractionBindings();
  this->rwLeft->SetRendererBackgroundColor(0.1, 0.1, 0.3); // 4/25/2010 ayamada    
  this->Script("place %s -relx 0 -rely 0.5 -anchor nw -relwidth 0.4 -relheight 0.5", 
  	       this->rwLeft->GetWidgetName());
//----

  //
  // Make 3D Viewer
  //

  // add events (for ViewerWidget and FiducialListWidget)
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  // use the events for the fiducial list widget as well

  this->ViewerWidget->Create();
  this->ViewerWidget->GetMainViewer()->SetRendererBackgroundColor (app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );
//  this->ViewerWidget->GetMainViewer()->SetRendererBackgroundColor (0,0,0);	// 10.01.23 ayamada
  this->ViewerWidget->SetAndObserveMRMLSceneEvents (app->GetApplicationGUI()->GetMRMLScene(), events );
  this->ViewerWidget->UpdateFromMRML();
  this->ViewerWidget->SetApplicationLogic ( app->GetApplicationGUI()->GetApplicationLogic() );

  this->ViewerWidget->PackWidget(this->MainFrame);
}

//----------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWindow::DisplayOnSecondaryMonitor()
{
  //-- display
  this->DeIconify();
  this->Raise();

  this->UpdateSecondaryMonitorPoisition();

  this->SetPosition(this->SecondaryMonitorPosition[0], this->SecondaryMonitorPosition[1]);
  this->SetSize(this->SecondaryMonitorSize[0], this->SecondaryMonitorSize[1]);

  if (this->MultipleMonitorsAvailable)
  {
    this->HideDecorationOn();
  }

}


//void vtkSlicerSecondaryViewerWindow::SetCurrentTarget(vtkTRProstateBiopsyTargetDescriptor* target)
//{
//  if (this->CurrentTarget!=target)
//  {
//    if (this->CurrentTarget)
//    {
//      this->CurrentTarget->UnRegister(this);
//    }
//    this->CurrentTarget=target;
//    if (this->CurrentTarget)
//    {
//      this->CurrentTarget->Register(this);
//    }
//  }
//  
//  vtkCornerAnnotation *anno = this->ViewerWidget->GetMainViewer()->GetCornerAnnotation();
//  if (anno)
//    {
//    static double lsf=10.0;
//    static double nlsf=0.35;
//    anno->SetLinearFontScaleFactor(lsf);
//    anno->SetNonlinearFontScaleFactor(nlsf);
//    anno->SetText(1, "Encoders not connected");
//
//    if (this->CurrentTarget!=NULL)
//    {
//      std::ostrstream os;        
//      os << this->CurrentTarget->GetFiducialIndex()+1 << ". " << this->CurrentTarget->GetNeedleTypeString() << "   "<<this->CurrentTarget->GetRASLocationString().c_str()<<std::endl;
//      os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(1);      
//      os << "Rotation: "<<this->CurrentTarget->GetAxisRotation()<<" deg"<<std::endl;
//      os << "Reachable: "<<this->CurrentTarget->GetReachableString().c_str()<<std::endl;
//      os << "Needle angle: "<<this->CurrentTarget->GetNeedleAngle()<<" deg"<<std::endl;
//      os << "Depth: "<<this->CurrentTarget->GetDepthCM()<<" cm"<<std::endl;            
//      os << std::ends;
//      anno->SetText(3, os.str());
//      os.rdbuf()->freeze();
//    }
//    else
//    {
//      anno->SetText(3, "No target");
//    }
//
//    this->ViewerWidget->GetMainViewer()->CornerAnnotationVisibilityOn();
//    // update the annotations
//    this->ViewerWidget->GetMainViewer()->Render();
//    } 
//}
