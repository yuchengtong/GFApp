#pragma once
#include <utility>
#include <vector>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <V3d_View.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <gp_Pnt.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <Standard_TypeDef.hxx>




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

	static TopoDS_Face CreateConnectingFace(const std::vector<TopoDS_Edge>& backwardEdges, const std::vector<TopoDS_Edge>& forwardEdgesB, const gp_Pnt& minXA, const gp_Pnt& maxXA, const gp_Pnt& minXB, const gp_Pnt& maxXB);

	static void MeshAndDisplayFace(const TopoDS_Face& faceA, Handle(AIS_InteractiveContext)& context, Handle(V3d_View)& view);

	static TopoDS_Shape GenerateMeshWireframeForFace(const TopoDS_Face& face, double meshSize);

};