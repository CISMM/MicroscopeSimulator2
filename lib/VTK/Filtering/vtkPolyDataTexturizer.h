/*=========================================================================

  Written by: Chris Weigle, University of North Carolina at Chapel Hill
  Email Contact: weigle@cs.unc.edu

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE
  OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF NORTH
  CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS
  ON AN "AS IS" BASIS, AND THE UNIVERSITY OF NORTH CAROLINA HAS NO
  OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.

=========================================================================*/
#ifndef __vtkPolyDataTexturizer_h
#define __vtkPolyDataTexturizer_h

#include <vtkPolyDataAlgorithm.h>

class vtkPolyDataTexturizer : public vtkPolyDataAlgorithm {
   public:
      vtkTypeMacro(vtkPolyDataTexturizer, vtkPolyDataAlgorithm);

      static vtkPolyDataTexturizer *New();

   protected:
      vtkPolyDataTexturizer();
      ~vtkPolyDataTexturizer();

      //BTX
      int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*);

      double SeedTexturePatches(vtkPolyData*, double*, int*);
      void GrowTexturePatches(vtkPolyData*, double*, int*, int*);
      void CutTexturePatches(vtkPolyData*, vtkPolyData*);
      void ParameterizeTexturePatches(vtkPolyData*, double = 1.0);
      //ETX
   private:
      vtkPolyDataTexturizer(const vtkPolyDataTexturizer&); // not implemented
      void operator=(const vtkPolyDataTexturizer&);        // not implemented
};

#endif
