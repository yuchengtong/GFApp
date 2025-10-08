//// 计算模型的中心点
					//auto CalculateModelCenter = [](const TopoDS_Shape& model) -> gp_Pnt {
					//	GProp_GProps props;
					//	BRepGProp::VolumeProperties(model, props);
					//	return props.CentreOfMass();
					//};

					//// 通过检查首尾点是否重合来判断线框是否封闭
					//auto IsWireClosedByEndPoints = [](const TopoDS_Wire& wire) -> bool {
					//	if (wire.IsNull()) return false;

					//	// 获取所有边
					//	std::vector<TopoDS_Edge> edges;
					//	for (TopExp_Explorer edgeExp(wire, TopAbs_EDGE); edgeExp.More(); edgeExp.Next()) {
					//		edges.push_back(TopoDS::Edge(edgeExp.Current()));
					//	}

					//	if (edges.empty()) return false;

					//	// 获取第一个边的起点
					//	TopoDS_Edge firstEdge = edges.front();
					//	Standard_Real first, last;
					//	Handle(Geom_Curve) firstCurve = BRep_Tool::Curve(firstEdge, first, last);
					//	if (firstCurve.IsNull()) return false;

					//	gp_Pnt startPoint = firstCurve->Value(first);

					//	// 获取最后一个边的终点
					//	TopoDS_Edge lastEdge = edges.back();
					//	Handle(Geom_Curve) lastCurve = BRep_Tool::Curve(lastEdge, first, last);
					//	if (lastCurve.IsNull()) return false;

					//	gp_Pnt endPoint = lastCurve->Value(last);

					//	// 检查首尾点距离
					//	return startPoint.Distance(endPoint) < 1e-6;
					//};

					//// 健壮的封闭性检查函数
					//auto IsWireClosed = [&](const TopoDS_Wire& wire) -> bool {
					//	if (wire.IsNull()) return false;

					//	// 方法1: 尝试创建面来判断是否封闭
					//	try {
					//		BRepBuilderAPI_MakeFace faceMaker(wire, Standard_True);
					//		if (faceMaker.IsDone()) {
					//			return true;
					//		}
					//	}
					//	catch (Standard_Failure&) {
					//		// 方法1失败，继续尝试其他方法
					//	}

					//	// 方法2: 检查首尾点是否重合
					//	return IsWireClosedByEndPoints(wire);
					//};

					//// 修正的连接边到线框的函数，返回线框序列
					//auto ConnectEdgesToWires_Corrected = [&](const TopoDS_Shape& edgesShape)->Handle(TopTools_HSequenceOfShape) {
					//	Handle(TopTools_HSequenceOfShape) wires = new TopTools_HSequenceOfShape();

					//	// 收集所有边到序列
					//	Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape();
					//	for (TopExp_Explorer exp(edgesShape, TopAbs_EDGE); exp.More(); exp.Next()) {
					//		edges->Append(exp.Current());
					//	}

					//	if (edges->IsEmpty()) {
					//		return wires;
					//	}

					//	// 使用静态方法连接边到线框，使用容差1e-6，不共享顶点（通过距离连接）
					//	ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, 1e-6, Standard_False, wires);

					//	return wires;
					//};

					//// 使用BRepAlgoAPI_Section的轮廓提取lambda函数
					//auto GetOuterContour_Lambda = [&](const TopoDS_Shape& model) -> TopoDS_Wire {

					//		// 计算模型的中心点
					//		gp_Pnt modelCenter = CalculateModelCenter(model);

					//		// 创建XOY平面，Z坐标位于模型中心
					//		gp_Pln xoyPlaneAtCenter(gp_Pnt(modelCenter.X(), modelCenter.Y(), modelCenter.Z()), gp_Dir(0, 0, 1));

					//		// 创建平面面，大小足够覆盖整个模型
					//		Bnd_Box bbox;
					//		BRepBndLib::Add(model, bbox);

					//		Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
					//		bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

					//		// 计算平面的边界，比模型稍大
					//		Standard_Real margin = std::max((xMax - xMin), (yMax - yMin)) * 0.1;
					//		Standard_Real xSize = (xMax - xMin) + 2 * margin;
					//		Standard_Real ySize = (yMax - yMin) + 2 * margin;

					//		TopoDS_Face planeFace = BRepBuilderAPI_MakeFace(
					//			xoyPlaneAtCenter,
					//			-xSize / 2, xSize / 2,
					//			-ySize / 2, ySize / 2
					//		);

					//		// 使用BRepAlgoAPI_Section计算截面
					//		BRepAlgoAPI_Section section(planeFace, model, Standard_False);
					//		section.Build();

					//		auto sectionShape = section.Shape();

					//		
					//		// 如果截面为空，返回空线框
					//		if (sectionShape.IsNull()) {
					//			return TopoDS_Wire();
					//		}

					//		// 连接边形成线框
					//		Handle(TopTools_HSequenceOfShape) wires = ConnectEdgesToWires_Corrected(sectionShape);

					//		if (wires->IsEmpty()) {
					//			std::cout << "未能形成线框" << std::endl;
					//			return TopoDS_Wire();
					//		}

					//		// 分派线框到闭合和开放化合物
					//		TopoDS_Compound closedWires, openWires;
					//		BRep_Builder builder;
					//		builder.MakeCompound(closedWires);
					//		builder.MakeCompound(openWires);
					//		ShapeAnalysis_FreeBounds::DispatchWires(wires, closedWires, openWires);

					//		// 找到最大的闭合线框（最外轮廓）
					//		TopoDS_Wire outerWire;
					//		Standard_Real maxArea = -1.0;

					//		for (TopExp_Explorer exp(closedWires, TopAbs_WIRE); exp.More(); exp.Next()) {
					//			TopoDS_Wire wire = TopoDS::Wire(exp.Current());

					//			if (wire.IsNull()) {
					//				continue;
					//			}

					//			// 检查线框是否封闭 - 如果不是封闭的，直接跳过（按理说DispatchWires已经分出了闭合线框，但我们可以再检查一次）
					//			if (!IsWireClosed(wire)) {
					//				std::cout << "跳过非封闭轮廓线" << std::endl;
					//				continue;
					//			}

					//			// 方法1: 尝试创建面并计算几何面积
					//			BRepBuilderAPI_MakeFace tempFace(wire, Standard_True);
					//			if (tempFace.IsDone()) {
					//				try {
					//					GProp_GProps props;
					//					BRepGProp::SurfaceProperties(tempFace.Face(), props);
					//					Standard_Real area = props.Mass();

					//					if (area > maxArea) {
					//						maxArea = area;
					//						outerWire = wire;
					//					}
					//					continue;
					//				}
					//				catch (Standard_Failure&) {
					//					// 面积计算失败，回退到方法2
					//				}
					//			}

					//			// 方法2: 使用边界框面积作为近似
					//			Bnd_Box wireBbox;
					//			BRepBndLib::Add(wire, wireBbox);

					//			if (!wireBbox.IsVoid()) {
					//				Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
					//				wireBbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

					//				Standard_Real bboxArea = (xMax - xMin) * (yMax - yMin);

					//				if (bboxArea > maxArea) {
					//					maxArea = bboxArea;
					//					outerWire = wire;
					//				}
					//			}
					//		}

					//		return outerWire;
					//		

					//};


					//auto occView = gfParent->GetOccView();
					//Handle(AIS_InteractiveContext) context = occView->getContext();
					//auto view = occView->getView();
					//context->RemoveAll(true);

					//auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
					//TopoDS_Shape model_shape = modelInfo.shape;

					//// 提取最外轮廓（自动使用模型中心的XOY平面）
					//auto outerContour = GetOuterContour_Lambda(model_shape);


					//// 2. 计算模型中心（通过边界框中心点）
					//Bnd_Box modelBBox;
					//BRepBndLib::Add(model_shape, modelBBox);
					//if (modelBBox.IsVoid()) {
					//	std::cerr << "模型边界框无效" << std::endl;
					//	return;
					//}
					//Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
					//modelBBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
					//gp_Pnt center((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, 0); // 模型中心点

					//// 3. 创建关于模型中心的对称变换（中心对称）
					//gp_Trsf symTrsf;
					//symTrsf.SetMirror(center); // 关于中心点对称的变换矩阵

					//// 4. 对原始始轮廓应用对称变换，得到对称轮廓
					//BRepBuilderAPI_Transform transform(outerContour, symTrsf);
					//TopoDS_Shape symContourShape = transform.Shape();
					//TopoDS_Wire symContour = TopoDS::Wire(symContourShape);

					//// 5. 合并原始轮廓和对称轮廓为复合形状
					//TopoDS_Compound mergedShapes;
					//BRep_Builder builder;
					//builder.MakeCompound(mergedShapes);
					//builder.Add(mergedShapes, outerContour);   // 添加原始轮廓
					//builder.Add(mergedShapes, symContour);     // 添加对称轮廓

					//// 6. 显示合并后的复合形状（同时包含原始和对称轮廓）
					//Handle(AIS_Shape) aisMerged = new AIS_Shape(mergedShapes);
					//// 可选：统一设置合并后形状的显示样式（若子形状无单独样式）
					//aisMerged->SetColor(Quantity_Color(Quantity_NOC_BLACK));  // 统一用黑色
					//aisMerged->SetWidth(2.0);                                 // 统一线宽
					//context->Display(aisMerged, Standard_True);               // 显示复合形状

					//// 调整视图以适配所有内容
					//view->FitAll();



					//if (!outerContour.IsNull()) {
					//	// 检查提取的轮廓是否封闭
					//	if (IsWireClosed(outerContour)) {
					//		std::cout << "成功提取封闭的最外轮廓线" << std::endl;

					//		// 显示轮廓
					//		Handle(AIS_Shape) aisContour = new AIS_Shape(outerContour);
					//		aisContour->SetColor(Quantity_Color(Quantity_NOC_RED));
					//		aisContour->SetWidth(3.0);
					//		context->Display(aisContour, Standard_True);

					//		// 显示原始模型（半透明）
					//		Handle(AIS_Shape) aisModel = new AIS_Shape(model_shape);
					//		aisModel->SetTransparency(0.9);
					//		context->Display(aisModel, Standard_True);

					//		view->FitAll();

					//	}
					//}