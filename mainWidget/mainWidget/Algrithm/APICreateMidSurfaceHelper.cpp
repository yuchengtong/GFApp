#include "APICreateMidSurfaceHelper.h"
#include <vector>
#include <algorithm>

#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepExtrema_DistShapeShape.hxx>

#include <BRepGProp.hxx>
#include <BRep_Tool.hxx>

#include <Bnd_Box.hxx>

#include <GCPnts_AbscissaPoint.hxx>
#include <Geom_Plane.hxx>
#include <GProp_GProps.hxx>
#include <gp_Pln.hxx>

#include <ShapeAnalysis.hxx>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>


std::pair<TopoDS_Face, TopoDS_Face> APICreateMidSurfaceHelper::GetIntersectionFacesWithCenterXOY(const TopoDS_Shape& model)
{
	TopoDS_Face upFace, downFace2;

	auto CalculateModelCenter = [](const TopoDS_Shape& model) -> gp_Pnt {
		GProp_GProps props;
		BRepGProp::VolumeProperties(model, props);
		return props.CentreOfMass();
	};
	gp_Pnt modelCenter = CalculateModelCenter(model);
	gp_Pln xoyPlaneAtCenter(gp_Pnt(modelCenter.X(), modelCenter.Y(), modelCenter.Z()), gp_Dir(0, 0, 1));

	Bnd_Box bbox;
	BRepBndLib::Add(model, bbox);
	Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
	bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

	Standard_Real margin = std::max(xMax - xMin, yMax - yMin) * 0.1;
	TopoDS_Face planeFace = BRepBuilderAPI_MakeFace(
		xoyPlaneAtCenter, xMin - margin, xMax + margin, yMin - margin, yMax + margin
	);

	BRepAlgoAPI_Common common(model, planeFace);
	common.Build();
	if (!common.IsDone()) 
	{
		return { upFace, downFace2 };
	}

	TopoDS_Shape intersection = common.Shape();

	std::vector<std::pair<double, TopoDS_Face>> candidateFaces;
	Standard_Real tolerance = 1e-6;
	for (TopExp_Explorer faceExp(intersection, TopAbs_FACE); faceExp.More(); faceExp.Next())
	{
		TopoDS_Face face = TopoDS::Face(faceExp.Current());
		Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
		Handle(Geom_Plane) facePlane = Handle(Geom_Plane)::DownCast(surface);
		if (facePlane.IsNull())
		{
			continue;
		}

		const gp_Pln& pln = facePlane->Pln();
		if (pln.Position().IsCoplanar(xoyPlaneAtCenter.Position(), tolerance, tolerance)) 
		{
			GProp_GProps props;
			BRepGProp::SurfaceProperties(face, props);
			double area = props.Mass();
			if (area > 1e-9) 
				candidateFaces.emplace_back(area, face);
		}
	}

	std::sort(candidateFaces.begin(), candidateFaces.end(),
		[](const auto& a, const auto& b) { return a.first > b.first; });

	auto calculateFaceCentroidY = [](const TopoDS_Face& face) -> double {
		if (face.IsNull())
		{
			return -1e20;
		}

		GProp_GProps props;
		BRepGProp::SurfaceProperties(face, props);
		return props.CentreOfMass().Y();
	};

	upFace = calculateFaceCentroidY(candidateFaces[0].second) > calculateFaceCentroidY(candidateFaces[1].second) ? candidateFaces[0].second : candidateFaces[1].second;
	downFace2 = (upFace.IsEqual(candidateFaces[0].second)) ? candidateFaces[1].second : candidateFaces[0].second;
	return { upFace, downFace2 };
}

void APICreateMidSurfaceHelper::FindMinMaxXPoints(
	const TopoDS_Shape& shape,
	gp_Pnt& minXPoint,
	gp_Pnt& maxXPoint,
	FacePosition facePos
)
{
	double minX = DBL_MAX;
	double maxX = -DBL_MAX;
	minXPoint = gp_Pnt();
	maxXPoint = gp_Pnt();

	TopoDS_Shape shapeToExplore = shape;
	if (shape.ShapeType() == TopAbs_FACE) 
	{
		TopExp_Explorer wireExplorer(shape, TopAbs_WIRE);
		if (wireExplorer.More()) 
		{
			shapeToExplore = wireExplorer.Current();
		}
	}


	TopExp_Explorer edgeExplorer(shapeToExplore, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) 
	{
		TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
		TopoDS_Vertex startVtx, endVtx;

		TopExp::Vertices(edge, startVtx, endVtx);
		if (startVtx.IsNull() || endVtx.IsNull())
		{
			continue;
		}

		gp_Pnt startPnt = BRep_Tool::Pnt(startVtx);
		gp_Pnt endPnt = BRep_Tool::Pnt(endVtx);

		auto ProcessVertex=[](
			const gp_Pnt & vertex,
			double& minX,
			double& maxX,
			gp_Pnt & minXPoint,
			gp_Pnt & maxXPoint,
			FacePosition facePos
		)
		{
			double x = vertex.X();
			double y = vertex.Y();
			const double eps = Precision::Confusion();


			if (x < minX - eps)
			{
				minX = x;
				minXPoint = vertex;
			}
			else if (x <= minX + eps)
			{
				if (facePos == Face_Upper)
				{
					if (y < minXPoint.Y() - eps)
					{
						minXPoint = vertex;
					}
				}
				else
				{
					if (y > minXPoint.Y() + eps)
					{
						minXPoint = vertex;
					}
				}
			}

			if (x > maxX + eps)
			{
				maxX = x;
				maxXPoint = vertex;
			}
			else if (x >= maxX - eps) {
				if (facePos == Face_Upper)
				{
					if (y < maxXPoint.Y() - eps)
					{
						maxXPoint = vertex;
					}
				}
				else
				{
					if (y > maxXPoint.Y() + eps)
					{
						maxXPoint = vertex;
					}
				}
			}
		};

		ProcessVertex(startPnt, minX, maxX, minXPoint, maxXPoint, facePos);
		ProcessVertex(endPnt, minX, maxX, minXPoint, maxXPoint, facePos);
	}
}


TopoDS_Wire APICreateMidSurfaceHelper::GetOuterWire(const TopoDS_Face& face)
{
	TopExp_Explorer wireExplorer(face, TopAbs_WIRE);
	if (wireExplorer.More())
	{
		return TopoDS::Wire(wireExplorer.Current());
	}
	return TopoDS_Wire();
}

void APICreateMidSurfaceHelper::CollectEdgesFromStartToEnd(
	const TopoDS_Wire& wire,
	const gp_Pnt& startPnt,
	const gp_Pnt& endPnt,
	std::vector<TopoDS_Edge>& forwardEdges,  // 起点到终点的边
	std::vector<TopoDS_Edge>& backwardEdges  // 终点回到起点的边（构成完整循环）
)
{
	forwardEdges.clear();
	backwardEdges.clear();

	std::vector<TopoDS_Edge> allEdges;
	TopExp_Explorer edgeExplorer(wire, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) 
	{
		allEdges.push_back(TopoDS::Edge(edgeExplorer.Current()));
	}

	size_t startIndex = allEdges.size();
	size_t endIndex = allEdges.size();
	bool isCollecting = false;

	for (size_t i = 0; i < allEdges.size(); ++i) {
		const TopoDS_Edge& edge = allEdges[i];
		TopoDS_Vertex vtx1, vtx2;
		TopExp::Vertices(edge, vtx1, vtx2);
		gp_Pnt p1 = BRep_Tool::Pnt(vtx1);
		gp_Pnt p2 = BRep_Tool::Pnt(vtx2);

		if (!isCollecting) 
		{		
			if (p1.IsEqual(startPnt, Precision::Confusion()))
			{
				isCollecting = true;
				startIndex = i; 
				forwardEdges.push_back(edge);
				
				if (p2.IsEqual(endPnt, Precision::Confusion()))
				{
					endIndex = i;
					break;
				}
			}
		}
		else {
			forwardEdges.push_back(edge);
			if (p2.IsEqual(endPnt, Precision::Confusion()))
			{
				endIndex = i; 
				break;
			}
		}
	}

	for (size_t i = endIndex + 1; i < allEdges.size(); ++i)
	{
		backwardEdges.push_back(allEdges[i]);
	}

	for (size_t i = 0; i < startIndex; ++i) 
	{
		backwardEdges.push_back(allEdges[i]);
	}
}
