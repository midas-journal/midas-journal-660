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
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkSecondaryWindowWithOpenCVLogic.h"

vtkCxxRevisionMacro(vtkSecondaryWindowWithOpenCVLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSecondaryWindowWithOpenCVLogic);

//---------------------------------------------------------------------------
vtkSecondaryWindowWithOpenCVLogic::vtkSecondaryWindowWithOpenCVLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkSecondaryWindowWithOpenCVLogic::DataCallback);

}


//---------------------------------------------------------------------------
vtkSecondaryWindowWithOpenCVLogic::~vtkSecondaryWindowWithOpenCVLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkSecondaryWindowWithOpenCVLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkSecondaryWindowWithOpenCVLogic *self = reinterpret_cast<vtkSecondaryWindowWithOpenCVLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkSecondaryWindowWithOpenCVLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkSecondaryWindowWithOpenCVLogic::UpdateAll()
{

}

