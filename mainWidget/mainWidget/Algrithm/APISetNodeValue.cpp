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
	

	// 仅处理 double 类型的 clamp 函数
	auto my_clamp = [](double value, double low, double high) {
		if (value < low)
			return low;
		if (value > high)
			return high;
		return value;
	};

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

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

			// 从角点计算矩形边界参数
			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;       
			const double ellipse_k = z_min + (x_max - x_min)/5;   
			const double rect_length = x_max - x_min;             
			const double rect_width = z_max - z_min;              
			const double ellipse_a = rect_length * 0.8 / 2.0;     
			const double ellipse_b = rect_width * 0.6 / 2.0;      

			double red_line_z = z_min+250;
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
				if (z < red_line_z)
				{
					nodeValues.push_back(max_value);
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
			int a = 0;
		}
		else if (angle == 45)
		{
			//点的坐标用0，渲染用45
			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

			aMesh = new MeshVS_Mesh();
			aMesh->SetDataSource(&modelMeshInfo.triangleStructure45);

			// 从角点计算矩形边界参数
			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = (z_min + z_max) / 2.0;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length  / 2.0;
			const double ellipse_b = rect_width  / 2.0;

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
					if (z < z_max + 50 && z>z_max - 50 && x< x_max + 50 && x>x_max - 50)
					{
						nodeValues.push_back(max_value);
					}
					else
					{
						nodeValues.push_back(max_value * 0.5);
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

			// 从角点计算矩形边界参数
			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

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

				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					if (x< x_max + 50 && x>x_max - 50)
					{
						nodeValues.push_back(max_value);
					}
					else if (x > ellipse_h)
					{
						nodeValues.push_back(0.6*max_value);
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
	return true;
}

bool APISetNodeValue::SetFallTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();


	// 仅处理 double 类型的 clamp 函数
	auto my_clamp = [](double value, double low, double high) {
		if (value < low)
			return low;
		if (value > high)
			return high;
		return value;
	};

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;

	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

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

			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = z_min + (x_max - x_min) / 5;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length * 0.8 / 2.0;
			const double ellipse_b = rect_width * 0.6 / 2.0;

			double red_line_z = z_min + 250;
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
				else
				{
					if (value <= threshold + Precision::Confusion())
					{
						nodeValues.push_back(0.5 * max_value);
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

			// 从角点计算矩形边界参数
			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

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
					if (x > ellipse_h && z < ellipse_k)
					{
						nodeValues.push_back(max_value * 0.3);
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

		// 从角点计算矩形边界参数
		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

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
				if (x< x_max + 50 && x>x_max - 50)
				{
					nodeValues.push_back(max_value);
				}
				else if (x > ellipse_h)
				{
					nodeValues.push_back(0.6 * max_value);
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

	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

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

			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;
			const double ellipse_k = z_min + (x_max - x_min) / 5;
			const double rect_length = x_max - x_min;
			const double rect_width = z_max - z_min;
			const double ellipse_a = rect_length * 0.8 / 2.0;
			const double ellipse_b = rect_width * 0.6 / 2.0;

			double red_line_z = z_min + 250;
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
				else 
				{
					if (value < threshold + Precision::Confusion())
					{
						nodeValues.push_back(0.5 * max_value);
					}
					else if (value == threshold + Precision::Confusion()&&abs(z- ellipse_h)<50)
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

			// 从角点计算矩形边界参数
			const double x_min = modelGeometryInfo.theXmin;
			const double x_max = modelGeometryInfo.theXmax;
			const double z_min = modelGeometryInfo.theZmin;
			const double z_max = modelGeometryInfo.theZmax;

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

				if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(max_value * 0.4);
				}
				else
				{
					if (x > ellipse_h && z < ellipse_k)
					{
						if (x< x_max + 50 && x>x_max - 130)
						{
							nodeValues.push_back(max_value);
						}
						else
						{
							nodeValues.push_back(max_value * 0.2);
						}
					}
					else
					{
						nodeValues.push_back(max_value * 0.4);
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

		// 从角点计算矩形边界参数
		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

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
				nodeValues.push_back(max_value*0.3);
			}
			else
			{
				if (x< x_max + 50 && x>x_max - 50)
				{
					nodeValues.push_back(max_value);
				}
				else if (x > ellipse_h)
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					nodeValues.push_back(max_value * 0.3);
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

bool APISetNodeValue::SetShootStressResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto shootSettingInfo=ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto shootAnalysisResultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();
	

	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

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

		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 4.0;

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
			else
			{
				if (dist_sq < circle_radius_sq+50*50)
				{
					nodeValues.push_back(max_value*0.5);
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

bool APISetNodeValue::SetShootStrainResult(OccView* occView, std::vector<double>& nodeValues)
{
	return false;
}

bool APISetNodeValue::SetShootTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto shootSettingInfo = ModelDataManager::GetInstance()->GetShootSettingInfo();
	auto shootAnalysisResultInfo = ModelDataManager::GetInstance()->GetShootAnalysisResultInfo();


	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

	if (shootAnalysisResultInfo.isChecked)
	{
		//点的坐标用0，渲染用90
		auto allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		auto nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		auto max_value = shootAnalysisResultInfo.temperatureMaxValue;
		auto min_value = shootAnalysisResultInfo.temperatureMinValue;

		auto aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);

		// 从角点计算矩形边界参数
		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

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

			if (value <= threshold + Precision::Confusion())
			{
				nodeValues.push_back(min_value);
			}
			else
			{
				if (x< x_max + 50 && x>x_max - 50)
				{
					nodeValues.push_back(max_value);
				}
				else if (x > ellipse_h)
				{
					nodeValues.push_back(0.6 * max_value);
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


	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

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

		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 5.0;

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
			else
			{
				nodeValues.push_back(max_value*0.5);
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


	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

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
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min+100;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 4.0;

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
	return false;
}

bool APISetNodeValue::SetFragmentationTemperatureResult(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	auto fragmentationSettingInfo = ModelDataManager::GetInstance()->GetFragmentationSettingInfo();
	auto fragmentationAnalysisResultInfo = ModelDataManager::GetInstance()->GetFragmentationAnalysisResultInfo();


	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

	if (fragmentationAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		//auto max_value = fragmentationAnalysisResultInfo.temperatureMaxValue;
		//auto min_value = fragmentationAnalysisResultInfo.temperatureMinValue;
		double max_value = 100;
		double min_value = 0;

		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

		// 从角点计算矩形边界参数
		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

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

			if (value <= threshold + Precision::Confusion())
			{
				nodeValues.push_back(min_value);
			}
			else
			{
				if (x< x_max + 50 && x>x_max - 50)
				{
					nodeValues.push_back(max_value);
				}
				else if (x > ellipse_h)
				{
					nodeValues.push_back(0.6 * max_value);
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


	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	Point p0{ (modelGeometryInfo.theXmin + modelGeometryInfo.theXmax) / 2.0,
		(modelGeometryInfo.theZmin + modelGeometryInfo.theZmax) / 2.0 };
	Point p1{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmin };
	Point p2{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmin };
	Point p3{ modelGeometryInfo.theXmax, modelGeometryInfo.theZmax };
	Point p4{ modelGeometryInfo.theXmin, modelGeometryInfo.theZmax };

	if (fragmentationAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		//auto max_value = fragmentationAnalysisResultInfo.stressMaxValue;
		//auto min_value = fragmentationAnalysisResultInfo.stressMinValue;
		double max_value = 100;
		double min_value = 0;


		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

		const double x_min = modelGeometryInfo.theXmin;
		const double x_max = modelGeometryInfo.theXmax;
		const double z_min = modelGeometryInfo.theZmin;
		const double z_max = modelGeometryInfo.theZmax;

		const double circle_center_x = (x_min + x_max) / 2.0;
		const double circle_center_z = z_min;

		// 计算矩形宽度 (x方向的长度)
		const double rect_width = z_max - z_min;

		// 计算圆的半径 (矩形宽度的1/4)
		const double circle_radius = rect_width / 4.0;

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
			else
			{
				nodeValues.push_back(max_value * 0.5);
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
