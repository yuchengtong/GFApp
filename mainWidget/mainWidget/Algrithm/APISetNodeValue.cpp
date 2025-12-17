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
		colormap.Bind(index + 1, Quantity_Color(r, g, b, Quantity_TOC_RGB));
		index++;
	}
	return colormap;
}


bool APISetNodeValue::SetFallStressResult(OccView* occView, std::vector<double>& nodeValues)
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

		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;

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

			double red_line_z = z_min+10;
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
				else if (z > red_line_z&&z< red_line_z+5)
				{
					nodeValues.push_back(min_value+(max_value- min_value)*0.8);
				}
				else if (z > red_line_z+5 && z < red_line_z + 10)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
				}
				else if (z > red_line_z+10 && z < red_line_z + 15)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
				}
				else if (z > red_line_z+15 && z < red_line_z + 20)
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
			const double ellipse_a = rect_length  / 2.0;
			const double ellipse_b = rect_width  / 2.0;

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
							&& x < x_max-20 && x > x_max-30)
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

		auto max_value = fallAnalysisResultInfo.stressMaxValue / youngModulus;
		auto min_value = fallAnalysisResultInfo.stressMinValue / youngModulus;

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

		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;

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

		const double ellipse_h = (x_min + x_max) / 2.0;		//椭圆在 x 轴上的中心位置
		const double ellipse_k = (z_min + z_max) / 2.0;		//椭圆在 z 轴上的中心位置

		const double ellipse_a_0 = rect_length / 2.0;			//椭圆在 x 方向的半轴长度
		const double ellipse_b_0 = rect_width / 2.0+ 0.2 * (rect_width / 2);			//椭圆在 z 方向的半轴长度

		const double scale_factor_a = 0.8;
		const double scale_factor_b = 0.6;
		const double ellipse_a_1 = ellipse_a_0 * scale_factor_a;
		const double ellipse_b_1 = ellipse_b_0 * scale_factor_b;

		// 预计算外椭圆（椭圆0）参数
		const double a0 = ellipse_a_0;
		const double b0 = ellipse_b_0;
		const double a0_sq = a0 * a0;
		const double b0_sq = b0 * b0;
		const double threshold0 = a0_sq * b0_sq;

		// 预计算内椭圆（椭圆1）参数
		const double a1 = ellipse_a_1;
		const double b1 = ellipse_b_1;
		const double a1_sq = a1 * a1;
		const double b1_sq = b1 * b1;
		const double threshold1 = a1_sq * b1_sq;

		//黄线
		const double yellow_line_z_min = z_min + 30;
		const double yellow_line_z_max = z_max - 30;

		const double tol = Precision::Confusion();

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			double dx = x - ellipse_h;
			double dz = z - ellipse_k;

			// 判断是否在椭圆0（外椭圆）内
			double value0 = dx * dx * b0_sq + dz * dz * a0_sq;

			if (value0 > threshold0 + tol) 
			{
				// 在椭圆0外部 → 赋 max_value
				nodeValues.push_back(max_value);
			}
			else 
			{
				// 在椭圆0内部 → 进一步判断是否在椭圆1（内椭圆）内
				double value1 = dx * dx * b1_sq + dz * dz * a1_sq;

				if (value1 <= threshold1 + tol)
				{
					// 在椭圆1内部（或边界）→ 赋 min_value
					nodeValues.push_back(min_value);
				}
				else
				{
					// 不在椭圆1内部→ 进一步判断是否在黄线内
					if (z < yellow_line_z_min || z > yellow_line_z_max)
					{
						//黄色区域
						nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 10.0);
					}
					else
					{
						//绿色区域
						nodeValues.push_back(min_value + (max_value - min_value) * 5.0 / 10.0);
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

bool APISetNodeValue::SetSlowCombustionTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{

	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
	auto slowCombustionSettingInfo = ModelDataManager::GetInstance()->GetSlowCombustionSettingInfo();
	auto slowCombustionAnalysisResultInfo = ModelDataManager::GetInstance()->GetSlowCombustionAnalysisResultInfo();


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

		// --- 2. 根据矩形角点计算椭圆参数 ---
		const double rect_length = x_max - x_min;
		const double rect_width = z_max - z_min;

		const double ellipse_h = (x_min + x_max) / 2.0;		//椭圆在 x 轴上的中心位置
		const double ellipse_k = (z_min + z_max) / 2.0;		//椭圆在 z 轴上的中心位置

		const double ellipse_a_0 = rect_length / 2.0;			//椭圆在 x 方向的半轴长度
		const double ellipse_b_0 = rect_width / 2.0 + 0.2 * (rect_width / 2);			//椭圆在 z 方向的半轴长度

		const double scale_factor_a = 0.8;
		const double scale_factor_b = 0.6;
		const double ellipse_a_1 = ellipse_a_0 * scale_factor_a;
		const double ellipse_b_1 = ellipse_b_0 * scale_factor_b;

		// 预计算外椭圆（椭圆0）参数
		const double a0 = ellipse_a_0;
		const double b0 = ellipse_b_0;
		const double a0_sq = a0 * a0;
		const double b0_sq = b0 * b0;
		const double threshold0 = a0_sq * b0_sq;

		// 预计算内椭圆（椭圆1）参数
		const double a1 = ellipse_a_1;
		const double b1 = ellipse_b_1;
		const double a1_sq = a1 * a1;
		const double b1_sq = b1 * b1;
		const double threshold1 = a1_sq * b1_sq;

		//黄线
		const double yellow_line_z_min = z_min + 30;
		const double yellow_line_z_max = z_max - 30;

		const double green_line_z_min = z_min + 60;
		const double green_line_z_max = z_max - 60;

		// 椭圆焦点生成2个圆，在圆内则为最小值
		double c0 = 0.0;
		double focus1_x = 0.0, focus1_z = 0.0;  // 焦点1坐标
		double focus2_x = 0.0, focus2_z = 0.0;  // 焦点2坐标
		if (a0 > b0) {
			// 长轴在x轴上，焦点在x轴方向
			c0 = sqrt(a0_sq - b0_sq);
			focus1_x = ellipse_h + c0;
			focus1_z = ellipse_k;
			focus2_x = ellipse_h - c0;
			focus2_z = ellipse_k;
		}
		else {
			// 长轴在z轴上，焦点在z轴方向
			c0 = sqrt(b0_sq - a0_sq);
			focus1_x = ellipse_h;
			focus1_z = ellipse_k + c0;
			focus2_x = ellipse_h;
			focus2_z = ellipse_k - c0;
		}

		// 定义焦点圆的半径（可根据业务需求调整）
		const double focus_circle_radius = 50.0;
		const double focus_circle_radius_sq = focus_circle_radius * focus_circle_radius;  // 预计算半径平方，避免开方


		const double tol = Precision::Confusion();

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			double dx = x - ellipse_h;
			double dz = z - ellipse_k;

			// 判断是否在椭圆0（外椭圆）内
			double value0 = dx * dx * b0_sq + dz * dz * a0_sq;

			if (value0 > threshold0 + tol)
			{
				// 在椭圆0外部 → 赋 max_value
				nodeValues.push_back(max_value);
			}
			else
			{
				// 在焦点圆内部
				// 计算节点到焦点1的距离平方
				double dx1 = x - focus1_x;
				double dz1 = z - focus1_z;
				double dist1_sq = dx1 * dx1 + dz1 * dz1;

				// 计算节点到焦点2的距离平方
				double dx2 = x - focus2_x;
				double dz2 = z - focus2_z;
				double dist2_sq = dx2 * dx2 + dz2 * dz2;

				if (dist1_sq <= focus_circle_radius_sq + tol || dist2_sq <= focus_circle_radius_sq + tol) {
					// 在任意一个焦点圆内，赋最小值
					nodeValues.push_back(min_value);
				}
				else
				{
					// 不在焦点圆内内部→ 进一步判断是否在黄线内
					if (z < yellow_line_z_min  || z > yellow_line_z_max )
					{
						nodeValues.push_back(max_value);
					}
					else if ( (z > yellow_line_z_min && z < green_line_z_min) || (z < yellow_line_z_max && z > green_line_z_max))
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 7 / 9.0);
					}
					else if ((z > green_line_z_min && z < (green_line_z_min + 30)) || (z < green_line_z_max && z > (green_line_z_max - 30) ))
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5 / 9.0);
					}
					else if ((z > (green_line_z_min + 30) && z < (green_line_z_min + 60)) || (z < (green_line_z_max - 30) && z >(green_line_z_max - 60)))
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 4 / 9.0);
					}
					else
					{
						//其他淡绿色区域
						nodeValues.push_back(min_value + (max_value - min_value) * 3 / 9.0);
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
	return false;
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

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			// 考虑浮点计算误差，使用一个小的容差
			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq&& dist_sq< circle_radius_sq+5*5)
			{
				nodeValues.push_back(min_value+(max_value- min_value)*0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		auto max_value = shootAnalysisResultInfo.stressMaxValue / youngModulus;
		auto min_value = shootAnalysisResultInfo.stressMinValue / youngModulus;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			// 考虑浮点计算误差，使用一个小的容差
			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		//auto min_value = shootAnalysisResultInfo.temperatureMaxValue;
		//auto max_value = shootAnalysisResultInfo.temperatureMinValue;

		double max_value = 88;
		double min_value = 73;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// --- 2. 根据矩形角点计算椭圆参数 ---
		const double ellipse_h = (x_min + x_max) / 2.0;
		const double ellipse_k = z_min + 20;
		const double rect_length = x_max - x_min;
		const double rect_width = z_max - z_min;
		const double ellipse_a = rect_length * 0.8 / 2.0;
		const double ellipse_b = rect_width * 0.4 / 2.0;

		double red_line_z = z_min + 20;
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
					nodeValues.push_back(min_value + 0.5 * (max_value - min_value));
				}
				else
				{
					nodeValues.push_back(min_value);
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

		auto max_value = shootAnalysisResultInfo.stressMaxValue;
		auto min_value = shootAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;

		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标


			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			// 考虑浮点计算误差，使用一个小的容差

			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
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

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			// 考虑浮点计算误差，使用一个小的容差
			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		auto max_value = jetImpactAnalysisResultInfo.stressMaxValue / youngModulus;
		auto min_value = jetImpactAnalysisResultInfo.stressMinValue / youngModulus;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			// 考虑浮点计算误差，使用一个小的容差
			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			// 考虑浮点计算误差，使用一个小的容差
			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		// --- 预计算公共参数 ---
		const double rect_height = z_max - z_min;           // 矩形高度（z方向）
		const double b = rect_height / 2.0;                 // 半长轴（z方向）
		const double a = b / 2.0;                           // 半短轴（x方向）

		const double a2 = a * a;
		const double b2 = b * b;
		const double h_center = (x_min + x_max) / 2.0;      // 所有椭圆共享 x 中心

		// 椭圆中心 z 坐标
		const double k0 = z_min;                            // 底边中点
		const double k1 = z_min - rect_height / 8.0;        // 下移 1/8
		const double k2 = z_min - rect_height / 4.0;        // 下移 1/4

		const double tol = Precision::Confusion();          // OpenCASCADE 容差


		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标

			// 判断是否在各椭圆内（使用乘法形式避免除法，更稳定）
			auto inEllipse = [&](double k) -> bool {
				double dx = x - h_center;
				double dz = z - k;
				return (dx * dx * b2 + dz * dz * a2) <= (a2 * b2) + tol;
			};

			bool inE0 = inEllipse(k0);
			bool inE1 = inEllipse(k1);
			bool inE2 = inEllipse(k2);

			if (inE0)
			{
				if (abs(rect_height / 2 + z_min - z) < 20)
				{
					nodeValues.push_back(max_value);
				}
				else if (inE1)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
				}
				else if (inE2)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 10.0);
				}
				else
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 7.5/10.0);
				}
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

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 10.0;
		const double circle_radius2 = rect_width / 10.0 + 20;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标


			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		auto max_value = fragmentationAnalysisResultInfo.stressMaxValue / youngModulus;
		auto min_value = fragmentationAnalysisResultInfo.stressMinValue / youngModulus;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 10.0;
		const double circle_radius2 = rect_width / 10.0 + 20;
		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标


			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else if (dist_sq > circle_radius_sq + 10 * 10 && dist_sq < circle_radius_sq + 15 * 15)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
			}
			else if (dist_sq > circle_radius_sq + 15 * 15 && dist_sq < circle_radius_sq + 20 * 20)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
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

		//auto  min_value = fragmentationAnalysisResultInfo.temperatureMaxValue;
		//auto  max_value = fragmentationAnalysisResultInfo.temperatureMinValue;

		double max_value = 95;
		double min_value = 50;


		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// --- 2. 根据矩形角点计算椭圆参数 ---
		const double ellipse_h = (x_min + x_max) / 2.0;
		const double ellipse_k = z_min + 20;
		const double rect_length = x_max - x_min;
		const double rect_width = z_max - z_min;
		const double ellipse_a = rect_length * 0.8 / 2.0;
		const double ellipse_b = rect_width * 0.4 / 2.0;

		double red_line_z = z_min + 20;
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
			//else if (z > red_line_z + 15 && z < red_line_z + 20)
			//{
			//	nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
			//}
			else
			{
				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value + 0.5 * (max_value - min_value));
				}
				else
				{
					nodeValues.push_back(min_value);
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

		auto max_value = fragmentationAnalysisResultInfo.stressMaxValue;
		auto min_value = fragmentationAnalysisResultInfo.stressMinValue;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 20.0;

		// 为了提高计算效率，预先计算半径的平方
		const double circle_radius_sq = circle_radius * circle_radius;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1); // 节点x坐标
			double z = nodecoords->Value(nodeID, 3); // 节点z坐标


			// --- 3. 修改后的数学判断逻辑：判断点是否在圆内 ---
			// 计算点到圆心的x和z方向的距离
			double dx = x - circle_center_x;
			double dz = z - circle_center_z;

			// 计算点到圆心的距离的平方
			// 如果这个值小于或等于半径的平方，则点在圆内或圆上
			double dist_sq = dx * dx + dz * dz;

			if (dist_sq < circle_radius_sq)
			{
				nodeValues.push_back(max_value);
			}
			else if (dist_sq > circle_radius_sq && dist_sq < circle_radius_sq + 5 * 5)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
			}
			else if (dist_sq > circle_radius_sq + 5 * 5 && dist_sq < circle_radius_sq + 10 * 10)
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
			}
			else
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
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

		// --- 椭圆参数 ---
		const double ellipse_center_x = (x_min + x_max) / 2.0;
		const double ellipse_center_z = z_min;
		const double a = (x_max - x_min) / 2.0 * 0.8;   // 半长轴（x方向）
		const double b = (z_max - z_min) / 2.0 * 0.8;   // 半短轴（z方向）

		// 预计算平方项，避免重复计算
		const double a2 = a * a;
		const double b2 = b * b;

		//离底边线的距离
		const double linedis = 20;

		const double tol = Precision::Confusion(); // 容差值，用于浮点数比较

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			// 计算相对偏移
			double dx = x - ellipse_center_x;
			double dz = z - ellipse_center_z;

			// 判断是否在椭圆内（使用乘法形式避免除法，更稳定）
			if (dx * dx * b2 + dz * dz * a2 <= a2 * b2 + tol)
			{
				if (z< z_min+ linedis)
				{
					if (abs(ellipse_center_x - x) < a*0.3)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
					}
					else if (abs(ellipse_center_x - x) >= a * 0.3 && abs(ellipse_center_x - x) < a * 0.5)
					{
						nodeValues.push_back(max_value);
					}
					else if (abs(ellipse_center_x - x) >= a * 0.8 && abs(ellipse_center_x - x) < a )
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 10.0);
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
				else
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 10.0);
				}
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

		auto max_value = explosiveBlastAnalysisResultInfo.stressMaxValue/ youngModulus;
		auto min_value = explosiveBlastAnalysisResultInfo.stressMinValue / youngModulus;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// --- 椭圆参数 ---
		const double ellipse_center_x = (x_min + x_max) / 2.0;
		const double ellipse_center_z = z_min;
		const double a = (x_max - x_min) / 2.0 * 0.8;   // 半长轴（x方向）
		const double b = (z_max - z_min) / 2.0 * 0.8;   // 半短轴（z方向）

		// 预计算平方项，避免重复计算
		const double a2 = a * a;
		const double b2 = b * b;

		//离底边线的距离
		const double linedis = 20;

		const double tol = Precision::Confusion(); // 容差值，用于浮点数比较

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			// 计算相对偏移
			double dx = x - ellipse_center_x;
			double dz = z - ellipse_center_z;

			// 判断是否在椭圆内（使用乘法形式避免除法，更稳定）
			if (dx * dx * b2 + dz * dz * a2 <= a2 * b2 + tol)
			{
				if (z < z_min + linedis)
				{
					if (abs(ellipse_center_x - x) < a * 0.3)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
					}
					else if (abs(ellipse_center_x - x) >= a * 0.3 && abs(ellipse_center_x - x) < a * 0.5)
					{
						nodeValues.push_back(max_value);
					}
					else if (abs(ellipse_center_x - x) >= a * 0.8 && abs(ellipse_center_x - x) < a)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 7.5 / 10.0);
					}
					else
					{
						nodeValues.push_back(min_value);
					}
				}
				else
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 10.0);
				}
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

		// --- 椭圆参数 ---
		const double ellipse_center_x = (x_min + x_max) / 2.0;
		const double ellipse_center_z = z_min;
		const double a = (x_max - x_min) / 2.0 * 0.8;   // 半长轴（x方向）
		const double b = (z_max - z_min) / 2.0 * 0.8;   // 半短轴（z方向）

		// 预计算平方项，避免重复计算
		const double a2 = a * a;
		const double b2 = b * b;

		//离底边线的距离
		const double linedis = 20;

		const double tol = Precision::Confusion(); // 容差值，用于浮点数比较

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			// 计算相对偏移
			double dx = x - ellipse_center_x;
			double dz = z - ellipse_center_z;

			// 判断是否在椭圆内（使用乘法形式避免除法，更稳定）
			if (dx * dx * b2 + dz * dz * a2 <= a2 * b2 + tol)
			{
				if (z < z_min + linedis)
				{
					if (abs(ellipse_center_x - x) < a * 0.3)
					{
						nodeValues.push_back(min_value );
					}
					else if (abs(ellipse_center_x - x) >= a * 0.3 && abs(ellipse_center_x - x) < a * 0.5)
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 10.0);
					}
					else if (abs(ellipse_center_x - x) >= a * 0.8 && abs(ellipse_center_x - x) < a)
					{
						nodeValues.push_back(max_value);
					}
					else
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5.0 / 10.0);
					}
				}
				else
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
				}
			}
			else
			{
				nodeValues.push_back(min_value + (max_value - min_value) * 5.0 / 10.0);
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

		// --- 2. 根据矩形角点计算椭圆参数 ---

		const double rect_length = x_max - x_min;
		const double rect_width = z_max - z_min;

		const double circle_center_x = (x_min + x_max) / 2.0; // 圆在 x 轴上的中心位置
		const double circle_center_z = (z_min + z_max) / 2.0; // 圆在 z 轴上的中心位置

	
		// 第一个椭圆参数
		const double a = rect_length / 2.0;
		const double b = rect_width / 2.0;

		// 第二个椭圆中心（底边中点）
		const double bottom_center_x = (x_min + x_max) / 2.0;
		const double bottom_center_z = z_min;

		//底线的距离
		const double linedis = 20;

		//红色点的位置离中点的距离
		const double redPoint = rect_length / 2.0 * 3.0 / 4.0;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			// 计算点相对于第一个椭圆的位置
			double dx1 = (x - circle_center_x) / a;
			double dz1 = (z - circle_center_z) / b;
			bool isInFirstEllipse = (dx1 * dx1 + dz1 * dz1 <= 1);

			// 计算点相对于第二个椭圆的位置
			double dx2 = (x - bottom_center_x) / a;
			double dz2 = (z - bottom_center_z) / b;
			bool isInSecondEllipse = (dx2 * dx2 + dz2 * dz2 <= 1);

			if (isInFirstEllipse)
			{
				nodeValues.push_back(min_value);
			}
			else
			{
				if (isInSecondEllipse)
				{
					if (z - z_min <= linedis)
					{
						if (abs(x - bottom_center_x) > redPoint - 30 && abs(x - bottom_center_x) < redPoint + 30)
						{
							nodeValues.push_back(max_value);//红色区域
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 10.0);
						}
					}
					else
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
					}
				}
				else
				{
					nodeValues.push_back(min_value);
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

		auto max_value = sacrificeExplosionAnalysisResultInfo.stressMaxValue / youngModulus;
		auto min_value = sacrificeExplosionAnalysisResultInfo.stressMinValue / youngModulus;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		//点的坐标用0，渲染用90
		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// --- 2. 根据矩形角点计算椭圆参数 ---

		const double rect_length = x_max - x_min;
		const double rect_width = z_max - z_min;

		const double circle_center_x = (x_min + x_max) / 2.0; // 圆在 x 轴上的中心位置
		const double circle_center_z = (z_min + z_max) / 2.0; // 圆在 z 轴上的中心位置


		// 第一个椭圆参数
		const double a = rect_length / 2.0;
		const double b = rect_width / 2.0;

		// 第二个椭圆中心（底边中点）
		const double bottom_center_x = (x_min + x_max) / 2.0;
		const double bottom_center_z = z_min;

		//底线的距离
		const double linedis = 20;

		//红色点的位置离中点的距离
		const double redPoint = rect_length / 2.0 * 3.0 / 4.0;

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			// 计算点相对于第一个椭圆的位置
			double dx1 = (x - circle_center_x) / a;
			double dz1 = (z - circle_center_z) / b;
			bool isInFirstEllipse = (dx1 * dx1 + dz1 * dz1 <= 1);

			// 计算点相对于第二个椭圆的位置
			double dx2 = (x - bottom_center_x) / a;
			double dz2 = (z - bottom_center_z) / b;
			bool isInSecondEllipse = (dx2 * dx2 + dz2 * dz2 <= 1);

			if (isInFirstEllipse)
			{
				nodeValues.push_back(min_value);
			}
			else
			{
				if (isInSecondEllipse)
				{
					if (z - z_min <= linedis)
					{
						if (abs(x - bottom_center_x) > redPoint - 30 && abs(x - bottom_center_x) < redPoint + 30)
						{
							nodeValues.push_back(max_value);//红色区域
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 10.0);
						}
					}
					else
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
					}
				}
				else
				{
					nodeValues.push_back(min_value);
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

		// --- 1. 计算矩形和椭圆参数 ---
		const double rect_length = x_max - x_min;   // 宽度（x方向）
		const double rect_width = z_max - z_min;   // 高度（z方向）

		// 公共半轴长度
		const double a = rect_length / 2.0;         // x方向半轴
		const double b = rect_width / 2.0;         // z方向半轴

		// 椭圆1：底边中点
		const double h1 = (x_min + x_max) / 2.0;
		const double k1 = z_min;

		// 椭圆2：向下偏移 rect_width / 8（注意：rect_width 是 z 方向高度）
		const double h2 = h1; // x 相同
		const double k2 = z_min - rect_width / 8.0;

		// 预计算 a², b²（两个椭圆相同）
		const double a2 = a * a;
		const double b2 = b * b;

		// 阈值：用于判断是否在椭圆内（避免除法）
		const double threshold = a2 * b2;  // 因为 (dx²/a² + dz²/b²) <= 1  ⇔ dx²*b² + dz²*a² <= a²*b²

		//底线的距离
		const double linedis = 20;

		//红色点的位置离中点的距离
		const double redPoint = rect_length / 2.0 * 3.0 / 4.0;

		const double tol = Precision::Confusion(); // OpenCASCADE 微小容差

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double z = nodecoords->Value(nodeID, 3);

			double dx1 = x - h1;
			double dz1 = z - k1;
			double value1 = dx1 * dx1 * b2 + dz1 * dz1 * a2;

			bool inEllipse1 = (value1 <= threshold + tol);

			// --- 判断是否在椭圆2内 ---
			double dx2 = x - h2;
			double dz2 = z - k2;
			double value2 = dx2 * dx2 * b2 + dz2 * dz2 * a2;

			bool inEllipse2 = (value2 <= threshold + tol);

			if (inEllipse1)
			{
				if (!inEllipse2)
				{
					nodeValues.push_back(min_value + (max_value - min_value) * 4.5 / 10.0);
				}
				else
				{
					if (z - z_min <= linedis)
					{
						if (abs(x - h1) > redPoint - 30 && abs(x - h1) < redPoint + 30)
						{
							nodeValues.push_back(max_value);//红色区域
						}
						else
						{
							nodeValues.push_back(min_value + (max_value - min_value) * 6.0 / 10.0);
						}
					}
					else
					{
						nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 10.0);
					}
				}
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
