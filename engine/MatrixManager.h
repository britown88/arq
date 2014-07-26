#pragma once

#include "Entity.h"
#include "Renderer.h"
#include "Matrix.h"

class MatrixManager : public IManager
{
public:
   virtual const Matrix *getMatrix(Entity *e)=0;
};

std::unique_ptr<MatrixManager> buildMatrixManager();