#pragma once
#include <utility>
#include <vector>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>

class APICreateMidSurfaceHelper
{
public:
	enum FacePosition {
		Face_Upper,
		Face_Lower
	};

	static std::pair<TopoDS_Face, TopoDS_Face> GetIntersectionFacesWithCenterXOY(const TopoDS_Shape& model);

	static void FindMinMaxXPoints(const TopoDS_Shape& shape, gp_Pnt& minXPoint, gp_Pnt& maxXPoint, FacePosition facePos);

	static TopoDS_Wire GetOuterWire(const TopoDS_Face& face);
	
	static void CollectEdgesFromStartToEnd(const TopoDS_Wire& wire, const gp_Pnt& startPnt, const gp_Pnt& endPnt, std::vector<TopoDS_Edge>& forwardEdges, std::vector<TopoDS_Edge>& backwardEdges);
	
};