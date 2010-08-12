#ifndef _MATRIX_H_
#define _MATRIX_H_

/** Simple container class defining a matrix. */
class Matrix {

public:
  Matrix(unsigned int rows, unsigned int columns);
  virtual ~Matrix();

  void SetElement(unsigned int row, unsigned int column, double value);
  double GetElement(unsigned int row, unsigned int column);

  double* GetPointer();

  // Finds x such that ||Ax - b||_2 is minimized where A is this matrix
  // and x and b are the function parameters.
  void LinearLeastSquaresSolve(double* x, double* b);

protected:
  // Not allowed to call without setting the matrix size.
  Matrix() {};

  unsigned int m_Rows;
  unsigned int m_Columns;
  double*      m_Elements;
};

#endif // _MATRIX_H_
