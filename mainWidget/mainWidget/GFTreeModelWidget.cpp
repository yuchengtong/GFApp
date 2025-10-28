#pragma execution_character_set("utf-8")
#include "GFTreeModelWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QIcon>
#include <QFileDialog>
#include <QDateTime>
#include <algorithm>
#include <QRegExp>
#include <QRegularExpression> 
#include <QValidator>
#include <QThread>


#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>

#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx> 

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>

#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include "GFImportModelWidget.h"
#include "TriangleStructure.h"
#include "occView.h"
#include "ModelDataManager.h"
#include "ProgressDialog.h"
#include "GeometryImportWorker.h"
#include "WordExporterWorker.h"
#include <HLRBRep_Algo.hxx>
#include <BRepProj_Projection.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <GProp_GProps.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include <BRepAlgoAPI_Section.hxx> 
#include <ShapeAnalysis_FreeBounds.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <Geom2d_Curve.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Bnd_Box2d.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <Geom_Plane.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Common.hxx>
// 仅处理 double 类型的 clamp 函数
double my_clamp(double value, double low, double high) {
	if (value < low) return low;
	if (value > high) return high;
	return value;
}

double getAverage(std::vector<double> data) {
	return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

double getStd(std::vector<double> data) {
	double mean = getAverage(data);
	double accum = 0.0;
	std::for_each(std::begin(data), std::end(data), [&](const double d) {
		accum += (d - mean) * (d - mean);
	});
	return sqrt(accum / data.size()); //除以n-1是无偏估计方差, 除以n是概率分布方差, 都行
}

GFTreeModelWidget::GFTreeModelWidget(QWidget*parent)
	:QWidget(parent)
{
	wordExporter = new WordExporter(this);

	qRegisterMetaType<ModelGeometryInfo>("ModelGeometryInfo");

	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");

	treeWidget = new GFTreeWidget(this);
	//treeWidget->setStyleSheet(R"(
	// QTreeWidget::branch:has-children:!has-siblings:closed,
	// QTreeWidget::branch:closed:has-children:has-siblings {
	//         border-image: none;
	//         image: url(:/src/treeclose.svg);
	// }
	//
	// QTreeWidget::branch:open:has-children:!has-siblings,
	// QTreeWidget::branch:open:has-children:has-siblings  {
	//         border-image: none;
	//         image: url(:/src/treeopen.svg);
	// }
	//)");

	treeWidget->setColumnCount(1);
	treeWidget->setHeaderLabels({ "项目结构" });
	treeWidget->setHeaderHidden(true);

	// 创建根节点
	QTreeWidgetItem*rootItem = new QTreeWidgetItem(treeWidget);
	rootItem->setText(0, "工程文件");
	rootItem->setData(0, Qt::UserRole, "Project");
	rootItem->setExpanded(true);
	//rootItem->setIcon(0, icon);

	// 几何模型节点
	QTreeWidgetItem* geometryNode = new QTreeWidgetItem(rootItem);
	geometryNode->setText(0, "几何模型");
	geometryNode->setData(0, Qt::UserRole, "Geometry");
	geometryNode->setIcon(0, error_icon);

	// 材料节点
	QTreeWidgetItem* databaseNode = new QTreeWidgetItem(rootItem);
	databaseNode->setText(0, "数据库");
	databaseNode->setData(0, Qt::UserRole, "Database");
	databaseNode->setIcon(0, error_icon);

	QTreeWidgetItem* materialNode = new QTreeWidgetItem();
	materialNode->setText(0, "材料库");
	materialNode->setData(0, Qt::UserRole, "Material");
	materialNode->setIcon(0, error_icon);

	databaseNode->addChild(materialNode);


	QTreeWidgetItem* steel = new QTreeWidgetItem();
	steel->setText(0, "壳体材料");
	steel->setData(0, Qt::UserRole, "Steel");
	steel->setIcon(0, error_icon);

	QTreeWidgetItem* propellant = new QTreeWidgetItem();
	propellant->setText(0, "推进剂材料");
	propellant->setData(0, Qt::UserRole, "Propellant");
	propellant->setIcon(0, error_icon);

	QTreeWidgetItem* outheat = new QTreeWidgetItem();
	outheat->setText(0, "外防热材料");
	outheat->setData(0, Qt::UserRole, "Outheat");
	outheat->setIcon(0, error_icon);

	QTreeWidgetItem* insulatingheat = new QTreeWidgetItem();
	insulatingheat->setText(0, "防隔热材料");
	insulatingheat->setData(0, Qt::UserRole, "Insulatingheat");
	insulatingheat->setIcon(0, error_icon);

	materialNode->addChild(steel);
	materialNode->addChild(propellant);
	materialNode->addChild(outheat);
	materialNode->addChild(insulatingheat);

	QTreeWidgetItem* judgment = new QTreeWidgetItem();
	judgment->setText(0, "评判标准数据库");
	judgment->setData(0, Qt::UserRole, "Judgment");
	judgment->setIcon(0, error_icon);

	QTreeWidgetItem* calculation = new QTreeWidgetItem();
	calculation->setText(0, "计算模型数据库");
	calculation->setData(0, Qt::UserRole, "Calculation");
	calculation->setIcon(0, checked_icon);

	
	databaseNode->addChild(judgment);
	databaseNode->addChild(calculation);

	//网格节点
	QTreeWidgetItem* meshItem = new QTreeWidgetItem(rootItem);
	meshItem->setText(0, "网格");
	meshItem->setData(0, Qt::UserRole, "Mesh");
	meshItem->setIcon(0, error_icon);

	// 分析设置节点
	QTreeWidgetItem* analysisNode = new QTreeWidgetItem(rootItem);
	analysisNode->setText(0, "工况设置");
	analysisNode->setData(0, Qt::UserRole, "Analysis");
	analysisNode->setIcon(0, error_icon);

	QTreeWidgetItem* fallAnalysis = new QTreeWidgetItem();
	fallAnalysis->setText(0, "1.跌落试验");
	fallAnalysis->setData(0, Qt::UserRole, "FallAnalysis");
	//fallAnalysis->setCheckState(0, Qt::Unchecked);
	fallAnalysis->setIcon(0, error_icon);


	QTreeWidgetItem* stressResult = new QTreeWidgetItem();
	stressResult->setText(0, "应力分析");
	stressResult->setData(0, Qt::UserRole, "StressResult");
	stressResult->setIcon(0, error_icon);

	QTreeWidgetItem* strainResult = new QTreeWidgetItem();
	strainResult->setText(0, "应变分析");
	strainResult->setData(0, Qt::UserRole, "StrainResult");
	strainResult->setIcon(0, error_icon);

	QTreeWidgetItem* temperatureResult = new QTreeWidgetItem();
	temperatureResult->setText(0, "温度分析");
	temperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	temperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* overpressureResult = new QTreeWidgetItem();
	overpressureResult->setText(0, "超压分析");
	overpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	overpressureResult->setIcon(0, error_icon);

	fallAnalysis->addChild(stressResult);
	fallAnalysis->addChild(strainResult);
	fallAnalysis->addChild(temperatureResult);
	fallAnalysis->addChild(overpressureResult);

	QTreeWidgetItem* fastCombustionAnalysis = new QTreeWidgetItem();
	fastCombustionAnalysis->setText(0, "2.快速烤燃试验");
	fastCombustionAnalysis->setData(0, Qt::UserRole, "FastCombustionAnalysis");
	//fastCombustionAnalysis->setCheckState(0, Qt::Unchecked);
	fastCombustionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* fastCombustionTemperatureResult = new QTreeWidgetItem();
	fastCombustionTemperatureResult->setText(0, "温度分析");
	fastCombustionTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	fastCombustionTemperatureResult->setIcon(0, error_icon);

	fastCombustionAnalysis->addChild(fastCombustionTemperatureResult);


	QTreeWidgetItem* slowCombustionAnalysis = new QTreeWidgetItem();
	slowCombustionAnalysis->setText(0, "3.慢速烤燃试验");
	slowCombustionAnalysis->setData(0, Qt::UserRole, "SlowCombustionAnalysis");
	//slowCombustionAnalysis->setCheckState(0, Qt::Unchecked);
	slowCombustionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* slowCombustionTemperatureResult = new QTreeWidgetItem();
	slowCombustionTemperatureResult->setText(0, "温度分析");
	slowCombustionTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	slowCombustionTemperatureResult->setIcon(0, error_icon);

	slowCombustionAnalysis->addChild(slowCombustionTemperatureResult);

	
	QTreeWidgetItem* shootAnalysis = new QTreeWidgetItem();
	shootAnalysis->setText(0, "4.枪击试验");
	shootAnalysis->setData(0, Qt::UserRole, "ShootAnalysis");
	//shootAnalysis->setCheckState(0, Qt::Unchecked);
	shootAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* shootStressResult = new QTreeWidgetItem();
	shootStressResult->setText(0, "应力分析");
	shootStressResult->setData(0, Qt::UserRole, "StressResult");
	shootStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootStrainResult = new QTreeWidgetItem();
	shootStrainResult->setText(0, "应变分析");
	shootStrainResult->setData(0, Qt::UserRole, "StrainResult");
	shootStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootTemperatureResult = new QTreeWidgetItem();
	shootTemperatureResult->setText(0, "温度分析");
	shootTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	shootTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootOverpressureResult = new QTreeWidgetItem();
	shootOverpressureResult->setText(0, "超压分析");
	shootOverpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	shootOverpressureResult->setIcon(0, error_icon);

	shootAnalysis->addChild(shootStressResult);
	shootAnalysis->addChild(shootStrainResult);
	shootAnalysis->addChild(shootTemperatureResult);
	shootAnalysis->addChild(shootOverpressureResult);


	QTreeWidgetItem* jetImpactAnalysis = new QTreeWidgetItem();
	jetImpactAnalysis->setText(0, "5.射流冲击试验");
	jetImpactAnalysis->setData(0, Qt::UserRole, "JetImpactAnalysis");
	//jetImpactAnalysis->setCheckState(0, Qt::Unchecked);
	jetImpactAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactStressResult = new QTreeWidgetItem();
	jetImpactStressResult->setText(0, "应力分析");
	jetImpactStressResult->setData(0, Qt::UserRole, "StressResult");
	jetImpactStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetStrainResult = new QTreeWidgetItem();
	jetStrainResult->setText(0, "应变分析");
	jetStrainResult->setData(0, Qt::UserRole, "StrainResult");
	jetStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactTemperatureResult = new QTreeWidgetItem();
	jetImpactTemperatureResult->setText(0, "温度分析");
	jetImpactTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	jetImpactTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactOverpressureResult = new QTreeWidgetItem();
	jetImpactOverpressureResult->setText(0, "超压分析");
	jetImpactOverpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	jetImpactOverpressureResult->setIcon(0, error_icon);

	jetImpactAnalysis->addChild(jetImpactStressResult);
	jetImpactAnalysis->addChild(jetStrainResult);
	jetImpactAnalysis->addChild(jetImpactTemperatureResult);
	jetImpactAnalysis->addChild(jetImpactOverpressureResult);


	QTreeWidgetItem* fragmentationImpactAnalysis = new QTreeWidgetItem();
	fragmentationImpactAnalysis->setText(0, "6.破片撞击试验");
	fragmentationImpactAnalysis->setData(0, Qt::UserRole, "FragmentationImpactAnalysis");
	//fragmentationImpactAnalysis->setCheckState(0, Qt::Unchecked);
	fragmentationImpactAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactStressResult = new QTreeWidgetItem();
	fragmentationImpactStressResult->setText(0, "应力分析");
	fragmentationImpactStressResult->setData(0, Qt::UserRole, "StressResult");
	fragmentationImpactStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationStrainResult = new QTreeWidgetItem();
	fragmentationStrainResult->setText(0, "应变分析");
	fragmentationStrainResult->setData(0, Qt::UserRole, "StrainResult");
	fragmentationStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactTemperatureResult = new QTreeWidgetItem();
	fragmentationImpactTemperatureResult->setText(0, "温度分析");
	fragmentationImpactTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	fragmentationImpactTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactOverpressureResult = new QTreeWidgetItem();
	fragmentationImpactOverpressureResult->setText(0, "超压分析");
	fragmentationImpactOverpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	fragmentationImpactOverpressureResult->setIcon(0, error_icon);

	fragmentationImpactAnalysis->addChild(fragmentationImpactStressResult);
	fragmentationImpactAnalysis->addChild(fragmentationStrainResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactTemperatureResult);
	fragmentationImpactAnalysis->addChild(fragmentationImpactOverpressureResult);


	QTreeWidgetItem* explosiveBlastAnalysis = new QTreeWidgetItem();
	explosiveBlastAnalysis->setText(0, "7.爆炸冲击波试验");
	explosiveBlastAnalysis->setData(0, Qt::UserRole, "ExplosiveBlastAnalysis");
	//explosiveBlastAnalysis->setCheckState(0, Qt::Unchecked);
	explosiveBlastAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastStressResult = new QTreeWidgetItem();
	explosiveBlastStressResult->setText(0, "应力分析");
	explosiveBlastStressResult->setData(0, Qt::UserRole, "StressResult");
	explosiveBlastStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastStrainResult = new QTreeWidgetItem();
	explosiveBlastStrainResult->setText(0, "应变分析");
	explosiveBlastStrainResult->setData(0, Qt::UserRole, "StrainResult");
	explosiveBlastStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastTemperatureResult = new QTreeWidgetItem();
	explosiveBlastTemperatureResult->setText(0, "温度分析");
	explosiveBlastTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	explosiveBlastTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastOverpressureResult = new QTreeWidgetItem();
	explosiveBlastOverpressureResult->setText(0, "超压分析");
	explosiveBlastOverpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	explosiveBlastOverpressureResult->setIcon(0, error_icon);

	explosiveBlastAnalysis->addChild(explosiveBlastStressResult);
	explosiveBlastAnalysis->addChild(explosiveBlastStrainResult);
	explosiveBlastAnalysis->addChild(explosiveBlastTemperatureResult);
	explosiveBlastAnalysis->addChild(explosiveBlastOverpressureResult);


	QTreeWidgetItem* sacrificeExplosionAnalysis = new QTreeWidgetItem();
	sacrificeExplosionAnalysis->setText(0, "8.殉爆试验");
	sacrificeExplosionAnalysis->setData(0, Qt::UserRole, "SacrificeExplosionAnalysis");
	//sacrificeExplosionAnalysis->setCheckState(0, Qt::Unchecked);
	sacrificeExplosionAnalysis->setIcon(0, error_icon);
	
	QTreeWidgetItem* sacrificeExplosionStressResult = new QTreeWidgetItem();
	sacrificeExplosionStressResult->setText(0, "应力分析");
	sacrificeExplosionStressResult->setData(0, Qt::UserRole, "StressResult");
	sacrificeExplosionStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionStrainResult = new QTreeWidgetItem();
	sacrificeExplosionStrainResult->setText(0, "应变分析");
	sacrificeExplosionStrainResult->setData(0, Qt::UserRole, "StrainResult");
	sacrificeExplosionStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionTemperatureResult = new QTreeWidgetItem();
	sacrificeExplosionTemperatureResult->setText(0, "温度分析");
	sacrificeExplosionTemperatureResult->setData(0, Qt::UserRole, "TemperatureResult");
	sacrificeExplosionTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionOverpressureResult = new QTreeWidgetItem();
	sacrificeExplosionOverpressureResult->setText(0, "超压分析");
	sacrificeExplosionOverpressureResult->setData(0, Qt::UserRole, "OverpressureResult");
	sacrificeExplosionOverpressureResult->setIcon(0, error_icon);

	sacrificeExplosionAnalysis->addChild(sacrificeExplosionStressResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionStrainResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionTemperatureResult);
	sacrificeExplosionAnalysis->addChild(sacrificeExplosionOverpressureResult);


	analysisNode->addChild(fallAnalysis);
	analysisNode->addChild(fastCombustionAnalysis);
	analysisNode->addChild(slowCombustionAnalysis);
	
	analysisNode->addChild(shootAnalysis);
	analysisNode->addChild(jetImpactAnalysis);
	analysisNode->addChild(fragmentationImpactAnalysis);
	analysisNode->addChild(explosiveBlastAnalysis);
	analysisNode->addChild(sacrificeExplosionAnalysis);
	
	
	// 安全特性分析
	QTreeWidgetItem* paramAnalyNode = new QTreeWidgetItem(rootItem);
	paramAnalyNode->setText(0, "安全特性分析");
	paramAnalyNode->setData(0, Qt::UserRole, "Results");
	paramAnalyNode->setIcon(0, error_icon);

	QTreeWidgetItem* paramAnalyResult = new QTreeWidgetItem();
	paramAnalyResult->setText(0, "分析报告");
	paramAnalyResult->setData(0, Qt::UserRole, "paramAnalyResult");
	paramAnalyResult->setIcon(0, error_icon);

	paramAnalyNode->addChild(paramAnalyResult);

	QVBoxLayout*layout = new QVBoxLayout();
	layout->addWidget(treeWidget);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);


	// 连接信号槽
	connect(treeWidget, &QTreeWidget::itemClicked, this, &GFTreeModelWidget::onTreeItemClicked);
}

GFTreeModelWidget::~GFTreeModelWidget()
{
}

void GFTreeModelWidget::onTreeItemClicked(QTreeWidgetItem* item, int column)
{
	QString itemData = item->data(0, Qt::UserRole).toString();
	emit itemClicked(itemData);
}

void GFTreeModelWidget::updataIcon()
{
	QIcon error_icon(":/src/Error.svg");
	QIcon checked_icon(":/src/Checked.svg");

	auto ins=ModelDataManager::GetInstance();
	auto geomInfo = ins->GetModelGeometryInfo();
	auto meshInfo = ins->GetModelMeshInfo();

	auto steelInfo = ins->GetSteelPropertyInfo();
	auto propellantInfo = ins->GetPropellantPropertyInfo();
	auto calculationInfo = ins->GetCalculationPropertyInfo();
	auto judgementPropertyInfo = ins->GetJudgementPropertyInfo();
	auto insulatingheatPropertyInfo = ins->GetInsulatingheatPropertyInfo();
	auto outheatPropertyInfo = ins->GetOutheatPropertyInfo();

	int size = treeWidget->topLevelItemCount();
	QTreeWidgetItem *child;
	for (int i = 0; i < size; i++)
	{
		child = treeWidget->topLevelItem(i);
		int childCount = child->childCount();
		for (int j = 0; j < childCount; ++j)
		{
			if (child->child(j)->text(0).contains("几何模型"))
			{
				if (geomInfo.path.isEmpty())
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("网格"))
			{
				if (!meshInfo.isChecked)
				{
					child->child(j)->setIcon(0, error_icon);
				}
				else
				{
					child->child(j)->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("数据库"))
			{
				QTreeWidgetItem *clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {

					if (clChild->child(m)->text(0).contains("评判标准数据库"))
					{
						if (!judgementPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					else if (clChild->child(m)->text(0).contains("材料库"))
					{
						QTreeWidgetItem *clChild_child = clChild->child(m);
						int clChildCount = clChild_child->childCount();
						for (int n = 0; n < clChildCount; ++n) {
							if (clChild_child->child(n)->text(0).contains("壳体材料"))
							{
								if (!steelInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("推进剂材料"))
							{
								if (!propellantInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("防隔热材料"))
							{
								if (!insulatingheatPropertyInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
							if (clChild_child->child(n)->text(0).contains("外防热材料"))
							{
								if (!outheatPropertyInfo.isChecked)
								{
									clChild_child->child(n)->setIcon(0, error_icon);
								}
								else
								{
									clChild_child->child(n)->setIcon(0, checked_icon);
								}
							}
						}
						if (outheatPropertyInfo.isChecked && insulatingheatPropertyInfo.isChecked && propellantInfo.isChecked && steelInfo.isChecked)
						{
							clChild_child->setIcon(0, checked_icon);
						}
					}
				}
				if (judgementPropertyInfo.isChecked && outheatPropertyInfo.isChecked && insulatingheatPropertyInfo.isChecked && propellantInfo.isChecked && steelInfo.isChecked)
				{
					clChild->setIcon(0, checked_icon);
				}
			}
			else if (child->child(j)->text(0).contains("材料库"))
			{
				QTreeWidgetItem *clChild = child->child(j);
				int clChildCount = clChild->childCount();
				for (int m = 0; m < clChildCount; ++m) {
					if (clChild->child(m)->text(0).contains("壳体材料"))
					{
						if (!steelInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("推进剂材料"))
					{
						if (!propellantInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("防隔热材料"))
					{
						if (!insulatingheatPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("外防热材料"))
					{
						if (!outheatPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
					if (clChild->child(m)->text(0).contains("评判标准数据库"))
					{
						if (!judgementPropertyInfo.isChecked)
						{
							clChild->child(m)->setIcon(0, error_icon);
						}
						else
						{
							clChild->child(m)->setIcon(0, checked_icon);
						}
					}
				}
				
			}
		}
	}
}

void GFTreeModelWidget::contextMenuEvent(QContextMenuEvent *event)
{
	QTreeWidgetItem *item = treeWidget->itemAt(event->pos());
	if (!item) {
		return;
	}
	//QString type = item->data(0, Qt::UserRole).toString();
	QString text = item->text(0);
	if (text == "工况设置")
	{
		contextMenu = new QMenu(this);
		QAction* calAction = new QAction("计算", this);
		QAction* exportAction = new QAction("导出报告", this);

		int childCount = item->childCount();
		QList<QTreeWidgetItem*> checkedChildItems;
		for (int i = 0; i < childCount; ++i) {
			QTreeWidgetItem* childItem = item->child(i);
			if (childItem->checkState(0) == Qt::Checked) {
				checkedChildItems.append(childItem);
			}
		}

		//connect(exportAction, &QAction::triggered, [this, text]() {
		//	QWidget* parent = parentWidget();
		//	while (parent)
		//	{
		//		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		//		if (gfParent)
		//		{						
		//			auto occView = gfParent->GetOccView();
		//			Handle(AIS_InteractiveContext) context = occView->getContext();
		//			auto view = occView->getView();
		//			context->RemoveAll(true);

		//			auto modelInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
		//			TopoDS_Shape model_shape=modelInfo.shape;

		//			auto projectToXOY_Manual = [](const TopoDS_Shape& edges3D) -> TopoDS_Shape {
		//				TopoDS_Compound comp;
		//				BRep_Builder builder;
		//				builder.MakeCompound(comp);

		//				for (TopExp_Explorer explorer(edges3D, TopAbs_EDGE); explorer.More(); explorer.Next()) {
		//					TopoDS_Edge edge = TopoDS::Edge(explorer.Current());

		//					BRepAdaptor_Curve curve(edge);
		//					Standard_Real startParam, endParam;
		//					BRep_Tool::Range(edge, startParam, endParam);
		//					gp_Pnt startPoint = curve.Value(startParam);
		//					gp_Pnt endPoint = curve.Value(endParam);
		//						
		//					gp_Pnt2d p2D_Start(startPoint.X(), startPoint.Y());
		//					gp_Pnt2d p2D_End(endPoint.X(), endPoint.Y());
		//					BRepBuilderAPI_MakeEdge2d makeEdge2d(p2D_Start, p2D_End);
		//					if (makeEdge2d.IsDone()) {
		//						builder.Add(comp, makeEdge2d.Edge());
		//					}
		//				}
		//				return comp;
		//			};
		//	
		//			gp_Ax2 viewAxes(
		//				gp_Pnt(0, 0, 0),    // 原点（投影坐标系原点）
		//				gp_Dir(0, 0, -1),   // 视图方向（Z轴负方向，看向XOY平面）
		//				gp_Dir(1, 0, 0)     // X轴方向（投影平面X轴，必须与视图方向垂直）
		//			);

		//			HLRAlgo_Projector projector(viewAxes);

		//			Handle(HLRBRep_Algo) hlrAlgo = new HLRBRep_Algo();
		//			hlrAlgo->Projector(projector); // 设置投影器
		//			hlrAlgo->Add(model_shape);     // 添加模型
		//			hlrAlgo->Update();             // 执行计算

		//			HLRBRep_HLRToShape hlrToShape(hlrAlgo);
		//			TopoDS_Shape visibleEdges3D = hlrToShape.VCompound();

		//			// 检查可见边缘是否有效（替代Update的返回值判断）
		//			if (visibleEdges3D.IsNull()) {
		//				std::cerr << "HLR计算失败：未提取到可见边缘！" << std::endl;
		//				return 1;
		//			}

		//			// 6. 后续投影与坐标提取（不变）
		//			TopoDS_Shape visibleEdges2D = projectToXOY_Manual(visibleEdges3D);

		//			Handle(AIS_Shape) aisContour = new AIS_Shape(visibleEdges2D);
		//			aisContour->SetColor(Quantity_Color(Quantity_NOC_BLUE));
		//			context->Display(aisContour, Standard_True);

		//			view->FitAll();








		//			// 计算模型的中心点
		//			auto CalculateModelCenter = [](const TopoDS_Shape& model) -> gp_Pnt {
		//				GProp_GProps props;
		//				BRepGProp::VolumeProperties(model, props);
		//				return props.CentreOfMass();
		//			};

		//			// 通过检查首尾点是否重合来判断线框是否封闭
		//			auto IsWireClosedByEndPoints = [](const TopoDS_Wire& wire) -> bool {
		//				if (wire.IsNull()) return false;

		//				// 获取所有边
		//				std::vector<TopoDS_Edge> edges;
		//				for (TopExp_Explorer edgeExp(wire, TopAbs_EDGE); edgeExp.More(); edgeExp.Next()) {
		//					edges.push_back(TopoDS::Edge(edgeExp.Current()));
		//				}

		//				if (edges.empty()) return false;

		//				// 获取第一个边的起点
		//				TopoDS_Edge firstEdge = edges.front();
		//				Standard_Real first, last;
		//				Handle(Geom_Curve) firstCurve = BRep_Tool::Curve(firstEdge, first, last);
		//				if (firstCurve.IsNull()) return false;

		//				gp_Pnt startPoint = firstCurve->Value(first);

		//				// 获取最后一个边的终点
		//				TopoDS_Edge lastEdge = edges.back();
		//				Handle(Geom_Curve) lastCurve = BRep_Tool::Curve(lastEdge, first, last);
		//				if (lastCurve.IsNull()) return false;

		//				gp_Pnt endPoint = lastCurve->Value(last);

		//				// 检查首尾点距离
		//				return startPoint.Distance(endPoint) < 1e-6;
		//			};

		//			// 健壮的封闭性检查函数
		//			auto IsWireClosed = [&](const TopoDS_Wire& wire) -> bool {
		//				if (wire.IsNull()) return false;

		//				// 方法1: 尝试创建面来判断是否封闭
		//				try {
		//					BRepBuilderAPI_MakeFace faceMaker(wire, Standard_True);
		//					if (faceMaker.IsDone()) {
		//						return true;
		//					}
		//				}
		//				catch (Standard_Failure&) {
		//					// 方法1失败，继续尝试其他方法
		//				}

		//				// 方法2: 检查首尾点是否重合
		//				return IsWireClosedByEndPoints(wire);
		//			};

		//			// 修正的连接边到线框的函数，返回线框序列
		//			auto ConnectEdgesToWires_Corrected = [&](const TopoDS_Shape& edgesShape)->Handle(TopTools_HSequenceOfShape) {
		//				Handle(TopTools_HSequenceOfShape) wires = new TopTools_HSequenceOfShape();

		//				// 收集所有边到序列
		//				Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape();
		//				for (TopExp_Explorer exp(edgesShape, TopAbs_EDGE); exp.More(); exp.Next()) {
		//					edges->Append(exp.Current());
		//				}

		//				if (edges->IsEmpty()) {
		//					return wires;
		//				}

		//				// 使用静态方法连接边到线框，使用容差1e-6，不共享顶点（通过距离连接）
		//				ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, 1e-6, Standard_False, wires);

		//				return wires;
		//			};

		//			// 使用BRepAlgoAPI_Section的轮廓提取lambda函数
		//			auto GetOuterContour_Lambda = [&](const TopoDS_Shape& model) -> TopoDS_Shape {

		//					// 计算模型的中心点
		//					gp_Pnt modelCenter = CalculateModelCenter(model);

		//					// 创建XOY平面，Z坐标位于模型中心
		//					gp_Pln xoyPlaneAtCenter(gp_Pnt(modelCenter.X(), modelCenter.Y(), modelCenter.Z()), gp_Dir(0, 0, 1));

		//					// 创建平面面，大小足够覆盖整个模型
		//					Bnd_Box bbox;
		//					BRepBndLib::Add(model, bbox);

		//					Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
		//					bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

		//					// 计算平面的边界，比模型稍大
		//					Standard_Real margin = std::max((xMax - xMin), (yMax - yMin)) * 0.1;
		//					Standard_Real xSize = (xMax - xMin) + 2 * margin;
		//					Standard_Real ySize = (yMax - yMin) + 2 * margin;

		//					TopoDS_Face planeFace = BRepBuilderAPI_MakeFace(
		//						xoyPlaneAtCenter,
		//						-xSize / 2, xSize / 2,
		//						-ySize / 2, ySize / 2
		//					);

		//					// 使用BRepAlgoAPI_Section计算截面
		//					BRepAlgoAPI_Section section(planeFace, model, Standard_False);
		//					section.Build();

		//					auto sectionShape = section.Shape();
		//					return sectionShape;
		//					
		//					//// 如果截面为空，返回空线框
		//					//if (sectionShape.IsNull()) {
		//					//	return TopoDS_Wire();
		//					//}

		//					//// 连接边形成线框
		//					//Handle(TopTools_HSequenceOfShape) wires = ConnectEdgesToWires_Corrected(sectionShape);

		//					//if (wires->IsEmpty()) {
		//					//	std::cout << "未能形成线框" << std::endl;
		//					//	return TopoDS_Wire();
		//					//}

		//					//// 分派线框到闭合和开放化合物
		//					//TopoDS_Compound closedWires, openWires;
		//					//BRep_Builder builder;
		//					//builder.MakeCompound(closedWires);
		//					//builder.MakeCompound(openWires);
		//					//ShapeAnalysis_FreeBounds::DispatchWires(wires, closedWires, openWires);

		//					//// 找到最大的闭合线框（最外轮廓）
		//					//TopoDS_Wire outerWire;
		//					//Standard_Real maxArea = -1.0;

		//					//for (TopExp_Explorer exp(closedWires, TopAbs_WIRE); exp.More(); exp.Next()) {
		//					//	TopoDS_Wire wire = TopoDS::Wire(exp.Current());

		//					//	if (wire.IsNull()) {
		//					//		continue;
		//					//	}

		//					//	// 检查线框是否封闭 - 如果不是封闭的，直接跳过（按理说DispatchWires已经分出了闭合线框，但我们可以再检查一次）
		//					//	if (!IsWireClosed(wire)) {
		//					//		std::cout << "跳过非封闭轮廓线" << std::endl;
		//					//		continue;
		//					//	}

		//					//	// 方法1: 尝试创建面并计算几何面积
		//					//	BRepBuilderAPI_MakeFace tempFace(wire, Standard_True);
		//					//	if (tempFace.IsDone()) {
		//					//		try {
		//					//			GProp_GProps props;
		//					//			BRepGProp::SurfaceProperties(tempFace.Face(), props);
		//					//			Standard_Real area = props.Mass();

		//					//			if (area > maxArea) {
		//					//				maxArea = area;
		//					//				outerWire = wire;
		//					//			}
		//					//			continue;
		//					//		}
		//					//		catch (Standard_Failure&) {
		//					//			// 面积计算失败，回退到方法2
		//					//		}
		//					//	}

		//					//	// 方法2: 使用边界框面积作为近似
		//					//	Bnd_Box wireBbox;
		//					//	BRepBndLib::Add(wire, wireBbox);

		//					//	if (!wireBbox.IsVoid()) {
		//					//		Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
		//					//		wireBbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

		//					//		Standard_Real bboxArea = (xMax - xMin) * (yMax - yMin);

		//					//		if (bboxArea > maxArea) {
		//					//			maxArea = bboxArea;
		//					//			outerWire = wire;
		//					//		}
		//					//	}
		//					//}

		//					//return outerWire;
		//					

		//			};

		//			// 提取最外轮廓（自动使用模型中心的XOY平面）
		//			//auto outerContour = GetOuterContour_Lambda(model_shape);

		//			//Handle(AIS_Shape) aisMerged = new AIS_Shape(outerContour);
		//			//aisMerged->SetColor(Quantity_Color(Quantity_NOC_RED));  // 统一用黑色
		//			//aisMerged->SetWidth(2.0);                                 // 统一线宽
		//			//context->Display(aisMerged, Standard_True);               // 显示复合形状

		//			// Lambda表达式：用TopoDS_Face（平面）与模型求交，获取所有相交面
		//			auto getIntersectionFacesWithCenterXOY = [&](const TopoDS_Shape& model) -> TopoDS_Shape {

		//				// 计算模型的中心点
		//				gp_Pnt modelCenter = CalculateModelCenter(model);

		//				// 创建XOY平面，Z坐标位于模型中心
		//				gp_Pln xoyPlaneAtCenter(gp_Pnt(modelCenter.X(), modelCenter.Y(), modelCenter.Z()), gp_Dir(0, 0, 1));

		//				// 创建平面面，大小足够覆盖整个模型
		//				Bnd_Box bbox;
		//				BRepBndLib::Add(model, bbox);

		//				Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
		//				bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

		//				// 计算平面的边界，比模型稍大
		//				Standard_Real margin = std::max((xMax - xMin), (yMax - yMin)) * 0.1;
		//				Standard_Real xSize = (xMax - xMin) + 2 * margin;
		//				Standard_Real ySize = (yMax - yMin) + 2 * margin;

		//				TopoDS_Face planeFace = BRepBuilderAPI_MakeFace(
		//					xoyPlaneAtCenter,
		//					-xSize / 2, xSize / 2,
		//					-ySize / 2, ySize / 2
		//				);

		//				// 步骤3：用BRepAlgoAPI_Common求模型与拓扑面的交集
		//				BRepAlgoAPI_Common common(model, planeFace);
		//				common.Build();
		//				if (!common.IsDone()) {
		//					std::cerr << "模型与平面求交失败！" << std::endl;
		//					return TopoDS_Shape();
		//				}
		//				TopoDS_Shape intersection = common.Shape(); // 交集结果

		//				// 步骤4：从交集中提取所有位于原XOY平面上的面
		//				TopoDS_Compound resultCompound;
		//				BRep_Builder builder;
		//				builder.MakeCompound(resultCompound);
		//				Standard_Real tolerance = 0; // 共面判断容差

		//				// 遍历交集结果中的所有面（过滤非面形状）
		//				for (TopExp_Explorer faceExp(intersection, TopAbs_FACE); faceExp.More(); faceExp.Next()) {
		//					TopoDS_Face face = TopoDS::Face(faceExp.Current());
		//					Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
		//					Handle(Geom_Plane) facePlane = Handle(Geom_Plane)::DownCast(surface);
		//					if (facePlane.IsNull()) continue; // 非平面，跳过

		//					// 检查是否与原XOY平面共面
		//					const gp_Pln& pln = facePlane->Pln();
		//					if (pln.Position().IsCoplanar(xoyPlaneAtCenter.Position(), tolerance, tolerance)) {
		//						builder.Add(resultCompound, face); // 添加到结果
		//					}
		//				}


		//				return intersection;
		//			};

		//			TopoDS_Shape intersectionFaces = getIntersectionFacesWithCenterXOY(model_shape);


		//				Handle(AIS_Shape) aisFaces = new AIS_Shape(intersectionFaces);
		//				aisFaces->SetColor(Quantity_Color(Quantity_NOC_MAGENTA)); // 洋红色标识相交面
		//				context->Display(aisFaces, Standard_True); // 显示

		//			
		//			// 调整视图以适配所有内容
		//			view->FitAll();




		//			break;
		//		}
		//		else
		//		{
		//			parent = parent->parentWidget();
		//		}
		//	}
		//	});

		connect(calAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{			
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>开始进行跌落试验";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();
					
					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					auto view = occView->getView();
					context->RemoveAll(true);

					view->SetProj(V3d_Yneg);
					view->Redraw();

					QString currentWorkDir = QDir::currentPath();
					// 读取STL文件
					auto theFile = currentWorkDir + "/480-pou-daiyuantong.stl";
					QString theFile1 = theFile.replace("/", "\\\\"); // 处理路径分隔符
					std::string utf8Path = theFile1.toUtf8().constData();

					// 读取STL文件获取三角化数据
					Handle(Poly_Triangulation) stlMesh = RWStl::ReadFile(utf8Path.c_str());
					if (stlMesh.IsNull()) {
						// 处理文件读取失败
						return;
					}

					// 初始化TriangleStructure并添加STL的三角化数据
					Handle(TriangleStructure) aDataSource = new TriangleStructure();
					TopLoc_Location loc; // 默认位置变换（无变换）
					aDataSource->AddTriangulation(stlMesh, loc);
					aDataSource->ExtractEdges();

					// 创建MeshVS_Mesh并关联数据源
					Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();
					aMesh->SetDataSource(aDataSource);

					// 获取节点信息（与原逻辑一致）
					auto allnode = aDataSource->GetAllNodes();
					auto nodecoords = aDataSource->GetmyNodeCoords();

					/*std::vector<double> nodeValues;*/

					auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
					auto propellantInfo = ModelDataManager::GetInstance()->GetPropellantPropertyInfo();
					auto calInfo = ModelDataManager::GetInstance()->GetCalculationPropertyInfo();
					auto fallInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
					auto modelGeomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

					auto A = 1;
					auto B = steelInfo.density;
					auto C = 0;
					auto D = steelInfo.thermalConductivity;
					auto E = steelInfo.specificHeatCapacity;

					auto F = propellantInfo.density;
					auto G = 0;
					auto H = propellantInfo.thermalConductivity;
					auto I = propellantInfo.specificHeatCapacity;
					auto J = fallInfo.high * 1000;//跌落高度
					auto K = modelGeomInfo.length;//长
					auto L = modelGeomInfo.width;//宽
					auto M = 5;//厚

					auto formulaCal = calInfo.calculation;

					auto calculateFormula = [](const QString& formula,
						double B, double C, double D, double E,
						double F, double G, double H, double I,
						double J, double K, double L, double M, double A)
					{
						QString processedFormula = formula;  // 复制到非const变量
						processedFormula.remove(' ');
						// 变量映射：通过变量名获取对应值（使用map提高可读性和可维护性）
						const QMap<QString, double> varMap = {
							{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E},
							{"F", F}, {"G", G}, {"H", H}, {"I", I}, {"J", J},
							{"K", K}, {"L", L}, {"M", M}
						};

						QRegExp regExp("([+-]?)(\\d+(?:\\.\\d*)?|\\.\\d+)(?:\\*([A-Z]))?");
						regExp.setMinimal(false);

						double result = 0.0;
						int pos = 0;
						int matchCount = 0; // 统计匹配到的项数，用于校验公式合法性

						// 处理公式开头的第一项（可能无符号）
						if (processedFormula[0] != '+' && processedFormula[0] != '-') {
							processedFormula = "+" + processedFormula; // 补全正号，统一格式
						}

						while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
							++matchCount;
							QString signStr = regExp.cap(1);       // 符号（+/-）
							QString coeffStr = regExp.cap(2);      // 系数
							QString varName = regExp.cap(3);       // 变量

							// 解析符号（默认正号）
							double sign = (signStr == "-") ? -1.0 : 1.0;

							// 解析系数（处理转换失败）
							bool ok = false;
							double coeff = coeffStr.toDouble(&ok);
							if (!ok) {
								throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
							}

							// 计算当前项的值
							double term = sign * coeff;
							if (!varName.isEmpty()) {
								if (!varMap.contains(varName)) {
									throw std::invalid_argument(QString("未知变量: %1").arg(varName).toStdString());
								}
								term *= varMap[varName];  // 变量项：符号×系数×变量值
							}

							result += term;
							pos += regExp.matchedLength();
						}

						// 校验公式是否完全解析（无残留无效字符）
						if (matchCount == 0) {
							throw std::invalid_argument(QString("公式格式错误: %1").arg(formula).toStdString());
						}

						return result;
					};

					std::vector<double> results;
					results.reserve(formulaCal.size());
					for (int i = 0; i < formulaCal.size(); ++i)
					{
						double res = calculateFormula(formulaCal[i], B, C, D, E, F, G, H, I, J, K, L, M, A);
						results.push_back(res);
					}
					for (size_t i = 0; i < results.size(); ++i) {
						results[i] = results[i] * 0.7 * 0.6;
						if (results[i] < 0)
						{
							results[i] = 0;
						}
					}



					double min_value = *std::min_element(results.begin(), results.end());
					double max_value = *std::max_element(results.begin(), results.end());

					// 更新结果

					double shellMaxValue = max_value; // 发动机壳体最大应力
					double shellMinValue = 0; // 发动机壳体最小应力
					double shellAvgValue = shellMaxValue * 0.6; // 发动机壳体平均应力
					double shellStandardValue = getStd(results); // 发动机壳体应力标准差
					double maxValue = max_value * 0.6; // 固体推进剂最大应力
					double minValue = 0; // 固体推进剂最小应力
					double avgValue = maxValue * 0.6; // 固体推进剂平均应力
					double standardValue = 0; // 固体推进剂应力标准差
					gfParent->GetStressResultWidget()->updateData(shellMaxValue, shellMinValue, shellAvgValue, shellStandardValue, maxValue, minValue, avgValue, standardValue);


					FallAnalysisResultInfo fallAnalysisResultInfo;

					fallAnalysisResultInfo.isChecked = true;
					fallAnalysisResultInfo.triangleStructure = aDataSource;
					fallAnalysisResultInfo.maxValue = max_value;
					fallAnalysisResultInfo.minValue = min_value;
					ModelDataManager::GetInstance()->SetFallAnalysisResultInfo(fallAnalysisResultInfo);

					// 应力分析结果
					StressResult fallStressResult;
					fallStressResult.metalsMaxStress = shellMaxValue;
					fallStressResult.metalsMinStress = shellMinValue;
					fallStressResult.metalsAvgStress = shellAvgValue;
					fallStressResult.metalsStandardStress = shellStandardValue;
					fallStressResult.propellantsMaxStress = maxValue;
					fallStressResult.propellantsMinStress = minValue;
					fallStressResult.propellantsAvgStress = avgValue;
					fallStressResult.propellantsStandardStress = standardValue;
					fallStressResult.outheatMaxStress = shellMaxValue;
					fallStressResult.outheatMinStress = shellMinValue;
					fallStressResult.outheatAvgStress = shellAvgValue;
					fallStressResult.outheatStandardStress = shellStandardValue;
					fallStressResult.insulatingheatMaxStress = maxValue;
					fallStressResult.insulatingheatMinStress = minValue;
					fallStressResult.insulatingheatAvgStress = avgValue;
					fallStressResult.insulatingheatStandardStress = standardValue;
					ModelDataManager::GetInstance()->SetFallStressResult(fallStressResult);

					// 应变分析结果
					StrainResult fallStrainResult;
					fallStrainResult.metalsMaxStrain = fallStressResult.metalsMaxStress *steelInfo.modulus;
					fallStrainResult.metalsMinStrain = fallStressResult.metalsMinStress * steelInfo.modulus;
					fallStrainResult.metalsAvgStrain = fallStressResult.metalsAvgStress * steelInfo.modulus;
					fallStrainResult.metalsStandardStrain = fallStressResult.metalsStandardStress * steelInfo.modulus;
					fallStrainResult.propellantsMaxStrain = fallStressResult.propellantsMaxStress * steelInfo.modulus;
					fallStrainResult.propellantsMinStrain = fallStressResult.propellantsMaxStress * steelInfo.modulus;
					fallStrainResult.mpropellantsAvgStrain = fallStressResult.propellantsAvgStress * steelInfo.modulus;
					fallStrainResult.propellantsStandardStrain = fallStressResult.propellantsStandardStress * steelInfo.modulus;
					fallStrainResult.outheatMaxStrain = fallStressResult.outheatMaxStress * steelInfo.modulus;
					fallStrainResult.outheatMinStrain = fallStressResult.outheatMinStress * steelInfo.modulus;
					fallStrainResult.outheatAvgStrain = fallStressResult.outheatAvgStress * steelInfo.modulus;
					fallStrainResult.outheatStandardStrain = fallStressResult.outheatStandardStress * steelInfo.modulus;
					fallStrainResult.insulatingheatMaxStrain = fallStressResult.insulatingheatMaxStress * steelInfo.modulus;
					fallStrainResult.insulatingheatMinStrain = fallStressResult.insulatingheatMinStress * steelInfo.modulus;
					fallStrainResult.insulatingheatAvgStrain = fallStressResult.insulatingheatAvgStress * steelInfo.modulus;
					fallStrainResult.insulatingheatStandardStrain = fallStressResult.insulatingheatStandardStress * steelInfo.modulus;
					ModelDataManager::GetInstance()->SetFallStrainResult(fallStrainResult);
					

					{
						QDateTime currentTime = QDateTime::currentDateTime();
						QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
						auto logWidget = gfParent->GetLogWidget();
						auto textEdit = logWidget->GetTextEdit();
						QString text = timeStr + "[信息]>跌落试验计算完成";
						textEdit->appendPlainText(text);
						logWidget->update();
					}

					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		connect(exportAction, &QAction::triggered, [this, text]() {
			QString directory = QFileDialog::getExistingDirectory(nullptr,
				tr("选择文件夹"),
				"/home", // 默认的起始目录
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // 选项
			if (!directory.isEmpty()) {
				exportWord(directory, "Hello, World!"); // 直接在Lambda中传递参数
			}
		});
		contextMenu->addAction(calAction); // 将动作添加到菜单中
		contextMenu->addAction(exportAction);
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
	else if (text == "几何模型")
	{
		contextMenu = new QMenu(this); // 创建菜单对象
		QAction *customAction = new QAction("导入", this); // 创建动作对象并添加到菜单中
		connect(customAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent) {
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					QString filePath = QFileDialog::getOpenFileName(this, "Open File", QDir::homePath(),
						"STEP Files (*.stp *.step);;STL Files (*.stl);;IGES Files (*.iges *.igs);;All Files (*.*)");

					if (filePath.isEmpty())
					{
						return;
					}
				
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>开始导入几何模型";
					textEdit->appendPlainText(text);
					logWidget->update();

					// 关键：强制刷新UI，确保日志立即显示
					QApplication::processEvents();
					

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("几何模型导入进度", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					GeometryImportWorker* worker = new GeometryImportWorker(filePath);
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &GeometryImportWorker::DoWork);
					connect(worker, &GeometryImportWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &GeometryImportWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &GeometryImportWorker::RequestInterruption);

					// 处理导入结果
					connect(worker, &GeometryImportWorker::WorkFinished, this,
						[=](bool success, const QString& msg, const ModelGeometryInfo& info) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);

							if (success && !info.shape.IsNull())
							{
								// 保存模型信息
								ModelDataManager::GetInstance()->SetModelGeometryInfo(info);
								updataIcon();

								// 更新显示
								auto occView = gfParent->GetOccView();
								Handle(AIS_InteractiveContext) context = occView->getContext();
								context->RemoveAll(true);

								Handle(AIS_Shape) modelPresentation = new AIS_Shape(info.shape);
								context->SetDisplayMode(modelPresentation, AIS_Shaded, true);
								context->SetColor(modelPresentation, Quantity_NOC_CYAN, true);
								context->Display(modelPresentation, false);
								occView->fitAll();

								// 更新属性窗口
								auto geomProWid = gfParent->findChild<GeomPropertyWidget*>();
								geomProWid->UpdataPropertyInfo();

							}
							else if (!success)
							{
								QMessageBox::warning(this, "导入失败", msg);
							}

							// 清理资源
							progressDialog->close();
							workerThread->quit();
							workerThread->wait();
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();
						});

					// 启动线程
					workerThread->start();
					break;					
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		contextMenu->addAction(customAction); // 将动作添加到菜单中
		contextMenu->exec(event->globalPos()); // 在鼠标位置显示菜单
	}
	else if (text == "网格")
	{
		contextMenu = new QMenu(this);
		QAction *meshAction = new QAction("网格划分", this);
		connect(meshAction, &QAction::triggered, this, [item, this]() {
			QWidget* parent = parentWidget();
			while (parent)
			{
				GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
				if (gfParent)
				{
					{
						// 测试截图
						QString m_privateDirPath = "src/template/测试模型.png";
						QDir privateDir(m_privateDirPath);
						wordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);


						QDateTime currentTime = QDateTime::currentDateTime();
						QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
						auto logWidget = gfParent->GetLogWidget();
						auto textEdit = logWidget->GetTextEdit();
						QString text = timeStr + "[信息]>启动网格划分引擎，采用自适应尺寸控制算法";
						textEdit->appendPlainText(text);
						logWidget->update();

						// 关键：强制刷新UI，确保日志立即显示
						QApplication::processEvents();
					}

					ModelMeshInfo meshInfo;

					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					auto view = occView->getView();
					context->RemoveAll(true);

					Handle(MeshVS_Mesh) aMesh = new MeshVS_Mesh();

					auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();

					auto aDataSource = new TriangleStructure(geomInfo.shape, 0.5);


					meshInfo.isChecked = true;
					meshInfo.triangleStructure = *aDataSource;
					ModelDataManager::GetInstance()->SetModelMeshInfo(meshInfo);


					BRep_Builder builder;
					TopoDS_Compound compound;
					builder.MakeCompound(compound);

					auto myEdges = aDataSource->GetMyEdge();

					auto myNodeCoords = aDataSource->GetmyNodeCoords();

					for (const auto& edge : myEdges)
					{
						Standard_Integer node1ID = edge.first;
						Standard_Integer node2ID = edge.second;

						Standard_Real x1 = myNodeCoords->Value(node1ID, 1);
						Standard_Real y1 = myNodeCoords->Value(node1ID, 2);
						Standard_Real z1 = myNodeCoords->Value(node1ID, 3);

						Standard_Real x2 = myNodeCoords->Value(node2ID, 1);
						Standard_Real y2 = myNodeCoords->Value(node2ID, 2);
						Standard_Real z2 = myNodeCoords->Value(node2ID, 3);

						gp_Pnt p1(x1, y1, z1);
						gp_Pnt p2(x2, y2, z2);

						TopoDS_Vertex v1 = BRepBuilderAPI_MakeVertex(p1);
						TopoDS_Vertex v2 = BRepBuilderAPI_MakeVertex(p2);

						TopoDS_Edge edgeShape = BRepBuilderAPI_MakeEdge(v1, v2);

						// 将边线添加到复合形状中
						builder.Add(compound, edgeShape);

						//Handle(AIS_Shape) aisEdge = new AIS_Shape(edgeShape);
						//context->Display(aisEdge, Standard_True);
					}

					Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
					context->Display(aisCompound, Standard_True);

					updataIcon();

					{
						QDateTime currentTime = QDateTime::currentDateTime();
						QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
						auto logWidget = gfParent->GetLogWidget();
						auto textEdit = logWidget->GetTextEdit();
						QString text = timeStr + "[信息]>网格划分完成";
						textEdit->appendPlainText(text);
					}

					auto meshProWid = gfParent->findChild<MeshPropertyWidget*>();
					meshProWid->UpdataPropertyInfo();

					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
		});
		contextMenu->addAction(meshAction);
		contextMenu->exec(event->globalPos());
	}
}

void GFTreeModelWidget::exportWord(const QString& directory, const QString& text)
{
	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			// 测试截图
			QString m_privateDirPath = "src/template/跌落试验/应力云图.png";
			QDir privateDir(m_privateDirPath);
			wordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);

			{
				QDateTime currentTime = QDateTime::currentDateTime();
				QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
				auto logWidget = gfParent->GetLogWidget();
				auto textEdit = logWidget->GetTextEdit();
				QString text = timeStr + "[信息]>开始导出跌落试验报告";
				textEdit->appendPlainText(text);
				logWidget->update();

				// 关键：强制刷新UI，确保日志立即显示
				QApplication::processEvents();
			}

			ProjectPropertyWidge *m_projectPropertyWidge = gfParent->GetProjectPropertyWidget();
			QTableWidget* m_projectTableWid = m_projectPropertyWidge->GetQTableWidget();

			GeomPropertyWidget *m_geomPropertyWidget = gfParent->GetGeomPropertyWidget();
			QTableWidget* m_geomTableWid = m_geomPropertyWidget->GetQTableWidget();

			MaterialPropertyWidget *m_materialPropertyWidget = gfParent->GetMaterialPropertyWidget();
			QTableWidget* m_materialTableWid = m_materialPropertyWidget->GetQTableWidget();

			DatabasePropertyWidget *m_databasePropertyWidget = gfParent->GetDatabasePropertyWidget();
			QTableWidget* m_databaseTableWid = m_databasePropertyWidget->GetQTableWidget();

			FallPropertyWidget *m_fallPropertyWidget = gfParent->GetFallPropertyWidget();
			QTableWidget* m_fallTableWid = m_fallPropertyWidget->GetQTableWidget();

			StressResultWidget *m_stressResultWidget = gfParent->GetStressResultWidget();
			QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

;			StrainResultWidget *m_strainResultWidget = gfParent->GetStrainResultWidget();
			QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

			TemperatureResultWidget *m_temperatureResultWidget = gfParent->GetTemperatureResultWidget();
			QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

			OverpressureResultWidget *m_overpressureResultWidge = gfParent->GetOverpressureResultWidget();
			QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

			QMap<QString, QVariant> data;
			data.insert("工程名称", m_projectTableWid->item(1, 2)->text());
			data.insert("工程地点", m_projectTableWid->item(2, 2)->text());
			data.insert("测试设备", m_projectTableWid->item(4, 2)->text());
			data.insert("发动机型号", m_geomTableWid->item(1, 2)->text());
			data.insert("工程时间", m_projectTableWid->item(3, 2)->text());
			data.insert("测试标准", m_databaseTableWid->item(1, 2)->text());
			data.insert("壳体材料", m_materialTableWid->item(1, 2)->text());
			data.insert("防隔热材料", m_materialTableWid->item(2, 2)->text());
			data.insert("外防热材料", m_materialTableWid->item(3, 2)->text());
			data.insert("推进剂材料", m_materialTableWid->item(4, 2)->text());
			// 跌落输入数据
			data.insert("测试项目", m_fallTableWid->item(1, 2)->text());
			data.insert("跌落高度", m_fallTableWid->item(2, 2)->text());
			QComboBox *comboBox = qobject_cast<QComboBox*>(m_fallTableWid->cellWidget(3, 2));
			if (comboBox)
			{
				QString selectedText = comboBox->currentText();
				data.insert("跌落姿态", selectedText);
			}
			else
			{
				data.insert("跌落姿态", "");
			}
			data.insert("跌落钢板硬度", m_fallTableWid->item(4, 2)->text());
			data.insert("温度传感器数量", m_fallTableWid->item(5, 2)->text());
			if (m_fallTableWid->item(6, 2))
			{
				data.insert("冲击波超压传感器数量",m_fallTableWid->item(6, 2)->text());
			}
			else
			{
				data.insert("冲击波超压传感器数量", "");
			}
			data.insert("风速", m_fallTableWid->item(7, 2)->text());

			// 输出数据
			data.insert("发动机壳体最大应力", m_stressTableWid->item(1, 2)->text());
			data.insert("发动机壳体最小应力", m_stressTableWid->item(2, 2)->text());
			data.insert("发动机壳体平均应力", m_stressTableWid->item(3, 2)->text());
			data.insert("发动机壳体应力标准差", m_stressTableWid->item(4, 2)->text());
			data.insert("固体推进剂最大应力", m_stressTableWid->item(5, 2)->text());
			data.insert("固体推进剂最小应力", m_stressTableWid->item(6, 2)->text());
			data.insert("固体推进剂平均应力", m_stressTableWid->item(7, 2)->text());
			data.insert("固体推进剂应力标准差", m_stressTableWid->item(8, 2)->text());
			data.insert("隔绝热最大应力", m_stressTableWid->item(9, 2)->text());
			data.insert("隔绝热最小应力", m_stressTableWid->item(10, 2)->text());
			data.insert("隔绝热平均应力", m_stressTableWid->item(11, 2)->text());
			data.insert("隔绝热应力标准差", m_stressTableWid->item(12, 2)->text());
			data.insert("外防热最大应力", m_stressTableWid->item(13, 2)->text());
			data.insert("外防热最小应力", m_stressTableWid->item(14, 2)->text());
			data.insert("外防热平均应力", m_stressTableWid->item(15, 2)->text());
			data.insert("外防热应力标准差", m_stressTableWid->item(16, 2)->text());

			data.insert("发动机壳体最大应变", m_strainTableWid->item(1, 2)->text());
			data.insert("发动机壳体最小应变", m_strainTableWid->item(2, 2)->text());
			data.insert("发动机壳体平均应变", m_strainTableWid->item(3, 2)->text());
			data.insert("发动机壳体应变标准差", m_strainTableWid->item(4, 2)->text());
			data.insert("固体推进剂最大应变", m_strainTableWid->item(5, 2)->text());
			data.insert("固体推进剂最小应变", m_strainTableWid->item(6, 2)->text());
			data.insert("固体推进剂平均应变", m_strainTableWid->item(7, 2)->text());
			data.insert("固体推进剂应变标准差", m_strainTableWid->item(8, 2)->text());
			data.insert("隔绝热最大应变", m_strainTableWid->item(9, 2)->text());
			data.insert("隔绝热最小应变", m_strainTableWid->item(10, 2)->text());
			data.insert("隔绝热平均应变", m_strainTableWid->item(11, 2)->text());
			data.insert("隔绝热应变标准差", m_strainTableWid->item(12, 2)->text());
			data.insert("外防热最大应变", m_strainTableWid->item(13, 2)->text());
			data.insert("外防热最小应变", m_strainTableWid->item(14, 2)->text());
			data.insert("外防热平均应变", m_strainTableWid->item(15, 2)->text());
			data.insert("外防热应变标准差", m_strainTableWid->item(16, 2)->text());

			data.insert("发动机壳体最高温度", m_temperatureTableWid->item(1, 2)->text());
			data.insert("发动机壳体最低温度", m_temperatureTableWid->item(2, 2)->text());
			data.insert("发动机壳体平均温度", m_temperatureTableWid->item(3, 2)->text());
			data.insert("发动机壳体温度标准差", m_temperatureTableWid->item(4, 2)->text());
			data.insert("固体推进剂最高温度", m_temperatureTableWid->item(5, 2)->text());
			data.insert("固体推进剂最低温度", m_temperatureTableWid->item(6, 2)->text());
			data.insert("固体推进剂平均温度", m_temperatureTableWid->item(7, 2)->text());
			data.insert("固体推进剂温度标准差", m_temperatureTableWid->item(8, 2)->text());
			data.insert("隔绝热最高温度", m_temperatureTableWid->item(9, 2)->text());
			data.insert("隔绝热最低温度", m_temperatureTableWid->item(10, 2)->text());
			data.insert("隔绝热平均温度", m_temperatureTableWid->item(11, 2)->text());
			data.insert("隔绝热温度标准差", m_temperatureTableWid->item(12, 2)->text());
			data.insert("外防热最高温度", m_temperatureTableWid->item(13, 2)->text());
			data.insert("外防热最低温度", m_temperatureTableWid->item(14, 2)->text());
			data.insert("外防热平均温度", m_temperatureTableWid->item(15, 2)->text());
			data.insert("外防热温度标准差", m_temperatureTableWid->item(16, 2)->text());

			data.insert("发动机壳体最大超压", m_overpressureTableWid->item(1, 2)->text());
			data.insert("发动机壳体最小超压", m_overpressureTableWid->item(2, 2)->text());
			data.insert("发动机壳体平均超压", m_overpressureTableWid->item(3, 2)->text());
			data.insert("发动机壳体超压标准差", m_overpressureTableWid->item(4, 2)->text());
			data.insert("固体推进剂最大超压", m_overpressureTableWid->item(5, 2)->text());
			data.insert("固体推进剂最小超压", m_overpressureTableWid->item(6, 2)->text());
			data.insert("固体推进剂平均超压", m_overpressureTableWid->item(7, 2)->text());
			data.insert("固体推进剂超压标准差", m_overpressureTableWid->item(8, 2)->text());
			data.insert("隔绝热最大超压", m_overpressureTableWid->item(9, 2)->text());
			data.insert("隔绝热最小超压", m_overpressureTableWid->item(10, 2)->text());
			data.insert("隔绝热平均超压", m_overpressureTableWid->item(11, 2)->text());
			data.insert("隔绝热超压标准差", m_overpressureTableWid->item(12, 2)->text());
			data.insert("外防热最大超压", m_overpressureTableWid->item(13, 2)->text());
			data.insert("外防热最小超压", m_overpressureTableWid->item(14, 2)->text());
			data.insert("外防热平均超压", m_overpressureTableWid->item(15, 2)->text());
			data.insert("外防热超压标准差", m_overpressureTableWid->item(16, 2)->text());


			QMap<QString, QString> imagePaths;
			QString a = QDir("src/template/测试模型.png").absolutePath();
			imagePaths.insert("测试模型", QDir("src/template/测试模型.png").absolutePath());
			imagePaths.insert("应力云图", QDir("src/template/跌落试验/应力云图.png").absolutePath());
			

			// 创建进度对话框
			ProgressDialog* progressDialog = new ProgressDialog("导出报告进度", gfParent);
			progressDialog->show();

			// 创建工作线程和工作对象
			WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/跌落仿真计算数据表.docx").absolutePath(), directory + "/跌落仿真计算数据表.docx", data, imagePaths);
			QThread* wordExporterThread = new QThread();
			wordExporterWorker->moveToThread(wordExporterThread);

			// 连接信号槽
			connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
			connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
			connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
			connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption);

			// 处理导入结果
			connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
				[=](bool success, const QString& msg) {
					

					if (success )
					{
						// 更新日志
						QMessageBox::information(this, "提示", QString("导出成功"));
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							auto logWidget = gfParent->GetLogWidget();
							auto textEdit = logWidget->GetTextEdit();
							QString text = timeStr + "[信息]>成功导出试验报告";
							text = text + "\n" + timeStr + "[信息]>试验报告：" + directory + "/跌落仿真计算数据表.docx";
							textEdit->appendPlainText(text);
							logWidget->update();

							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

					}
					else if (!success)
					{
						QMessageBox::warning(this, "导出失败", msg);
					}
					// 清理资源
					progressDialog->close();
					wordExporterThread->quit();
					wordExporterThread->wait();
					wordExporterWorker->deleteLater();
					wordExporterThread->deleteLater();
					progressDialog->deleteLater();
				});

			// 启动线程
			wordExporterThread->start();
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}
	
}