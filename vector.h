#ifndef _VECTOR_H_
#define _VECTOR_H_

extern void VecNormalize(Vec vec);
extern Flt VecNormalizeL(Vec vec);
extern void makeUnitNormalFromVerts(Vec tp[], Vec norm);
extern void makeUnitNormalFromVerts3(Vec p0, Vec p1, Vec p2, Vec norm);
extern void makeNormalFromVerts(Vec tp[], Vec norm);
extern void identity(Matrix mat);
extern void matrixCat(Matrix m1, Matrix m2, Matrix dest);
extern void transVector(Matrix mat, Vec in, Vec out);
extern void transNormal(Matrix mat, Vec in, Vec out);
extern void concatenateToMatrix(Transform *t);
extern void matrixInverse(Matrix in, Matrix out);
extern void matrixCopy(Matrix m1, Matrix m2);
extern void makeViewMatrix(Vec p1, Vec p2, Matrix m);

#endif //_VECTOR_H_

