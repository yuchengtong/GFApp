#pragma once
#include <TopoDS_Shape.hxx>
#include <Standard_Real.hxx>
#include "TriangleStructure.h"

class APIRotateMeshHelper
{
public:
	static bool RotateAroundCenter(TriangleStructure& mesh,
        const TopoDS_Shape& shape,
        const gp_Dir& rotationAxis,
        Standard_Real angleRadians);

};