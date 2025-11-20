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
#include <QRegExp>
#include <QRegularExpression> 
#include <QValidator>
#include <QThread>
#include <algorithm>

#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepProj_Projection.hxx>
#include <BRepGProp.hxx>


#include <GProp_GProps.hxx>

#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>

#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_ColorRGBA.hxx>

#include <RWStl.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Reader.hxx>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp_Explorer.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>


#include <V3d_View.hxx>
#include <V3d_Viewer.hxx> 




#include "GFImportModelWidget.h"
#include "TriangleStructure.h"
#include "occView.h"
#include "ModelDataManager.h"
#include "ProgressDialog.h"
#include "GeometryImportWorker.h"
#include "WordExporterWorker.h"
#include "APICreateMidSurfaceHelper.h"




#include <QScreen>
#include <QtCore/qstandardpaths.h>
#include "TriangulationWorker.h"
#include "APICalculateHepler.h"



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
	qRegisterMetaType<ModelMeshInfo>("ModelMeshInfo");


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
	databaseNode->setExpanded(true);

	QTreeWidgetItem* materialNode = new QTreeWidgetItem();
	materialNode->setText(0, "材料库");
	materialNode->setData(0, Qt::UserRole, "Material");
	materialNode->setIcon(0, error_icon);
	materialNode->setExpanded(true);

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
	analysisNode->setExpanded(true);

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
	fastCombustionTemperatureResult->setData(0, Qt::UserRole, "FastCombustionTemperatureResult");
	fastCombustionTemperatureResult->setIcon(0, error_icon);

	fastCombustionAnalysis->addChild(fastCombustionTemperatureResult);


	QTreeWidgetItem* slowCombustionAnalysis = new QTreeWidgetItem();
	slowCombustionAnalysis->setText(0, "3.慢速烤燃试验");
	slowCombustionAnalysis->setData(0, Qt::UserRole, "SlowCombustionAnalysis");
	//slowCombustionAnalysis->setCheckState(0, Qt::Unchecked);
	slowCombustionAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* slowCombustionTemperatureResult = new QTreeWidgetItem();
	slowCombustionTemperatureResult->setText(0, "温度分析");
	slowCombustionTemperatureResult->setData(0, Qt::UserRole, "slowCombustionTemperatureResult");
	slowCombustionTemperatureResult->setIcon(0, error_icon);

	slowCombustionAnalysis->addChild(slowCombustionTemperatureResult);

	
	QTreeWidgetItem* shootAnalysis = new QTreeWidgetItem();
	shootAnalysis->setText(0, "4.枪击试验");
	shootAnalysis->setData(0, Qt::UserRole, "ShootAnalysis");
	//shootAnalysis->setCheckState(0, Qt::Unchecked);
	shootAnalysis->setIcon(0, error_icon);

	QTreeWidgetItem* shootStressResult = new QTreeWidgetItem();
	shootStressResult->setText(0, "应力分析");
	shootStressResult->setData(0, Qt::UserRole, "ShootStressResult");
	shootStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootStrainResult = new QTreeWidgetItem();
	shootStrainResult->setText(0, "应变分析");
	shootStrainResult->setData(0, Qt::UserRole, "ShootStrainResult");
	shootStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootTemperatureResult = new QTreeWidgetItem();
	shootTemperatureResult->setText(0, "温度分析");
	shootTemperatureResult->setData(0, Qt::UserRole, "ShootTemperatureResult");
	shootTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* shootOverpressureResult = new QTreeWidgetItem();
	shootOverpressureResult->setText(0, "超压分析");
	shootOverpressureResult->setData(0, Qt::UserRole, "ShootOverpressureResult");
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
	jetImpactStressResult->setData(0, Qt::UserRole, "JetImpactStressResult");
	jetImpactStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetStrainResult = new QTreeWidgetItem();
	jetStrainResult->setText(0, "应变分析");
	jetStrainResult->setData(0, Qt::UserRole, "JetImpactStrainResult");
	jetStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactTemperatureResult = new QTreeWidgetItem();
	jetImpactTemperatureResult->setText(0, "温度分析");
	jetImpactTemperatureResult->setData(0, Qt::UserRole, "JetImpactTemperatureResult");
	jetImpactTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* jetImpactOverpressureResult = new QTreeWidgetItem();
	jetImpactOverpressureResult->setText(0, "超压分析");
	jetImpactOverpressureResult->setData(0, Qt::UserRole, "JetImpactOverpressureResult");
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
	fragmentationImpactStressResult->setData(0, Qt::UserRole, "FragmentationImpactStressResult");
	fragmentationImpactStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationStrainResult = new QTreeWidgetItem();
	fragmentationStrainResult->setText(0, "应变分析");
	fragmentationStrainResult->setData(0, Qt::UserRole, "FragmentationImpactStrainResult");
	fragmentationStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactTemperatureResult = new QTreeWidgetItem();
	fragmentationImpactTemperatureResult->setText(0, "温度分析");
	fragmentationImpactTemperatureResult->setData(0, Qt::UserRole, "FragmentationImpactTemperatureResult");
	fragmentationImpactTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* fragmentationImpactOverpressureResult = new QTreeWidgetItem();
	fragmentationImpactOverpressureResult->setText(0, "超压分析");
	fragmentationImpactOverpressureResult->setData(0, Qt::UserRole, "FragmentationImpactOverpressureResult");
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
	explosiveBlastStressResult->setData(0, Qt::UserRole, "ExplosiveBlastStressResult");
	explosiveBlastStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastStrainResult = new QTreeWidgetItem();
	explosiveBlastStrainResult->setText(0, "应变分析");
	explosiveBlastStrainResult->setData(0, Qt::UserRole, "ExplosiveBlastStrainResult");
	explosiveBlastStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastTemperatureResult = new QTreeWidgetItem();
	explosiveBlastTemperatureResult->setText(0, "温度分析");
	explosiveBlastTemperatureResult->setData(0, Qt::UserRole, "ExplosiveBlastTemperatureResult");
	explosiveBlastTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* explosiveBlastOverpressureResult = new QTreeWidgetItem();
	explosiveBlastOverpressureResult->setText(0, "超压分析");
	explosiveBlastOverpressureResult->setData(0, Qt::UserRole, "ExplosiveBlastOverpressureResult");
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
	sacrificeExplosionStressResult->setData(0, Qt::UserRole, "SacrificeExplosioStressResult");
	sacrificeExplosionStressResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionStrainResult = new QTreeWidgetItem();
	sacrificeExplosionStrainResult->setText(0, "应变分析");
	sacrificeExplosionStrainResult->setData(0, Qt::UserRole, "SacrificeExplosioStrainResult");
	sacrificeExplosionStrainResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionTemperatureResult = new QTreeWidgetItem();
	sacrificeExplosionTemperatureResult->setText(0, "温度分析");
	sacrificeExplosionTemperatureResult->setData(0, Qt::UserRole, "SacrificeExplosioTemperatureResult");
	sacrificeExplosionTemperatureResult->setIcon(0, error_icon);

	QTreeWidgetItem* sacrificeExplosionOverpressureResult = new QTreeWidgetItem();
	sacrificeExplosionOverpressureResult->setText(0, "超压分析");
	sacrificeExplosionOverpressureResult->setData(0, Qt::UserRole, "SacrificeExplosioOverpressureResult");
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

	if (itemData.contains("StressResult")|| itemData.contains("StrainResult") || itemData.contains("TemperatureResult") || itemData.contains("OverpressureResult") )
	{
		QWidget* parent = parentWidget();
		while (parent) {
			GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
			if (gfParent)
			{
				// 截图结果云图
				QString m_privateDirPath = "";
				if (itemData == "StressResult")
				{
					m_privateDirPath = "src/template/跌落试验/应力云图.png";
				}
				else if (itemData == "StrainResult")
				{
					m_privateDirPath = "src/template/跌落试验/应变云图.png";
				}
				else if (itemData == "TemperatureResult")
				{
					m_privateDirPath = "src/template/跌落试验/温度云图.png";
				}
				else if (itemData == "OverpressureResult")
				{
					m_privateDirPath = "src/template/跌落试验/超压云图.png";
				}

				if (itemData == "ShootStressResult")
				{
					m_privateDirPath = "src/template/枪击试验/应力云图.png";
				}
				else if (itemData == "ShootStrainResult")
				{
					m_privateDirPath = "src/template/枪击试验/应变云图.png";
				}
				else if (itemData == "ShootTemperatureResult")
				{
					m_privateDirPath = "src/template/枪击试验/温度云图.png";
				}
				else if (itemData == "ShootOverpressureResult")
				{
					m_privateDirPath = "src/template/枪击试验/超压云图.png";
				}

				if (itemData == "FragmentationImpactStressResult")
				{
					m_privateDirPath = "src/template/破片撞击试验/应力云图.png";
				}
				else if (itemData == "FragmentationImpactStrainResult")
				{
					m_privateDirPath = "src/template/破片撞击试验/应变云图.png";
				}
				else if (itemData == "FragmentationImpactTemperatureResult")
				{
					m_privateDirPath = "src/template/破片撞击试验/温度云图.png";
				}
				else if (itemData == "FragmentationImpactOverpressureResult")
				{
					m_privateDirPath = "src/template/破片撞击试验/超压云图.png";
				}
				
				QDir privateDir(m_privateDirPath);
				wordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);
				break;
			}
			else
			{
				parent = parent->parentWidget();
			}
		}
	}
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
			if (childItem->checkState(0) == Qt::Checked)
			{
				checkedChildItems.append(childItem);
			}
		}

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

					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					Handle(V3d_View) view = occView->getView();

					for (int i = 0; i < item->childCount(); ++i) {
						QTreeWidgetItem* childItem = item->child(i);
						auto originalName = childItem->text(0);
						int dotIndex = originalName.indexOf('.');
						QString processedName;
						if (dotIndex != -1)
						{
							processedName = originalName.mid(dotIndex + 1).trimmed();
						}
						else {
							processedName = originalName;
						}

						bool isChecked = (childItem->checkState(0) == Qt::Checked);
						if (isChecked)
						{
							QString text = timeStr + "[信息]>开始进行" + processedName;
							textEdit->appendPlainText(text);

							if (processedName == "跌落试验")
							{
								std::vector<double> resultValue;
								resultValue.reserve(8);
								bool success = APICalculateHepler::CalculateFallAnalysisResult(occView, resultValue);

								QDateTime currentTime = QDateTime::currentDateTime();
								QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
								if (success)
								{
									QString text = timeStr + "[信息]>跌落试验计算完成";
									textEdit->appendPlainText(text);

									context->EraseAll(true);
									view->SetProj(V3d_Yneg);
									view->Redraw();

									auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
									auto oriShape = geomInfo.shape;

									//BRep_Builder builder;
									//TopoDS_Compound compound;
									//builder.MakeCompound(compound);
									//builder.Add(compound, rotatedShape);
									//Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
									//context->Display(aisCompound, Standard_True);
									//occView->fitAll();

									gfParent->GetStressResultWidget()->updateData(resultValue[0], resultValue[1], resultValue[2], resultValue[3], 
										resultValue[4], resultValue[5], resultValue[6], resultValue[7]);

									auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
									gfParent->GetStrainResultWidget()->updateData(resultValue[0] * steelInfo.modulus, resultValue[1] * steelInfo.modulus, resultValue[2] * steelInfo.modulus, resultValue[3] * steelInfo.modulus,
										resultValue[4] * steelInfo.modulus, resultValue[5] * steelInfo.modulus, resultValue[6] * steelInfo.modulus, resultValue[7] * steelInfo.modulus);
								

								}
								else
								{
									QString text = timeStr + "[信息]>跌落试验计算失败";
									textEdit->appendPlainText(text);
								}
							}
							else if (processedName == "快速烤燃试验")
							{

							}
							else if (processedName == "慢速烤燃试验")
							{

							}
							else if (processedName == "枪击试验")
							{
								std::vector<double> resultValue;
								resultValue.reserve(8);
								bool success = APICalculateHepler::CalculateShootingAnalysisResult(occView, resultValue);

								QDateTime currentTime = QDateTime::currentDateTime();
								QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
								if (success)
								{
									QString text = timeStr + "[信息]>枪击试验计算完成";
									textEdit->appendPlainText(text);

									context->EraseAll(true);
									view->SetProj(V3d_Yneg);
									view->Redraw();

									auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
									auto oriShape = geomInfo.shape;

									gfParent->GetShootStressResultWidget()->updateData(resultValue[0], resultValue[1], resultValue[2], resultValue[3],
										resultValue[4], resultValue[5], resultValue[6], resultValue[7]);

									auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
									gfParent->GetShootStrainResultWidget()->updateData(resultValue[0] * steelInfo.modulus, resultValue[1] * steelInfo.modulus, resultValue[2] * steelInfo.modulus, resultValue[3] * steelInfo.modulus,
										resultValue[4] * steelInfo.modulus, resultValue[5] * steelInfo.modulus, resultValue[6] * steelInfo.modulus, resultValue[7] * steelInfo.modulus);
								}
								else
								{
									QString text = timeStr + "[信息]>枪击试验计算失败";
									textEdit->appendPlainText(text);
								}
							}
							else if (processedName == "射流冲击试验")
							{

							}
							else if (processedName == "破片撞击试验")
							{
								std::vector<double> resultValue;
								resultValue.reserve(8);
								bool success = APICalculateHepler::CalculateFragmentationAnalysisResult(occView, resultValue);

								QDateTime currentTime = QDateTime::currentDateTime();
								QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
								if (success)
								{
									QString text = timeStr + "[信息]>破片试验计算完成";
									textEdit->appendPlainText(text);

									context->EraseAll(true);
									view->SetProj(V3d_Yneg);
									view->Redraw();

									auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
									auto oriShape = geomInfo.shape;

									gfParent->GetFragmentationImpactStressResultWidget()->updateData(resultValue[0], resultValue[1], resultValue[2], resultValue[3],
										resultValue[4], resultValue[5], resultValue[6], resultValue[7]);

									auto steelInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
									gfParent->GetFragmentationImpactStrainResultWidget()->updateData(resultValue[0] * steelInfo.modulus, resultValue[1] * steelInfo.modulus, resultValue[2] * steelInfo.modulus, resultValue[3] * steelInfo.modulus,
										resultValue[4] * steelInfo.modulus, resultValue[5] * steelInfo.modulus, resultValue[6] * steelInfo.modulus, resultValue[7] * steelInfo.modulus);
								}
								else
								{
									QString text = timeStr + "[信息]>破片试验计算失败";
									textEdit->appendPlainText(text);
								}
							}
							else if (processedName == "爆炸冲击波试验")
							{

							}
							else if (processedName == "殉爆试验")
							{

							}
						}
					}
					logWidget->update();
				
					break;
				}
				else
				{
					parent = parent->parentWidget();
				}
			}
			});
			
	
		connect(exportAction, &QAction::triggered, [this, item]() {
			QString directory = QFileDialog::getExistingDirectory(nullptr,
				tr("选择文件夹"),
				"/home", // 默认的起始目录
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks); // 选项
			if (!directory.isEmpty()) {
				exportWord(directory, item); // 直接在Lambda中传递参数
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
					ProgressDialog* progressDialog = new ProgressDialog("几何模型导入", gfParent);
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
						worker, &GeometryImportWorker::RequestInterruption,
						Qt::DirectConnection); 

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
								context->EraseAll(true);

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
							if (!workerThread->wait(500)) 
							{  
								workerThread->terminate();
							}
							worker->deleteLater();
							workerThread->deleteLater();
							progressDialog->deleteLater();

							// 截图计算模型
							QString m_privateDirPath = "src/template/计算模型.png";
							QDir privateDir(m_privateDirPath);
							wordExporter->captureWidgetToFile(gfParent->GetOccView(), m_privateDirPath);
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
					QDateTime currentTime = QDateTime::currentDateTime();
					QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
					auto logWidget = gfParent->GetLogWidget();
					auto textEdit = logWidget->GetTextEdit();
					QString text = timeStr + "[信息]>启动网格划分引擎，采用自适应尺寸控制算法";
					textEdit->appendPlainText(text);
					logWidget->update();
					
					auto occView = gfParent->GetOccView();
					Handle(AIS_InteractiveContext) context = occView->getContext();
					auto view = occView->getView();
					context->EraseAll(true);

					// 创建进度对话框
					ProgressDialog* progressDialog = new ProgressDialog("网格划分", gfParent);
					progressDialog->show();

					// 创建工作线程和工作对象
					auto geomInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
					TriangulationWorker* worker = new TriangulationWorker(geomInfo.shape);
					QThread* workerThread = new QThread();
					worker->moveToThread(workerThread);

					// 连接信号槽
					connect(workerThread, &QThread::started, worker, &TriangulationWorker::DoWork);
					connect(worker, &TriangulationWorker::ProgressUpdated,
						progressDialog, &ProgressDialog::SetProgress);
					connect(worker, &TriangulationWorker::StatusUpdated,
						progressDialog, &ProgressDialog::SetStatusText);
					connect(progressDialog, &ProgressDialog::Canceled,
						worker, &TriangulationWorker::RequestInterruption,
						Qt::DirectConnection);

					// 处理导入结果
					connect(worker, &TriangulationWorker::WorkFinished, this,
						[=](bool success, const QString& msg, const ModelMeshInfo& info) {
							// 更新日志
							QDateTime finishTime = QDateTime::currentDateTime();
							QString finishTimeStr = finishTime.toString("yyyy-MM-dd hh:mm:ss");
							textEdit->appendPlainText(finishTimeStr + "[" + (success ? "信息" : "错误") + "]>" + msg);
							if (success)
							{
								ModelDataManager::GetInstance()->SetModelMeshInfo(info);
								BRep_Builder builder;
								TopoDS_Compound compound;
								builder.MakeCompound(compound);

								auto aDataSource = info.triangleStructure;
								auto myEdges = aDataSource.GetMyEdge();
								auto myNodeCoords = aDataSource.GetmyNodeCoords();
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

									builder.Add(compound, edgeShape);
								}
								Handle(AIS_Shape) aisCompound = new AIS_Shape(compound);
								context->Display(aisCompound, Standard_True);

								updataIcon();

								QDateTime currentTime = QDateTime::currentDateTime();
								QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
								QString text = timeStr + "[信息]>网格划分完成";
								textEdit->appendPlainText(text);

								auto meshProWid = gfParent->findChild<MeshPropertyWidget*>();
								meshProWid->UpdataPropertyInfo();
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
		contextMenu->addAction(meshAction);
		contextMenu->exec(event->globalPos());
	}
}

void GFTreeModelWidget::exportWord(const QString& directory, QTreeWidgetItem* item)
{
	QWidget* parent = parentWidget();
	while (parent) {
		GFImportModelWidget* gfParent = dynamic_cast<GFImportModelWidget*>(parent);
		if (gfParent)
		{
			auto logWidget = gfParent->GetLogWidget();
			auto textEdit = logWidget->GetTextEdit();

			ProjectPropertyWidge* m_projectPropertyWidge = gfParent->GetProjectPropertyWidget();
			QTableWidget* m_projectTableWid = m_projectPropertyWidge->GetQTableWidget();

			GeomPropertyWidget* m_geomPropertyWidget = gfParent->GetGeomPropertyWidget();
			QTableWidget* m_geomTableWid = m_geomPropertyWidget->GetQTableWidget();

			MaterialPropertyWidget* m_materialPropertyWidget = gfParent->GetMaterialPropertyWidget();
			QTableWidget* m_materialTableWid = m_materialPropertyWidget->GetQTableWidget();

			DatabasePropertyWidget* m_databasePropertyWidget = gfParent->GetDatabasePropertyWidget();
			QTableWidget* m_databaseTableWid = m_databasePropertyWidget->GetQTableWidget();

			for (int i = 0; i < item->childCount(); ++i) {
				QTreeWidgetItem* childItem = item->child(i);
				auto originalName = childItem->text(0);
				int dotIndex = originalName.indexOf('.');
				QString processedName;
				if (dotIndex != -1)
				{
					processedName = originalName.mid(dotIndex + 1).trimmed();
				}
				else {
					processedName = originalName;
				}

				bool isChecked = (childItem->checkState(0) == Qt::Checked);
				if (isChecked)
				{
					if (processedName == "跌落试验")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出跌落试验报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						FallPropertyWidget* m_fallPropertyWidget = gfParent->GetFallPropertyWidget();
						QTableWidget* m_fallTableWid = m_fallPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);
						
						// 跌落输入数据
						data.insert("测试项目", m_fallTableWid->item(1, 2)->text());
						data.insert("跌落高度", m_fallTableWid->item(2, 2)->text());
						QComboBox* comboBox = qobject_cast<QComboBox*>(m_fallTableWid->cellWidget(3, 2));
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
							data.insert("冲击波超压传感器数量", m_fallTableWid->item(6, 2)->text());
						}
						else
						{
							data.insert("冲击波超压传感器数量", "");
						}
						data.insert("风速", m_fallTableWid->item(7, 2)->text());

						

						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir("src/template/计算模型.png").absolutePath());
						imagePaths.insert("应力云图", QDir("src/template/跌落试验/应力云图.png").absolutePath());
						imagePaths.insert("应变云图", QDir("src/template/跌落试验/应变云图.png").absolutePath());
						imagePaths.insert("温度云图", QDir("src/template/跌落试验/温度云图.png").absolutePath());
						imagePaths.insert("超压云图", QDir("src/template/跌落试验/超压云图.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出跌落仿真报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/跌落仿真计算数据表.docx").absolutePath(), directory + "/跌落仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出跌落试验报告";
										text = text + "\n" + timeStr + "[信息]>跌落试验报告：" + directory + "/跌落仿真计算数据表.docx";
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

					}
					else if (processedName == "快速烤燃试验")
					{

					}
					else if (processedName == "慢速烤燃试验")
					{

					}
					else if (processedName == "枪击试验")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出枪击试验报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						ShootPropertyWidget* m_shootPropertyWidget = gfParent->GetShootPropertyWidget();
						QTableWidget* m_shootTableWid = m_shootPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetShootStressResultWidget();
						QTableWidget* m_stressTableWid = m_stressResultWidget->GetQTableWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetShootStrainResultWidget();
						QTableWidget* m_strainTableWid = m_strainResultWidget->GetQTableWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetShootTemperatureResultWidget();
						QTableWidget* m_temperatureTableWid = m_temperatureResultWidget->GetQTableWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetShootOverpressureResultWidget();
						QTableWidget* m_overpressureTableWid = m_overpressureResultWidge->GetQTableWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_shootTableWid->item(1, 2)->text());
						data.insert("撞击速度", m_shootTableWid->item(2, 2)->text());
						data.insert("撞击角度", m_shootTableWid->item(3, 2)->text());
						data.insert("子弹型式", m_shootTableWid->item(4, 2)->text());
						data.insert("子弹直径", m_shootTableWid->item(5, 2)->text());
						data.insert("子弹硬度", m_shootTableWid->item(6, 2)->text());
						data.insert("温度传感器数量", m_shootTableWid->item(7, 2)->text());
						data.insert("超压传感器数量", m_shootTableWid->item(8, 2)->text());
						data.insert("风速", m_shootTableWid->item(9, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir("src/template/计算模型.png").absolutePath());
						imagePaths.insert("应力云图", QDir("src/template/枪击试验/应力云图.png").absolutePath());
						imagePaths.insert("应变云图", QDir("src/template/枪击试验/应变云图.png").absolutePath());
						imagePaths.insert("温度云图", QDir("src/template/枪击试验/温度云图.png").absolutePath());
						imagePaths.insert("超压云图", QDir("src/template/枪击试验/超压云图.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出枪击仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/枪击仿真计算数据表.docx").absolutePath(), directory + "/枪击仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出枪击试验报告";
										text = text + "\n" + timeStr + "[信息]>枪击试验报告：" + directory + "/枪击仿真计算数据表.docx";
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
					}
					else if (processedName == "射流冲击试验")
					{

					}
					else if (processedName == "破片撞击试验")
					{
						{
							QDateTime currentTime = QDateTime::currentDateTime();
							QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
							QString text = timeStr + "[信息]>开始导出破片撞击试验报告";
							textEdit->appendPlainText(text);
							logWidget->update();
							// 关键：强制刷新UI，确保日志立即显示
							QApplication::processEvents();
						}

						FragmentationImpactPropertyWidget* m_fragmentationImpactPropertyWidget = gfParent->GetFragmentationImpactPropertyWidget();
						QTableWidget* m_fragmentationImpactTableWid = m_fragmentationImpactPropertyWidget->GetQTableWidget();

						StressResultWidget* m_stressResultWidget = gfParent->GetFragmentationImpactStressResultWidget();

						StrainResultWidget* m_strainResultWidget = gfParent->GetFragmentationImpactStrainResultWidget();

						TemperatureResultWidget* m_temperatureResultWidget = gfParent->GetFragmentationImpactTemperatureResultWidget();

						OverpressureResultWidget* m_overpressureResultWidge = gfParent->GetFragmentationImpactOverpressureResultWidget();

						QMap<QString, QVariant> data = convertTextData(m_projectPropertyWidge,
							m_geomPropertyWidget,
							m_materialPropertyWidget,
							m_databasePropertyWidget,
							m_stressResultWidget,
							m_strainResultWidget,
							m_temperatureResultWidget,
							m_overpressureResultWidge);

						// 跌落输入数据
						data.insert("测试项目", m_fragmentationImpactTableWid->item(1, 2)->text());
						data.insert("撞击速度", m_fragmentationImpactTableWid->item(2, 2)->text());
						data.insert("撞击角度", m_fragmentationImpactTableWid->item(3, 2)->text());
						data.insert("破片形状", m_fragmentationImpactTableWid->item(4, 2)->text());
						data.insert("破片直径", m_fragmentationImpactTableWid->item(5, 2)->text());
						data.insert("破片质量", m_fragmentationImpactTableWid->item(6, 2)->text());
						data.insert("破片硬度", m_fragmentationImpactTableWid->item(7, 2)->text());
						data.insert("温度传感器数量", m_fragmentationImpactTableWid->item(8, 2)->text());
						data.insert("超压传感器数量", m_fragmentationImpactTableWid->item(9, 2)->text());
						data.insert("风速", m_fragmentationImpactTableWid->item(10, 2)->text());


						QMap<QString, QString> imagePaths;
						imagePaths.insert("计算模型", QDir("src/template/计算模型.png").absolutePath());
						imagePaths.insert("应力云图", QDir("src/template/破片撞击试验/应力云图.png").absolutePath());
						imagePaths.insert("应变云图", QDir("src/template/破片撞击试验/应变云图.png").absolutePath());
						imagePaths.insert("温度云图", QDir("src/template/破片撞击试验/温度云图.png").absolutePath());
						imagePaths.insert("超压云图", QDir("src/template/破片撞击试验/超压云图.png").absolutePath());
						QMap<QString, QVector<QVector<QVariant>>> tableData;

						// 创建进度对话框
						ProgressDialog* progressDialog = new ProgressDialog("导出破片撞击仿真计算报告进度", gfParent);
						progressDialog->show();

						// 创建工作线程和工作对象
						WordExporterWorker* wordExporterWorker = new WordExporterWorker(QDir("src/template/破片撞击仿真计算数据表.docx").absolutePath(), directory + "/破片撞击仿真计算数据表.docx", data, imagePaths, tableData);
						QThread* wordExporterThread = new QThread();
						wordExporterWorker->moveToThread(wordExporterThread);

						// 连接信号槽
						connect(wordExporterThread, &QThread::started, wordExporterWorker, &WordExporterWorker::DoWork);
						connect(wordExporterWorker, &WordExporterWorker::ProgressUpdated, progressDialog, &ProgressDialog::SetProgress);
						connect(wordExporterWorker, &WordExporterWorker::StatusUpdated, progressDialog, &ProgressDialog::SetStatusText);
						connect(progressDialog, &ProgressDialog::Canceled, wordExporterWorker, &WordExporterWorker::RequestInterruption, Qt::DirectConnection);

						// 处理导入结果
						connect(wordExporterWorker, &WordExporterWorker::WorkFinished, this,
							[=](bool success, const QString& msg)
							{
								if (success)
								{
									// 更新日志
									{
										QDateTime currentTime = QDateTime::currentDateTime();
										QString timeStr = currentTime.toString("yyyy-MM-dd hh:mm:ss");
										QString text = timeStr + "[信息]>成功导出破片撞击试验报告";
										text = text + "\n" + timeStr + "[信息]>破片撞击试验报告：" + directory + "/破片撞击仿真计算数据表.docx";
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
					}
					else if (processedName == "爆炸冲击波试验")
					{

					}
					else if (processedName == "殉爆试验")
					{

					}
				}
			}
			break;
		}
		else
		{
			parent = parent->parentWidget();
		}
	}
}




QMap<QString, QVariant> GFTreeModelWidget::convertTextData(ProjectPropertyWidge* projectPropertyWidge,
	GeomPropertyWidget* geomPropertyWidget,
	MaterialPropertyWidget* materialPropertyWidget,
	DatabasePropertyWidget* databasePropertyWidget,
	StressResultWidget* stressResultWidget,
	StrainResultWidget* strainResultWidget,
	TemperatureResultWidget* temperatureResultWidget,
	OverpressureResultWidget* overpressureResultWidge)
{
	QTableWidget* m_projectTableWid = projectPropertyWidge->GetQTableWidget();

	QTableWidget* m_geomTableWid = geomPropertyWidget->GetQTableWidget();

	QTableWidget* m_materialTableWid = materialPropertyWidget->GetQTableWidget();

	QTableWidget* m_databaseTableWid = databasePropertyWidget->GetQTableWidget();

	QTableWidget* m_stressTableWid = stressResultWidget->GetQTableWidget();

	QTableWidget* m_strainTableWid = strainResultWidget->GetQTableWidget();

	QTableWidget* m_temperatureTableWid = temperatureResultWidget->GetQTableWidget();

	QTableWidget* m_overpressureTableWid = overpressureResultWidge->GetQTableWidget();

	QMap<QString, QVariant> data;
	// 标题数据
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


	// 计算输出数据
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

	return data;
}