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

#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkMatrix4x4.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkSlicerSecondaryViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerViewerInteractorStyle.h"

#include "vtkProp3D.h"
#include "vtkActor.h"
#include "vtkImageActor.h"
#include "vtkFollower.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkOutlineSource.h"
#include "vtkVectorText.h"
#include "vtkRenderWindow.h"
#include "vtkImplicitBoolean.h"
#include "vtkPlane.h"
#include "vtkClipPolyData.h"

#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLProceduralColorNode.h"

#include "vtkSlicerModelHierarchyLogic.h"

#include "vtkKWWidget.h"

// for picking
#include "vtkWorldPointPicker.h"
#include "vtkPropPicker.h"
#include "vtkCellPicker.h"
#include "vtkPointPicker.h"


#include "vtkSlicerBoxWidget2.h"
#include "vtkSlicerBoxRepresentation.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerSecondaryViewerWidget );
vtkCxxRevisionMacro ( vtkSlicerSecondaryViewerWidget, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerSecondaryViewerWidget::vtkSlicerSecondaryViewerWidget ( )
{

}




//---------------------------------------------------------------------------
vtkSlicerSecondaryViewerWidget::~vtkSlicerSecondaryViewerWidget ( )
{
}

//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->Superclass::PrintSelf ( os, indent );

    os << indent << "vtkSlicerSecondaryViewerWidget: " << this->GetClassName ( ) << "\n";

}

//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{ 
  if (this->ProcessingMRMLEvent != 0 )
    {
    return;
    }

  this->ProcessingMRMLEvent = event;

  vtkDebugMacro("ProcessMRMLEvents: processing event " << event);

  if (event == vtkMRMLScene::SceneClosingEvent )
    {
    this->RemoveHierarchyObservers(0);
    this->RemoveModelObservers(0);
    }
  else if (event == vtkMRMLScene::SceneCloseEvent )
    {
    this->SceneClosing = true;
    this->RemoveModelProps();
    this->RemoveHierarchyObservers(1);
    this->RemoveModelObservers(1);
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    this->UpdateFromMRML();
    //this->MainViewer->RemoveAllViewProps();
    this->RequestRender();
    }
  else 
    {
    this->SceneClosing = false;
    }

  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node = (vtkMRMLNode*) (callData);
    if (node != NULL && node->IsA("vtkMRMLDisplayableNode") )
      {
      vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast(node);
      if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        this->RemoveDisplayable(dnode);
        this->UpdateFromMRML();
        }
      else
        {
        this->UpdateFromMRMLRequested = 1;
        this->RequestRender();
        }
      }
    else if (node != NULL && node->IsA("vtkMRMLDisplayNode") )
      {
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != NULL && node->IsA("vtkMRMLModelHierarchyNode") )
      {
      this->UpdateModelHierarchies();
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      //this->UpdateFromMRML();
      }
    else if (node != NULL && node->IsA("vtkMRMLClipModelsNode") )
      {
      if (event == vtkMRMLScene::NodeAddedEvent)
        {
        vtkSetAndObserveMRMLNodeMacro(this->ClipModelsNode, node);
        }
      else if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        vtkSetMRMLNodeMacro(this->ClipModelsNode, NULL);
        }
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      //this->UpdateFromMRML();
      }
    }
  else if (vtkMRMLCameraNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessingMRML: got a camera node modified event");
    this->UpdateCameraNode();
    this->RequestRender();
    }
  else if (vtkMRMLViewNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessingMRML: got a view node modified event");
    this->UpdateViewNode();
    this->RequestRender();
    }
  else if (vtkMRMLDisplayableNode::SafeDownCast(caller) != NULL)
    {
    // check for events on a model node
    vtkMRMLDisplayableNode *modelNode = vtkMRMLDisplayableNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent ||
        event == vtkMRMLDisplayableNode::PolyDataModifiedEvent ||
        event == vtkMRMLDisplayableNode::DisplayModifiedEvent) 
      {
      // if the node is already cached with an actor process only this one
      // if it was not visible and is still not visible do nothing
      std::vector< vtkMRMLDisplayNode *> dnodes = this->GetDisplayNode(modelNode);
      bool updateModel = false;
      bool updateMRML = false;
      for (unsigned int i=0; i<dnodes.size(); i++)
        {
        vtkMRMLDisplayNode *dnode = dnodes[i];
        int visibility = dnode->GetVisibility();
        //if (hdnode)
        //  {
        //  visibility = hdnode->GetVisibility();
        //  }
        if (this->DisplayedActors.find(dnode->GetID()) == this->DisplayedActors.end())
          {
          updateMRML = true;
          break;
          }
        if (!(dnode && visibility == 0 && this->GetDisplayedModelsVisibility(dnode) == 0))
          {
          updateModel = true;
          break;
          }
        }
      if (updateModel) 
        {
        this->UpdateClipSlicesFormMRML();
        //this->UpdateClipSlicesFromMRML(); // 4/22/2010 ayamada 
        this->UpdateModifiedModel(modelNode);
        this->RequestRender( );
        }
      if (updateMRML)
        {
        this->UpdateFromMRMLRequested = 1;
        this->RequestRender();
        }
      }
    else
      {
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      //this->UpdateFromMRML();
      }
    }
  else if (vtkMRMLClipModelsNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    //this->UpdateFromMRML();
    }
  else if (vtkMRMLSliceNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent && (this->UpdateClipSlicesFormMRML() || this->ClippingOn))
      //     event == vtkCommand::ModifiedEvent && (this->UpdateClipSlicesFromMRML() || this->ClippingOn)) // 4/22/2010 ayamada
    {
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    //this->UpdateFromMRML();
    }
  else if (vtkMRMLModelHierarchyNode::SafeDownCast(caller) &&
           event == vtkCommand::ModifiedEvent)
    {
    this->UpdateModelHierarchies();
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    //this->UpdateFromMRML();
    }
  else
    {
    vtkDebugMacro("ProcessMRMLEvents: unhandled event " << event << " " << ((event == 31) ? "ModifiedEvent" : "not ModifiedEvent"));
    if (vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene) { vtkDebugMacro("\ton the mrml scene"); }
    if (vtkMRMLNode::SafeDownCast(caller) != NULL) { vtkDebugMacro("\tmrml node id = " << vtkMRMLNode::SafeDownCast(caller)->GetID()); }
    }
  
  this->ProcessingMRMLEvent = 0;
}
//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::UpdateCameraNode()
{
  if (this->SceneClosing)
    {
    return;
    }
  // find an active camera
  // or any camera if none is active
  //vtkMRMLCameraNode *node = NULL;
  
  /*
  std::vector<vtkMRMLNode *> cnodes;
  int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLCameraNode", cnodes);
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLCameraNode::SafeDownCast (cnodes[n]);
    if (node->GetActive())
      {
      break;
      }
    }

  if ( this->CameraNode != NULL && node != NULL && this->CameraNode != node)
    {
    // local CameraNode is out of sync with the scene
    this->SetAndObserveCameraNode (NULL);
    }
  if ( this->CameraNode != NULL && this->MRMLScene->GetNodeByID(this->CameraNode->GetID()) == NULL)
    {
    // local node not in the scene
    this->SetAndObserveCameraNode (NULL);
    }
  if ( this->CameraNode == NULL )
    {
    if ( node == NULL )
      {
      // no camera in the scene and local
      // create an active camera
      node = vtkMRMLCameraNode::New();
      node->SetActive(1);
      this->MRMLScene->AddNode(node);
      node->Delete();
      }
    this->SetAndObserveCameraNode (node);
    }
 */

    if ( this->CameraNode == NULL )
    {
      // no camera in the scene and local
      // create an active camera
      vtkMRMLCameraNode *node = vtkMRMLCameraNode::New();
      node->SetActive(0);
      //node->SetActiveTag(0);    // ayamada 4/22/2010        
      this->MRMLScene->AddNode(node);
      this->SetAndObserveCameraNode (node);
      node->Delete();
    }


  vtkRenderWindowInteractor *rwi = this->MainViewer->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkInteractorObserver *iobs = rwi->GetInteractorStyle();
    vtkSlicerViewerInteractorStyle *istyle = vtkSlicerViewerInteractorStyle::SafeDownCast(iobs);
    if (istyle)
      {
      istyle->SetCameraNode(this->CameraNode);
      if (istyle->GetApplicationLogic() == NULL &&
          this->GetApplicationLogic() != NULL)
        {
        vtkDebugMacro("Updating interactor style's application logic, since it was null");
        istyle->SetApplicationLogic(this->GetApplicationLogic());
        }
      }
    }
  this->MainViewer->GetRenderer()->SetActiveCamera(this->CameraNode->GetCamera());
}


//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::UpdateViewNode()
{
  if (this->SceneClosing)
    {
    return;
    }

  vtkMRMLViewNode *node =  vtkMRMLViewNode::SafeDownCast (
       this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLViewNode"));

  if ( this->ViewNode != NULL && node != NULL && this->ViewNode != node)
    {
    // local ViewNode is out of sync with the scene
    this->SetAndObserveViewNode (NULL);
    }
  if ( this->ViewNode != NULL && this->MRMLScene->GetNodeByID(this->ViewNode->GetID()) == NULL)
    {
    // local node not in the scene
    this->SetAndObserveViewNode (NULL);
    }
  if ( this->ViewNode == NULL )
    {
    if ( node == NULL )
      {
      // no view in the scene and local
      // create an active camera
      node = vtkMRMLViewNode::New();
      this->MRMLScene->AddNode(node);
      node->Delete();
      }
    this->SetAndObserveViewNode (node);
    }
 
}



//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::CreateWidget ( )
{
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();

  this->ViewerFrame = vtkKWFrame::New ( );
  this->ViewerFrame->SetParent ( this->GetParent ( ) );
  this->ViewerFrame->Create ( );

  //double *c = slicerStyle->GetHighLightColor();
  double c[3] = { 0.4, 0.0, 0.6 };
//  this->ViewerFrame->SetConfigurationOptionAsInt("-highlightthickness", 2);
  this->ViewerFrame->SetConfigurationOptionAsInt("-highlightthickness", 1);	// 10.01.23 ayamada
  this->ViewerFrame->SetConfigurationOptionAsColor("-highlightcolor",
                                                   c[0], c[1], c[2]);
  // Revert back to KW superclass renderwidget to address
  // window corruption on some linux boxes:
  //this->MainViewer = vtkSlicerRenderWidget::New ( );  
  this->MainViewer = vtkKWRenderWidget::New ( );  
  
  // added to enable active/passive stereo mode.  If user started slicer with --stereo
  // command line option, then stereo might be enabled sometime during this session.  Therefore
  // we need to request the correct OpenGL visual BEFORE the window is opened.  Quadbuffered
  // stereo has to be requested before the first Render() call on the window.
  
  if (vtkSlicerApplication::GetInstance()->GetStereoEnabled())
    {
      vtkDebugMacro("Opening Stereo Capable Window");
      vtkRenderWindow* renWin = this->MainViewer->GetRenderWindow();
      renWin->SetStereoCapableWindow(1);
      renWin->SetStereoTypeToCrystalEyes();
    }
  
  this->MainViewer->SetParent (this->ViewerFrame );
  this->MainViewer->Create ( );

  // tell the render widget not to respond to the Render() method
  // - this class turns on rendering explicitly when it's own
  //   Render() method is called.  This avoids redundant renders
  //   when, for example, the annotation is changed.
  //  TODO: this is disabled for until there is a way to observe 
  //  for direct render requests to the renderer (so they can be placed
  //  in the event queue)
  //this->MainViewer->RenderStateOff();

  // make a Slicer viewer interactor style to process our events
  // look at the InteractorStyle to get our events
  vtkRenderWindowInteractor *rwi = this->MainViewer->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkSlicerViewerInteractorStyle *iStyle = vtkSlicerViewerInteractorStyle::New();
    iStyle->SetViewerWidget( this );
    if (this->ApplicationLogic != NULL)
      {
      iStyle->SetApplicationLogic ( this->ApplicationLogic );
      }
    else
      {
      vtkDebugMacro("Not setting interactor style's application logic to null.");
      }

    rwi->SetInteractorStyle (iStyle);
    iStyle->Delete();
    }


  // Set the viewer's minimum dimension to be the modifiedState as that for
  // the three main Slice viewers.
  this->MainViewer->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();      
    this->MainViewer->SetWidth ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
    this->MainViewer->SetHeight ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
    }
    

  // observe scene for add/remove nodes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosingEvent);
  //events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEvents(this->MRMLScene, events);
  events->Delete();

  this->CreateClipSlices();

  this->CreateAxis();

  //this->PackWidget ( );
  this->MainViewer->ResetCamera ( );
}

//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::UpdateFromMRML()
{
  this->UpdateAxis();

  this->UpdateCameraNode();
  this->AddAxisActors();

  this->UpdateClipSlicesFormMRML();
  //this->UpdateClipSlicesFromMRML(); // 4/22/2010 ayamada

  this->RemoveModelProps ( );
  
  this->UpdateModelsFromMRML();

  this->RequestRender ( );

  this->UpdateFromMRMLRequested = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::UpdateModelsFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  std::vector<vtkMRMLDisplayableNode *> slices;

  // find volume slices
  bool clearDisplayedModels = false;
  
  std::vector<vtkMRMLNode *> dnodes;
  int nnodes = scene->GetNodesByClass("vtkMRMLDisplayableNode", dnodes);
  for (int n=0; n<nnodes; n++)
    {
    node = dnodes[n];
    vtkMRMLDisplayableNode *model = vtkMRMLDisplayableNode::SafeDownCast(node);
    // render slices last so that transparent objects are rendered in front of them
    if (!strcmp(model->GetName(), "Red Volume Slice") ||
        !strcmp(model->GetName(), "Green Volume Slice") ||
        !strcmp(model->GetName(), "Yellow Volume Slice"))
      {
      slices.push_back(model);
      vtkMRMLDisplayNode *dnode = model->GetDisplayNode();
      if (dnode && this->DisplayedActors.find(dnode->GetID()) == this->DisplayedActors.end() )
        {
        clearDisplayedModels = true;
        }
      }
    }

  if (clearDisplayedModels)
    {
    std::map<std::string, vtkProp3D *>::iterator iter;
    for (iter = this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++)
      {
      this->MainViewer->RemoveViewProp(iter->second);
      }
    //this->MainViewer->RemoveAllViewProps();
    this->RemoveModelObservers(1);
    this->RemoveHierarchyObservers(1);
    this->DisplayedActors.clear();
    this->DisplayedNodes.clear();
    this->DisplayedClipState.clear();
    this->DisplayedVisibility.clear();
    this->AddAxisActors();
    this->UpdateModelHierarchies();
    }

  // render slices first
  for (unsigned int i=0; i<slices.size(); i++)
    {
    vtkMRMLDisplayableNode *model = slices[i];
    // add nodes that are not in the list yet
    vtkMRMLDisplayNode *dnode = model->GetDisplayNode();
    if (dnode && this->DisplayedActors.find(dnode->GetID()) == this->DisplayedActors.end() )
      {
      this->UpdateModel(model);
      } 
    this->SetModelDisplayProperty(model);
    }
  
  // render the rest of the models
  //int nmodels = scene->GetNumberOfNodesByClass("vtkMRMLDisplayableNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLDisplayableNode *model = vtkMRMLDisplayableNode::SafeDownCast(dnodes[n]);
    // render slices last so that transparent objects are rendered in fron of them
    if (model)
      {
      if (!strcmp(model->GetName(), "Red Volume Slice") ||
          !strcmp(model->GetName(), "Green Volume Slice") ||
          !strcmp(model->GetName(), "Yellow Volume Slice"))
        {
        continue;
        }
      this->UpdateModifiedModel(model);
      }
    } // end while

}

int vtkSlicerSecondaryViewerWidget::GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model)
{
  int visibility = 1;
  
  std::map<std::string, int>::iterator iter;
  iter = this->DisplayedVisibility.find(model->GetID());
  if (iter != this->DisplayedVisibility.end())
    {
    visibility = iter->second;
    }

  return visibility;
}


//---------------------------------------------------------------------------
void vtkSlicerSecondaryViewerWidget::Render()
{
  if (this->UpdateFromMRMLRequested)
    {
    this->UpdateFromMRML();
    }
  // *** added code to check the RenderState and restore to whatever it
  // was before the specific request to render, instead of just setting
  // renderState to OFF.
  int currentRenderState = this->MainViewer->GetRenderState();
  this->MainViewer->RenderStateOn();
  this->MainViewer->Render();
  vtkDebugMacro("vtkSlicerViewerWidget::Render called render" << endl);
  //this->MainViewer->RenderStateOff();
  this->MainViewer->SetRenderState(currentRenderState);
  this->SetRenderPending(0);
}
