#include <Matrix.h>

#include <algorithm>
#include <iostream>

extern "C" {
  #include <f2c.h>
  #include <clapack.h>
}


Matrix
::Matrix(unsigned int rows, unsigned int columns) {
  m_Elements = new double[rows*columns];
  m_Rows = rows;
  m_Columns = columns;
}


Matrix
::~Matrix() {
  if (m_Elements) {
    delete[] m_Elements;
  }
}


void
Matrix
::SetElement(unsigned int row, unsigned int column, double value) {
  if (row < m_Rows && column < m_Columns) {
    // Data is stored in column-major order to make it easy to pass to
    // CLAPACK routines.
    m_Elements[column*m_Rows + row] = value;
  }
}


double
Matrix
::GetElement(unsigned int row, unsigned int column) {
  if (row < m_Rows && column < m_Columns) { 
    return m_Elements[column*m_Rows + row];
  }

  return 0.0;
}


double*
Matrix
::GetPointer() {
  return m_Elements;
}


void
Matrix
::LinearLeastSquaresSolve(double* x, double* b) {

  char trans = 'N';
  integer m = static_cast<integer>(m_Rows);
  integer n = static_cast<integer>(m_Columns);
  integer nrhs = 1;
  integer lda = m;
  integer ldb = static_cast<int>(m_Rows > m_Columns ? m_Rows : m_Columns);
  integer info = 0;

  // lwork must be greater than or equal to max(1, mn + max(mn, nrhs)) where
  // mn = min(m, n)
  integer mn = min(m, n);
  integer lwork = max(1, mn + max(mn, nrhs));
  double* work = new double[lwork];

  dgels_(&trans, &m, &n, &nrhs, m_Elements, &lda, b, &ldb, work, &lwork, &info);

  for (unsigned int i = 0; i < m_Columns; i++) {
    x[i] = b[i];
  }

  delete[] work;
}


void
Matrix
::PrintSelf() {
  for (unsigned int i = 0; i < m_Rows; i++) {
    for (unsigned int j = 0; j < m_Columns; j++) {
      printf("%9.04f ", GetElement(i, j));
    }
    printf("\n");
  }
}
