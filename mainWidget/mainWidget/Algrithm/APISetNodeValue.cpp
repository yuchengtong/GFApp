#include "APISetNodeValue.h"
#include "ModelDataManager.h"
#include <MeshVS_Mesh.hxx>
#include <MeshVS_DataMapOfIntegerColor.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>


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
	if (max <= min) {
		Quantity_Color defaultColor(0.5, 0.5, 0.5, Quantity_TOC_RGB); // 灰色
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


bool APISetNodeValue::SetFallStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();

		// 创建旋转网格
		auto meshData = modelMeshInfo.triangleStructure.RotateXY(angle, (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
			(modelGeometryInfo.theYmin + modelGeometryInfo.theYmax) / 2.0);
		aMesh->SetDataSource(meshData);

		auto center1 = modelGeometryInfo.bottomEndPoint;
		auto center2 = modelGeometryInfo.bottomEndPoint2;

		const double maxDistance = 400.0;  
		const int layerCount = 9;        
		const double layerWidth = maxDistance / layerCount; 
		if (angle == 0)
		{
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1);
				double y = nodecoords->Value(nodeID, 2);
				double z = nodecoords->Value(nodeID, 3);

				if (z == center1.Z())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					double dist1 = std::sqrt((x - center1.X()) * (x - center1.X()) + (y - center1.Y()) * (y - center1.Y()));
					double dist2 = std::sqrt((x - center2.X()) * (x - center2.X()) + (y - center2.Y()) * (y - center2.Y()));

					double minDist = std::min(dist1, dist2);

					if (minDist > maxDistance)
					{
						nodeValues.push_back(min_value);
					}
					else
					{
						int layer = static_cast<int>(minDist / layerWidth);
						if (layer >= layerCount) layer = layerCount - 1;  // 边界保护

						double ratio = static_cast<double>(layerCount - 1 - layer) / (layerCount - 1);
						double value = min_value + (max_value - min_value) * ratio;
						nodeValues.push_back(value);
					}
				}
			}
		}
		else if (angle > 0 && angle <= 30)
		{

		}
		else if (angle > 0 && angle <= 60)
		{

		}
		else if (angle > 60 && angle < 90)
		{

		}
		else if (angle == 90)
		{

		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetFallStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();


	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;


	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fallAnalysisResultInfo.strainMaxValue;
		auto min_value = fallAnalysisResultInfo.strainMinValue;

		//std::vector<double> nodeValues;
		Handle(MeshVS_Mesh) aMesh = nullptr;
		if (angle == 0)
		{
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure);


			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = z_min + 20;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length * 0.8 / 2.0;
			const double ellipse_b = rect_width * 0.4 / 2.0;

			double red_line_z = z_min + 10;
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标
				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				// 考虑浮点计算误差，使用一个小的容差
				if (z < red_line_z)
				{
					nodeValues.push_back(max_value);
				}
				else if (z > red_line_z && z < red_line_z + 5)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
				}
				else if (z > red_line_z + 5 && z < red_line_z + 10)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
				}
				else if (z > red_line_z + 10 && z < red_line_z + 15)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
				}
				else if (z > red_line_z + 15 && z < red_line_z + 20)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
				}
				else
				{
					if (value <= threshold + Precision::Confusion())
					{
						nodeValues.push_back(0.3 * max_value);
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
			}
		}
		else if (angle == 45)
		{
			//点的坐标用0，渲染用45
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure45);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length / 2.0;
			const double ellipse_b = rect_width / 2.0;

			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				// 考虑浮点计算误差，使用一个小的容差
				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					if (x > ellipse_h && z < ellipse_k)
					{
						if (z >= z_min && z<z_min + 20 && x>x_max - 20 && x <= x_max)
						{
							nodeValues.push_back(max_value);
						}
						else if (z > z_min + 20 && z < z_min + 30
							&& x < x_max - 20 && x > x_max - 30)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
						}
						else if (z > z_min + 30 && z < z_min + 40
							&& x < x_max - 30 && x > x_max - 40)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
						}
						else if (z > z_min + 40 && z < z_min + 50
							&& x < x_max - 40 && x > x_max - 50)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.3);
						}
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}



			}
		}
		else if (angle == 90)
		{
			//点的坐标用0，渲染用90
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length / 2.0;
			const double ellipse_b = rect_width / 2.0;

			//double red_line_z = z_min + 200;

			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					if (x > (x_min + x_max) / 2.0)
					{
						if (x< x_max && x>x_max - 10)
						{
							nodeValues.push_back(max_value);
						}
						else if (x< x_max - 10 && x>x_max - 20)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
						}
						else if (x< x_max - 20 && x>x_max - 30)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
						}
						else if (x< x_max - 30 && x>x_max - 40)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
						}
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetFallTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fallAnalysisResultInfo.temperatureMaxValue;
		auto min_value = fallAnalysisResultInfo.temperatureMinValue;

		//std::vector<double> nodeValues;

		Handle(MeshVS_Mesh) aMesh = nullptr;
		if (angle == 0)
		{
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure);


			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = z_min + 20;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length * 0.8 / 2.0;
			const double ellipse_b = rect_width * 0.4 / 2.0;

			double red_line_z = z_min + 10;
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) 
			{
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);


				if (z < red_line_z)
				{
					nodeValues.push_back(max_value);
				}
				else if (z > red_line_z && z < red_line_z + 5)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
				}
				else if (z > red_line_z + 5 && z < red_line_z + 10)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
				}
				//else if (z > red_line_z + 10 && z < red_line_z + 15)
				//{
				//	nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
				//}
				else
				{
					if (value <= threshold + Precision::Confusion())
					{
						nodeValues.push_back(min_value+0.5 * (max_value- min_value));
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
			}
		}
		else if (angle == 45)
		{
			//点的坐标用0，渲染用45
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure45);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length / 2.0;
			const double ellipse_b = rect_width / 2.0;

			//double red_line_z = z_min + 200;
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				// 考虑浮点计算误差，使用一个小的容差
				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					if (x > ellipse_h && z < ellipse_k)
					{
						if (z > z_min && z<z_min + 10 && x>x_max - 10 && x < x_max)
						{
							nodeValues.push_back(max_value);
						}
						else if (z > z_min + 10 && z < z_min + 20
							&& x < x_max - 10 && x > x_max - 20)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
						}
						else if (z > z_min + 20 && z < z_min + 30
							&& x < x_max - 20 && x > x_max - 30)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
						}
						else if (z > z_min + 30 && z < z_min + 40
							&& x < x_max - 30 && x > x_max - 40)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.3);
						}
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
			}

		}
		else if (angle == 90)
		{
			//点的坐标用0，渲染用90
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length / 2.0;
			const double ellipse_b = rect_width / 2.0;

			//double red_line_z = z_min + 200;
			double min_z_value = std::numeric_limits<double>::max();
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标
				if (z < min_z_value)
				{
					min_z_value = z;
				}
				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				// 考虑浮点计算误差，使用一个小的容差
				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					if (x > (x_min + x_max) / 2.0)
					{
						if (x< x_max && x>x_max - 10)
						{
							nodeValues.push_back(max_value);
						}
						else if (x< x_max - 10 && x>x_max - 20)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
						}
						else if (x< x_max - 20 && x>x_max - 30)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
						}
						else if (x< x_max - 30 && x>x_max - 40)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
						}
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetFallOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fallAnalysisResultInfo.overpressureMaxValue;
		auto min_value = fallAnalysisResultInfo.overpressureMinValue;

		//std::vector<double> nodeValues;

		Handle(MeshVS_Mesh) aMesh = nullptr;
		if (angle == 0)
		{
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = z_min + 20;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length * 0.8 / 2.0;
			const double ellipse_b = rect_width * 0.4 / 2.0;

			double red_line_z = z_min + 10;
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
			{
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				// 考虑浮点计算误差，使用一个小的容差
				if (z < red_line_z)
				{
					nodeValues.push_back(max_value);
				}
				else if (z > red_line_z && z < red_line_z + 5)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
				}
				else if (z > red_line_z + 5 && z < red_line_z + 10)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
				}
				else
				{
					if (value < threshold + Precision::Confusion())
					{
						nodeValues.push_back(0.5 * max_value);
					}
					else if (value == threshold + Precision::Confusion() && abs(z - ellipse_h) < 50)
					{
						nodeValues.push_back(min_value);
					}
					else
					{
						nodeValues.push_back(0.35 * max_value);
					}
				}

			}
		}
		else if (angle == 45)
		{
			//点的坐标用0，渲染用45
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure45);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length / 2.0;
			const double ellipse_b = rect_width / 2.0;

			//double red_line_z = z_min + 200;

			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标

				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					if (x > ellipse_h && z < ellipse_k)
					{
						if (z > z_min && z<z_min + 10 && x>x_max - 10 && x < x_max)
						{
							nodeValues.push_back(max_value);
						}
						else if (z > z_min + 10 && z < z_min + 20
							&& x < x_max - 10 && x > x_max - 20)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
						}
						else if (z > z_min + 20 && z < z_min + 30
							&& x < x_max - 20 && x > x_max - 30)
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
						}
						//else if (z > z_min + 30 && z < z_min + 40
						//	&& x < x_max - 30 && x > x_max - 40)
						//{
						//	nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
						//}
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}


			}
		}
		else if (angle == 90)
		{
			//点的坐标用0，渲染用90
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length / 2.0;
			const double ellipse_b = rect_width / 2.0;

			//double red_line_z = z_min + 200;
			double min_z_value = std::numeric_limits<double>::max();
			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
				int nodeID = it.Key();
				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
				double z = nodecoords->Value(nodeID, 3); // 节点z坐标
				if (z < min_z_value)
				{
					min_z_value = z;
				}
				// --- 数学判断逻辑 ---
				// 计算椭圆方程左边的值
				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
				double dx = x - ellipse_h;
				double dz = z - ellipse_k;

				// 为了提高精度和效率，可以比较平方和，避免开方和除法
				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);

				// 考虑浮点计算误差，使用一个小的容差
				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.3);
				}
				else
				{
					if (x< x_max && x>x_max - 10)
					{
						nodeValues.push_back(max_value);
					}
					else if (x< x_max - 10 && x>x_max - 20)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
					}
					else if (x< x_max - 20 && x>x_max - 30)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
					}
					else if (x< x_max - 30 && x>x_max - 40)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
					}
					else
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 0.3);
					}
				}
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetFastCombustionTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto fastCombustionSettingInfo = ModelDataManager::GetInstance()->GetFastCombustionSettingInfo();
	auto fastCombustionAnalysisResultInfo = ModelDataManager::GetInstance()->GetFastCombustionAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fastCombustionAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fastCombustionAnalysisResultInfo.temperatureMaxValue;
		auto min_value = fastCombustionAnalysisResultInfo.temperatureMinValue;


		Handle(MeshVS_Mesh) aMesh = nullptr;


		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// --- 2. 根据矩形角点计算椭圆参数 ---
		const double rect_length = x_max - x_min;
		const double rect_width = z_max - z_min;

		const double h = (x_min + x_max) / 2.0;     // 椭圆中心 X
		const double k = (z_min + z_max) / 2.0;     // 椭圆中心 Z

		// 主椭圆半轴（注意：b 被放大了 20%）
		const double a_main = rect_length / 2.0;
		const double b_main = (rect_width / 2.0) * 1.1;

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

		//黄线
		const double yellow_line_z_min = z_min + 30;
		const double yellow_line_z_max = z_max - 30;

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(min_value);			
			}
			else if (isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else
			{		
				nodeValues.push_back(max_value);
			}
			
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetSlowCombustionTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto slowCombustionSettingInfo = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo();
	auto slowCombustionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSlowCombustionAnalysisResultInfo();


	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (slowCombustionAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = slowCombustionAnalysisResultInfo.temperatureMaxValue;
		auto min_value = slowCombustionAnalysisResultInfo.temperatureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double rect_width = x_max - x_min;
		const double rect_height = z_max - z_min;
		const double h = (x_min + x_max) / 2.0; // 矩形/主椭圆中心 X
		const double k = (z_min + z_max) / 2.0; // 矩形/主椭圆中心 Z
		const double a_main = rect_width / 2.0; // 主椭圆长半轴
		const double b_main = rect_height / 2.0; // 主椭圆短半轴

		// 上边线三个椭圆的参数（中心在顶部中点）
		const double top_center_z = k + rect_height / 2.0;
		std::vector<std::pair<double, double>> top_ellipses = {
			{a_main * 0.6, b_main * 0.3}, // 椭圆1
			{a_main * 0.7, b_main * 0.4}, // 椭圆2
			{a_main * 0.8, b_main * 0.5}, // 椭圆3
			{a_main * 0.9, b_main * 0.6}  // 椭圆4
		};

		// 下边线三个椭圆
		const double bottom_center_z = k - rect_height / 2.0;
		std::vector<std::pair<double, double>> bottom_ellipses = {
			{a_main * 0.6, b_main * 0.3},
			{a_main * 0.7, b_main * 0.4},
			{a_main * 0.8, b_main * 0.5},
			{a_main * 0.9, b_main * 0.6}
		};

		// 左右对称圆（实际是 a = b 的椭圆）
		const double offset_x = a_main * 0.6;
		const double left_center_x = h - offset_x;
		const double right_center_x = h + offset_x;
		std::vector<double> circle_radii = { b_main * 0.5 * 0.4, b_main * 0.5 * 0.8 }; // 两个半径

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) 
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			// 1. 判断是否在主内切椭圆内
			if (isInEllipse(x, z, h, k, a_main, b_main))
			{
				double a_t0 = top_ellipses[0].first;
				double b_t0 = top_ellipses[0].second;
				double a_t1 = top_ellipses[1].first;
				double b_t1 = top_ellipses[1].second;
				double a_t2 = top_ellipses[2].first;
				double b_t2 = top_ellipses[2].second;
				double a_t3 = top_ellipses[3].first;
				double b_t3 = top_ellipses[3].second;

				double a_b0 = bottom_ellipses[0].first;
				double b_b0 = bottom_ellipses[0].second;
				double a_b1 = bottom_ellipses[1].first;
				double b_b1 = bottom_ellipses[1].second;
				double a_b2 = bottom_ellipses[2].first;
				double b_b2 = bottom_ellipses[2].second;
				double a_b3 = bottom_ellipses[3].first;
				double b_b3 = bottom_ellipses[3].second;


				if (isInEllipse(x, z, h, top_center_z, a_t0, b_t0)|| 
					isInEllipse(x, z, h, bottom_center_z, a_b0, b_b0))
				{
					nodeValues.push_back(max_value);
				}
				else if (isInEllipse(x, z, h, top_center_z, a_t1, b_t1) || 
					isInEllipse(x, z, h, bottom_center_z, a_b1, b_b1))
				{
					nodeValues.push_back(min_value+(max_value-min_value)*7.5/9.0);//橙色
				}
				else if (isInEllipse(x, z, h, top_center_z, a_t2, b_t2) || 
					isInEllipse(x, z, h, bottom_center_z, a_b2, b_b2))
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);//黄色
				}
				else if (isInEllipse(x, z, h, top_center_z, a_t3, b_t3) || 
					isInEllipse(x, z, h, bottom_center_z, a_b3, b_b3))
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);//绿色
				}
				else
				{
					if (isInEllipse(x, z, left_center_x, k, circle_radii[0], circle_radii[0])|| 
						isInEllipse(x, z, right_center_x, k, circle_radii[0], circle_radii[0]))
					{
						nodeValues.push_back(min_value);
					}
					else if (isInEllipse(x, z, left_center_x, k, circle_radii[1], circle_radii[1]) ||
						isInEllipse(x, z, right_center_x, k, circle_radii[1], circle_radii[1]))
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);//蓝色
					}
					else
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);//浅蓝色
					}
				}
			}
			else
			{
				nodeValues.push_back(max_value);
			}						
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}
	return true;
}

bool APISetNodeValue::SetShootStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto shootSettingInfo=ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto shootAnalysisResultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	
	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (shootAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = shootAnalysisResultInfo.stressMaxValue;
		auto min_value = shootAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min ;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 3.0;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
			{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
			{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.65},
			{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.7},
			{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.8},
			{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if (isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second)) 
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetShootStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto shootSettingInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto shootAnalysisResultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();

	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (shootAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = shootAnalysisResultInfo.strainMaxValue;
		auto min_value = shootAnalysisResultInfo.strainMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 3.0;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
			{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
			{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.65},
			{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.7},
			{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.8},
			{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if (isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetShootTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto shootSettingInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto shootAnalysisResultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();


	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (shootAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto raw_min = shootAnalysisResultInfo.temperatureMinValue;
		auto raw_max = shootAnalysisResultInfo.temperatureMaxValue;

		// 保证 min_value <= max_value
		double min_value = std::min(raw_min, raw_max);
		double max_value = std::max(raw_min, raw_max);

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 2.5;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
			{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
			{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.65},
			{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.7},
			{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.75},
			{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.85},
			{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
		};


		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if (isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetShootOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto shootSettingInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto shootAnalysisResultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();


	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (shootAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = shootAnalysisResultInfo.overpressureMaxValue;
		auto min_value = shootAnalysisResultInfo.overpressureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 2.5;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
	{12,semi_major_z_1 * 0.4},
	{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.45},
	{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.5},
	{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.6},
	{semi_minor_x_1 * 0.35, semi_major_z_1 * 0.65},
	{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.7},
	{semi_minor_x_1 * 0.45, semi_major_z_1 * 0.8},
	{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.85},
	{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.9}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if (isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetJetImpactStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto jetImpactAnalysisResultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();


	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (jetImpactAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = jetImpactAnalysisResultInfo.stressMaxValue;
		auto min_value = jetImpactAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);


		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1/3.0 ;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,31},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
			{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.5},
			{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.7},
			{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.8},
			{semi_minor_x_1 * 0.9, semi_major_z_1 * 0.9}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if (isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetJetImpactStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto jetImpactAnalysisResultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();

	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (jetImpactAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = jetImpactAnalysisResultInfo.strainMaxValue;
		auto min_value = jetImpactAnalysisResultInfo.strainMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 3.0;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,31},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
			{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.5},
			{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.7},
			{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.8},
			{semi_minor_x_1 * 0.9, semi_major_z_1 * 0.9}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetJetImpactTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto jetImpactAnalysisResultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();


	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (jetImpactAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = jetImpactAnalysisResultInfo.temperatureMaxValue;
		auto min_value = jetImpactAnalysisResultInfo.temperatureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 2.5;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,31},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
			{semi_minor_x_1 * 0.4, semi_major_z_1 * 0.35},
			{semi_minor_x_1 * 0.5, semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.65, semi_major_z_1 * 0.55},
			{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.75},
			{semi_minor_x_1 * 0.9, semi_major_z_1 * 0.8}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetJetImpactOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto jetImpactSettingInfo = ModelDataManager::GetInstance()->GetJetImpactSettingInfo();
	auto jetImpactAnalysisResultInfo = ModelDataManager::GetInstance()->GetJetImpactAnalysisResultInfo();


	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (jetImpactAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = jetImpactAnalysisResultInfo.overpressureMaxValue;
		auto min_value = jetImpactAnalysisResultInfo.overpressureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = z_max - z_min;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 / 2.5;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,31},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.2},
			{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.25},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.3},
			{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.55},
			{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.95, semi_major_z_1 * 0.7},
			{semi_minor_x_1 * 1.0, semi_major_z_1 * 0.85}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetFragmentationStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto fragmentationAnalysisResultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();


	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fragmentationAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fragmentationAnalysisResultInfo.stressMaxValue;
		auto min_value = fragmentationAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = (z_max - z_min)*0.4;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1*2.5;    // 最大椭圆的 a

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

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}



		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetFragmentationStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto fragmentationAnalysisResultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fragmentationAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fragmentationAnalysisResultInfo.strainMaxValue;
		auto min_value = fragmentationAnalysisResultInfo.strainMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = (z_max - z_min) * 0.4;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 * 2.5;    // 最大椭圆的 a

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


		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}



		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetFragmentationTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto fragmentationAnalysisResultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fragmentationAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto  raw_max = fragmentationAnalysisResultInfo.temperatureMaxValue;
		auto  raw_min = fragmentationAnalysisResultInfo.temperatureMinValue;

		double min_value = std::min(raw_min, raw_max);
		double max_value = std::max(raw_min, raw_max);

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = (z_max - z_min) * 0.4;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 * 2.5;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,31},
			{semi_minor_x_1 * 0.15, semi_major_z_1 * 0.35},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.4},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.55},
			{semi_minor_x_1 * 0.6, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.65, semi_major_z_1 * 0.75},
			{semi_minor_x_1 * 0.8, semi_major_z_1 * 0.8},
			{semi_minor_x_1 * 0.95, semi_major_z_1 * 0.95},
			{semi_minor_x_1 * 1.0, semi_major_z_1 * 1.0}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetFragmentationOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto fragmentationAnalysisResultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (fragmentationAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = fragmentationAnalysisResultInfo.overpressureMaxValue;
		auto min_value = fragmentationAnalysisResultInfo.overpressureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double h = (x_min + x_max) / 2.0;          // 椭圆中心 X（底边中点）
		const double k = z_min;                          // 椭圆中心 Z（底边）
		const double semi_major_z_1 = (z_max - z_min) * 0.4;   // 最大椭圆的 b
		const double semi_minor_x_1 = semi_major_z_1 * 2.5;    // 最大椭圆的 a

		std::vector<std::pair<double, double>> ellipses = {
			{12,31},
			{semi_minor_x_1 * 0.2, semi_major_z_1 * 0.35},
			{semi_minor_x_1 * 0.25, semi_major_z_1 * 0.45},
			{semi_minor_x_1 * 0.3, semi_major_z_1 * 0.5},
			{semi_minor_x_1 * 0.55, semi_major_z_1 * 0.6},
			{semi_minor_x_1 * 0.7, semi_major_z_1 * 0.75},
			{semi_minor_x_1 * 0.85, semi_major_z_1 * 0.8},
			{semi_minor_x_1 * 0.95, semi_major_z_1 * 0.9},
			{semi_minor_x_1 * 1.0, semi_major_z_1 * 1.0}
		};

		// cx    // 椭圆中心 x
		// cz    // 椭圆中心 z
		// a     // 椭圆长半轴
		// b     // 椭圆短半轴
		auto isInEllipse = [](double x, double z, double cx, double cz, double a, double b) -> bool {
			double dx = x - cx;
			double dz = z - cz;
			if (a <= gp::Resolution() || b <= gp::Resolution()) return false;
			double value = (dx * dx) / (a * a) + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9; // 容差处理
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			if (isInEllipse(x, z, h, k, ellipses[0].first, ellipses[0].second))
			{
				nodeValues.push_back(-1);
			}
			else if(isInEllipse(x, z, h, k, ellipses[1].first, ellipses[1].second))
			{
				nodeValues.push_back(max_value);
			}
			else if (isInEllipse(x, z, h, k, ellipses[2].first, ellipses[2].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[3].first, ellipses[3].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[4].first, ellipses[4].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[5].first, ellipses[5].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[6].first, ellipses[6].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[7].first, ellipses[7].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[8].first, ellipses[8].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 1.5 / 9.0);
			}
			else if (isInEllipse(x, z, h, k, ellipses[9].first, ellipses[9].second))
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.5 / 9.0);
			}
			else
			{
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();

	}
	return true;
}

bool APISetNodeValue::SetExplosiveBlastStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto explosiveBlastAnalysisResultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
			(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (explosiveBlastAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = explosiveBlastAnalysisResultInfo.stressMaxValue;
		auto min_value = explosiveBlastAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetExplosiveBlastStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto explosiveBlastAnalysisResultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (explosiveBlastAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = explosiveBlastAnalysisResultInfo.strainMaxValue;
		auto min_value = explosiveBlastAnalysisResultInfo.strainMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetExplosiveBlastTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto explosiveBlastAnalysisResultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (explosiveBlastAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = explosiveBlastAnalysisResultInfo.temperatureMaxValue;
		auto min_value = explosiveBlastAnalysisResultInfo.temperatureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;
		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.85, 0.6, 0.5, 0.4, 0.25, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetExplosiveBlastOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto explosiveBlastSettingInfo = ModelDataManager::GetInstance()->GetExplosiveBlastSettingInfo();
	auto explosiveBlastAnalysisResultInfo = ModelDataManager::GetInstance()->GetExplosiveBlastAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (explosiveBlastAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = explosiveBlastAnalysisResultInfo.overpressureMaxValue;
		auto min_value = explosiveBlastAnalysisResultInfo.overpressureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.9, 0.8, 0.7, 0.5, 0.3, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetSacrificeExplosionStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto sacrificeExplosionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
			(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (sacrificeExplosionAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = sacrificeExplosionAnalysisResultInfo.stressMaxValue;
		auto min_value = sacrificeExplosionAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0 * 2.0 / 3.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}
bool APISetNodeValue::SetSacrificeExplosionStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto sacrificeExplosionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (sacrificeExplosionAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = sacrificeExplosionAnalysisResultInfo.strainMaxValue;
		auto min_value = sacrificeExplosionAnalysisResultInfo.strainMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;
		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);
		// --- 2. 根据矩形角点计算椭圆参数 ---

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0 * 2.0 / 3.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.85, 0.7, 0.55, 0.4, 0.25, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetSacrificeExplosionTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto sacrificeExplosionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (sacrificeExplosionAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = sacrificeExplosionAnalysisResultInfo.temperatureMaxValue;
		auto min_value = sacrificeExplosionAnalysisResultInfo.temperatureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;
		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0*2.0/3.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴

		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.9, 0.8, 0.7, 0.5, 0.3, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}


		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetSacrificeExplosionOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto sacrificeExplosionSettingInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionSettingInfo();
	auto sacrificeExplosionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSacrificeExplosionAnalysisResultInfo();


	//auto high = fallSettingInfo.high;
	//auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.z_min };
	Point p2{ meshInfo.x_max, meshInfo.z_min };
	Point p3{ meshInfo.x_max, meshInfo.z_max };
	Point p4{ meshInfo.x_min, meshInfo.z_max };

	// 从角点计算矩形边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double z_min = meshInfo.z_min;
	const double z_max = meshInfo.z_max;

	if (sacrificeExplosionAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		auto max_value = sacrificeExplosionAnalysisResultInfo.overpressureMaxValue;
		auto min_value = sacrificeExplosionAnalysisResultInfo.overpressureMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 椭圆公共中心（底边中点）
		const double cx = (x_min + x_max) / 2.0;
		const double cz = z_min;

		// 椭圆尺寸
		const double a = (x_max - x_min) / 2.0 * 2.0 / 3.0;          // 半长轴（固定）
		const double b_full = (z_max - z_min) / 2.0;     // 原始半短轴


		// 短轴缩放比例（从大到小）
		std::vector<double> b_scales = { 1.0, 0.85, 0.8, 0.6, 0.4, 0.2, 0.1 };

		// 预计算 1/a²（a 不变）
		const double inv_a2 = 1.0 / (a * a);

		// 椭圆判断 lambda
		auto isInEllipse = [](double x, double z, double cx, double cz, double inv_a2, double b) -> bool {
			if (b <= gp::Resolution()) return false;
			double dx = x - cx;
			double dz = z - cz;
			double value = dx * dx * inv_a2 + (dz * dz) / (b * b);
			return value <= 1.0 + 1e-9;
		};

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			bool found = false;
			// 从最大椭圆（i=0）开始检查 → 最小椭圆（i=6）
			for (int i = static_cast<int>(b_scales.size()) - 1; i >= 0; --i)
			{
				double b = b_full * b_scales[i];
				if (isInEllipse(x, z, cx, cz, inv_a2, b))
				{
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

					double val = min_value + (max_value - min_value) * ratio;
					nodeValues.push_back(val);
					found = true;
					break;
				}
			}

			if (!found) {
				nodeValues.push_back(min_value);
			}
		}

		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}
