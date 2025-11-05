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

	// 确定要遍历的轮廓（优先取面的外轮廓Wire）
	TopoDS_Shape shapeToExplore = shape;
	if (shape.ShapeType() == TopAbs_FACE) {
		TopExp_Explorer wireExplorer(shape, TopAbs_WIRE);
		if (wireExplorer.More()) {
			shapeToExplore = wireExplorer.Current(); // 取外轮廓Wire
		}
	}

	// 遍历所有边，仅处理顶点
	TopExp_Explorer edgeExplorer(shapeToExplore, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
		TopoDS_Vertex startVtx, endVtx;

		// 提取边的两个顶点（TopExp::Vertices无返回值，通过顶点是否为Null判断有效性）
		TopExp::Vertices(edge, startVtx, endVtx);

		// 检查顶点是否有效（非Null）
		if (startVtx.IsNull() || endVtx.IsNull()) {
			continue; // 无效边（无顶点），跳过
		}

		// 提取顶点的几何坐标
		gp_Pnt startPnt = BRep_Tool::Pnt(startVtx);
		gp_Pnt endPnt = BRep_Tool::Pnt(endVtx);

		// 处理两个顶点
		ProcessVertex(startPnt, minX, maxX, minXPoint, maxXPoint, facePos);
		ProcessVertex(endPnt, minX, maxX, minXPoint, maxXPoint, facePos);
	}
}

// 辅助函数：处理单个顶点的比较逻辑
void APICreateMidSurfaceHelper::ProcessVertex(
	const gp_Pnt& vertex,
	double& minX,
	double& maxX,
	gp_Pnt& minXPoint,
	gp_Pnt& maxXPoint,
	FacePosition facePos
)
{
	double x = vertex.X();
	double y = vertex.Y();
	const double eps = Precision::Confusion(); // 几何精度阈值

	// 处理最小X顶点
	if (x < minX - eps) {  // X更小（考虑精度）
		minX = x;
		minXPoint = vertex;
	}
	else if (x <= minX + eps) {  // X相同（在精度范围内）
		if (facePos == Face_Upper) {
			// 上方面：取Y最小的点
			if (y < minXPoint.Y() - eps) {
				minXPoint = vertex;
			}
		}
		else {  // 下方面
			// 下方面：取Y最大的点
			if (y > minXPoint.Y() + eps) {
				minXPoint = vertex;
			}
		}
	}

	// 处理最大X顶点
	if (x > maxX + eps) {  // X更大（考虑精度）
		maxX = x;
		maxXPoint = vertex;
	}
	else if (x >= maxX - eps) {  // X相同（在精度范围内）
		if (facePos == Face_Upper) {
			// 上方面：取Y最小的点
			if (y < maxXPoint.Y() - eps) {
				maxXPoint = vertex;
			}
		}
		else {  // 下方面
			// 下方面：取Y最大的点
			if (y > maxXPoint.Y() + eps) {
				maxXPoint = vertex;
			}
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


	FindMinMaxXPoints(faceA, minXA, maxXA,Face_Upper);
	FindMinMaxXPoints(faceB, minXB, maxXB, Face_Lower);

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


	FindMinMaxXPoints(faceA, minXA, maxXA, Face_Upper);
	FindMinMaxXPoints(faceB, minXB, maxXB, Face_Lower);

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



std::vector<gp_Pnt> APICreateMidSurfaceHelper::GetContourVertices(const TopoDS_Face& face)
{
	std::vector<gp_Pnt> vertices;

	// 获取外轮廓线
	TopoDS_Wire outerWire = GetOuterWire(face);
	if (outerWire.IsNull()) {
		return vertices;
	}

	// 遍历轮廓线中的边，按顺序收集顶点
	TopExp_Explorer edgeExplorer(outerWire, TopAbs_EDGE);

	// 首先获取第一条边的起点
	if (edgeExplorer.More()) {
		TopoDS_Edge firstEdge = TopoDS::Edge(edgeExplorer.Current());
		TopoDS_Vertex startVertex, endVertex;
		TopExp::Vertices(firstEdge, startVertex, endVertex);

		if (!startVertex.IsNull()) {
			vertices.push_back(BRep_Tool::Pnt(startVertex));
		}

		// 继续遍历所有边
		for (; edgeExplorer.More(); edgeExplorer.Next()) {
			TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
			TopExp::Vertices(edge, startVertex, endVertex);

			// 只添加终点，起点已经由上一条边添加过了
			if (!endVertex.IsNull()) {
				vertices.push_back(BRep_Tool::Pnt(endVertex));
			}
		}
	}

	return vertices;
}




// 辅助函数：判断两个点是否在精度范围内相等
bool APICreateMidSurfaceHelper::IsPointEqual(const gp_Pnt& p1, const gp_Pnt& p2)
{
	return p1.IsEqual(p2, Precision::Confusion());
}

// 从起始点到终点收集边（按遍历顺序）
void APICreateMidSurfaceHelper::CollectEdgesFromStartToEnd(
	const TopoDS_Wire& wire,
	const gp_Pnt& startPnt,  // 输入的起始点
	const gp_Pnt& endPnt,    // 输入的终点
	std::vector<TopoDS_Edge>& forwardEdges  // 存放从起点到终点的边
)
{
	forwardEdges.clear();
	bool isCollecting = false;  // 是否开始收集边的标志

	TopExp_Explorer edgeExplorer(wire, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
		TopoDS_Vertex vtx1, vtx2;
		TopExp::Vertices(edge, vtx1, vtx2);
		gp_Pnt p1 = BRep_Tool::Pnt(vtx1);  // 边的起点
		gp_Pnt p2 = BRep_Tool::Pnt(vtx2);  // 边的终点

		// 情况1：还未开始收集，检查当前边的起点是否等于输入起始点
		if (!isCollecting) {
			if (IsPointEqual(p1, startPnt)) {
				isCollecting = true;  // 开始收集
				forwardEdges.push_back(edge);
				// 若当前边的终点就是输入终点，直接结束
				if (IsPointEqual(p2, endPnt)) {
					break;
				}
			}
		}
		// 情况2：正在收集，将边加入容器，直到遇到终点
		else {
			forwardEdges.push_back(edge);
			// 检查当前边的终点是否等于输入终点
			if (IsPointEqual(p2, endPnt)) {
				break;
			}
		}
	}
}

// 从终点反向遍历到起始点（以终点为起点，原起点为终点）
void APICreateMidSurfaceHelper::CollectEdgesFromEndToStart(
	const TopoDS_Wire& wire,
	const gp_Pnt& startPnt,  // 原起始点（现在作为终点）
	const gp_Pnt& endPnt,    // 原终点（现在作为起点）
	std::vector<TopoDS_Edge>& backwardEdges  // 存放从终点到起点的边
)
{
	backwardEdges.clear();
	bool isCollecting = false;  // 是否开始收集边的标志

	TopExp_Explorer edgeExplorer(wire, TopAbs_EDGE);
	for (; edgeExplorer.More(); edgeExplorer.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
		TopoDS_Vertex vtx1, vtx2;
		TopExp::Vertices(edge, vtx1, vtx2);
		gp_Pnt p1 = BRep_Tool::Pnt(vtx1);  // 边的起点
		gp_Pnt p2 = BRep_Tool::Pnt(vtx2);  // 边的终点

		// 情况1：还未开始收集，检查当前边的起点是否等于原终点（新起点）
		if (!isCollecting) {
			if (IsPointEqual(p1, endPnt)) {
				isCollecting = true;  // 开始收集
				backwardEdges.push_back(edge);
				// 若当前边的终点就是原起始点（新终点），直接结束
				if (IsPointEqual(p2, startPnt)) {
					break;
				}
			}
		}
		// 情况2：正在收集，将边加入容器，直到遇到原起始点
		else {
			backwardEdges.push_back(edge);
			// 检查当前边的终点是否等于原起始点
			if (IsPointEqual(p2, startPnt)) {
				break;
			}
		}
	}
}