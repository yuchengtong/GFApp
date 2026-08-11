#include "APISetNodeValue.h"
#include "ModelDataManager.h"
#include <MeshVS_Mesh.hxx>
#include <MeshVS_DataMapOfIntegerColor.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>
#include <BRepBuilderAPI_Transform.hxx>


struct Point {
	double x;
	double z;
};

void APISetNodeValue::HSVtoRGB(double h, double s, double v, double& r, double& g, double& b)
{
	if (s <= 0.0) {
		// 无饱和度时为灰度
		r = v;
		g = v;
		b = v;
		return;
	}

	// HSV转RGB核心计算
	double c = v * s;                  // 色度
	double x = c * (1.0 - std::fabs(std::fmod(h * 6.0, 2.0) - 1.0));
	double m = v - c;                  // 明度偏移值

	// 根据Hue值确定RGB分量
	double r_temp, g_temp, b_temp;
	if (h < 1.0 / 6.0) {
		r_temp = c;
		g_temp = x;
		b_temp = 0.0;
	}
	else if (h < 2.0 / 6.0) {
		r_temp = x;
		g_temp = c;
		b_temp = 0.0;
	}
	else if (h < 3.0 / 6.0) {
		r_temp = 0.0;
		g_temp = c;
		b_temp = x;
	}
	else if (h < 4.0 / 6.0) {
		r_temp = 0.0;
		g_temp = x;
		b_temp = c;
	}
	else if (h < 5.0 / 6.0) {
		r_temp = x;
		g_temp = 0.0;
		b_temp = c;
	}
	else {
		r_temp = c;
		g_temp = 0.0;
		b_temp = x;
	}

	// 应用明度偏移并限制范围
	r = std::clamp(r_temp + m, 0.0, 1.0);
	g = std::clamp(g_temp + m, 0.0, 1.0);
	b = std::clamp(b_temp + m, 0.0, 1.0);
}

MeshVS_DataMapOfIntegerColor APISetNodeValue::GetMeshDataMap(std::vector<double> tt, double min, double max)
{
	double a, r, g, b;
	MeshVS_DataMapOfIntegerColor colormap;
	int index = 0;

	// 处理特殊情况：避免除零
	if (max < min) {
		Quantity_Color defaultColor(0.5, 0.5, 0.5, Quantity_TOC_RGB); // 灰色
		for (size_t i = 0; i < tt.size(); ++i) {
			colormap.Bind(i + 1, defaultColor);
		}
		return colormap;
	}
	else if (max == min)
	{
		Quantity_Color defaultColor(0.0, 0.0, 1.0, Quantity_TOC_RGB); // 蓝色
		for (size_t i = 0; i < tt.size(); ++i) {
			colormap.Bind(i + 1, defaultColor);
		}
		return colormap;
	}

	for (double t : tt)
	{
		// 1. 归一化到[0,1]范围
		a = (t - min) / (max - min);
		a = std::clamp(a, 0.0, 1.0); // 确保值在有效范围

		// 2. 定义HSV参数（保持蓝→绿→红趋势）
		// H: 0.666(蓝) → 0.333(绿) → 0(红)，对应HSV色轮
		double h = 0.666 - a * 0.666; // 从蓝色(0.666)过渡到红色(0)
		double s = 1.0;               // 最大饱和度
		double v = 1.0;               // 最大明度

		// 3. 手动转换HSV到RGB
		HSVtoRGB(h, s, v, r, g, b);

		// 4. 绑定颜色到索引
		if (t == -1.0) { // 如果检测到特殊值，则设置为灰色
			colormap.Bind(index + 1, Quantity_Color(0.5, 0.5, 0.5, Quantity_TOC_RGB));
		}
		else {
			colormap.Bind(index + 1, Quantity_Color(r, g, b, Quantity_TOC_RGB));
		}
		index++;
	}
	return colormap;
}

Handle(AIS_Shape) APISetNodeValue::RotateAIS_ShapeXY(const Handle(AIS_Shape)& aisShape,
	Standard_Real angleDeg,
	Standard_Real x0,
	Standard_Real y0)
{
	if (aisShape.IsNull()) {
		return aisShape;
	}

	TopoDS_Shape shape = aisShape->Shape();

	gp_Trsf rotation;
	gp_Ax1 rotAxis(gp_Pnt(x0, y0, 0.0), gp_Dir(0.0, 0.0, 1.0));
	rotation.SetRotation(rotAxis, angleDeg * M_PI / 180.0);

	BRepBuilderAPI_Transform transform(shape, rotation, Standard_True);
	TopoDS_Shape newShape = transform.Shape();

	Handle(AIS_Shape) rotatedAIS = new AIS_Shape(newShape);

	// 复制显示属性（颜色、透明度、材质等）
	rotatedAIS->SetAttributes(aisShape->Attributes());

	return rotatedAIS;
}

double APISetNodeValue::CalculateStd(const std::vector<double> data)
{
	if (data.empty()) {
		return 0.0;
	}
	double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
	double accum = 0.0;
	std::for_each(data.begin(), data.end(), [&](double d) {
		accum += (d - mean) * (d - mean);
		});
	double variance = accum / data.size();
	return std::sqrt(variance);
}


bool APISetNodeValue::CalculateAllFallStressNodeValues()
{
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	FallAnalysisResultInfo resultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	if (!resultInfo.isChecked)
	{
		return false;
	}

	resultInfo.shellStressNodeValues.clear();
	resultInfo.nozzleStressNodeValues.clear();
	resultInfo.propellantStressNodeValues.clear();
	resultInfo.heatInsulatingStressNodeValues.clear();

	auto fallStressResult = ModelDataManager::GetInstance()->GetFallStressResult();
	auto angle = fallSettingInfo.angle;

	gp_Pnt ptShellLeftBottom = modelGeometryInfo.ptShellLeftBottom;
	gp_Pnt ptShellRightBottom = modelGeometryInfo.ptShellRightBottom;
	gp_Pnt ptNozzleInletBottom = modelGeometryInfo.ptNozzleInletBottom;
	gp_Pnt ptNozzleOutletBottom = modelGeometryInfo.ptNozzleOutletBottom;

	// ============================================================
	// 1.(壳体 + 喷管)跌落应力
	// ============================================================
	{
		auto max_value = fallStressResult.metalsMaxStress;
		auto min_value = fallStressResult.metalsMinStress;

		const int layerCount = 9;
		const std::vector<double> thresholds = {
			10.0, 25.0, 50.0, 90.0, 160.0, 260.0, 300.0, 330.0, 400.0, 500.0
		};
		const std::vector<double> thresholds90 = {
			40.0, 100.0, 150.0, 200.0, 300.0, 350.0,
			1000.0, 2000.0, 3000.0, 3500.0
		};

		auto calculateMetalStress = [&](const gp_Pnt& currentNode) -> double
		{
			double value = min_value;

			if (angle == 0)
			{
				double dist1 = currentNode.Distance(ptShellLeftBottom);
				double dist2 = currentNode.Distance(ptShellRightBottom);
				double minDist = std::min(dist1, dist2);
				if (minDist > 500.0)
				{
					value = min_value;
				}
				else
				{
					int layer = layerCount - 1;
					for (int i = 0; i < layerCount; ++i)
					{
						if (minDist <= thresholds[i])
						{
							layer = i;
							break;
						}
					}
					double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
					value = min_value + (max_value - min_value) * ratio;
				}
			}
			else if (angle > 0 && angle <= 15)
			{
				gp_Pnt cor_ptShellRightBottom(ptShellRightBottom.X() + 30, ptShellRightBottom.Y(), ptShellRightBottom.Z());
				double distToShellRight = currentNode.Distance(cor_ptShellRightBottom);
				gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 30, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
				double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
				double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

				double valueShellRight = min_value;
				double valueNozzleInlet = min_value;
				double valueNozzleOutlet = min_value;

				if (distToShellRight < 100.0)
					valueShellRight = max_value;
				else if (distToShellRight < 150.0)
					valueShellRight = min_value + (max_value - min_value) * 0.7;
				else if (distToShellRight < 200.0)
					valueShellRight = min_value + (max_value - min_value) * 0.6;
				else if (distToShellRight < 300.0)
					valueShellRight = min_value + (max_value - min_value) * 0.5;
				else if (distToShellRight < 400.0)
					valueShellRight = min_value + (max_value - min_value) * 0.4;
				else if (distToShellRight < 500.0)
					valueShellRight = min_value + (max_value - min_value) * 0.3;
				else if (distToShellRight < 600.0)
					valueShellRight = min_value + (max_value - min_value) * 0.2;

				if (distToNozzleInlet < 50.0)
					valueNozzleInlet = max_value;
				else if (distToNozzleInlet < 100.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
				else if (distToNozzleInlet < 150.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleInlet < 200.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleInlet < 400.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleInlet < 500.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleInlet < 600.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.2;

				if (distToNozzleOutlet < 50.0)
					valueNozzleOutlet = max_value;
				else if (distToNozzleOutlet < 100.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleOutlet < 150.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleOutlet < 200.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleOutlet < 250.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleOutlet < 300.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.2;
				else if (distToNozzleOutlet < 700.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.1;

				value = std::max({ valueShellRight, valueNozzleInlet, valueNozzleOutlet });
			}
			else if (angle > 15 && angle <= 60)
			{
				gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
				double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
				double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

				double valueNozzleInlet = min_value;
				double valueNozzleOutlet = min_value;

				if (distToNozzleInlet < 50.0)
					valueNozzleInlet = max_value;
				else if (distToNozzleInlet < 100.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
				else if (distToNozzleInlet < 150.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleInlet < 300.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleInlet < 500.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleInlet < 600.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleInlet < 800.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.2;

				if (distToNozzleOutlet < 50.0)
					valueNozzleOutlet = max_value;
				else if (distToNozzleOutlet < 100.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleOutlet < 150.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleOutlet < 250.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleOutlet < 300.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleOutlet < 900.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.2;

				value = std::max({ valueNozzleInlet, valueNozzleOutlet });
			}
			else if (angle > 60 && angle < 90)
			{
				gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
				double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
				double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

				double valueNozzleInlet = min_value;
				double valueNozzleOutlet = min_value;

				if (cor_ptNozzleInletBottom.X() - currentNode.X() < 400)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;

				if (distToNozzleInlet < 50.0)
					valueNozzleInlet = max_value;
				else if (distToNozzleInlet < 100.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
				else if (distToNozzleInlet < 150.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleInlet < 200.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleInlet < 400.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleInlet < 500.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleInlet < 600.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;

				if (distToNozzleOutlet < 50.0)
					valueNozzleOutlet = max_value;
				else if (distToNozzleOutlet < 100.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleOutlet < 150.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleOutlet < 250.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleOutlet < 400.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleOutlet < 700.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.2;
				else if (distToNozzleOutlet < 900.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.1;

				value = std::max({ valueNozzleInlet, valueNozzleOutlet });
			}
			else if (angle == 90)
			{
				double startX = ptNozzleInletBottom.X();
				double xDist = std::abs(currentNode.X() - startX);

				if (xDist > 3500)
				{
					value = min_value;
				}
				else
				{
					int layer = layerCount - 1;
					for (int i = 0; i < layerCount; ++i)
					{
						if (xDist <= thresholds90[i])
						{
							layer = i;
							break;
						}
					}
					double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
					value = min_value + (max_value - min_value) * ratio;
				}
			}

			return value;
		};

		// 壳体网格
		if (!modelMeshInfo.shellMesh.IsNull())
		{
			TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
			Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
			resultInfo.shellStressNodeValues.reserve(shellNodes.Extent());

			for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = shellNodeCoords->Value(nodeID, 1);
				double y = shellNodeCoords->Value(nodeID, 2);
				double z = shellNodeCoords->Value(nodeID, 3);
				gp_Pnt currentNode(x, y, z);
				resultInfo.shellStressNodeValues.push_back(calculateMetalStress(currentNode));
			}
		}

		// 喷管网格
		if (!modelMeshInfo.nozzleMesh.IsNull())
		{
			TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
			Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
			resultInfo.nozzleStressNodeValues.reserve(nozzleNodes.Extent());

			for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = nozzleNodeCoords->Value(nodeID, 1);
				double y = nozzleNodeCoords->Value(nodeID, 2);
				double z = nozzleNodeCoords->Value(nodeID, 3);
				gp_Pnt currentNode(x, y, z);
				resultInfo.nozzleStressNodeValues.push_back(calculateMetalStress(currentNode));
			}
		}
	}

	// ============================================================
	// 2. 推进剂跌落应力
	// ============================================================
	{
		auto max_value = fallStressResult.propellantsMaxStress;
		auto min_value = fallStressResult.propellantsMinStress;
		auto high = fallSettingInfo.high;

		const int layerCount = 9;
		std::vector<double> layerBoundaries = { 10.0, 40.0, 90.0, 150.0, 200.0, 260.0, 300.0, 330.0, 350.0 };

		if (!modelMeshInfo.propellantMesh.IsNull())
		{
			TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
			Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
			resultInfo.propellantStressNodeValues.reserve(propellantNodes.Extent());

			for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = propellantNodeCoords->Value(nodeID, 1);
				double y = propellantNodeCoords->Value(nodeID, 2);
				double z = propellantNodeCoords->Value(nodeID, 3);
				gp_Pnt currentNode(x, y, z);

				double value = min_value;

				if (angle == 0)
				{
					gp_Pnt A = ptShellLeftBottom;
					gp_Pnt B = ptShellRightBottom;
					gp_Pnt P = currentNode;

					gp_Vec AB(B.X() - A.X(), B.Y() - A.Y(), B.Z() - A.Z());
					gp_Vec AP(P.X() - A.X(), P.Y() - A.Y(), P.Z() - A.Z());

					double abLenSq = AB.SquareMagnitude();
					double distToSegment = 0.0;

					if (abLenSq < 1e-12)
					{
						distToSegment = P.Distance(A);
					}
					else
					{
						double t = AP.Dot(AB) / abLenSq;
						if (t <= 0.0)
							distToSegment = P.Distance(A);
						else if (t >= 1.0)
							distToSegment = P.Distance(B);
						else
						{
							gp_Vec cross = AB.Crossed(AP);
							distToSegment = cross.Magnitude() / std::sqrt(abLenSq);
						}
					}

					double zFactor = 1.0;
					if (z > ptShellLeftBottom.Z())
					{
						zFactor = std::max(0.0, 1.0 - (z - ptShellLeftBottom.Z()) / high);
					}

					int layer = -1;
					for (int i = 0; i < layerCount; ++i)
					{
						if (distToSegment <= layerBoundaries[i] + 1e-6)
						{
							layer = i;
							break;
						}
					}

					if (layer == -1)
					{
						value = min_value;
					}
					else
					{
						double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
						value = min_value + (max_value - min_value) * ratio * zFactor;
					}
				}
				else if (angle > 0 && angle <= 15)
				{
					double distToShellRight = currentNode.Distance(ptShellRightBottom);
					double valueShellRight = min_value;

					if (distToShellRight < 50.0)
						valueShellRight = max_value;
					else if (distToShellRight < 80.0)
						valueShellRight = min_value + (max_value - min_value) * 0.7;
					else if (distToShellRight < 120.0)
						valueShellRight = min_value + (max_value - min_value) * 0.6;
					else if (distToShellRight < 180.0)
						valueShellRight = min_value + (max_value - min_value) * 0.5;
					else if (distToShellRight < 230.0)
						valueShellRight = min_value + (max_value - min_value) * 0.4;
					else if (distToShellRight < 300.0)
						valueShellRight = min_value + (max_value - min_value) * 0.3;
					else if (distToShellRight < 400.0)
						valueShellRight = min_value + (max_value - min_value) * 0.2;

					gp_Pnt ptShellRightUp(ptShellRightBottom.X(), ptShellRightBottom.Y() - 1700, ptShellRightBottom.Z());
					double distToShellRightUp = currentNode.Distance(ptShellRightUp);
					if (distToShellRightUp < 30.0)
						valueShellRight = max_value;
					else if (distToShellRightUp < 40.0)
						valueShellRight = min_value + (max_value - min_value) * 0.7;
					else if (distToShellRightUp < 60.0)
						valueShellRight = min_value + (max_value - min_value) * 0.6;
					else if (distToShellRightUp < 90.0)
						valueShellRight = min_value + (max_value - min_value) * 0.5;
					else if (distToShellRightUp < 120.0)
						valueShellRight = min_value + (max_value - min_value) * 0.4;
					else if (distToShellRightUp < 250.0)
						valueShellRight = min_value + (max_value - min_value) * 0.3;
					else if (distToShellRightUp < 300.0)
						valueShellRight = min_value + (max_value - min_value) * 0.2;

					if (std::abs(ptNozzleInletBottom.X() - x) < 200)
						valueShellRight = min_value + (max_value - min_value) * 0.3;

					value = valueShellRight;
				}
				else if (angle > 15 && angle <= 60)
				{
					gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
					double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
					double valueNozzleInlet = min_value;

					if (distToNozzleInlet < 50.0)
						valueNozzleInlet = max_value;
					else if (distToNozzleInlet < 100.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
					else if (distToNozzleInlet < 150.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
					else if (distToNozzleInlet < 200.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
					else if (distToNozzleInlet < 300.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
					else if (distToNozzleInlet < 500.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
					else if (distToNozzleInlet < 800.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.2;

					value = valueNozzleInlet;
				}
				else if (angle > 60 && angle < 90)
				{
					gp_Pnt capsuleCenter(
						modelGeometryInfo.theXmin + (modelGeometryInfo.theXmax - modelGeometryInfo.theXmin) / 2.0,
						modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
						ptNozzleInletBottom.Z()
					);

					double capsuleLength = 4000.0;
					double capsuleRadius = 500.0;
					double cylinderHalfLength = (capsuleLength - 2.0 * capsuleRadius) / 2.0;
					if (cylinderHalfLength < 0)
						cylinderHalfLength = 0;

					gp_Dir axisDir(1.0, 0.0, 0.0);
					gp_Vec axisVec(axisDir);

					gp_Pnt cylStart(capsuleCenter.X() - cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());
					gp_Pnt cylEnd(capsuleCenter.X() + cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());

					gp_Pnt P = currentNode;
					gp_Vec centerToP(P.X() - capsuleCenter.X(), P.Y() - capsuleCenter.Y(), P.Z() - capsuleCenter.Z());
					double t = centerToP.Dot(axisVec);

					double distToSurface = 0.0;

					if (t < -cylinderHalfLength - capsuleRadius)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength + capsuleRadius)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else if (t < -cylinderHalfLength)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else
					{
						gp_Pnt projPoint(capsuleCenter.X() + t, capsuleCenter.Y(), capsuleCenter.Z());
						double distToAxis = P.Distance(projPoint);
						distToSurface = distToAxis - capsuleRadius;
					}

					double effectiveDist = std::max(0.0, distToSurface);

					if (effectiveDist < 100.0)
						value = min_value;
					else if (effectiveDist < 200.0)
						value = min_value + (max_value - min_value) * 0.2;
					else
						value = min_value + (max_value - min_value) * 0.3;
				}
				else if (angle == 90)
				{
					gp_Pnt capsuleCenter(
						modelGeometryInfo.theXmin + (modelGeometryInfo.theXmax - modelGeometryInfo.theXmin) / 2.0,
						modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
						ptNozzleInletBottom.Z()
					);

					double capsuleLength = 4000.0;
					double capsuleRadius = 500.0;
					double cylinderHalfLength = (capsuleLength - 2.0 * capsuleRadius) / 2.0;
					if (cylinderHalfLength < 0)
						cylinderHalfLength = 0;

					gp_Dir axisDir(1.0, 0.0, 0.0);
					gp_Vec axisVec(axisDir);

					gp_Pnt cylStart(capsuleCenter.X() - cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());
					gp_Pnt cylEnd(capsuleCenter.X() + cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());

					gp_Pnt P = currentNode;
					gp_Vec centerToP(P.X() - capsuleCenter.X(), P.Y() - capsuleCenter.Y(), P.Z() - capsuleCenter.Z());
					double t = centerToP.Dot(axisVec);

					double distToSurface = 0.0;

					if (t < -cylinderHalfLength - capsuleRadius)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength + capsuleRadius)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else if (t < -cylinderHalfLength)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else
					{
						gp_Pnt projPoint(capsuleCenter.X() + t, capsuleCenter.Y(), capsuleCenter.Z());
						double distToAxis = P.Distance(projPoint);
						distToSurface = distToAxis - capsuleRadius;
					}

					double effectiveDist = std::max(0.0, distToSurface);

					double valueCapsule;
					if (effectiveDist < 100.0)
						valueCapsule = min_value;
					else if (effectiveDist < 500.0)
						valueCapsule = min_value + (max_value - min_value) * 0.2;
					else if (effectiveDist < 1000.0)
						valueCapsule = min_value + (max_value - min_value) * 0.1;
					else
						valueCapsule = min_value;

					gp_Pnt circleCenter(
						ptNozzleInletBottom.X() - 200,
						modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
						ptNozzleInletBottom.Z()
					);

					double dx = std::abs(circleCenter.X() - x);
					double dy = std::abs(circleCenter.Y() - y);

					double valueRect;
					if (dx < 30.0 && dy < 300.0)
					{
						valueRect = max_value;
					}
					else
					{
						double distX = std::max(0.0, dx - 30.0);
						double distY = std::max(0.0, dy - 300.0);
						double distToRect = std::sqrt(distX * distX + distY * distY);

						if (distToRect < 40.0)
							valueRect = min_value + (max_value - min_value) * 0.8;
						else if (distToRect < 50.0)
							valueRect = min_value + (max_value - min_value) * 0.7;
						else if (distToRect < 70.0)
							valueRect = min_value + (max_value - min_value) * 0.6;
						else if (distToRect < 90.0)
							valueRect = min_value + (max_value - min_value) * 0.5;
						else if (distToRect < 100.0)
							valueRect = min_value + (max_value - min_value) * 0.4;
						else if (distToRect < 120.0)
							valueRect = min_value + (max_value - min_value) * 0.2;
						else
							valueRect = min_value;
					}

					value = std::max(valueCapsule, valueRect);
				}

				resultInfo.propellantStressNodeValues.push_back(value);
			}
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger heatNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingStressNodeValues.assign(heatNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(resultInfo);


	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	fallStressResult.metalsAvgStress = computeAvg(resultInfo.shellStressNodeValues);
	fallStressResult.outheatAvgStress = fallStressResult.metalsAvgStress;

	fallStressResult.propellantsAvgStress = computeAvg(resultInfo.propellantStressNodeValues);
	double shellMin = fallStressResult.metalsMinStress;
	double shellMax = fallStressResult.metalsMaxStress;
	if (shellMax > shellMin) {
		fallStressResult.insulatingheatAvgStress =
			shellMin + (shellMax - shellMin) * (fallStressResult.metalsAvgStress - shellMin) / (shellMax - shellMin);
	}
	else {
		fallStressResult.insulatingheatAvgStress = shellMin;
	}

	fallStressResult.metalsStandardStress = CalculateStd(resultInfo.shellStressNodeValues);
	fallStressResult.outheatStandardStress = fallStressResult.metalsStandardStress;
	fallStressResult.propellantsStandardStress = CalculateStd(resultInfo.propellantStressNodeValues);
	fallStressResult.insulatingheatStandardStress = CalculateStd(resultInfo.heatInsulatingStressNodeValues);


	ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);

	return true;
}


bool APISetNodeValue::CalculateAllFallStrainNodeValues()
{
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	FallAnalysisResultInfo resultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	if (!resultInfo.isChecked)
	{
		return false;
	}

	resultInfo.shellStrainNodeValues.clear();
	resultInfo.nozzleStrainNodeValues.clear();
	resultInfo.propellantStrainNodeValues.clear();
	resultInfo.heatInsulatingStrainNodeValues.clear();

	auto fallStrainResult = ModelDataManager::GetInstance()->GetFallStrainResult();
	auto angle = fallSettingInfo.angle;

	gp_Pnt ptShellLeftBottom = modelGeometryInfo.ptShellLeftBottom;
	gp_Pnt ptShellRightBottom = modelGeometryInfo.ptShellRightBottom;
	gp_Pnt ptNozzleInletBottom = modelGeometryInfo.ptNozzleInletBottom;
	gp_Pnt ptNozzleOutletBottom = modelGeometryInfo.ptNozzleOutletBottom;

	// ============================================================
	// 1. 金属材料（壳体 + 喷管）跌落应力
	// ============================================================
	{
		auto max_value = fallStrainResult.metalsMaxStrain;
		auto min_value = fallStrainResult.metalsMinStrain;

		const int layerCount = 9;
		const std::vector<double> thresholds = {
			10.0, 25.0, 50.0, 90.0, 160.0, 260.0, 300.0, 330.0, 400.0, 500.0
		};
		const std::vector<double> thresholds90 = {
			40.0, 100.0, 150.0, 200.0, 300.0, 350.0,
			1000.0, 2000.0, 3000.0, 3500.0
		};

		auto calculateMetalStrain = [&](const gp_Pnt& currentNode) -> double
		{
			double value = min_value;

			if (angle == 0)
			{
				double dist1 = currentNode.Distance(ptShellLeftBottom);
				double dist2 = currentNode.Distance(ptShellRightBottom);
				double minDist = std::min(dist1, dist2);
				if (minDist > 500.0)
				{
					value = min_value;
				}
				else
				{
					int layer = layerCount - 1;
					for (int i = 0; i < layerCount; ++i)
					{
						if (minDist <= thresholds[i])
						{
							layer = i;
							break;
						}
					}
					double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
					value = min_value + (max_value - min_value) * ratio;
				}
			}
			else if (angle > 0 && angle <= 15)
			{
				gp_Pnt cor_ptShellRightBottom(ptShellRightBottom.X() + 30, ptShellRightBottom.Y(), ptShellRightBottom.Z());
				double distToShellRight = currentNode.Distance(cor_ptShellRightBottom);
				gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 30, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
				double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
				double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

				double valueShellRight = min_value;
				double valueNozzleInlet = min_value;
				double valueNozzleOutlet = min_value;

				if (distToShellRight < 100.0)
					valueShellRight = max_value;
				else if (distToShellRight < 150.0)
					valueShellRight = min_value + (max_value - min_value) * 0.7;
				else if (distToShellRight < 200.0)
					valueShellRight = min_value + (max_value - min_value) * 0.6;
				else if (distToShellRight < 300.0)
					valueShellRight = min_value + (max_value - min_value) * 0.5;
				else if (distToShellRight < 400.0)
					valueShellRight = min_value + (max_value - min_value) * 0.4;
				else if (distToShellRight < 500.0)
					valueShellRight = min_value + (max_value - min_value) * 0.3;
				else if (distToShellRight < 600.0)
					valueShellRight = min_value + (max_value - min_value) * 0.2;

				if (distToNozzleInlet < 50.0)
					valueNozzleInlet = max_value;
				else if (distToNozzleInlet < 100.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
				else if (distToNozzleInlet < 150.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleInlet < 200.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleInlet < 400.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleInlet < 500.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleInlet < 600.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.2;

				if (distToNozzleOutlet < 50.0)
					valueNozzleOutlet = max_value;
				else if (distToNozzleOutlet < 100.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleOutlet < 150.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleOutlet < 200.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleOutlet < 250.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleOutlet < 300.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.2;
				else if (distToNozzleOutlet < 700.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.1;

				value = std::max({ valueShellRight, valueNozzleInlet, valueNozzleOutlet });
			}
			else if (angle > 15 && angle <= 60)
			{
				gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
				double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
				double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

				double valueNozzleInlet = min_value;
				double valueNozzleOutlet = min_value;

				if (distToNozzleInlet < 50.0)
					valueNozzleInlet = max_value;
				else if (distToNozzleInlet < 100.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
				else if (distToNozzleInlet < 150.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleInlet < 300.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleInlet < 500.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleInlet < 600.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleInlet < 800.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.2;

				if (distToNozzleOutlet < 50.0)
					valueNozzleOutlet = max_value;
				else if (distToNozzleOutlet < 100.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleOutlet < 150.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleOutlet < 250.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleOutlet < 300.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleOutlet < 900.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.2;

				value = std::max({ valueNozzleInlet, valueNozzleOutlet });
			}
			else if (angle > 60 && angle < 90)
			{
				gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
				double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
				double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

				double valueNozzleInlet = min_value;
				double valueNozzleOutlet = min_value;

				if (cor_ptNozzleInletBottom.X() - currentNode.X() < 400)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;

				if (distToNozzleInlet < 50.0)
					valueNozzleInlet = max_value;
				else if (distToNozzleInlet < 100.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
				else if (distToNozzleInlet < 150.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleInlet < 200.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleInlet < 400.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleInlet < 500.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleInlet < 600.0)
					valueNozzleInlet = min_value + (max_value - min_value) * 0.3;

				if (distToNozzleOutlet < 50.0)
					valueNozzleOutlet = max_value;
				else if (distToNozzleOutlet < 100.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.6;
				else if (distToNozzleOutlet < 150.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.5;
				else if (distToNozzleOutlet < 250.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.4;
				else if (distToNozzleOutlet < 400.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.3;
				else if (distToNozzleOutlet < 700.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.2;
				else if (distToNozzleOutlet < 900.0)
					valueNozzleOutlet = min_value + (max_value - min_value) * 0.1;

				value = std::max({ valueNozzleInlet, valueNozzleOutlet });
			}
			else if (angle == 90)
			{
				double startX = ptNozzleInletBottom.X();
				double xDist = std::abs(currentNode.X() - startX);

				if (xDist > 3500)
				{
					value = min_value;
				}
				else
				{
					int layer = layerCount - 1;
					for (int i = 0; i < layerCount; ++i)
					{
						if (xDist <= thresholds90[i])
						{
							layer = i;
							break;
						}
					}
					double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
					value = min_value + (max_value - min_value) * ratio;
				}
			}

			return value;
		};

		// 壳体网格
		if (!modelMeshInfo.shellMesh.IsNull())
		{
			TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
			Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
			resultInfo.shellStrainNodeValues.reserve(shellNodes.Extent());

			for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = shellNodeCoords->Value(nodeID, 1);
				double y = shellNodeCoords->Value(nodeID, 2);
				double z = shellNodeCoords->Value(nodeID, 3);
				gp_Pnt currentNode(x, y, z);
				resultInfo.shellStrainNodeValues.push_back(calculateMetalStrain(currentNode));
			}
		}

		// 喷管网格
		if (!modelMeshInfo.nozzleMesh.IsNull())
		{
			TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
			Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
			resultInfo.nozzleStrainNodeValues.reserve(nozzleNodes.Extent());

			for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = nozzleNodeCoords->Value(nodeID, 1);
				double y = nozzleNodeCoords->Value(nodeID, 2);
				double z = nozzleNodeCoords->Value(nodeID, 3);
				gp_Pnt currentNode(x, y, z);
				resultInfo.nozzleStrainNodeValues.push_back(calculateMetalStrain(currentNode));
			}
		}
	}

	// ============================================================
	// 2. 推进剂跌落应力
	// ============================================================	
	{
		auto max_value = fallStrainResult.propellantsMaxStrain;
		auto min_value = fallStrainResult.propellantsMinStrain;
		auto high = fallSettingInfo.high;

		const int layerCount = 9;
		std::vector<double> layerBoundaries = { 10.0, 40.0, 90.0, 150.0, 200.0, 260.0, 300.0, 330.0, 350.0 };

		if (!modelMeshInfo.propellantMesh.IsNull())
		{
			TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
			Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
			resultInfo.propellantStrainNodeValues.reserve(propellantNodes.Extent());

			for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = propellantNodeCoords->Value(nodeID, 1);
				double y = propellantNodeCoords->Value(nodeID, 2);
				double z = propellantNodeCoords->Value(nodeID, 3);
				gp_Pnt currentNode(x, y, z);

				double value = min_value;

				if (angle == 0)
				{
					gp_Pnt A = ptShellLeftBottom;
					gp_Pnt B = ptShellRightBottom;
					gp_Pnt P = currentNode;

					gp_Vec AB(B.X() - A.X(), B.Y() - A.Y(), B.Z() - A.Z());
					gp_Vec AP(P.X() - A.X(), P.Y() - A.Y(), P.Z() - A.Z());

					double abLenSq = AB.SquareMagnitude();
					double distToSegment = 0.0;

					if (abLenSq < 1e-12)
					{
						distToSegment = P.Distance(A);
					}
					else
					{
						double t = AP.Dot(AB) / abLenSq;
						if (t <= 0.0)
							distToSegment = P.Distance(A);
						else if (t >= 1.0)
							distToSegment = P.Distance(B);
						else
						{
							gp_Vec cross = AB.Crossed(AP);
							distToSegment = cross.Magnitude() / std::sqrt(abLenSq);
						}
					}

					double zFactor = 1.0;
					if (z > ptShellLeftBottom.Z())
					{
						zFactor = std::max(0.0, 1.0 - (z - ptShellLeftBottom.Z()) / high);
					}

					int layer = -1;
					for (int i = 0; i < layerCount; ++i)
					{
						if (distToSegment <= layerBoundaries[i] + 1e-6)
						{
							layer = i;
							break;
						}
					}

					if (layer == -1)
					{
						value = min_value;
					}
					else
					{
						double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
						value = min_value + (max_value - min_value) * ratio * zFactor;
					}
				}
				else if (angle > 0 && angle <= 15)
				{
					double distToShellRight = currentNode.Distance(ptShellRightBottom);
					double valueShellRight = min_value;

					if (distToShellRight < 50.0)
						valueShellRight = max_value;
					else if (distToShellRight < 80.0)
						valueShellRight = min_value + (max_value - min_value) * 0.7;
					else if (distToShellRight < 120.0)
						valueShellRight = min_value + (max_value - min_value) * 0.6;
					else if (distToShellRight < 180.0)
						valueShellRight = min_value + (max_value - min_value) * 0.5;
					else if (distToShellRight < 230.0)
						valueShellRight = min_value + (max_value - min_value) * 0.4;
					else if (distToShellRight < 300.0)
						valueShellRight = min_value + (max_value - min_value) * 0.3;
					else if (distToShellRight < 400.0)
						valueShellRight = min_value + (max_value - min_value) * 0.2;

					gp_Pnt ptShellRightUp(ptShellRightBottom.X(), ptShellRightBottom.Y() - 1700, ptShellRightBottom.Z());
					double distToShellRightUp = currentNode.Distance(ptShellRightUp);
					if (distToShellRightUp < 30.0)
						valueShellRight = max_value;
					else if (distToShellRightUp < 40.0)
						valueShellRight = min_value + (max_value - min_value) * 0.7;
					else if (distToShellRightUp < 60.0)
						valueShellRight = min_value + (max_value - min_value) * 0.6;
					else if (distToShellRightUp < 90.0)
						valueShellRight = min_value + (max_value - min_value) * 0.5;
					else if (distToShellRightUp < 120.0)
						valueShellRight = min_value + (max_value - min_value) * 0.4;
					else if (distToShellRightUp < 250.0)
						valueShellRight = min_value + (max_value - min_value) * 0.3;
					else if (distToShellRightUp < 300.0)
						valueShellRight = min_value + (max_value - min_value) * 0.2;

					if (std::abs(ptNozzleInletBottom.X() - x) < 200)
						valueShellRight = min_value + (max_value - min_value) * 0.3;

					value = valueShellRight;
				}
				else if (angle > 15 && angle <= 60)
				{
					gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60, ptNozzleInletBottom.Y(), ptNozzleInletBottom.Z());
					double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
					double valueNozzleInlet = min_value;

					if (distToNozzleInlet < 50.0)
						valueNozzleInlet = max_value;
					else if (distToNozzleInlet < 100.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.7;
					else if (distToNozzleInlet < 150.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.6;
					else if (distToNozzleInlet < 200.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.5;
					else if (distToNozzleInlet < 300.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.4;
					else if (distToNozzleInlet < 500.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.3;
					else if (distToNozzleInlet < 800.0)
						valueNozzleInlet = min_value + (max_value - min_value) * 0.2;

					value = valueNozzleInlet;
				}
				else if (angle > 60 && angle < 90)
				{
					gp_Pnt capsuleCenter(
						modelGeometryInfo.theXmin + (modelGeometryInfo.theXmax - modelGeometryInfo.theXmin) / 2.0,
						modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
						ptNozzleInletBottom.Z()
					);

					double capsuleLength = 4000.0;
					double capsuleRadius = 500.0;
					double cylinderHalfLength = (capsuleLength - 2.0 * capsuleRadius) / 2.0;
					if (cylinderHalfLength < 0)
						cylinderHalfLength = 0;

					gp_Dir axisDir(1.0, 0.0, 0.0);
					gp_Vec axisVec(axisDir);

					gp_Pnt cylStart(capsuleCenter.X() - cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());
					gp_Pnt cylEnd(capsuleCenter.X() + cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());

					gp_Pnt P = currentNode;
					gp_Vec centerToP(P.X() - capsuleCenter.X(), P.Y() - capsuleCenter.Y(), P.Z() - capsuleCenter.Z());
					double t = centerToP.Dot(axisVec);

					double distToSurface = 0.0;

					if (t < -cylinderHalfLength - capsuleRadius)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength + capsuleRadius)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else if (t < -cylinderHalfLength)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else
					{
						gp_Pnt projPoint(capsuleCenter.X() + t, capsuleCenter.Y(), capsuleCenter.Z());
						double distToAxis = P.Distance(projPoint);
						distToSurface = distToAxis - capsuleRadius;
					}

					double effectiveDist = std::max(0.0, distToSurface);

					if (effectiveDist < 100.0)
						value = min_value;
					else if (effectiveDist < 200.0)
						value = min_value + (max_value - min_value) * 0.2;
					else
						value = min_value + (max_value - min_value) * 0.3;
				}
				else if (angle == 90)
				{
					gp_Pnt capsuleCenter(
						modelGeometryInfo.theXmin + (modelGeometryInfo.theXmax - modelGeometryInfo.theXmin) / 2.0,
						modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
						ptNozzleInletBottom.Z()
					);

					double capsuleLength = 4000.0;
					double capsuleRadius = 500.0;
					double cylinderHalfLength = (capsuleLength - 2.0 * capsuleRadius) / 2.0;
					if (cylinderHalfLength < 0)
						cylinderHalfLength = 0;

					gp_Dir axisDir(1.0, 0.0, 0.0);
					gp_Vec axisVec(axisDir);

					gp_Pnt cylStart(capsuleCenter.X() - cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());
					gp_Pnt cylEnd(capsuleCenter.X() + cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());

					gp_Pnt P = currentNode;
					gp_Vec centerToP(P.X() - capsuleCenter.X(), P.Y() - capsuleCenter.Y(), P.Z() - capsuleCenter.Z());
					double t = centerToP.Dot(axisVec);

					double distToSurface = 0.0;

					if (t < -cylinderHalfLength - capsuleRadius)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength + capsuleRadius)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else if (t < -cylinderHalfLength)
						distToSurface = P.Distance(cylStart) - capsuleRadius;
					else if (t > cylinderHalfLength)
						distToSurface = P.Distance(cylEnd) - capsuleRadius;
					else
					{
						gp_Pnt projPoint(capsuleCenter.X() + t, capsuleCenter.Y(), capsuleCenter.Z());
						double distToAxis = P.Distance(projPoint);
						distToSurface = distToAxis - capsuleRadius;
					}

					double effectiveDist = std::max(0.0, distToSurface);

					double valueCapsule;
					if (effectiveDist < 100.0)
						valueCapsule = min_value;
					else if (effectiveDist < 500.0)
						valueCapsule = min_value + (max_value - min_value) * 0.2;
					else if (effectiveDist < 1000.0)
						valueCapsule = min_value + (max_value - min_value) * 0.1;
					else
						valueCapsule = min_value;

					gp_Pnt circleCenter(
						ptNozzleInletBottom.X() - 200,
						modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
						ptNozzleInletBottom.Z()
					);

					double dx = std::abs(circleCenter.X() - x);
					double dy = std::abs(circleCenter.Y() - y);

					double valueRect;
					if (dx < 30.0 && dy < 300.0)
					{
						valueRect = max_value;
					}
					else
					{
						double distX = std::max(0.0, dx - 30.0);
						double distY = std::max(0.0, dy - 300.0);
						double distToRect = std::sqrt(distX * distX + distY * distY);

						if (distToRect < 40.0)
							valueRect = min_value + (max_value - min_value) * 0.8;
						else if (distToRect < 50.0)
							valueRect = min_value + (max_value - min_value) * 0.7;
						else if (distToRect < 70.0)
							valueRect = min_value + (max_value - min_value) * 0.6;
						else if (distToRect < 90.0)
							valueRect = min_value + (max_value - min_value) * 0.5;
						else if (distToRect < 100.0)
							valueRect = min_value + (max_value - min_value) * 0.4;
						else if (distToRect < 120.0)
							valueRect = min_value + (max_value - min_value) * 0.2;
						else
							valueRect = min_value;
					}

					value = std::max(valueCapsule, valueRect);
				}

				resultInfo.propellantStrainNodeValues.push_back(value);
			}
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger heatNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingStrainNodeValues.assign(heatNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(resultInfo);

	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	fallStrainResult.metalsAvgStrain = computeAvg(resultInfo.shellStrainNodeValues);
	fallStrainResult.mpropellantsAvgStrain = computeAvg(resultInfo.propellantStrainNodeValues);

	double shellMin = fallStrainResult.metalsMinStrain;
	double shellMax = fallStrainResult.metalsMaxStrain;
	if (shellMax > shellMin) {
		fallStrainResult.insulatingheatAvgStrain =
			shellMin + (shellMax - shellMin) * (fallStrainResult.metalsAvgStrain - shellMin) / (shellMax - shellMin);
	}
	else {
		fallStrainResult.insulatingheatAvgStrain = shellMin;
	}


	ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);

	return true;
}


bool APISetNodeValue::CalculateAllFallTempNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	// 拷贝一份用于修改，计算完成后通过 Set 写回
	auto resultInfo = resultInfoConst;

	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto angle = fallSettingInfo.angle;
	auto high = fallSettingInfo.high;

	// 清空历史结果
	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto fallTemperatureResult = ModelDataManager::GetInstance()->GetFallTemperatureResult();
	double metalMax = fallTemperatureResult.metalsMaxTemperature;
	double metalMin = fallTemperatureResult.metalsMinTemperature;
	double propMax = fallTemperatureResult.propellantsMaxTemperature;
	double propMin = fallTemperatureResult.propellantsMinTemperature;

	gp_Pnt ptShellLeftBottom = modelGeometryInfo.ptShellLeftBottom;
	gp_Pnt ptShellRightBottom = modelGeometryInfo.ptShellRightBottom;
	gp_Pnt ptNozzleInletBottom = modelGeometryInfo.ptNozzleInletBottom;
	gp_Pnt ptNozzleOutletBottom = modelGeometryInfo.ptNozzleOutletBottom;

	const int layerCount = 9;
	const double maxDistance = 400.0;
	const double layerWidth = maxDistance / layerCount;

	// =====================================================================
	// Lambda：金属部件温度计算（壳体、喷管共用）
	// =====================================================================
	auto calcMetalTemp = [&](const gp_Pnt& currentNode) -> double
	{
		double value = metalMin;

		if (angle == 0)
		{
			double dist1 = currentNode.Distance(ptShellLeftBottom);
			double dist2 = currentNode.Distance(ptShellRightBottom);
			double minDist = std::min(dist1, dist2);

			double zFactor = 1.0;
			if (currentNode.Z() > ptShellLeftBottom.Z())
			{
				zFactor = std::max(0.0, 1.0 - (currentNode.Z() - ptShellLeftBottom.Z()) / high);
			}

			if (minDist > maxDistance)
			{
				value = metalMin;
			}
			else
			{
				int layer = static_cast<int>(minDist / layerWidth);
				if (layer >= layerCount) layer = layerCount - 1;
				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = metalMin + (metalMax - metalMin) * ratio * zFactor;
			}
		}
		else if (angle > 0 && angle <= 15)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = metalMin;
			if (distToShellRight < 20.0)
				valueShellRight = metalMax;
			else if (distToShellRight < 30.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToShellRight < 40.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToShellRight < 60.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToShellRight < 80.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToShellRight < 100.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToShellRight < 120.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.2;
			value = valueShellRight;
		}
		else if (angle > 15 && angle <= 60)
		{
			double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);
			double valueShellRight = metalMin;
			if (distToNozzleOutlet < 50.0)
				valueShellRight = metalMax;
			else if (distToNozzleOutlet < 70.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToNozzleOutlet < 80.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleOutlet < 90.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleOutlet < 100.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleOutlet < 110.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleOutlet < 120.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.2;
			value = valueShellRight;
		}
		else if (angle > 60 && angle < 90)
		{
			gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 40,
				ptNozzleInletBottom.Y(),
				ptNozzleInletBottom.Z());
			double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
			double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

			double valueNozzleInlet = metalMin;
			double valueNozzleOutlet = metalMin;

			if (distToNozzleInlet < 50.0)
				valueNozzleInlet = metalMax;
			else if (distToNozzleInlet < 70.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleInlet < 90.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleInlet < 100.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.3;

			if (distToNozzleOutlet < 50.0)
				valueNozzleOutlet = metalMax;
			else if (distToNozzleOutlet < 70.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToNozzleOutlet < 80.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleOutlet < 90.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleOutlet < 100.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleOutlet < 110.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleOutlet < 120.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.2;

			value = std::max({ valueNozzleInlet, valueNozzleOutlet });
		}
		else if (angle == 90)
		{
			if (abs(currentNode.Z() - ptNozzleInletBottom.Z()) < 20)
			{
				if (abs(currentNode.X() - ptNozzleInletBottom.X()) < 50)
					value = metalMin + (metalMax - metalMin) * 0.3;
				else if (abs(currentNode.X() - ptNozzleInletBottom.X()) < 100)
					value = metalMin + (metalMax - metalMin) * 0.5;
				else if (ptNozzleInletBottom.X() - currentNode.X() < 150 &&
					ptNozzleInletBottom.X() - currentNode.X() > 0)
					value = metalMin + (metalMax - metalMin) * 0.8;
				else if (ptNozzleInletBottom.X() - currentNode.X() < 200 &&
					ptNozzleInletBottom.X() - currentNode.X() > 0)
					value = metalMin + (metalMax - metalMin) * 1.0;
			}
			else
			{
				value = metalMin;
			}

			if (abs(currentNode.X() - ptNozzleInletBottom.X()) < 40)
			{
				value = metalMin + (metalMax - metalMin) * 0.3;
			}
		}

		return value;
	};

	// =====================================================================
	// Lambda：推进剂温度计算
	// =====================================================================
	auto calcPropellantTemp = [&](const gp_Pnt& currentNode) -> double
	{
		double value = propMin;
		double x = currentNode.X();
		double y = currentNode.Y();
		double z = currentNode.Z();

		if (angle == 0)
		{
			gp_Pnt A = ptShellLeftBottom;
			gp_Pnt B = ptShellRightBottom;
			gp_Pnt P = currentNode;

			gp_Vec AB(B.X() - A.X(), B.Y() - A.Y(), B.Z() - A.Z());
			gp_Vec AP(P.X() - A.X(), P.Y() - A.Y(), P.Z() - A.Z());

			double abLenSq = AB.SquareMagnitude();
			double distToSegment = 0.0;

			if (abLenSq < 1e-12)
			{
				distToSegment = P.Distance(A);
			}
			else
			{
				double t = AP.Dot(AB) / abLenSq;
				if (t <= 0.0)
					distToSegment = P.Distance(A);
				else if (t >= 1.0)
					distToSegment = P.Distance(B);
				else
				{
					gp_Vec cross = AB.Crossed(AP);
					distToSegment = cross.Magnitude() / std::sqrt(abLenSq);
				}
			}

			double zFactor = 1.0;
			if (z > ptShellLeftBottom.Z())
			{
				zFactor = std::max(0.0, 1.0 - (z - ptShellLeftBottom.Z()) / high);
			}

			std::vector<double> layerBoundaries = {
				10.0, 20.0, 30.0, 40.0, 50.0, 260.0, 330.0, 340.0, 350.0
			};

			int layer = -1;
			for (int i = 0; i < layerCount; ++i)
			{
				if (distToSegment <= layerBoundaries[i] + 1e-6)
				{
					layer = i;
					break;
				}
			}

			if (layer == -1)
			{
				value = propMin;
			}
			else
			{
				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = propMin + (propMax - propMin) * ratio * zFactor;
			}
		}
		else if (angle > 0 && angle <= 15)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = propMin;
			if (distToShellRight < 20.0)
				valueShellRight = propMax;
			else if (distToShellRight < 30.0)
				valueShellRight = propMin + (propMax - propMin) * 0.7;
			else if (distToShellRight < 40.0)
				valueShellRight = propMin + (propMax - propMin) * 0.6;
			else if (distToShellRight < 50.0)
				valueShellRight = propMin + (propMax - propMin) * 0.5;
			else if (distToShellRight < 60.0)
				valueShellRight = propMin + (propMax - propMin) * 0.4;
			else if (distToShellRight < 70.0)
				valueShellRight = propMin + (propMax - propMin) * 0.3;
			else if (distToShellRight < 80.0)
				valueShellRight = propMin + (propMax - propMin) * 0.2;
			value = valueShellRight;
		}
		else if (angle > 15 && angle <= 60)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = propMin;
			if (distToShellRight < 20.0)
				valueShellRight = propMax;
			else if (distToShellRight < 30.0)
				valueShellRight = propMin + (propMax - propMin) * 0.7;
			else if (distToShellRight < 40.0)
				valueShellRight = propMin + (propMax - propMin) * 0.6;
			else if (distToShellRight < 50.0)
				valueShellRight = propMin + (propMax - propMin) * 0.5;
			else if (distToShellRight < 60.0)
				valueShellRight = propMin + (propMax - propMin) * 0.4;
			else if (distToShellRight < 70.0)
				valueShellRight = propMin + (propMax - propMin) * 0.3;
			else if (distToShellRight < 80.0)
				valueShellRight = propMin + (propMax - propMin) * 0.2;
			value = valueShellRight;
		}
		else if (angle > 60 && angle < 90)
		{
			// 修正：原代码此处 Y/Z 误用了 X 坐标
			gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 40,
				ptNozzleInletBottom.Y(),
				ptNozzleInletBottom.Z());
			double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);

			double valueNozzleInlet = propMin;
			if (distToNozzleInlet < 50.0)
				valueNozzleInlet = propMax;
			else if (distToNozzleInlet < 70.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.6;
			else if (distToNozzleInlet < 90.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.5;
			else if (distToNozzleInlet < 100.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.3;
			value = valueNozzleInlet;
		}
		else if (angle == 90)
		{
			double startX = ptNozzleInletBottom.X();
			double xDist = std::abs(x - startX);
			const std::vector<double> thresholds90 = {
				40.0, 50.0, 100.0, 150.0, 200.0, 250.0, 300.0, 350.0, 400.0, 500.0
			};

			if (xDist > 1000)
			{
				value = propMin;
			}
			else
			{
				int layer = layerCount - 1;
				for (int i = 0; i < layerCount; ++i)
				{
					if (xDist <= thresholds90[i])
					{
						layer = i;
						break;
					}
				}
				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = propMin + (propMax - propMin) * ratio;
			}
		}

		return value;
	};

	// =====================================================================
	// 1) 壳体网格
	// =====================================================================
	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			double z = shellNodeCoords->Value(nodeID, 3);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalTemp(gp_Pnt(x, y, z)));
		}
	}

	// =====================================================================
	// 2) 喷管网格
	// =====================================================================
	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			double z = nozzleNodeCoords->Value(nodeID, 3);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalTemp(gp_Pnt(x, y, z)));
		}
	}

	// =====================================================================
	// 3) 推进剂网格
	// =====================================================================
	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			double z = propellantNodeCoords->Value(nodeID, 3);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantTemp(gp_Pnt(x, y, z)));
		}
	}

	// =====================================================================
	// 4) 隔热层网格（若存在）
	// =====================================================================
	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) hiNodeCoords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(hiNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = hiNodeCoords->Value(nodeID, 1);
			double y = hiNodeCoords->Value(nodeID, 2);
			double z = hiNodeCoords->Value(nodeID, 3);
			// 隔热层暂使用与推进剂相同的温度范围与逻辑
			resultInfo.heatInsulatingTemperatureNodeValues.push_back(calcPropellantTemp(gp_Pnt(x, y, z)));
		}
	}


	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	fallTemperatureResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	fallTemperatureResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);

	double shellMin = fallTemperatureResult.metalsMinTemperature;
	double shellMax = fallTemperatureResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		fallTemperatureResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (fallTemperatureResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		fallTemperatureResult.insulatingheatAvgTemperature = shellMin;
	}

	// 写回结果
	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(resultInfo);
	return true;
}


bool APISetNodeValue::CalculateAllFallPressureNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	// 拷贝一份用于修改，计算完成后通过 Set 写回
	auto resultInfo = resultInfoConst;

	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto angle = fallSettingInfo.angle;
	auto high = fallSettingInfo.high;

	// 清空历史结果
	resultInfo.shellOverpressureNodeValues.clear();
	resultInfo.nozzleOverpressureNodeValues.clear();
	resultInfo.propellantOverpressureNodeValues.clear();
	resultInfo.heatInsulatingOverpressureNodeValues.clear();

	auto fallOverpressureResult = ModelDataManager::GetInstance()->GetFallOverpressureResult();
	double metalMax = fallOverpressureResult.metalsMaxOverpressure;
	double metalMin = fallOverpressureResult.metalsMinOverpressure;
	double propMax = fallOverpressureResult.propellantsMaxOverpressure;
	double propMin = fallOverpressureResult.propellantsMinOverpressure;

	gp_Pnt ptShellLeftBottom = modelGeometryInfo.ptShellLeftBottom;
	gp_Pnt ptShellRightBottom = modelGeometryInfo.ptShellRightBottom;
	gp_Pnt ptNozzleInletBottom = modelGeometryInfo.ptNozzleInletBottom;
	gp_Pnt ptNozzleOutletBottom = modelGeometryInfo.ptNozzleOutletBottom;

	const int layerCount = 9;

	const std::vector<double> thresholds = {
		10.0, 25.0, 50.0, 90.0, 160.0, 260.0, 300.0, 330.0, 400.0, 500.0
	};
	const std::vector<double> thresholds90 = {
		40.0, 100.0, 150.0, 200.0, 300.0, 350.0,
		1000.0, 2000.0, 3000.0, 3500.0
	};

	// =====================================================================
	// Lambda：金属部件压力计算（壳体、喷管共用）
	// =====================================================================
	auto calcMetalStress = [&](const gp_Pnt& currentNode) -> double
	{
		double value = metalMin;

		if (angle == 0)
		{
			double dist1 = currentNode.Distance(ptShellLeftBottom);
			double dist2 = currentNode.Distance(ptShellRightBottom);
			double minDist = std::min(dist1, dist2);

			if (minDist > 600.0)
			{
				value = metalMin;
				if (abs(currentNode.Y() - ptShellLeftBottom.Y()) < 10)
					value = metalMin + (metalMax - metalMin) * 0.25;
			}
			else
			{
				int layer = layerCount - 1;
				for (int i = 0; i < layerCount; ++i)
				{
					if (minDist <= thresholds[i])
					{
						layer = i;
						if (layer >= 6) layer = 6;
						break;
					}
				}

				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = metalMin + (metalMax - metalMin) * ratio;

				if (value == metalMin)
					value = metalMin + (metalMax - metalMin) * 0.25;
			}
		}
		else if (angle > 0 && angle <= 15)
		{
			gp_Pnt cor_ptShellRightBottom(ptShellRightBottom.X() + 30,
				ptShellRightBottom.Y(),
				ptShellRightBottom.Z());
			double distToShellRight = currentNode.Distance(cor_ptShellRightBottom);
			gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 30,
				ptNozzleInletBottom.Y(),
				ptNozzleInletBottom.Z());
			double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
			double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

			double valueShellRight = metalMin;
			double valueNozzleInlet = metalMin;
			double valueNozzleOutlet = metalMin;

			if (distToShellRight < 100.0)
				valueShellRight = metalMax;
			else if (distToShellRight < 150.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToShellRight < 200.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToShellRight < 350.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToShellRight < 550.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToShellRight < 600.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToShellRight < 800.0)
				valueShellRight = metalMin + (metalMax - metalMin) * 0.2;

			if (distToNozzleInlet < 50.0)
				valueNozzleInlet = metalMax;
			else if (distToNozzleInlet < 100.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToNozzleInlet < 200.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleInlet < 350.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleInlet < 500.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleInlet < 600.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleInlet < 800.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.2;

			if (distToNozzleOutlet < 50.0)
				valueNozzleOutlet = metalMax;
			else if (distToNozzleOutlet < 100.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleOutlet < 150.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleOutlet < 250.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleOutlet < 300.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleOutlet < 500.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.2;
			else if (distToNozzleOutlet < 800.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.1;

			value = std::max({ valueShellRight, valueNozzleInlet, valueNozzleOutlet });
		}
		else if (angle > 15 && angle <= 60)
		{
			gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60,
				ptNozzleInletBottom.Y(),
				ptNozzleInletBottom.Z());
			double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
			double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

			double valueNozzleInlet = metalMin;
			double valueNozzleOutlet = metalMin;

			if (distToNozzleInlet < 50.0)
				valueNozzleInlet = metalMax;
			else if (distToNozzleInlet < 100.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToNozzleInlet < 200.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleInlet < 400.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleInlet < 550.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleInlet < 700.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleInlet < 900.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.2;

			if (distToNozzleOutlet < 50.0)
				valueNozzleOutlet = metalMax;
			else if (distToNozzleOutlet < 100.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleOutlet < 200.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleOutlet < 300.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleOutlet < 500.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleOutlet < 900.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.2;

			value = std::max({ valueNozzleInlet, valueNozzleOutlet });
		}
		else if (angle > 60 && angle < 90)
		{
			gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60,
				ptNozzleInletBottom.Y(),
				ptNozzleInletBottom.Z());
			double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);
			double distToNozzleOutlet = currentNode.Distance(ptNozzleOutletBottom);

			double valueNozzleInlet = metalMin;
			double valueNozzleOutlet = metalMin;

			if (cor_ptNozzleInletBottom.X() - currentNode.X() < 400)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.3;

			if (distToNozzleInlet < 50.0)
				valueNozzleInlet = metalMax;
			else if (distToNozzleInlet < 100.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.7;
			else if (distToNozzleInlet < 200.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleInlet < 350.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleInlet < 450.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleInlet < 600.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleInlet < 700.0)
				valueNozzleInlet = metalMin + (metalMax - metalMin) * 0.3;

			if (distToNozzleOutlet < 50.0)
				valueNozzleOutlet = metalMax;
			else if (distToNozzleOutlet < 100.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.6;
			else if (distToNozzleOutlet < 200.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.5;
			else if (distToNozzleOutlet < 250.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.4;
			else if (distToNozzleOutlet < 300.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.3;
			else if (distToNozzleOutlet < 600.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.2;
			else if (distToNozzleOutlet < 900.0)
				valueNozzleOutlet = metalMin + (metalMax - metalMin) * 0.1;

			value = std::max({ valueNozzleInlet, valueNozzleOutlet });
		}
		else if (angle == 90)
		{
			double startX = ptNozzleInletBottom.X();
			double xDist = std::abs(currentNode.X() - startX);

			if (xDist > 3000)
			{
				value = metalMin;
			}
			else
			{
				int layer = layerCount - 1;
				for (int i = 0; i < layerCount; ++i)
				{
					if (xDist <= thresholds90[i])
					{
						layer = i;
						break;
					}
				}
				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = metalMin + (metalMax - metalMin) * ratio;
			}
		}

		return value;
	};

	// =====================================================================
	// Lambda：推进剂压力计算
	// =====================================================================
	auto calcPropellantStress = [&](const gp_Pnt& currentNode) -> double
	{
		double value = propMin;
		double x = currentNode.X();
		double y = currentNode.Y();
		double z = currentNode.Z();

		if (angle == 0)
		{
			gp_Pnt A = ptShellLeftBottom;
			gp_Pnt B = ptShellRightBottom;
			gp_Pnt P = currentNode;

			gp_Vec AB(B.X() - A.X(), B.Y() - A.Y(), B.Z() - A.Z());
			gp_Vec AP(P.X() - A.X(), P.Y() - A.Y(), P.Z() - A.Z());

			double abLenSq = AB.SquareMagnitude();
			double distToSegment = 0.0;

			if (abLenSq < 1e-12)
			{
				distToSegment = P.Distance(A);
			}
			else
			{
				double t = AP.Dot(AB) / abLenSq;
				if (t <= 0.0)
					distToSegment = P.Distance(A);
				else if (t >= 1.0)
					distToSegment = P.Distance(B);
				else
				{
					gp_Vec cross = AB.Crossed(AP);
					distToSegment = cross.Magnitude() / std::sqrt(abLenSq);
				}
			}

			double zFactor = 1.0;
			if (z > ptShellLeftBottom.Z())
			{
				zFactor = std::max(0.0, 1.0 - (z - ptShellLeftBottom.Z()) / high);
			}

			std::vector<double> layerBoundaries = {
				10.0, 50.0, 100.0, 180.0, 230.0, 290.0, 350.0, 430.0, 550.0
			};

			int layer = -1;
			for (int i = 0; i < layerCount; ++i)
			{
				if (distToSegment <= layerBoundaries[i] + 1e-6)
				{
					layer = i;
					break;
				}
			}

			if (layer == -1)
				value = propMin;
			else
			{
				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = propMin + (propMax - propMin) * ratio * zFactor;
			}
		}
		else if (angle > 0 && angle <= 15)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = propMin;
			if (distToShellRight < 50.0)
				valueShellRight = propMax;
			else if (distToShellRight < 80.0)
				valueShellRight = propMin + (propMax - propMin) * 0.7;
			else if (distToShellRight < 120.0)
				valueShellRight = propMin + (propMax - propMin) * 0.6;
			else if (distToShellRight < 180.0)
				valueShellRight = propMin + (propMax - propMin) * 0.5;
			else if (distToShellRight < 230.0)
				valueShellRight = propMin + (propMax - propMin) * 0.4;
			else if (distToShellRight < 300.0)
				valueShellRight = propMin + (propMax - propMin) * 0.3;
			else if (distToShellRight < 400.0)
				valueShellRight = propMin + (propMax - propMin) * 0.2;

			gp_Pnt ptShellRightUp(ptShellRightBottom.X(),
				ptShellRightBottom.Y() - 1700,
				ptShellRightBottom.Z());
			double distToShellRightUp = currentNode.Distance(ptShellRightUp);
			if (distToShellRightUp < 30.0)
				valueShellRight = propMax;
			else if (distToShellRightUp < 40.0)
				valueShellRight = propMin + (propMax - propMin) * 0.7;
			else if (distToShellRightUp < 60.0)
				valueShellRight = propMin + (propMax - propMin) * 0.6;
			else if (distToShellRightUp < 90.0)
				valueShellRight = propMin + (propMax - propMin) * 0.5;
			else if (distToShellRightUp < 120.0)
				valueShellRight = propMin + (propMax - propMin) * 0.4;
			else if (distToShellRightUp < 250.0)
				valueShellRight = propMin + (propMax - propMin) * 0.3;
			else if (distToShellRightUp < 300.0)
				valueShellRight = propMin + (propMax - propMin) * 0.2;

			if (abs(ptNozzleInletBottom.X() - x) < 200)
				valueShellRight = propMin + (propMax - propMin) * 0.3;

			value = valueShellRight;
		}
		else if (angle > 15 && angle <= 60)
		{
			gp_Pnt cor_ptNozzleInletBottom(ptNozzleInletBottom.X() + 60,
				ptNozzleInletBottom.Y(),
				ptNozzleInletBottom.Z());
			double distToNozzleInlet = currentNode.Distance(cor_ptNozzleInletBottom);

			double valueNozzleInlet = propMin;
			if (distToNozzleInlet < 50.0)
				valueNozzleInlet = propMax;
			else if (distToNozzleInlet < 100.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.7;
			else if (distToNozzleInlet < 150.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.6;
			else if (distToNozzleInlet < 200.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.5;
			else if (distToNozzleInlet < 300.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.4;
			else if (distToNozzleInlet < 500.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.3;
			else if (distToNozzleInlet < 800.0)
				valueNozzleInlet = propMin + (propMax - propMin) * 0.2;

			value = valueNozzleInlet;
		}
		else if (angle > 60 && angle < 90)
		{
			gp_Pnt capsuleCenter(
				modelGeometryInfo.theXmin + (modelGeometryInfo.theXmax - modelGeometryInfo.theXmin) / 2.0,
				modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
				ptNozzleInletBottom.Z()
			);

			double capsuleLength = 4000.0;
			double capsuleRadius = 500.0;
			double cylinderHalfLength = (capsuleLength - 2.0 * capsuleRadius) / 2.0;
			if (cylinderHalfLength < 0) cylinderHalfLength = 0;

			gp_Dir axisDir(1.0, 0.0, 0.0);
			gp_Vec axisVec(axisDir);

			gp_Pnt cylStart(capsuleCenter.X() - cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());
			gp_Pnt cylEnd(capsuleCenter.X() + cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());

			gp_Pnt P = currentNode;
			gp_Vec centerToP(P.X() - capsuleCenter.X(), P.Y() - capsuleCenter.Y(), P.Z() - capsuleCenter.Z());
			double t = centerToP.Dot(axisVec);

			double distToSurface = 0.0;

			if (t < -cylinderHalfLength - capsuleRadius)
				distToSurface = P.Distance(cylStart) - capsuleRadius;
			else if (t > cylinderHalfLength + capsuleRadius)
				distToSurface = P.Distance(cylEnd) - capsuleRadius;
			else if (t < -cylinderHalfLength)
				distToSurface = P.Distance(cylStart) - capsuleRadius;
			else if (t > cylinderHalfLength)
				distToSurface = P.Distance(cylEnd) - capsuleRadius;
			else
			{
				gp_Pnt projPoint(capsuleCenter.X() + t, capsuleCenter.Y(), capsuleCenter.Z());
				double distToAxis = P.Distance(projPoint);
				distToSurface = distToAxis - capsuleRadius;
			}

			double effectiveDist = std::max(0.0, distToSurface);

			if (effectiveDist < 100.0)
				value = propMin;
			else if (effectiveDist < 200.0)
				value = propMin + (propMax - propMin) * 0.2;
			else
				value = propMin + (propMax - propMin) * 0.3;
		}
		else if (angle == 90)
		{
			// ---------- 胶囊影响（大区域背景）----------
			gp_Pnt capsuleCenter(
				modelGeometryInfo.theXmin + (modelGeometryInfo.theXmax - modelGeometryInfo.theXmin) / 2.0,
				modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
				ptNozzleInletBottom.Z()
			);

			double capsuleLength = 4000.0;
			double capsuleRadius = 500.0;
			double cylinderHalfLength = (capsuleLength - 2.0 * capsuleRadius) / 2.0;
			if (cylinderHalfLength < 0) cylinderHalfLength = 0;

			gp_Dir axisDir(1.0, 0.0, 0.0);
			gp_Vec axisVec(axisDir);

			gp_Pnt cylStart(capsuleCenter.X() - cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());
			gp_Pnt cylEnd(capsuleCenter.X() + cylinderHalfLength, capsuleCenter.Y(), capsuleCenter.Z());

			gp_Pnt P = currentNode;
			gp_Vec centerToP(P.X() - capsuleCenter.X(), P.Y() - capsuleCenter.Y(), P.Z() - capsuleCenter.Z());
			double t = centerToP.Dot(axisVec);

			double distToSurface = 0.0;

			if (t < -cylinderHalfLength - capsuleRadius)
				distToSurface = P.Distance(cylStart) - capsuleRadius;
			else if (t > cylinderHalfLength + capsuleRadius)
				distToSurface = P.Distance(cylEnd) - capsuleRadius;
			else if (t < -cylinderHalfLength)
				distToSurface = P.Distance(cylStart) - capsuleRadius;
			else if (t > cylinderHalfLength)
				distToSurface = P.Distance(cylEnd) - capsuleRadius;
			else
			{
				gp_Pnt projPoint(capsuleCenter.X() + t, capsuleCenter.Y(), capsuleCenter.Z());
				double distToAxis = P.Distance(projPoint);
				distToSurface = distToAxis - capsuleRadius;
			}

			double effectiveDist = std::max(0.0, distToSurface);

			double valueCapsule;
			if (effectiveDist < 100.0)
				valueCapsule = propMin;
			else if (effectiveDist < 500.0)
				valueCapsule = propMin + (propMax - propMin) * 0.2;
			else if (effectiveDist < 1000.0)
				valueCapsule = propMin + (propMax - propMin) * 0.1;
			else
				valueCapsule = propMin;

			// ---------- 矩形核心影响（局部高应力区）----------
			gp_Pnt circleCenter(
				ptNozzleInletBottom.X() - 200,
				modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
				ptNozzleInletBottom.Z()
			);

			double dx = abs(circleCenter.X() - x);
			double dy = abs(circleCenter.Y() - y);

			double valueRect;
			if (dx < 30.0 && dy < 300.0)
			{
				valueRect = propMax;
			}
			else
			{
				double distX = std::max(0.0, dx - 30.0);
				double distY = std::max(0.0, dy - 300.0);
				double distToRect = std::sqrt(distX * distX + distY * distY);

				if (distToRect < 40.0)
					valueRect = propMin + (propMax - propMin) * 0.8;
				else if (distToRect < 50.0)
					valueRect = propMin + (propMax - propMin) * 0.7;
				else if (distToRect < 70.0)
					valueRect = propMin + (propMax - propMin) * 0.6;
				else if (distToRect < 90.0)
					valueRect = propMin + (propMax - propMin) * 0.5;
				else if (distToRect < 100.0)
					valueRect = propMin + (propMax - propMin) * 0.4;
				else if (distToRect < 120.0)
					valueRect = propMin + (propMax - propMin) * 0.2;
				else
					valueRect = propMin;
			}

			value = std::max(valueCapsule, valueRect);
		}

		return value;
	};

	// =====================================================================
	// 1) 壳体网格
	// =====================================================================
	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			double z = shellNodeCoords->Value(nodeID, 3);
			resultInfo.shellOverpressureNodeValues.push_back(calcMetalStress(gp_Pnt(x, y, z)));
		}
	}

	// =====================================================================
	// 2) 喷管网格
	// =====================================================================
	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			double z = nozzleNodeCoords->Value(nodeID, 3);
			resultInfo.nozzleOverpressureNodeValues.push_back(calcMetalStress(gp_Pnt(x, y, z)));
		}
	}

	// =====================================================================
	// 3) 推进剂网格
	// =====================================================================
	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			double z = propellantNodeCoords->Value(nodeID, 3);
			resultInfo.propellantOverpressureNodeValues.push_back(calcPropellantStress(gp_Pnt(x, y, z)));
		}
	}

	// =====================================================================
	// 4) 隔热层网格（若存在）
	// =====================================================================
	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) hiNodeCoords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(hiNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = hiNodeCoords->Value(nodeID, 1);
			double y = hiNodeCoords->Value(nodeID, 2);
			double z = hiNodeCoords->Value(nodeID, 3);
			// 隔热层暂使用与推进剂相同的压力范围与逻辑
			resultInfo.heatInsulatingOverpressureNodeValues.push_back(calcPropellantStress(gp_Pnt(x, y, z)));
		}
	}


	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	fallOverpressureResult.metalsAvgOverpressure = computeAvg(resultInfo.shellOverpressureNodeValues);
	fallOverpressureResult.mpropellantsAvgOverpressure = computeAvg(resultInfo.propellantOverpressureNodeValues);

	double shellMin = fallOverpressureResult.metalsMinOverpressure;
	double shellMax = fallOverpressureResult.metalsMaxOverpressure;
	if (shellMax > shellMin) {
		fallOverpressureResult.insulatingheatAvgOverpressure =
			shellMin + (shellMax - shellMin) * (fallOverpressureResult.metalsAvgOverpressure - shellMin) / (shellMax - shellMin);
	}
	else {
		fallOverpressureResult.insulatingheatAvgOverpressure = shellMin;
	}

	// 写回结果
	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(resultInfo);
	return true;
}


bool APISetNodeValue::CalculateAllFallReactionDegreeNodeValues()
{
	auto resultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	// 清空历史结果
	resultInfo.propellantReactionDegreeNodeValues.clear();

	if (!resultInfo.isChecked)
	{
		return false;
	}

	auto fallReactionDegreeResult = ModelDataManager::GetInstance()->GetFallReactionDegreeResult();
	auto max_value = fallReactionDegreeResult.propellantsMaxReactionDegree;
	auto min_value = fallReactionDegreeResult.propellantsMinReactionDegree;

	TColStd_PackedMapOfInteger allnode = modelMeshInfo.propellantMesh->GetAllNodes();
	Handle(TColStd_HArray2OfReal) nodecoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();

	gp_Pnt ptShellLeftBottom = modelGeometryInfo.ptShellLeftBottom;
	gp_Pnt ptShellRightBottom = modelGeometryInfo.ptShellRightBottom;
	gp_Pnt ptNozzleInletBottom = modelGeometryInfo.ptNozzleInletBottom;

	const int layerCount = 9;

	std::vector<double> layerBoundaries = { 10.0, 25.0, 50.0, 90.0, 160.0, 260.0, 300.0, 330.0, 350.0 };
	double maxDistance = layerBoundaries.back();

	for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
	{
		int nodeID = it.Key();
		double x = nodecoords->Value(nodeID, 1);
		double y = nodecoords->Value(nodeID, 2);
		double z = nodecoords->Value(nodeID, 3);
		gp_Pnt currentNode(x, y, z);

		double value = min_value;

		if (angle == 0)
		{
			gp_Pnt A = ptShellLeftBottom;
			gp_Pnt B = ptShellRightBottom;
			gp_Pnt P = currentNode;

			gp_Vec AB(B.X() - A.X(), B.Y() - A.Y(), B.Z() - A.Z());
			gp_Vec AP(P.X() - A.X(), P.Y() - A.Y(), P.Z() - A.Z());

			double abLenSq = AB.SquareMagnitude();
			double distToSegment = 0.0;

			if (abLenSq < 1e-12)
			{
				// 两底部点重合，退化为到单点的距离
				distToSegment = P.Distance(A);
			}
			else
			{
				double t = AP.Dot(AB) / abLenSq;

				if (t <= 0.0)
				{
					distToSegment = P.Distance(A);
				}
				else if (t >= 1.0)
				{
					distToSegment = P.Distance(B);
				}
				else
				{
					gp_Vec cross = AB.Crossed(AP);
					distToSegment = cross.Magnitude() / std::sqrt(abLenSq);
				}
			}

			// Z方向衰减因子
			double zFactor = 1.0;
			if (z > ptShellLeftBottom.Z())
			{
				zFactor = std::max(0.0, 1.0 - (z - ptShellLeftBottom.Z()) / high);
			}

			// 根据自定义边界判断所属层
			int layer = -1;
			for (int i = 0; i < layerCount; ++i)
			{
				if (distToSegment <= layerBoundaries[i] + 1e-6)
				{
					layer = i;
					break;
				}
			}

			if (layer == -1)
			{
				// 超出最外层边界
				value = min_value;
			}
			else
			{
				// 从内到外逐层递减
				double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
				value = min_value + (max_value - min_value) * ratio * zFactor;
			}
		}
		else if (angle > 0 && angle <= 15)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = min_value + (max_value - min_value) * 0.4;
			// ptShellRightBottom 影响
			if (distToShellRight < 300.0)
				valueShellRight = min_value + (max_value - min_value) * 0.7;

			value = valueShellRight;
		}
		else if (angle > 15 && angle <= 60)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = min_value + (max_value - min_value) * 0.4;
			// ptShellRightBottom 影响
			if (distToShellRight < 300.0)
				valueShellRight = min_value + (max_value - min_value) * 0.7;

			value = valueShellRight;
		}
		else if (angle > 60 && angle < 90)
		{
			double distToShellRight = currentNode.Distance(ptShellRightBottom);
			double valueShellRight = min_value + (max_value - min_value) * 0.4;
			// ptShellRightBottom 影响
			if (distToShellRight < 200.0)
				valueShellRight = min_value + (max_value - min_value) * 0.7;

			gp_Pnt ptShellRightUp(ptShellRightBottom.X(), ptShellRightBottom.Y() - 1200, ptShellRightBottom.Z());
			double distToShellRightUp = currentNode.Distance(ptShellRightUp);
			if (distToShellRightUp < 300)
				valueShellRight = min_value + (max_value - min_value) * 0.6;


			value = valueShellRight;
		}
		else if (angle == 90)
		{
			gp_Pnt circleCenter(ptNozzleInletBottom.X() - 200,
				modelGeometryInfo.theYmin + (modelGeometryInfo.theYmax - modelGeometryInfo.theYmin) / 2.0,
				ptNozzleInletBottom.Z());

			double dx = abs(circleCenter.X() - x);
			double dy = abs(circleCenter.Y() - y);

			// 核心矩形区域：保持不变
			if (dx < 20.0 && dy < 300.0)
			{
				value = max_value;
			}
			else
			{
				double distX = std::max(0.0, dx - 30.0);
				double distY = std::max(0.0, dy - 300.0);
				double distToRect = std::sqrt(distX * distX + distY * distY);

				// 硬编码阶梯衰减
				if (distToRect < 100.0)
				{
					value = min_value + (max_value - min_value) * 0.8;
				}
				else if (distToRect < 130.0)
				{
					value = min_value + (max_value - min_value) * 0.7;
				}
				else if (distToRect < 150.0)
				{
					value = min_value + (max_value - min_value) * 0.6;
				}
				else if (distToRect < 300.0)
				{
					value = min_value + (max_value - min_value) * 0.5;
				}
				else if (distToRect < 1000.0)
				{
					value = min_value + (max_value - min_value) * 0.4;
				}
				else if (distToRect < 4000.0)
				{
					value = min_value + (max_value - min_value) * 0.3;
				}
				else if (distToRect < 4200.0)
				{
					value = min_value + (max_value - min_value) * 0.2;
				}
				else
				{
					value = min_value;
				}
			}
		}

		resultInfo.propellantReactionDegreeNodeValues.push_back(value);
	}


	// 推进剂平均值
	double sumProp = 0.0;
	size_t countProp = 0;
	for (double val : resultInfo.propellantReactionDegreeNodeValues) {
		if (val != -1.0) {
			sumProp += val;
			++countProp;
		}
	}
	fallReactionDegreeResult.propellantsAvgReactionDegree = (countProp > 0) ? (sumProp / countProp) : 0.0;


	// 写回结果
	ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(resultInfo);

	return true;
}


bool APISetNodeValue::CalculateAllFastCombustionTempNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetFastCombustionAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	auto resultInfo = resultInfoConst;
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto fastCombustionTemperatureResult = ModelDataManager::GetInstance()->GetFastCombustionTemperatureResult();
	double metalMax = fastCombustionTemperatureResult.metalsMaxTemperature;
	double metalMin = fastCombustionTemperatureResult.metalsMinTemperature;
	double propMax = fastCombustionTemperatureResult.propellantsMaxTemperature;
	double propMin = fastCombustionTemperatureResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = (y_min + y_max) / 2.0;
	const double a_main = (x_max - x_min) / 2.0;
	const double b_main = (y_max - y_min) / 2.0 * 1.1;

	std::vector<std::pair<double, double>> ellipses = {
		{a_main * 0.8, b_main * 0.6},
		{a_main * 0.82, b_main * 0.62},
		{a_main * 0.85, b_main * 0.65},
		{a_main * 0.9, b_main * 0.7},
		{a_main * 0.95, b_main * 0.75},
		{a_main * 0.98, b_main * 0.78},
		{a_main * 1.0, b_main * 0.8},
		{a_main * 1.0, b_main * 1.0}
	};

	auto isInEllipse = [](double x, double y, double cx, double cy, double a, double b) -> bool {
		double dx = x - cx;
		double dy = y - cy;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dy * dy) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcEllipseValue = [&](double x, double y, double min_val, double max_val) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second))
			return min_val;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second))
			return min_val + (max_val - min_val) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second))
			return min_val + (max_val - min_val) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second))
			return min_val + (max_val - min_val) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second))
			return min_val + (max_val - min_val) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second))
			return min_val + (max_val - min_val) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second))
			return min_val + (max_val - min_val) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second))
			return min_val + (max_val - min_val) * 7.5 / 9.0;
		else
			return max_val;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcEllipseValue(x, y, metalMin, metalMax));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcEllipseValue(x, y, metalMin, metalMax));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcEllipseValue(x, y, propMin, propMax));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingTemperatureNodeValues.assign(hiNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetFastCombustionAnalysisResultInfo(resultInfo);

	// ========== 计算统计量（跳过 -1.0） ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	fastCombustionTemperatureResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	fastCombustionTemperatureResult.outheatAvgTemperature = fastCombustionTemperatureResult.metalsAvgTemperature;
	fastCombustionTemperatureResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	fastCombustionTemperatureResult.outheatStandardTemperature = fastCombustionTemperatureResult.metalsStandardTemperature;

	fastCombustionTemperatureResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	fastCombustionTemperatureResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = fastCombustionTemperatureResult.metalsMinTemperature;
	double shellMax = fastCombustionTemperatureResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		fastCombustionTemperatureResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (fastCombustionTemperatureResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		fastCombustionTemperatureResult.insulatingheatAvgTemperature = shellMin;
	}
	fastCombustionTemperatureResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);

	ModelDataManager::GetInstance()->SetFastCombustionTemperatureResult(fastCombustionTemperatureResult);

	return true;
}


bool APISetNodeValue::CalculateAllSlowCombustionTempNodeValues()
{
	const auto resultInfoConst = ModelDataManager::GetInstance()->GetSlowCombustionAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	auto resultInfo = resultInfoConst;
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto slowCombustionTemperatureResult = ModelDataManager::GetInstance()->GetSlowCombustionTemperatureResult();
	double metalMax = slowCombustionTemperatureResult.metalsMaxTemperature;
	double metalMin = slowCombustionTemperatureResult.metalsMinTemperature;
	double propMax = slowCombustionTemperatureResult.propellantsMaxTemperature;
	double propMin = slowCombustionTemperatureResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double rect_width = x_max - x_min;
	const double rect_height = y_max - y_min;
	const double h = (x_min + x_max) / 2.0;
	const double k = (y_min + y_max) / 2.0;
	const double a_main = rect_width / 2.0;
	const double b_main = rect_height / 2.0;

	const double top_center_z = k + rect_height / 2.0;
	const double bottom_center_z = k - rect_height / 2.0;
	const double left_center_x = h - a_main * 0.6;
	const double right_center_x = h + a_main * 0.6;

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	// 金属部件（壳体、喷管）使用4层上下椭圆
	std::vector<std::pair<double, double>> top_ellipses_metal = {
		{a_main * 0.6, b_main * 0.3},
		{a_main * 0.7, b_main * 0.4},
		{a_main * 0.8, b_main * 0.5},
		{a_main * 0.9, b_main * 0.6}
	};
	std::vector<std::pair<double, double>> bottom_ellipses_metal = {
		{a_main * 0.6, b_main * 0.3},
		{a_main * 0.7, b_main * 0.4},
		{a_main * 0.8, b_main * 0.5},
		{a_main * 0.9, b_main * 0.6}
	};
	std::vector<double> circle_radii_metal = { b_main * 0.5 * 0.4, b_main * 0.5 * 0.8 };

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, a_main, b_main))
		{
			if (isInEllipse(x, y, h, top_center_z, top_ellipses_metal[0].first, top_ellipses_metal[0].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_metal[0].first, bottom_ellipses_metal[0].second))
				return metalMax;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_metal[1].first, top_ellipses_metal[1].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_metal[1].first, bottom_ellipses_metal[1].second))
				return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_metal[2].first, top_ellipses_metal[2].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_metal[2].first, bottom_ellipses_metal[2].second))
				return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_metal[3].first, top_ellipses_metal[3].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_metal[3].first, bottom_ellipses_metal[3].second))
				return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
			else
			{
				if (isInEllipse(x, y, left_center_x, k, circle_radii_metal[0], circle_radii_metal[0]) ||
					isInEllipse(x, y, right_center_x, k, circle_radii_metal[0], circle_radii_metal[0]))
					return metalMin;
				else if (isInEllipse(x, y, left_center_x, k, circle_radii_metal[1], circle_radii_metal[1]) ||
					isInEllipse(x, y, right_center_x, k, circle_radii_metal[1], circle_radii_metal[1]))
					return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
				else
					return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
			}
		}
		else
		{
			return metalMax;
		}
	};

	// 推进剂使用6层上下椭圆
	std::vector<std::pair<double, double>> top_ellipses_prop = {
		{a_main * 0.5, b_main * 0.2},
		{a_main * 0.6, b_main * 0.3},
		{a_main * 0.7, b_main * 0.4},
		{a_main * 0.8, b_main * 0.5},
		{a_main * 0.9, b_main * 0.6},
		{a_main * 1.0, b_main * 0.7}
	};
	std::vector<std::pair<double, double>> bottom_ellipses_prop = {
		{a_main * 0.5, b_main * 0.2},
		{a_main * 0.6, b_main * 0.3},
		{a_main * 0.7, b_main * 0.4},
		{a_main * 0.8, b_main * 0.5},
		{a_main * 0.9, b_main * 0.6},
		{a_main * 1.0, b_main * 0.7}
	};
	std::vector<double> circle_radii_prop = { b_main * 0.5 * 0.4, b_main * 0.5 * 0.8 };

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, a_main, b_main))
		{
			if (isInEllipse(x, y, h, top_center_z, top_ellipses_prop[0].first, top_ellipses_prop[0].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_prop[0].first, bottom_ellipses_prop[0].second))
				return propMax;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_prop[1].first, top_ellipses_prop[1].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_prop[1].first, bottom_ellipses_prop[1].second))
				return propMin + (propMax - propMin) * 7.5 / 9.0;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_prop[2].first, top_ellipses_prop[2].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_prop[2].first, bottom_ellipses_prop[2].second))
				return propMin + (propMax - propMin) * 6.5 / 9.0;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_prop[3].first, top_ellipses_prop[3].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_prop[3].first, bottom_ellipses_prop[3].second))
				return propMin + (propMax - propMin) * 5.5 / 9.0;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_prop[4].first, top_ellipses_prop[4].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_prop[4].first, bottom_ellipses_prop[4].second))
				return propMin + (propMax - propMin) * 4.5 / 9.0;
			else if (isInEllipse(x, y, h, top_center_z, top_ellipses_prop[5].first, top_ellipses_prop[5].second) ||
				isInEllipse(x, y, h, bottom_center_z, bottom_ellipses_prop[5].first, bottom_ellipses_prop[5].second))
				return propMin + (propMax - propMin) * 3.5 / 9.0;
			else
			{
				if (isInEllipse(x, y, left_center_x, k, circle_radii_prop[0], circle_radii_prop[0]) ||
					isInEllipse(x, y, right_center_x, k, circle_radii_prop[0], circle_radii_prop[0]))
					return propMin;
				else if (isInEllipse(x, y, left_center_x, k, circle_radii_prop[1], circle_radii_prop[1]) ||
					isInEllipse(x, y, right_center_x, k, circle_radii_prop[1], circle_radii_prop[1]))
					return propMin + (propMax - propMin) * 1.5 / 9.0;
				else
					return propMin + (propMax - propMin) * 2.5 / 9.0;
			}
		}
		else
		{
			return propMax;
		}
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingTemperatureNodeValues.assign(hiNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetSlowCombustionAnalysisResultInfo(resultInfo);

	// ========== 统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	slowCombustionTemperatureResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	slowCombustionTemperatureResult.outheatAvgTemperature = slowCombustionTemperatureResult.metalsAvgTemperature;
	slowCombustionTemperatureResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	slowCombustionTemperatureResult.outheatStandardTemperature = slowCombustionTemperatureResult.metalsStandardTemperature;

	slowCombustionTemperatureResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	slowCombustionTemperatureResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = slowCombustionTemperatureResult.metalsMinTemperature;
	double shellMax = slowCombustionTemperatureResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		slowCombustionTemperatureResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (slowCombustionTemperatureResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		slowCombustionTemperatureResult.insulatingheatAvgTemperature = shellMin;
	}
	slowCombustionTemperatureResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);


	ModelDataManager::GetInstance()->SetSlowCombustionTemperatureResult(slowCombustionTemperatureResult);

	return true;
}


bool APISetNodeValue::CalculateAllShootStressNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	auto resultInfo = resultInfoConst;
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	resultInfo.shellStressNodeValues.clear();
	resultInfo.nozzleStressNodeValues.clear();
	resultInfo.propellantStressNodeValues.clear();
	resultInfo.heatInsulatingStressNodeValues.clear();

	auto shootStressResult = ModelDataManager::GetInstance()->GetShootStressResult();
	double metalMax = shootStressResult.metalsMaxStress;
	double metalMin = shootStressResult.metalsMinStress;
	double propMax = shootStressResult.propellantsMaxStress;
	double propMin = shootStressResult.propellantsMinStress;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 3.0;

	std::vector<std::pair<double, double>> ellipses = {
		{12.0, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.65},
		{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return metalMax;
				return metalMin + (metalMax - metalMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return propMax;
				return propMin + (propMax - propMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			resultInfo.shellStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			resultInfo.nozzleStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			resultInfo.propellantStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingStressNodeValues.assign(hiNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(resultInfo);

	// ========== 统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	shootStressResult.metalsAvgStress = computeAvg(resultInfo.shellStressNodeValues);
	shootStressResult.outheatAvgStress = shootStressResult.metalsAvgStress;
	shootStressResult.metalsStandardStress = computeStd(resultInfo.shellStressNodeValues);
	shootStressResult.outheatStandardStress = shootStressResult.metalsStandardStress;

	shootStressResult.propellantsAvgStress = computeAvg(resultInfo.propellantStressNodeValues);
	shootStressResult.propellantsStandardStress = computeStd(resultInfo.propellantStressNodeValues);

	double shellMin = shootStressResult.metalsMinStress;
	double shellMax = shootStressResult.metalsMaxStress;
	if (shellMax > shellMin) {
		shootStressResult.insulatingheatAvgStress =
			shellMin + (shellMax - shellMin) * (shootStressResult.metalsAvgStress - shellMin) / (shellMax - shellMin);
	}
	else {
		shootStressResult.insulatingheatAvgStress = shellMin;
	}
	shootStressResult.insulatingheatStandardStress = computeStd(resultInfo.heatInsulatingStressNodeValues);

	ModelDataManager::GetInstance()->SetShootStressResult(shootStressResult);

	return true;
}


bool APISetNodeValue::CalculateAllShootStrainNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	auto resultInfo = resultInfoConst;
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	resultInfo.shellStrainNodeValues.clear();
	resultInfo.nozzleStrainNodeValues.clear();
	resultInfo.propellantStrainNodeValues.clear();
	resultInfo.heatInsulatingStrainNodeValues.clear();

	auto shootStrainResult = ModelDataManager::GetInstance()->GetShootStrainResult();
	double metalMax = shootStrainResult.metalsMaxStrain;
	double metalMin = shootStrainResult.metalsMinStrain;
	double propMax = shootStrainResult.propellantsMaxStrain;
	double propMin = shootStrainResult.propellantsMinStrain;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 3.0;

	std::vector<std::pair<double, double>> ellipses = {
		{12.0, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.65},
		{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return metalMax;
				return metalMin + (metalMax - metalMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return propMax;
				return propMin + (propMax - propMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			resultInfo.shellStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			resultInfo.nozzleStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			resultInfo.propellantStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingStrainNodeValues.assign(hiNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(resultInfo);

	// ========== 统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	shootStrainResult.metalsAvgStrain = computeAvg(resultInfo.shellStrainNodeValues);
	shootStrainResult.outheatAvgStrain = shootStrainResult.metalsAvgStrain;
	shootStrainResult.metalsStandardStrain = computeStd(resultInfo.shellStrainNodeValues);
	shootStrainResult.outheatStandardStrain = shootStrainResult.metalsStandardStrain;

	shootStrainResult.mpropellantsAvgStrain = computeAvg(resultInfo.propellantStrainNodeValues);
	shootStrainResult.propellantsStandardStrain = computeStd(resultInfo.propellantStrainNodeValues);

	double shellMin = shootStrainResult.metalsMinStrain;
	double shellMax = shootStrainResult.metalsMaxStrain;
	if (shellMax > shellMin) {
		shootStrainResult.insulatingheatAvgStrain =
			shellMin + (shellMax - shellMin) * (shootStrainResult.metalsAvgStrain - shellMin) / (shellMax - shellMin);
	}
	else {
		shootStrainResult.insulatingheatAvgStrain = shellMin;
	}
	shootStrainResult.insulatingheatStandardStrain = computeStd(resultInfo.heatInsulatingStrainNodeValues);

	ModelDataManager::GetInstance()->SetShootStrainResult(shootStrainResult);

	return true;
}


bool APISetNodeValue::CalculateAllShootTempNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	auto resultInfo = resultInfoConst;
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto shootTemperatureResult = ModelDataManager::GetInstance()->GetShootTemperatureResult();
	double metalMax = shootTemperatureResult.metalsMaxTemperature;
	double metalMin = shootTemperatureResult.metalsMinTemperature;
	double propMax = shootTemperatureResult.propellantsMaxTemperature;
	double propMin = shootTemperatureResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 2.0;

	std::vector<std::pair<double, double>> ellipses = {
		{12.0, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.65},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.75},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.85},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return metalMax;
				return metalMin + (metalMax - metalMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return propMax;
				return propMin + (propMax - propMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingTemperatureNodeValues.assign(hiNodes.Extent(), -1.0);
	}

	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(resultInfo);

	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	shootTemperatureResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	shootTemperatureResult.outheatAvgTemperature = shootTemperatureResult.metalsAvgTemperature;
	shootTemperatureResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	shootTemperatureResult.outheatStandardTemperature = shootTemperatureResult.metalsStandardTemperature;

	shootTemperatureResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	shootTemperatureResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = shootTemperatureResult.metalsMinTemperature;
	double shellMax = shootTemperatureResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		shootTemperatureResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (shootTemperatureResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		shootTemperatureResult.insulatingheatAvgTemperature = shellMin;
	}
	shootTemperatureResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);


	ModelDataManager::GetInstance()->SetShootTemperatureResult(shootTemperatureResult);

	return true;
}


bool APISetNodeValue::CalculateAllShootPressureNodeValues()
{
	const auto& resultInfoConst = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	if (!resultInfoConst.isChecked)
	{
		return false;
	}

	auto resultInfo = resultInfoConst;
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	resultInfo.shellOverpressureNodeValues.clear();
	resultInfo.nozzleOverpressureNodeValues.clear();
	resultInfo.propellantOverpressureNodeValues.clear();
	resultInfo.heatInsulatingOverpressureNodeValues.clear();

	auto shootOverpressureResult = ModelDataManager::GetInstance()->GetShootOverpressureResult();
	double metalMax = shootOverpressureResult.metalsMaxOverpressure;
	double metalMin = shootOverpressureResult.metalsMinOverpressure;
	double propMax = shootOverpressureResult.propellantsMaxOverpressure;
	double propMin = shootOverpressureResult.propellantsMinOverpressure;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 1.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12.0, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.65},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.85},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return metalMax;
				return metalMin + (metalMax - metalMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (size_t i = 0; i < ellipses.size(); ++i)
		{
			if (isInEllipse(x, y, h, k, ellipses[i].first, ellipses[i].second))
			{
				if (i == 0) return -1.0;
				if (i == 1) return propMax;
				return propMin + (propMax - propMin) * (9.5 - static_cast<double>(i)) / 9.0;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellNodeCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = shellNodeCoords->Value(nodeID, 1);
			double y = shellNodeCoords->Value(nodeID, 2);
			resultInfo.shellOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleNodeCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nozzleNodeCoords->Value(nodeID, 1);
			double y = nozzleNodeCoords->Value(nodeID, 2);
			resultInfo.nozzleOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propellantNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propellantNodeCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propellantNodes); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = propellantNodeCoords->Value(nodeID, 1);
			double y = propellantNodeCoords->Value(nodeID, 2);
			resultInfo.propellantOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		resultInfo.heatInsulatingOverpressureNodeValues.assign(hiNodes.Extent(), -1.0);
	}

	// ========== 统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	shootOverpressureResult.metalsAvgOverpressure = computeAvg(resultInfo.shellOverpressureNodeValues);
	shootOverpressureResult.outheatAvgOverpressure = shootOverpressureResult.metalsAvgOverpressure;
	shootOverpressureResult.metalsStandardOverpressure = computeStd(resultInfo.shellOverpressureNodeValues);
	shootOverpressureResult.outheatStandardOverpressure = shootOverpressureResult.metalsStandardOverpressure;

	shootOverpressureResult.mpropellantsAvgOverpressure = computeAvg(resultInfo.propellantOverpressureNodeValues);
	shootOverpressureResult.propellantsStandardOverpressure = computeStd(resultInfo.propellantOverpressureNodeValues);

	double shellMin = shootOverpressureResult.metalsMinOverpressure;
	double shellMax = shootOverpressureResult.metalsMaxOverpressure;
	if (shellMax > shellMin) {
		shootOverpressureResult.insulatingheatAvgOverpressure =
			shellMin + (shellMax - shellMin) * (shootOverpressureResult.metalsAvgOverpressure - shellMin) / (shellMax - shellMin);
	}
	else {
		shootOverpressureResult.insulatingheatAvgOverpressure = shellMin;
	}
	shootOverpressureResult.insulatingheatStandardOverpressure = computeStd(resultInfo.heatInsulatingOverpressureNodeValues);



	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(resultInfo);

	return true;
}


bool APISetNodeValue::CalculateAllShootReactionDegreeNodeValues()
{
	auto shootSettingInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	if (!resultInfo.isChecked)
		return false;

	resultInfo.propellantReactionDegreeNodeValues.clear();

	auto shootReactionDegreeResult = ModelDataManager::GetInstance()->GetShootReactionDegreeResult();
	double max_value = shootReactionDegreeResult.propellantsMaxReactionDegree;
	double min_value = shootReactionDegreeResult.propellantsMinReactionDegree;

	if (modelMeshInfo.propellantMesh.IsNull())
	{
		ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(resultInfo);
		return false;
	}

	TColStd_PackedMapOfInteger allnode = modelMeshInfo.propellantMesh->GetAllNodes();
	Handle(TColStd_HArray2OfReal) nodecoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.65},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.9},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 1.0}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
	{
		int nodeID = it.Key();
		double x = nodecoords->Value(nodeID, 1);
		double y = nodecoords->Value(nodeID, 2);

		double value = min_value;

		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second))
		{
			value = -1.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second))
		{
			value = max_value;
		}
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second))
		{
			value = min_value + (max_value - min_value) * 7.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second))
		{
			value = min_value + (max_value - min_value) * 6.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second))
		{
			value = min_value + (max_value - min_value) * 5.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second))
		{
			value = min_value + (max_value - min_value) * 4.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second))
		{
			value = min_value + (max_value - min_value) * 3.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second))
		{
			value = min_value + (max_value - min_value) * 2.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second))
		{
			value = min_value + (max_value - min_value) * 1.5 / 9.0;
		}
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second))
		{
			value = min_value + (max_value - min_value) * 0.5 / 9.0;
		}

		resultInfo.propellantReactionDegreeNodeValues.push_back(value);
	}

	ModelDataManager::GetInstance()->SetShootAnalysisResultInfo(resultInfo);

	// ========== 反应度统计 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	shootReactionDegreeResult.propellantsAvgReactionDegree = computeAvg(resultInfo.propellantReactionDegreeNodeValues);
	shootReactionDegreeResult.propellantsStandardReactionDegree = computeStd(resultInfo.propellantReactionDegreeNodeValues);

	ModelDataManager::GetInstance()->SetShootReactionDegreeResult(shootReactionDegreeResult);

	return true;
}


bool APISetNodeValue::CalculateAllJetImpactStressNodeValues()
{
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
	if (!resultInfo.isChecked)
		return false;

	resultInfo.shellStressNodeValues.clear();
	resultInfo.nozzleStressNodeValues.clear();
	resultInfo.propellantStressNodeValues.clear();
	resultInfo.heatInsulatingStressNodeValues.clear();

	auto stressResult = ModelDataManager::GetInstance()->GetJetImpactStressResult();
	double metalMax = stressResult.metalsMaxStress;
	double metalMin = stressResult.metalsMinStress;
	double propMax = stressResult.propellantsMaxStress;
	double propMin = stressResult.propellantsMinStress;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 3.0;

	std::vector<std::pair<double, double>> ellipses = {
		{12, 31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.9, semi_major_z_1 * 0.9}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second))
			return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second))
			return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second))
			return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second))
			return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second))
			return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second))
			return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second))
			return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second))
			return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second))
			return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second))
			return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else
			return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second))
			return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second))
			return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second))
			return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second))
			return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second))
			return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second))
			return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second))
			return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second))
			return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second))
			return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second))
			return propMin + (propMax - propMin) * 0.5 / 9.0;
		else
			return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger shellNodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) shellCoords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(shellNodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = shellCoords->Value(id, 1);
			double y = shellCoords->Value(id, 2);
			resultInfo.shellStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nozzleNodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) nozzleCoords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nozzleNodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = nozzleCoords->Value(id, 1);
			double y = nozzleCoords->Value(id, 2);
			resultInfo.nozzleStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger propNodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) propCoords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(propNodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = propCoords->Value(id, 1);
			double y = propCoords->Value(id, 2);
			resultInfo.propellantStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger hiNodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) hiCoords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(hiNodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = hiCoords->Value(id, 1);
			double y = hiCoords->Value(id, 2);
			resultInfo.heatInsulatingStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetJetImpactAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	stressResult.metalsAvgStress = computeAvg(resultInfo.shellStressNodeValues);
	stressResult.outheatAvgStress = stressResult.metalsAvgStress;
	stressResult.metalsStandardStress = computeStd(resultInfo.shellStressNodeValues);
	stressResult.outheatStandardStress = stressResult.metalsStandardStress;

	stressResult.propellantsAvgStress = computeAvg(resultInfo.propellantStressNodeValues);
	stressResult.propellantsStandardStress = computeStd(resultInfo.propellantStressNodeValues);

	double shellMin = stressResult.metalsMinStress;
	double shellMax = stressResult.metalsMaxStress;
	if (shellMax > shellMin) {
		stressResult.insulatingheatAvgStress =
			shellMin + (shellMax - shellMin) * (stressResult.metalsAvgStress - shellMin) / (shellMax - shellMin);
	}
	else {
		stressResult.insulatingheatAvgStress = shellMin;
	}
	stressResult.insulatingheatStandardStress = computeStd(resultInfo.heatInsulatingStressNodeValues);

	ModelDataManager::GetInstance()->SetJetImpactStressResult(stressResult);

	return true;
}


bool APISetNodeValue::CalculateAllJetImpactStrainNodeValues()
{
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
	if (!resultInfo.isChecked)
		return false;

	resultInfo.shellStrainNodeValues.clear();
	resultInfo.nozzleStrainNodeValues.clear();
	resultInfo.propellantStrainNodeValues.clear();
	resultInfo.heatInsulatingStrainNodeValues.clear();

	auto strainResult = ModelDataManager::GetInstance()->GetJetImpactStrainResult();
	double metalMax = strainResult.metalsMaxStrain;
	double metalMin = strainResult.metalsMinStrain;
	double propMax = strainResult.propellantsMaxStrain;
	double propMin = strainResult.propellantsMinStrain;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 3.0;

	std::vector<std::pair<double, double>> ellipses = {
		{12, 31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.9, semi_major_z_1 * 0.9}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetJetImpactAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	strainResult.metalsAvgStrain = computeAvg(resultInfo.shellStrainNodeValues);
	strainResult.outheatAvgStrain = strainResult.metalsAvgStrain;
	strainResult.metalsStandardStrain = computeStd(resultInfo.shellStrainNodeValues);
	strainResult.outheatStandardStrain = strainResult.metalsStandardStrain;

	strainResult.mpropellantsAvgStrain = computeAvg(resultInfo.propellantStrainNodeValues);
	strainResult.propellantsStandardStrain = computeStd(resultInfo.propellantStrainNodeValues);

	double shellMin = strainResult.metalsMinStrain;
	double shellMax = strainResult.metalsMaxStrain;
	if (shellMax > shellMin) {
		strainResult.insulatingheatAvgStrain =
			shellMin + (shellMax - shellMin) * (strainResult.metalsAvgStrain - shellMin) / (shellMax - shellMin);
	}
	else {
		strainResult.insulatingheatAvgStrain = shellMin;
	}
	strainResult.insulatingheatStandardStrain = computeStd(resultInfo.heatInsulatingStrainNodeValues);

	ModelDataManager::GetInstance()->SetJetImpactStrainResult(strainResult);

	return true;
}


bool APISetNodeValue::CalculateAllJetImpactTempNodeValues()
{
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
	if (!resultInfo.isChecked)
		return false;

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto tempResult = ModelDataManager::GetInstance()->GetJetImpactTemperatureResult();
	double metalMax = tempResult.metalsMaxTemperature;
	double metalMin = tempResult.metalsMinTemperature;
	double propMax = tempResult.propellantsMaxTemperature;
	double propMin = tempResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 2.0;

	std::vector<std::pair<double, double>> ellipses = {
		{12, 31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.35},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.65, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.75},
		{semi_minor_x_1 * 0.9, semi_major_z_1 * 0.8}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetJetImpactAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	tempResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatAvgTemperature = tempResult.metalsAvgTemperature;
	tempResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatStandardTemperature = tempResult.metalsStandardTemperature;

	tempResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	tempResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = tempResult.metalsMinTemperature;
	double shellMax = tempResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		tempResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (tempResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		tempResult.insulatingheatAvgTemperature = shellMin;
	}
	tempResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);

	ModelDataManager::GetInstance()->SetJetImpactTemperatureResult(tempResult);

	return true;
}

bool APISetNodeValue::CalculateAllJetImpactPressureNodeValues()
{
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
	if (!resultInfo.isChecked)
		return false;

	resultInfo.shellOverpressureNodeValues.clear();
	resultInfo.nozzleOverpressureNodeValues.clear();
	resultInfo.propellantOverpressureNodeValues.clear();
	resultInfo.heatInsulatingOverpressureNodeValues.clear();

	auto pressResult = ModelDataManager::GetInstance()->GetJetImpactOverpressureResult();
	double metalMax = pressResult.metalsMaxOverpressure;
	double metalMin = pressResult.metalsMinOverpressure;
	double propMax = pressResult.propellantsMaxOverpressure;
	double propMin = pressResult.propellantsMinOverpressure;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 1.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12, 31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.25},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.95, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 1.0, semi_major_z_1 * 0.85}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetJetImpactAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	pressResult.metalsAvgOverpressure = computeAvg(resultInfo.shellOverpressureNodeValues);
	pressResult.outheatAvgOverpressure = pressResult.metalsAvgOverpressure;
	pressResult.metalsStandardOverpressure = computeStd(resultInfo.shellOverpressureNodeValues);
	pressResult.outheatStandardOverpressure = pressResult.metalsStandardOverpressure;

	pressResult.mpropellantsAvgOverpressure = computeAvg(resultInfo.propellantOverpressureNodeValues);
	pressResult.propellantsStandardOverpressure = computeStd(resultInfo.propellantOverpressureNodeValues);

	double shellMin = pressResult.metalsMinOverpressure;
	double shellMax = pressResult.metalsMaxOverpressure;
	if (shellMax > shellMin) {
		pressResult.insulatingheatAvgOverpressure =
			shellMin + (shellMax - shellMin) * (pressResult.metalsAvgOverpressure - shellMin) / (shellMax - shellMin);
	}
	else {
		pressResult.insulatingheatAvgOverpressure = shellMin;
	}
	pressResult.insulatingheatStandardOverpressure = computeStd(resultInfo.heatInsulatingOverpressureNodeValues);

	ModelDataManager::GetInstance()->SetJetImpactOverpressureResult(pressResult);

	return true;
}


bool APISetNodeValue::CalculateAllJetImpactReactionDegreeNodeValues()
{
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();
	if (!resultInfo.isChecked)
		return false;

	resultInfo.propellantReactionDegreeNodeValues.clear();

	auto reactResult = ModelDataManager::GetInstance()->GetJetImpactReactionDegreeResult();
	double maxVal = reactResult.propellantsMaxReactionDegree;
	double minVal = reactResult.propellantsMinReactionDegree;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = y_max - y_min;
	const double semi_minor_x_1 = semi_major_z_1 / 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12, 31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.25},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.95, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 1.0, semi_major_z_1 * 1.0}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return maxVal;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return minVal + (maxVal - minVal) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return minVal + (maxVal - minVal) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return minVal + (maxVal - minVal) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return minVal + (maxVal - minVal) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return minVal + (maxVal - minVal) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return minVal + (maxVal - minVal) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return minVal + (maxVal - minVal) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return minVal + (maxVal - minVal) * 0.5 / 9.0;
		else return minVal;
	};

	if (!modelMeshInfo.propellantMesh.IsNull())
	{
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next())
		{
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantReactionDegreeNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetJetImpactAnalysisResultInfo(resultInfo);

	// ========== 计算推进剂平均值 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	reactResult.propellantsAvgReactionDegree = computeAvg(resultInfo.propellantReactionDegreeNodeValues);
	reactResult.propellantsStandardReactionDegree = computeStd(resultInfo.propellantReactionDegreeNodeValues);

	ModelDataManager::GetInstance()->SetJetImpactReactionDegreeResult(reactResult);

	return true;
}


bool APISetNodeValue::CalculateAllFragmentationStressNodeValues()
{
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellStressNodeValues.clear();
	resultInfo.nozzleStressNodeValues.clear();
	resultInfo.propellantStressNodeValues.clear();
	resultInfo.heatInsulatingStressNodeValues.clear();

	auto stressResult = ModelDataManager::GetInstance()->GetFragmentationImpactStressResult();
	double metalMax = stressResult.metalsMaxStress;
	double metalMin = stressResult.metalsMinStress;
	double propMax = stressResult.propellantsMaxStress;
	double propMin = stressResult.propellantsMinStress;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = (y_max - y_min) * 0.4;
	const double semi_minor_x_1 = semi_major_z_1 * 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12,31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.9},
		{semi_minor_x_1 * 0.9, semi_major_z_1 * 1.0}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	stressResult.metalsAvgStress = computeAvg(resultInfo.shellStressNodeValues);
	stressResult.outheatAvgStress = stressResult.metalsAvgStress;
	stressResult.metalsStandardStress = computeStd(resultInfo.shellStressNodeValues);
	stressResult.outheatStandardStress = stressResult.metalsStandardStress;

	stressResult.propellantsAvgStress = computeAvg(resultInfo.propellantStressNodeValues);
	stressResult.propellantsStandardStress = computeStd(resultInfo.propellantStressNodeValues);

	double shellMin = stressResult.metalsMinStress;
	double shellMax = stressResult.metalsMaxStress;
	if (shellMax > shellMin) {
		stressResult.insulatingheatAvgStress =
			shellMin + (shellMax - shellMin) * (stressResult.metalsAvgStress - shellMin) / (shellMax - shellMin);
	}
	else {
		stressResult.insulatingheatAvgStress = shellMin;
	}
	stressResult.insulatingheatStandardStress = computeStd(resultInfo.heatInsulatingStressNodeValues);

	ModelDataManager::GetInstance()->SetFragmentationImpactStressResult(stressResult);

	return true;
}


bool APISetNodeValue::CalculateAllFragmentationStrainNodeValues()
{
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellStrainNodeValues.clear();
	resultInfo.nozzleStrainNodeValues.clear();
	resultInfo.propellantStrainNodeValues.clear();
	resultInfo.heatInsulatingStrainNodeValues.clear();

	auto strainResult = ModelDataManager::GetInstance()->GetFragmentationImpactStrainResult();
	double metalMax = strainResult.metalsMaxStrain;
	double metalMin = strainResult.metalsMinStrain;
	double propMax = strainResult.propellantsMaxStrain;
	double propMin = strainResult.propellantsMinStrain;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = (y_max - y_min) * 0.4;
	const double semi_minor_x_1 = semi_major_z_1 * 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12,31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.3},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.7},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.9},
		{semi_minor_x_1 * 0.9, semi_major_z_1 * 1.0}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	strainResult.metalsAvgStrain = computeAvg(resultInfo.shellStrainNodeValues);
	strainResult.outheatAvgStrain = strainResult.metalsAvgStrain;
	strainResult.metalsStandardStrain = computeStd(resultInfo.shellStrainNodeValues);
	strainResult.outheatStandardStrain = strainResult.metalsStandardStrain;

	strainResult.mpropellantsAvgStrain = computeAvg(resultInfo.propellantStrainNodeValues);
	strainResult.propellantsStandardStrain = computeStd(resultInfo.propellantStrainNodeValues);

	double shellMin = strainResult.metalsMinStrain;
	double shellMax = strainResult.metalsMaxStrain;
	if (shellMax > shellMin) {
		strainResult.insulatingheatAvgStrain =
			shellMin + (shellMax - shellMin) * (strainResult.metalsAvgStrain - shellMin) / (shellMax - shellMin);
	}
	else {
		strainResult.insulatingheatAvgStrain = shellMin;
	}
	strainResult.insulatingheatStandardStrain = computeStd(resultInfo.heatInsulatingStrainNodeValues);

	ModelDataManager::GetInstance()->SetFragmentationImpactStrainResult(strainResult);

	return true;
}


bool APISetNodeValue::CalculateAllFragmentationTemperatureNodeValues()
{
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto tempResult = ModelDataManager::GetInstance()->GetFragmentationImpactTemperatureResult();
	double metalMax = tempResult.metalsMaxTemperature;
	double metalMin = tempResult.metalsMinTemperature;
	double propMax = tempResult.propellantsMaxTemperature;
	double propMin = tempResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = (y_max - y_min) * 0.5;
	const double semi_minor_x_1 = semi_major_z_1 * 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12,31},
		{semi_minor_x_1 * 0.15, semi_major_z_1 * 0.35},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
		{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.65, semi_major_z_1 * 0.70},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.75},
		{semi_minor_x_1 * 0.75, semi_major_z_1 * 0.80},
		{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.85}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	tempResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatAvgTemperature = tempResult.metalsAvgTemperature;
	tempResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatStandardTemperature = tempResult.metalsStandardTemperature;

	tempResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	tempResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = tempResult.metalsMinTemperature;
	double shellMax = tempResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		tempResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (tempResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		tempResult.insulatingheatAvgTemperature = shellMin;
	}
	tempResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);

	ModelDataManager::GetInstance()->SetFragmentationImpactTemperatureResult(tempResult);

	return true;
}


bool APISetNodeValue::CalculateAllFragmentationOverpressureNodeValues()
{
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellOverpressureNodeValues.clear();
	resultInfo.nozzleOverpressureNodeValues.clear();
	resultInfo.propellantOverpressureNodeValues.clear();
	resultInfo.heatInsulatingOverpressureNodeValues.clear();

	auto overpressureResult = ModelDataManager::GetInstance()->GetFragmentationImpactOverpressureResult();
	double metalMax = overpressureResult.metalsMaxOverpressure;
	double metalMin = overpressureResult.metalsMinOverpressure;
	double propMax = overpressureResult.propellantsMaxOverpressure;
	double propMin = overpressureResult.propellantsMinOverpressure;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = (y_max - y_min) * 0.55;
	const double semi_minor_x_1 = semi_major_z_1 * 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12,31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.35},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.70},
		{semi_minor_x_1 * 0.65, semi_major_z_1 * 0.75},
		{semi_minor_x_1 * 0.75, semi_major_z_1 * 0.80},
		{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.85}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return metalMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return metalMin + (metalMax - metalMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return metalMin + (metalMax - metalMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return metalMin + (metalMax - metalMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return metalMin + (metalMax - metalMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return metalMin + (metalMax - metalMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return metalMin + (metalMax - metalMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return metalMin + (metalMax - metalMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return metalMin + (metalMax - metalMin) * 0.5 / 9.0;
		else return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) return -1.0;
		else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) return propMax;
		else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) return propMin + (propMax - propMin) * 7.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) return propMin + (propMax - propMin) * 6.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) return propMin + (propMax - propMin) * 5.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) return propMin + (propMax - propMin) * 4.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) return propMin + (propMax - propMin) * 3.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) return propMin + (propMax - propMin) * 2.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) return propMin + (propMax - propMin) * 1.5 / 9.0;
		else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) return propMin + (propMax - propMin) * 0.5 / 9.0;
		else return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	overpressureResult.metalsAvgOverpressure = computeAvg(resultInfo.shellOverpressureNodeValues);
	overpressureResult.outheatAvgOverpressure = overpressureResult.metalsAvgOverpressure;
	overpressureResult.metalsStandardOverpressure = computeStd(resultInfo.shellOverpressureNodeValues);
	overpressureResult.outheatStandardOverpressure = overpressureResult.metalsStandardOverpressure;

	overpressureResult.mpropellantsAvgOverpressure = computeAvg(resultInfo.propellantOverpressureNodeValues);
	overpressureResult.propellantsStandardOverpressure = computeStd(resultInfo.propellantOverpressureNodeValues);

	double shellMin = overpressureResult.metalsMinOverpressure;
	double shellMax = overpressureResult.metalsMaxOverpressure;
	if (shellMax > shellMin) {
		overpressureResult.insulatingheatAvgOverpressure =
			shellMin + (shellMax - shellMin) * (overpressureResult.metalsAvgOverpressure - shellMin) / (shellMax - shellMin);
	}
	else {
		overpressureResult.insulatingheatAvgOverpressure = shellMin;
	}
	overpressureResult.insulatingheatStandardOverpressure = computeStd(resultInfo.heatInsulatingOverpressureNodeValues);

	ModelDataManager::GetInstance()->SetFragmentationImpactOverpressureResult(overpressureResult);

	return true;
}


bool APISetNodeValue::CalculateAllFragmentationReactionDegreeNodeValues()
{
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.propellantReactionDegreeNodeValues.clear();

	auto reactResult = ModelDataManager::GetInstance()->GetFragmentationImpactReactionDegreeResult();
	double maxVal = reactResult.propellantsMaxReactionDegree;
	double minVal = reactResult.propellantsMinReactionDegree;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double h = (x_min + x_max) / 2.0;
	const double k = y_min;
	const double semi_major_z_1 = (y_max - y_min) * 0.55;
	const double semi_minor_x_1 = semi_major_z_1 * 2.5;

	std::vector<std::pair<double, double>> ellipses = {
		{12,31},
		{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.4},
		{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.45},
		{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.5},
		{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.6},
		{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.8},
		{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.85},
		{semi_minor_x_1 * 0.65, semi_major_z_1 * 0.9},
		{semi_minor_x_1 * 0.7, semi_major_z_1 * 1.0}
	};

	auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
		double dx = x - cx;
		double dz = z - cz;
		if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
		double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);

			double value;
			if (isInEllipse(x, y, h, k, ellipses[0].first, ellipses[0].second)) value = -1.0;
			else if (isInEllipse(x, y, h, k, ellipses[1].first, ellipses[1].second)) value = maxVal;
			else if (isInEllipse(x, y, h, k, ellipses[2].first, ellipses[2].second)) value = minVal + (maxVal - minVal) * 7.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[3].first, ellipses[3].second)) value = minVal + (maxVal - minVal) * 6.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[4].first, ellipses[4].second)) value = minVal + (maxVal - minVal) * 5.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[5].first, ellipses[5].second)) value = minVal + (maxVal - minVal) * 4.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[6].first, ellipses[6].second)) value = minVal + (maxVal - minVal) * 3.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[7].first, ellipses[7].second)) value = minVal + (maxVal - minVal) * 2.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[8].first, ellipses[8].second)) value = minVal + (maxVal - minVal) * 1.5 / 9.0;
			else if (isInEllipse(x, y, h, k, ellipses[9].first, ellipses[9].second)) value = minVal + (maxVal - minVal) * 0.5 / 9.0;
			else value = minVal;

			resultInfo.propellantReactionDegreeNodeValues.push_back(value);
		}
	}

	ModelDataManager::GetInstance()->SetFragmentationAnalysisResultInfo(resultInfo);

	// ========== 计算推进剂平均值 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	reactResult.propellantsAvgReactionDegree = computeAvg(resultInfo.propellantReactionDegreeNodeValues);
	reactResult.propellantsStandardReactionDegree = computeStd(resultInfo.propellantReactionDegreeNodeValues);

	ModelDataManager::GetInstance()->SetFragmentationImpactReactionDegreeResult(reactResult);

	return true;
}


bool APISetNodeValue::CalculateAllExplosiveBlastStressNodeValues()
{
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellStressNodeValues.clear();
	resultInfo.nozzleStressNodeValues.clear();
	resultInfo.propellantStressNodeValues.clear();
	resultInfo.heatInsulatingStressNodeValues.clear();

	auto stressResult = ModelDataManager::GetInstance()->GetExplosiveBlastStressResult();
	double metalMax = stressResult.metalsMaxStress;
	double metalMin = stressResult.metalsMinStress;
	double propMax = stressResult.propellantsMaxStress;
	double propMin = stressResult.propellantsMinStress;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetExplosiveBlastAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	stressResult.metalsAvgStress = computeAvg(resultInfo.shellStressNodeValues);
	stressResult.outheatAvgStress = stressResult.metalsAvgStress;
	stressResult.metalsStandardStress = computeStd(resultInfo.shellStressNodeValues);
	stressResult.outheatStandardStress = stressResult.metalsStandardStress;

	stressResult.propellantsAvgStress = computeAvg(resultInfo.propellantStressNodeValues);
	stressResult.propellantsStandardStress = computeStd(resultInfo.propellantStressNodeValues);

	double shellMin = stressResult.metalsMinStress;
	double shellMax = stressResult.metalsMaxStress;
	if (shellMax > shellMin) {
		stressResult.insulatingheatAvgStress =
			shellMin + (shellMax - shellMin) * (stressResult.metalsAvgStress - shellMin) / (shellMax - shellMin);
	}
	else {
		stressResult.insulatingheatAvgStress = shellMin;
	}
	stressResult.insulatingheatStandardStress = computeStd(resultInfo.heatInsulatingStressNodeValues);

	ModelDataManager::GetInstance()->SetExplosiveBlastStressResult(stressResult);

	return true;
}


bool APISetNodeValue::CalculateAllExplosiveBlastStrainNodeValues()
{
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellStrainNodeValues.clear();
	resultInfo.nozzleStrainNodeValues.clear();
	resultInfo.propellantStrainNodeValues.clear();
	resultInfo.heatInsulatingStrainNodeValues.clear();

	auto strainResult = ModelDataManager::GetInstance()->GetExplosiveBlastStrainResult();
	double metalMax = strainResult.metalsMaxStrain;
	double metalMin = strainResult.metalsMinStrain;
	double propMax = strainResult.propellantsMaxStrain;
	double propMin = strainResult.propellantsMinStrain;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetExplosiveBlastAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	strainResult.metalsAvgStrain = computeAvg(resultInfo.shellStrainNodeValues);
	strainResult.outheatAvgStrain = strainResult.metalsAvgStrain;
	strainResult.metalsStandardStrain = computeStd(resultInfo.shellStrainNodeValues);
	strainResult.outheatStandardStrain = strainResult.metalsStandardStrain;

	strainResult.mpropellantsAvgStrain = computeAvg(resultInfo.propellantStrainNodeValues);
	strainResult.propellantsStandardStrain = computeStd(resultInfo.propellantStrainNodeValues);

	double shellMin = strainResult.metalsMinStrain;
	double shellMax = strainResult.metalsMaxStrain;
	if (shellMax > shellMin) {
		strainResult.insulatingheatAvgStrain =
			shellMin + (shellMax - shellMin) * (strainResult.metalsAvgStrain - shellMin) / (shellMax - shellMin);
	}
	else {
		strainResult.insulatingheatAvgStrain = shellMin;
	}
	strainResult.insulatingheatStandardStrain = computeStd(resultInfo.heatInsulatingStrainNodeValues);

	ModelDataManager::GetInstance()->SetExplosiveBlastStrainResult(strainResult);

	return true;
}


bool APISetNodeValue::CalculateAllExplosiveBlastTemperatureNodeValues()
{
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto tempResult = ModelDataManager::GetInstance()->GetExplosiveBlastTemperatureResult();
	double metalMax = tempResult.metalsMaxTemperature;
	double metalMin = tempResult.metalsMinTemperature;
	double propMax = tempResult.propellantsMaxTemperature;
	double propMin = tempResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.2, 0.95, 0.8, 0.6, 0.45, 0.25, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetExplosiveBlastAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	tempResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatAvgTemperature = tempResult.metalsAvgTemperature;
	tempResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatStandardTemperature = tempResult.metalsStandardTemperature;

	tempResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	tempResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = tempResult.metalsMinTemperature;
	double shellMax = tempResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		tempResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (tempResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		tempResult.insulatingheatAvgTemperature = shellMin;
	}
	tempResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);

	ModelDataManager::GetInstance()->SetExplosiveBlastTemperatureResult(tempResult);

	return true;
}


bool APISetNodeValue::CalculateAllExplosiveBlastOverpressureNodeValues()
{
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellOverpressureNodeValues.clear();
	resultInfo.nozzleOverpressureNodeValues.clear();
	resultInfo.propellantOverpressureNodeValues.clear();
	resultInfo.heatInsulatingOverpressureNodeValues.clear();

	auto overpressureResult = ModelDataManager::GetInstance()->GetExplosiveBlastOverpressureResult();
	double metalMax = overpressureResult.metalsMaxOverpressure;
	double metalMin = overpressureResult.metalsMinOverpressure;
	double propMax = overpressureResult.propellantsMaxOverpressure;
	double propMin = overpressureResult.propellantsMinOverpressure;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.5, 1.2, 1.0, 0.7, 0.5, 0.3, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetExplosiveBlastAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	overpressureResult.metalsAvgOverpressure = computeAvg(resultInfo.shellOverpressureNodeValues);
	overpressureResult.outheatAvgOverpressure = overpressureResult.metalsAvgOverpressure;
	overpressureResult.metalsStandardOverpressure = computeStd(resultInfo.shellOverpressureNodeValues);
	overpressureResult.outheatStandardOverpressure = overpressureResult.metalsStandardOverpressure;

	overpressureResult.mpropellantsAvgOverpressure = computeAvg(resultInfo.propellantOverpressureNodeValues);
	overpressureResult.propellantsStandardOverpressure = computeStd(resultInfo.propellantOverpressureNodeValues);

	double shellMin = overpressureResult.metalsMinOverpressure;
	double shellMax = overpressureResult.metalsMaxOverpressure;
	if (shellMax > shellMin) {
		overpressureResult.insulatingheatAvgOverpressure =
			shellMin + (shellMax - shellMin) * (overpressureResult.metalsAvgOverpressure - shellMin) / (shellMax - shellMin);
	}
	else {
		overpressureResult.insulatingheatAvgOverpressure = shellMin;
	}
	overpressureResult.insulatingheatStandardOverpressure = computeStd(resultInfo.heatInsulatingOverpressureNodeValues);

	ModelDataManager::GetInstance()->SetExplosiveBlastOverpressureResult(overpressureResult);

	return true;
}


bool APISetNodeValue::CalculateAllExplosiveBlastReactionDegreeNodeValues()
{
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.propellantReactionDegreeNodeValues.clear();

	auto reactResult = ModelDataManager::GetInstance()->GetExplosiveBlastReactionDegreeResult();
	double maxVal = reactResult.propellantsMaxReactionDegree;
	double minVal = reactResult.propellantsMinReactionDegree;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.4, 1.2, 1.0, 0.7, 0.5, 0.4, 0.2 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);

			double value = minVal;
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
				double b = b_full * b_scales[i];
				if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
					double ratio;
					switch (i) {
					case 6: ratio = 8.5 / 9.0; break;
					case 5: ratio = 7.5 / 9.0; break;
					case 4: ratio = 6.5 / 9.0; break;
					case 3: ratio = 5.5 / 9.0; break;
					case 2: ratio = 4.5 / 9.0; break;
					case 1: ratio = 3.5 / 9.0; break;
					case 0: ratio = 2.5 / 9.0; break;
					default: ratio = 0.0;
					}
					value = minVal + (maxVal - minVal) * ratio;
					break;
				}
			}
			resultInfo.propellantReactionDegreeNodeValues.push_back(value);
		}
	}

	ModelDataManager::GetInstance()->SetExplosiveBlastAnalysisResultInfo(resultInfo);

	// ========== 计算推进剂平均值 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	reactResult.propellantsAvgReactionDegree = computeAvg(resultInfo.propellantReactionDegreeNodeValues);
	reactResult.propellantsStandardReactionDegree = computeStd(resultInfo.propellantReactionDegreeNodeValues);

	ModelDataManager::GetInstance()->SetExplosiveBlastReactionDegreeResult(reactResult);

	return true;
}


bool APISetNodeValue::CalculateAllSacrificeExplosionStressNodeValues()
{
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellStressNodeValues.clear();
	resultInfo.nozzleStressNodeValues.clear();
	resultInfo.propellantStressNodeValues.clear();
	resultInfo.heatInsulatingStressNodeValues.clear();

	auto stressResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStressResult();
	double metalMax = stressResult.metalsMaxStress;
	double metalMin = stressResult.metalsMinStress;
	double propMax = stressResult.propellantsMaxStress;
	double propMin = stressResult.propellantsMinStress;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0 * 2.0 / 3.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStressNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStressNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	stressResult.metalsAvgStress = computeAvg(resultInfo.shellStressNodeValues);
	stressResult.outheatAvgStress = stressResult.metalsAvgStress;
	stressResult.metalsStandardStress = computeStd(resultInfo.shellStressNodeValues);
	stressResult.outheatStandardStress = stressResult.metalsStandardStress;

	stressResult.propellantsAvgStress = computeAvg(resultInfo.propellantStressNodeValues);
	stressResult.propellantsStandardStress = computeStd(resultInfo.propellantStressNodeValues);

	double shellMin = stressResult.metalsMinStress;
	double shellMax = stressResult.metalsMaxStress;
	if (shellMax > shellMin) {
		stressResult.insulatingheatAvgStress =
			shellMin + (shellMax - shellMin) * (stressResult.metalsAvgStress - shellMin) / (shellMax - shellMin);
	}
	else {
		stressResult.insulatingheatAvgStress = shellMin;
	}
	stressResult.insulatingheatStandardStress = computeStd(resultInfo.heatInsulatingStressNodeValues);

	ModelDataManager::GetInstance()->SetSacrificeExplosionStressResult(stressResult);

	return true;
}


bool APISetNodeValue::CalculateAllSacrificeExplosionStrainNodeValues()
{
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellStrainNodeValues.clear();
	resultInfo.nozzleStrainNodeValues.clear();
	resultInfo.propellantStrainNodeValues.clear();
	resultInfo.heatInsulatingStrainNodeValues.clear();

	auto strainResult = ModelDataManager::GetInstance()->GetSacrificeExplosionStrainResult();
	double metalMax = strainResult.metalsMaxStrain;
	double metalMin = strainResult.metalsMinStrain;
	double propMax = strainResult.propellantsMaxStrain;
	double propMin = strainResult.propellantsMinStrain;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0 * 2.0 / 3.0;
	const double b_full = (y_max - y_min) / 2.0;
	std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleStrainNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingStrainNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	strainResult.metalsAvgStrain = computeAvg(resultInfo.shellStrainNodeValues);
	strainResult.outheatAvgStrain = strainResult.metalsAvgStrain;
	strainResult.metalsStandardStrain = computeStd(resultInfo.shellStrainNodeValues);
	strainResult.outheatStandardStrain = strainResult.metalsStandardStrain;

	strainResult.mpropellantsAvgStrain = computeAvg(resultInfo.propellantStrainNodeValues);
	strainResult.propellantsStandardStrain = computeStd(resultInfo.propellantStrainNodeValues);

	double shellMin = strainResult.metalsMinStrain;
	double shellMax = strainResult.metalsMaxStrain;
	if (shellMax > shellMin) {
		strainResult.insulatingheatAvgStrain =
			shellMin + (shellMax - shellMin) * (strainResult.metalsAvgStrain - shellMin) / (shellMax - shellMin);
	}
	else {
		strainResult.insulatingheatAvgStrain = shellMin;
	}
	strainResult.insulatingheatStandardStrain = computeStd(resultInfo.heatInsulatingStrainNodeValues);

	ModelDataManager::GetInstance()->SetSacrificeExplosionStrainResult(strainResult);

	return true;
}


bool APISetNodeValue::CalculateAllSacrificeExplosionTemperatureNodeValues()
{
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellTemperatureNodeValues.clear();
	resultInfo.nozzleTemperatureNodeValues.clear();
	resultInfo.propellantTemperatureNodeValues.clear();
	resultInfo.heatInsulatingTemperatureNodeValues.clear();

	auto tempResult = ModelDataManager::GetInstance()->GetSacrificeExplosionTemperatureResult();
	double metalMax = tempResult.metalsMaxTemperature;
	double metalMin = tempResult.metalsMinTemperature;
	double propMax = tempResult.propellantsMaxTemperature;
	double propMin = tempResult.propellantsMinTemperature;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0 * 2.0 / 2.7;
	const double b_full = (y_max - y_min) / 1.7;
	std::vector<double> b_scales = { 1.0, 0.9, 0.8, 0.7, 0.5, 0.3, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleTemperatureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingTemperatureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	tempResult.metalsAvgTemperature = computeAvg(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatAvgTemperature = tempResult.metalsAvgTemperature;
	tempResult.metalsStandardTemperature = computeStd(resultInfo.shellTemperatureNodeValues);
	tempResult.outheatStandardTemperature = tempResult.metalsStandardTemperature;

	tempResult.mpropellantsAvgTemperature = computeAvg(resultInfo.propellantTemperatureNodeValues);
	tempResult.propellantsStandardTemperature = computeStd(resultInfo.propellantTemperatureNodeValues);

	double shellMin = tempResult.metalsMinTemperature;
	double shellMax = tempResult.metalsMaxTemperature;
	if (shellMax > shellMin) {
		tempResult.insulatingheatAvgTemperature =
			shellMin + (shellMax - shellMin) * (tempResult.metalsAvgTemperature - shellMin) / (shellMax - shellMin);
	}
	else {
		tempResult.insulatingheatAvgTemperature = shellMin;
	}
	tempResult.insulatingheatStandardTemperature = computeStd(resultInfo.heatInsulatingTemperatureNodeValues);

	ModelDataManager::GetInstance()->SetSacrificeExplosionTemperatureResult(tempResult);

	return true;
}


bool APISetNodeValue::CalculateAllSacrificeExplosionOverpressureNodeValues()
{
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.shellOverpressureNodeValues.clear();
	resultInfo.nozzleOverpressureNodeValues.clear();
	resultInfo.propellantOverpressureNodeValues.clear();
	resultInfo.heatInsulatingOverpressureNodeValues.clear();

	auto overpressureResult = ModelDataManager::GetInstance()->GetSacrificeExplosionOverpressureResult();
	double metalMax = overpressureResult.metalsMaxOverpressure;
	double metalMin = overpressureResult.metalsMinOverpressure;
	double propMax = overpressureResult.propellantsMaxOverpressure;
	double propMin = overpressureResult.propellantsMinOverpressure;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0 * 2.0 / 2.5;
	const double b_full = (y_max - y_min) / 1.5;
	std::vector<double> b_scales = { 1.0, 0.85, 0.8, 0.6, 0.4, 0.2, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	auto calcMetalValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return metalMin + (metalMax - metalMin) * ratio;
			}
		}
		return metalMin;
	};

	auto calcPropellantValue = [&](double x, double y) -> double {
		for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
			double b = b_full * b_scales[i];
			if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
				double ratio;
				switch (i) {
				case 6: ratio = 8.5 / 9.0; break;
				case 5: ratio = 7.5 / 9.0; break;
				case 4: ratio = 6.5 / 9.0; break;
				case 3: ratio = 5.5 / 9.0; break;
				case 2: ratio = 4.5 / 9.0; break;
				case 1: ratio = 3.5 / 9.0; break;
				case 0: ratio = 2.5 / 9.0; break;
				default: ratio = 0.0;
				}
				return propMin + (propMax - propMin) * ratio;
			}
		}
		return propMin;
	};

	if (!modelMeshInfo.shellMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.shellMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.shellMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.shellOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.nozzleMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.nozzleMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.nozzleMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.nozzleOverpressureNodeValues.push_back(calcMetalValue(x, y));
		}
	}

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.propellantOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	if (!modelMeshInfo.heatInsulatingLayerMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.heatInsulatingLayerMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.heatInsulatingLayerMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);
			resultInfo.heatInsulatingOverpressureNodeValues.push_back(calcPropellantValue(x, y));
		}
	}

	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(resultInfo);

	// ========== 计算统计量 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				sum += v;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	overpressureResult.metalsAvgOverpressure = computeAvg(resultInfo.shellOverpressureNodeValues);
	overpressureResult.outheatAvgOverpressure = overpressureResult.metalsAvgOverpressure;
	overpressureResult.metalsStandardOverpressure = computeStd(resultInfo.shellOverpressureNodeValues);
	overpressureResult.outheatStandardOverpressure = overpressureResult.metalsStandardOverpressure;

	overpressureResult.mpropellantsAvgOverpressure = computeAvg(resultInfo.propellantOverpressureNodeValues);
	overpressureResult.propellantsStandardOverpressure = computeStd(resultInfo.propellantOverpressureNodeValues);

	double shellMin = overpressureResult.metalsMinOverpressure;
	double shellMax = overpressureResult.metalsMaxOverpressure;
	if (shellMax > shellMin) {
		overpressureResult.insulatingheatAvgOverpressure =
			shellMin + (shellMax - shellMin) * (overpressureResult.metalsAvgOverpressure - shellMin) / (shellMax - shellMin);
	}
	else {
		overpressureResult.insulatingheatAvgOverpressure = shellMin;
	}
	overpressureResult.insulatingheatStandardOverpressure = computeStd(resultInfo.heatInsulatingOverpressureNodeValues);

	ModelDataManager::GetInstance()->SetSacrificeExplosionOverpressureResult(overpressureResult);

	return true;
}


bool APISetNodeValue::CalculateAllSacrificeExplosionReactionDegreeNodeValues()
{
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto resultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();
	if (!resultInfo.isChecked) return false;

	resultInfo.propellantReactionDegreeNodeValues.clear();

	auto reactResult = ModelDataManager::GetInstance()->GetSacrificeExplosionReactionDegreeResult();
	double maxVal = reactResult.propellantsMaxReactionDegree;
	double minVal = reactResult.propellantsMinReactionDegree;

	const double x_min = modelMeshInfo.propellant_x_min;
	const double x_max = modelMeshInfo.propellant_x_max;
	const double y_min = modelMeshInfo.propellant_y_min;
	const double y_max = modelMeshInfo.propellant_y_max;

	const double cx = (x_min + x_max) / 2.0;
	const double cz = y_min;
	const double a = (x_max - x_min) / 2.0 * 2.0 / 2.5;
	const double b_full = (y_max - y_min) / 1.5;
	std::vector<double> b_scales = { 1.2, 0.95, 0.85, 0.6, 0.45, 0.3, 0.1 };
	const double inv_a2 = 1.0 / (a * a);

	auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
		if (b <= gp::Resolution()) return false;
		double dx = x - cx;
		double dz = z - cz;
		double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
		return value <= 1.0 + 1e-9;
	};

	if (!modelMeshInfo.propellantMesh.IsNull()) {
		TColStd_PackedMapOfInteger nodes = modelMeshInfo.propellantMesh->GetAllNodes();
		Handle(TColStd_HArray2OfReal) coords = modelMeshInfo.propellantMesh->GetmyNodeCoords();
		for (TColStd_PackedMapOfInteger::Iterator it(nodes); it.More(); it.Next()) {
			int id = it.Key();
			double x = coords->Value(id, 1);
			double y = coords->Value(id, 2);

			double value = minVal;
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i) {
				double b = b_full * b_scales[i];
				if (isInEllipse(x, y, cx, cz, inv_a2, b)) {
					double ratio;
					switch (i) {
					case 6: ratio = 8.5 / 9.0; break;
					case 5: ratio = 7.5 / 9.0; break;
					case 4: ratio = 6.5 / 9.0; break;
					case 3: ratio = 5.5 / 9.0; break;
					case 2: ratio = 4.5 / 9.0; break;
					case 1: ratio = 3.5 / 9.0; break;
					case 0: ratio = 2.5 / 9.0; break;
					default: ratio = 0.0;
					}
					value = minVal + (maxVal - minVal) * ratio;
					break;
				}
			}
			resultInfo.propellantReactionDegreeNodeValues.push_back(value);
		}
	}

	ModelDataManager::GetInstance()->SetSacrificeExplosionAnalysisResultInfo(resultInfo);

	// ========== 计算推进剂平均值 ==========
	auto computeAvg = [](const std::vector<double>& values) -> double {
		double sum = 0.0;
		size_t count = 0;
		for (double val : values) {
			if (val != -1.0) {
				sum += val;
				++count;
			}
		}
		return count > 0 ? (sum / count) : 0.0;
	};

	auto computeStd = [&](const std::vector<double>& values) -> double {
		double mean = computeAvg(values);
		double sumSq = 0.0;
		size_t count = 0;
		for (double v : values) {
			if (v != -1.0) {
				double diff = v - mean;
				sumSq += diff * diff;
				++count;
			}
		}
		return count > 1 ? std::sqrt(sumSq / count) : 0.0;
	};

	reactResult.propellantsAvgReactionDegree = computeAvg(resultInfo.propellantReactionDegreeNodeValues);
	reactResult.propellantsStandardReactionDegree = computeStd(resultInfo.propellantReactionDegreeNodeValues);

	ModelDataManager::GetInstance()->SetSacrificeExplosionReactionDegreeResult(reactResult);

	return true;
}