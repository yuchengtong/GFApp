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
		Face_Upper,  // 上方的面
		Face_Lower   // 下方的面
	};


	static std::pair<TopoDS_Face, TopoDS_Face> GetIntersectionFacesWithCenterXOY(const TopoDS_Shape& model);

	static void FindMinMaxXPoints(const TopoDS_Shape& shape, gp_Pnt& minXPoint, gp_Pnt& maxXPoint, FacePosition facePos);

	//static void FindMinMaxXPoints(const TopoDS_Shape& shape, gp_Pnt& minXPoint, gp_Pnt& maxXPoint);

	static void ProcessVertex(const gp_Pnt& vertex, double& minX, double& maxX, gp_Pnt& minXPoint, gp_Pnt& maxXPoint, FacePosition facePos);

	static TopoDS_Wire CreateWireFromPointToPoint(const TopoDS_Wire& originalWire, const gp_Pnt& startPoint, const gp_Pnt& endPoint);

	static bool SplitWireByMinMaxX(const TopoDS_Wire& originalWire, TopoDS_Wire& wire1, TopoDS_Wire& wire2);

	static bool IsSamePoint(const gp_Pnt& p1, const gp_Pnt& p2, double tolerance);

	static TopoDS_Wire GetOuterWire(const TopoDS_Face& face);

	static TopoDS_Wire GetContour(const TopoDS_Face& face, const gp_Pnt& startPoint, const gp_Pnt& endPoint);

	static TopoDS_Face CreateNewFace(const TopoDS_Face& faceA, const TopoDS_Face& faceB);

	static TopoDS_Wire CreateNewframe(const TopoDS_Face& faceA, const TopoDS_Face& faceB);
	static std::vector<gp_Pnt> GetContourVertices(const TopoDS_Face& face);
	static bool IsPointEqual(const gp_Pnt& p1, const gp_Pnt& p2);
	static void CollectEdgesFromStartToEnd(const TopoDS_Wire& wire, const gp_Pnt& startPnt, const gp_Pnt& endPnt, std::vector<TopoDS_Edge>& forwardEdges);
	static void CollectEdgesFromEndToStart(const TopoDS_Wire& wire, const gp_Pnt& startPnt, const gp_Pnt& endPnt, std::vector<TopoDS_Edge>& backwardEdges);
};