/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkFluorescenceImageSource.h,v $
  Language:  C++
  Date:      $Date: 2009/09/14 13:57:30 $
  Version:   $Revision: 1.10 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkFluorescenceImageSource_h
#define __itkFluorescenceImageSource_h

#include "itkParameterizedImageSource.h"
#include "itkNumericTraits.h"

#include <FluorescenceImageSource.h>


namespace itk
{

/** \class FluorescenceImageSource
 * \brief Generate a fluorescence image from given model object parameters.
 *
 * \ingroup DataSources
 */
template <class TOutputImage>
class FluorescenceImageSource : public ParameterizedImageSource<TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef FluorescenceImageSource         Self;
  typedef ParameterizedImageSource<TOutputImage> Superclass;
  typedef SmartPointer<Self>        Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Typedef for the output image PixelType. */
  typedef TOutputImage                        OutputImageType;
  typedef typename OutputImageType::PixelType PixelType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  itkStaticConstMacro(ImageDimension,
		      unsigned int,
		      TOutputImage::ImageDimension);

  /** Run-time type information (and related methods). */
  itkTypeMacro(FluorescenceImageSource,ParameterizedImageSource);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  typedef typename Superclass::ParametersValueType ParametersValueType;
  typedef typename Superclass::ParametersType      ParametersType;
  
  /** Specify the origin of the output image (in nanometers). */
  itkSetVectorMacro(Origin,double,TOutputImage::ImageDimension);

  /** Get the origin of the output image (in nanometers). */
  itkGetVectorMacro(Origin,double,TOutputImage::ImageDimension);

  void SetFluorescenceImageSource(FluorescenceImageSource* source);

  /** Expects the parameters argument to contain values for ALL parameters. */
  virtual void SetParameters(const ParametersType& parameters);

  /** Gets the full parameters list. */
  virtual ParametersType GetParameters() const;

  /** Gets the total number of parameters. */
  virtual unsigned int GetNumberOfParameters() const;


protected:
  FluorescenceImageSource();
  ~FluorescenceImageSource();
  void PrintSelf(std::ostream& os, Indent indent) const;
  
  virtual void 
  ThreadedGenerateData(const OutputImageRegionType& 
                       outputRegionForThread, int threadId );
  virtual void GenerateOutputInformation();


private:
  FluorescenceImageSource(const FluorescenceImageSource&); //purposely not implemented
  void operator=(const FluorescenceImageSource&); //purposely not implemented

  unsigned long *m_Size;         //size of the output image
  double        *m_Spacing;      //spacing
  double        *m_Origin;       //origin

  FluorescenceImageSource* m_ImageSource;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkFluorescenceImageSource.cxx"
#endif

#endif
