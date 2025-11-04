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

void APICreateMidSurfaceHelper::FindMinMaxXPoints(const TopoDS_Shape& shape, gp_Pnt& minXPoint, gp_Pnt& maxXPoint)
{
	double minX = DBL_MAX;
	double maxX = -DBL_MAX;
	minXPoint = gp_Pnt();
	maxXPoint = gp_Pnt();

	// 如果是面，获取其外轮廓wire
	TopoDS_Shape shapeToExplore = shape;
	if (shape.ShapeType() == TopAbs_FACE) {
		TopExp_Explorer wireExplorer(shape, TopAbs_WIRE);
		if (wireExplorer.More()) {
			shapeToExplore = wireExplorer.Current();
		}
	}

	// 遍历所有边
	TopExp_Explorer edgeExplorer(shapeToExplore, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
		BRepAdaptor_Curve curve(edge);

		double first = curve.FirstParameter();
		double last = curve.LastParameter();

		// 检查起点
		gp_Pnt startPoint = curve.Value(first);
		if (startPoint.X() < minX) {
			minX = startPoint.X();
			minXPoint = startPoint;
		}
		if (startPoint.X() > maxX) {
			maxX = startPoint.X();
			maxXPoint = startPoint;
		}

		// 检查终点
		gp_Pnt endPoint = curve.Value(last);
		if (endPoint.X() < minX) {
			minX = endPoint.X();
			minXPoint = endPoint;
		}
		if (endPoint.X() > maxX) {
			maxX = endPoint.X();
			maxXPoint = endPoint;
		}
	}
}

TopoDS_Wire APICreateMidSurfaceHelper::CreateWireFromPointToPoint(const TopoDS_Wire& originalWire,
	const gp_Pnt& startPoint, const gp_Pnt& endPoint)
{
	std::vector<TopoDS_Edge> edges;

	auto IsSamePoint = [](const gp_Pnt& p1, const gp_Pnt& p2, double tolerance = 1e-6)
	{
		return p1.Distance(p2) < tolerance;
	};

	TopExp_Explorer edgeExplorer(originalWire, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) 
	{
		edges.push_back(TopoDS::Edge(edgeExplorer.Current()));
	}

	// 找到起点所在的边
	int startIndex = -1;
	for (size_t i = 0; i < edges.size(); i++) {
		TopoDS_Edge edge = edges[i];
		BRepAdaptor_Curve curve(edge);

		double first = curve.FirstParameter();
		double last = curve.LastParameter();

		gp_Pnt edgeStart = curve.Value(first);
		gp_Pnt edgeEnd = curve.Value(last);

		if (IsSamePoint(edgeStart, startPoint) || IsSamePoint(edgeEnd, startPoint)) {
			startIndex = i;
			break;
		}
	}

	if (startIndex == -1) {
		return TopoDS_Wire();
	}

	BRepBuilderAPI_MakeWire wireBuilder;
	bool foundEnd = false;
	int currentIndex = startIndex;
	int count = 0;

	while (!foundEnd && count < edges.size())
	{
		TopoDS_Edge currentEdge = edges[currentIndex];
		wireBuilder.Add(currentEdge);

		// 检查当前边的终点是否是目标终点
		BRepAdaptor_Curve curve(currentEdge);
		gp_Pnt edgeEnd = curve.Value(curve.LastParameter());

		if (IsSamePoint(edgeEnd, endPoint)) {
			foundEnd = true;
		}
		else {
			// 移动到下一条边
			currentIndex = (currentIndex + 1) % edges.size();
			count++;
		}
	}

	if (wireBuilder.IsDone() && foundEnd) {
		return wireBuilder.Wire();
	}

	return TopoDS_Wire();
}


bool APICreateMidSurfaceHelper::SplitWireByMinMaxX(const TopoDS_Wire& originalWire, TopoDS_Wire& wire1, TopoDS_Wire& wire2)
{
	gp_Pnt minXPoint, maxXPoint;

	//std::pair<gp_Pnt, gp_Pnt> points= FindMinMaxXPoints(originalWire);
	//minXPoint = points.first;
	//maxXPoint = points.second;

	//wire1 = CreateWireFromPointToPoint(originalWire, minXPoint, maxXPoint);
	//wire2 = CreateWireFromPointToPoint(originalWire, maxXPoint, minXPoint);

	return !wire1.IsNull() && !wire2.IsNull();
}


// 检查两个点是否相同（在容差范围内）
bool APICreateMidSurfaceHelper::IsSamePoint(const gp_Pnt& p1, const gp_Pnt& p2, double tolerance = 1e-6)
{
	return p1.Distance(p2) < tolerance;
}

// 获取面的外轮廓wire
TopoDS_Wire APICreateMidSurfaceHelper::GetOuterWire(const TopoDS_Face& face)
{
	TopExp_Explorer wireExplorer(face, TopAbs_WIRE);
	if (wireExplorer.More()) {
		return TopoDS::Wire(wireExplorer.Current());
	}
	return TopoDS_Wire();
}

// 获取面的轮廓线（从startPoint到endPoint）
TopoDS_Wire APICreateMidSurfaceHelper::GetContour(const TopoDS_Face& face, const gp_Pnt& startPoint, const gp_Pnt& endPoint)
{
	TopoDS_Wire outerWire = GetOuterWire(face);
	if (outerWire.IsNull()) {
		return TopoDS_Wire();
	}

	std::vector<TopoDS_Edge> edges;

	// 收集所有边
	TopExp_Explorer edgeExplorer(outerWire, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) {
		edges.push_back(TopoDS::Edge(edgeExplorer.Current()));
	}

	// 找到起点所在的边
	int startIndex = -1;
	for (size_t i = 0; i < edges.size(); i++) {
		TopoDS_Edge edge = edges[i];
		BRepAdaptor_Curve curve(edge);

		double first = curve.FirstParameter();
		double last = curve.LastParameter();

		gp_Pnt edgeStart = curve.Value(first);
		gp_Pnt edgeEnd = curve.Value(last);

		if (IsSamePoint(edgeStart, startPoint) || IsSamePoint(edgeEnd, startPoint)) {
			startIndex = i;
			break;
		}
	}

	if (startIndex == -1) {
		return TopoDS_Wire();
	}

	// 沿着wire收集边，直到遇到终点
	BRepBuilderAPI_MakeWire wireBuilder;
	bool foundEnd = false;
	int currentIndex = startIndex;
	int count = 0;

	while (!foundEnd && count < edges.size()) {
		TopoDS_Edge currentEdge = edges[currentIndex];
		wireBuilder.Add(currentEdge);

		// 检查当前边的终点是否是目标终点
		BRepAdaptor_Curve curve(currentEdge);
		gp_Pnt edgeEnd = curve.Value(curve.LastParameter());

		if (IsSamePoint(edgeEnd, endPoint)) {
			foundEnd = true;
		}
		else {
			// 移动到下一条边
			currentIndex = (currentIndex + 1) % edges.size();
			count++;
		}
	}

	if (wireBuilder.IsDone() && foundEnd) {
		return wireBuilder.Wire();
	}

	return TopoDS_Wire();
}



// 主函数：创建新面
TopoDS_Face APICreateMidSurfaceHelper::CreateNewFace(const TopoDS_Face& faceA, const TopoDS_Face& faceB)
{
	// 找到面A和面B的X最小和最大点
	gp_Pnt minXA, maxXA, minXB, maxXB;


	FindMinMaxXPoints(faceA, minXA, maxXA);
	FindMinMaxXPoints(faceB, minXB, maxXB);

	// 获取面A的下轮廓线（从minXA到maxXA）
	TopoDS_Wire bottomContourA = GetContour(faceA, minXA, maxXA);

	// 获取面B的上轮廓线（从maxXB到minXB）
	TopoDS_Wire topContourB = GetContour(faceB, maxXB, minXB);

	// 创建连接边
	TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(maxXA, maxXB); // 连接A的maxX到B的maxX
	TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(minXB, minXA); // 连接B的minX到A的minX

	// 构建封闭的wire
	BRepBuilderAPI_MakeWire newWireBuilder;
	newWireBuilder.Add(bottomContourA);
	newWireBuilder.Add(edge1);
	newWireBuilder.Add(topContourB);
	newWireBuilder.Add(edge2);

	if (!newWireBuilder.IsDone()) {
		return TopoDS_Face();
	}

	// 创建面
	BRepBuilderAPI_MakeFace newFaceBuilder(newWireBuilder.Wire(), true); // true表示平面
	if (!newFaceBuilder.IsDone()) {
		return TopoDS_Face();
	}

	return newFaceBuilder.Face();
}

TopoDS_Wire APICreateMidSurfaceHelper::CreateNewframe(const TopoDS_Face& faceA, const TopoDS_Face& faceB)
{
	// 找到面A和面B的X最小和最大点
	gp_Pnt minXA, maxXA, minXB, maxXB;


	FindMinMaxXPoints(faceA, minXA, maxXA);
	FindMinMaxXPoints(faceB, minXB, maxXB);

	// 获取面A的下轮廓线（从minXA到maxXA）
	TopoDS_Wire bottomContourA = GetContour(faceA, minXA, maxXA);

	// 获取面B的上轮廓线（从maxXB到minXB）
	TopoDS_Wire topContourB = GetContour(faceB, maxXB, minXB);

	// 创建连接边
	TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(maxXA, maxXB); // 连接A的maxX到B的maxX
	TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(minXB, minXA); // 连接B的minX到A的minX

	// 构建封闭的wire
	BRepBuilderAPI_MakeWire newWireBuilder;
	newWireBuilder.Add(bottomContourA);
	newWireBuilder.Add(topContourB);

	return newWireBuilder.Wire();
}



