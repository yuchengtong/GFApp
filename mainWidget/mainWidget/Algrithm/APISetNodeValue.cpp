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

	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;


	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		allnode = fallAnalysisResultInfo.triangleStructure.GetAllNodes();
		nodecoords = fallAnalysisResultInfo.triangleStructure.GetmyNodeCoords();

		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;

		//std::vector<double> nodeValues;

		Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&fallAnalysisResultInfo.triangleStructure);

		
		Point topLeft, bottomLeft, topRight, bottomRight; // 存储四个角点坐标
		bool isFirstNode = true;

		// 使用 OCCT 的迭代器遍历所有节点 ID
		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			const Standard_Integer nodeID = it.Key();
			if (nodeID < nodecoords->LowerRow() || nodeID > nodecoords->UpperRow())
			{
				continue;
			}

			// 从坐标数组中获取当前节点的 X 和 Z 坐标
			// 假设列 1 = X, 列 2 = Y, 列 3 = Z
			const Standard_Real x = nodecoords->Value(nodeID, 1);
			const Standard_Real z = nodecoords->Value(nodeID, 3);

			// 封装成 Point 结构体方便操作
			Point currPoint = { x, z };

			if (isFirstNode)
			{
				// 如果是第一个有效节点，初始化所有角点
				topLeft = currPoint;
				bottomLeft = currPoint;
				topRight = currPoint;
				bottomRight = currPoint;

				isFirstNode = Standard_False;
			}
			else
			{
				// --- 更新最左的点 (x 坐标最小) ---
				if (currPoint.x < topLeft.x - Precision::Confusion())
				{
					// 找到一个更靠左的点，更新左上和左下
					topLeft = currPoint;
					bottomLeft = currPoint;
				}
				else if (Abs(currPoint.x - topLeft.x) < Precision::Confusion())
				{
					// 如果 x 坐标相同（考虑浮点精度），再比较 z 坐标
					if (currPoint.z > topLeft.z + Precision::Confusion())
					{
						topLeft = currPoint;
					}
					if (currPoint.z < bottomLeft.z - Precision::Confusion())
					{
						bottomLeft = currPoint;
					}
				}

				// --- 更新最右的点 (x 坐标最大) ---
				if (currPoint.x > topRight.x + Precision::Confusion())
				{
					// 找到一个更靠右的点，更新右上和右下
					topRight = currPoint;
					bottomRight = currPoint;
				}
				else if (Abs(currPoint.x - topRight.x) < Precision::Confusion())
				{
					// 如果 x 坐标相同（考虑浮点精度），再比较 z 坐标
					if (currPoint.z > topRight.z + Precision::Confusion())
					{
						topRight = currPoint;
					}
					if (currPoint.z < bottomRight.z - Precision::Confusion())
					{
						bottomRight = currPoint;
					}
				}
			}
		}

		if (angle == 0)
		{
			// 从角点计算矩形边界参数
			const double x_min = topLeft.x;                  // 左边界（左上/左下 x 坐标）
			const double x_max = topRight.x;                 // 右边界（右上/右下 x 坐标）
			const double z_min = bottomLeft.z;               // 下边界（左下/右下 z 坐标）
			const double z_max = topLeft.z;                  // 上边界（左上/右上 z 坐标）

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;       // 椭圆中心 X = 矩形中心 X
			const double ellipse_k = z_min + 100.0;               // 椭圆中心 Z = z_min + 100
			const double rect_length = x_max - x_min;             // 矩形长度（x方向）
			const double rect_width = z_max - z_min;              // 矩形宽度（z方向）
			const double ellipse_a = rect_length * 0.8 / 2.0;     // 椭圆长半轴 = 矩形长度×0.8/2
			const double ellipse_b = rect_width * 0.6 / 2.0;      // 椭圆短半轴 = 矩形宽度×0.6/2

			double red_line_z = z_min+10;
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
				if (z < red_line_z)
				{
					nodeValues.push_back(max_value);
				}
				else if (value <= threshold + Precision::Confusion()) 
				{
					nodeValues.push_back(0.3*max_value);
				}
				else
				{
					nodeValues.push_back(min_value);
				}				
			}
		}
		else if (angle == 45)
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

	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;


	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		allnode = fallAnalysisResultInfo.triangleStructure.GetAllNodes();
		nodecoords = fallAnalysisResultInfo.triangleStructure.GetmyNodeCoords();

		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;

		//std::vector<double> nodeValues;

		Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&fallAnalysisResultInfo.triangleStructure);


		Point topLeft, bottomLeft, topRight, bottomRight; // 存储四个角点坐标
		bool isFirstNode = true;

		// 使用 OCCT 的迭代器遍历所有节点 ID
		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			const Standard_Integer nodeID = it.Key();
			if (nodeID < nodecoords->LowerRow() || nodeID > nodecoords->UpperRow())
			{
				continue;
			}

			// 从坐标数组中获取当前节点的 X 和 Z 坐标
			// 假设列 1 = X, 列 2 = Y, 列 3 = Z
			const Standard_Real x = nodecoords->Value(nodeID, 1);
			const Standard_Real z = nodecoords->Value(nodeID, 3);

			// 封装成 Point 结构体方便操作
			Point currPoint = { x, z };

			if (isFirstNode)
			{
				// 如果是第一个有效节点，初始化所有角点
				topLeft = currPoint;
				bottomLeft = currPoint;
				topRight = currPoint;
				bottomRight = currPoint;

				isFirstNode = Standard_False;
			}
			else
			{
				// --- 更新最左的点 (x 坐标最小) ---
				if (currPoint.x < topLeft.x - Precision::Confusion())
				{
					// 找到一个更靠左的点，更新左上和左下
					topLeft = currPoint;
					bottomLeft = currPoint;
				}
				else if (Abs(currPoint.x - topLeft.x) < Precision::Confusion())
				{
					// 如果 x 坐标相同（考虑浮点精度），再比较 z 坐标
					if (currPoint.z > topLeft.z + Precision::Confusion())
					{
						topLeft = currPoint;
					}
					if (currPoint.z < bottomLeft.z - Precision::Confusion())
					{
						bottomLeft = currPoint;
					}
				}

				// --- 更新最右的点 (x 坐标最大) ---
				if (currPoint.x > topRight.x + Precision::Confusion())
				{
					// 找到一个更靠右的点，更新右上和右下
					topRight = currPoint;
					bottomRight = currPoint;
				}
				else if (Abs(currPoint.x - topRight.x) < Precision::Confusion())
				{
					// 如果 x 坐标相同（考虑浮点精度），再比较 z 坐标
					if (currPoint.z > topRight.z + Precision::Confusion())
					{
						topRight = currPoint;
					}
					if (currPoint.z < bottomRight.z - Precision::Confusion())
					{
						bottomRight = currPoint;
					}
				}
			}
		}

		if (angle == 0)
		{
			// 从角点计算矩形边界参数
			const double x_min = topLeft.x;                  // 左边界（左上/左下 x 坐标）
			const double x_max = topRight.x;                 // 右边界（右上/右下 x 坐标）
			const double z_min = bottomLeft.z;               // 下边界（左下/右下 z 坐标）
			const double z_max = topLeft.z;                  // 上边界（左上/右上 z 坐标）

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;       // 椭圆中心 X = 矩形中心 X
			const double ellipse_k = z_min + 100.0;               // 椭圆中心 Z = z_min + 100
			const double rect_length = x_max - x_min;             // 矩形长度（x方向）
			const double rect_width = z_max - z_min;              // 矩形宽度（z方向）
			const double ellipse_a = rect_length * 0.8 / 2.0;     // 椭圆长半轴 = 矩形长度×0.8/2
			const double ellipse_b = rect_width * 0.6 / 2.0;      // 椭圆短半轴 = 矩形宽度×0.6/2

			double red_line_z = z_min + 10;
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
				if (z < red_line_z)
				{
					nodeValues.push_back(max_value);
				}
				else if (value <= threshold + Precision::Confusion())
				{
					nodeValues.push_back(0.45 * max_value);
				}
				else
				{
					nodeValues.push_back(min_value);
				}
			}
		}
		else if (angle == 45)
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

	}

	return true;
}

bool APISetNodeValue::SetFallOverpressureResult(OccView* occView, std::vector<double>& nodeValues)
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

	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();

	auto high = fallSettingInfo.high;
	auto angle = fallSettingInfo.angle;
	auto youngModulus = steelPropertyInfoInfo.modulus;


	if (fallAnalysisResultInfo.isChecked)
	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;

		allnode = fallAnalysisResultInfo.triangleStructure.GetAllNodes();
		nodecoords = fallAnalysisResultInfo.triangleStructure.GetmyNodeCoords();

		auto max_value = fallAnalysisResultInfo.stressMaxValue;
		auto min_value = fallAnalysisResultInfo.stressMinValue;

		//std::vector<double> nodeValues;

		Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&fallAnalysisResultInfo.triangleStructure);


		Point topLeft, bottomLeft, topRight, bottomRight; // 存储四个角点坐标
		bool isFirstNode = true;

		// 使用 OCCT 的迭代器遍历所有节点 ID
		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			const Standard_Integer nodeID = it.Key();
			if (nodeID < nodecoords->LowerRow() || nodeID > nodecoords->UpperRow())
			{
				continue;
			}

			// 从坐标数组中获取当前节点的 X 和 Z 坐标
			// 假设列 1 = X, 列 2 = Y, 列 3 = Z
			const Standard_Real x = nodecoords->Value(nodeID, 1);
			const Standard_Real z = nodecoords->Value(nodeID, 3);

			// 封装成 Point 结构体方便操作
			Point currPoint = { x, z };

			if (isFirstNode)
			{
				// 如果是第一个有效节点，初始化所有角点
				topLeft = currPoint;
				bottomLeft = currPoint;
				topRight = currPoint;
				bottomRight = currPoint;

				isFirstNode = Standard_False;
			}
			else
			{
				// --- 更新最左的点 (x 坐标最小) ---
				if (currPoint.x < topLeft.x - Precision::Confusion())
				{
					// 找到一个更靠左的点，更新左上和左下
					topLeft = currPoint;
					bottomLeft = currPoint;
				}
				else if (Abs(currPoint.x - topLeft.x) < Precision::Confusion())
				{
					// 如果 x 坐标相同（考虑浮点精度），再比较 z 坐标
					if (currPoint.z > topLeft.z + Precision::Confusion())
					{
						topLeft = currPoint;
					}
					if (currPoint.z < bottomLeft.z - Precision::Confusion())
					{
						bottomLeft = currPoint;
					}
				}

				// --- 更新最右的点 (x 坐标最大) ---
				if (currPoint.x > topRight.x + Precision::Confusion())
				{
					// 找到一个更靠右的点，更新右上和右下
					topRight = currPoint;
					bottomRight = currPoint;
				}
				else if (Abs(currPoint.x - topRight.x) < Precision::Confusion())
				{
					// 如果 x 坐标相同（考虑浮点精度），再比较 z 坐标
					if (currPoint.z > topRight.z + Precision::Confusion())
					{
						topRight = currPoint;
					}
					if (currPoint.z < bottomRight.z - Precision::Confusion())
					{
						bottomRight = currPoint;
					}
				}
			}
		}

		if (angle == 0)
		{
			// 从角点计算矩形边界参数
			const double x_min = topLeft.x;                  // 左边界（左上/左下 x 坐标）
			const double x_max = topRight.x;                 // 右边界（右上/右下 x 坐标）
			const double z_min = bottomLeft.z;               // 下边界（左下/右下 z 坐标）
			const double z_max = topLeft.z;                  // 上边界（左上/右上 z 坐标）

			// --- 2. 根据矩形角点计算椭圆参数 ---
			const double ellipse_h = (x_min + x_max) / 2.0;       // 椭圆中心 X = 矩形中心 X
			const double ellipse_k = z_min + 100.0;               // 椭圆中心 Z = z_min + 100
			const double rect_length = x_max - x_min;             // 矩形长度（x方向）
			const double rect_width = z_max - z_min;              // 矩形宽度（z方向）
			const double ellipse_a = rect_length * 0.8 / 2.0;     // 椭圆长半轴 = 矩形长度×0.8/2
			const double ellipse_b = rect_width * 0.6 / 2.0;      // 椭圆短半轴 = 矩形宽度×0.6/2

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
				else if (value < threshold + Precision::Confusion())
				{
					nodeValues.push_back(0.3 * max_value);
				}
				else if (value == threshold + Precision::Confusion())
				{
					nodeValues.push_back(min_value);
				}
				else
				{
					nodeValues.push_back(0.2*min_value);
				}
			}
		}
		else if (angle == 45)
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
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, true);

	}

	return true;
}
