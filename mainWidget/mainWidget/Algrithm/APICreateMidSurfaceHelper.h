#pragma once
#include <utility>


#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

class APICreateMidSurfaceHelper
{
public:
	static std::pair<TopoDS_Face, TopoDS_Face> GetIntersectionFacesWithCenterXOY(const TopoDS_Shape& model);

	static void FindMinMaxXPoints(const TopoDS_Shape& shape, gp_Pnt& minXPoint, gp_Pnt& maxXPoint);

	static TopoDS_Wire CreateWireFromPointToPoint(const TopoDS_Wire& originalWire, const gp_Pnt& startPoint, const gp_Pnt& endPoint);

	static bool SplitWireByMinMaxX(const TopoDS_Wire& originalWire, TopoDS_Wire& wire1, TopoDS_Wire& wire2);

	static bool IsSamePoint(const gp_Pnt& p1, const gp_Pnt& p2, double tolerance);

	static TopoDS_Wire GetOuterWire(const TopoDS_Face& face);

	static TopoDS_Wire GetContour(const TopoDS_Face& face, const gp_Pnt& startPoint, const gp_Pnt& endPoint);

	static TopoDS_Face CreateNewFace(const TopoDS_Face& faceA, const TopoDS_Face& faceB);

	static TopoDS_Wire CreateNewframe(const TopoDS_Face& faceA, const TopoDS_Face& faceB);
};