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

#ifndef __vtkSecondaryWindowWithOpenCVWin32Header_h
#define __vtkSecondaryWindowWithOpenCVWin32Header_h

#include <vtkSecondaryWindowWithOpenCVConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(SecondaryWindowWithOpenCV_EXPORTS)
#define VTK_SecondaryWindowWithOpenCV_EXPORT __declspec( dllexport ) 
#else
#define VTK_SecondaryWindowWithOpenCV_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_SecondaryWindowWithOpenCV_EXPORT 
#endif
#endif
