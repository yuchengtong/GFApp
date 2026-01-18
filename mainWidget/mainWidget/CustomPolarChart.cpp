#pragma execution_character_set("utf-8")
#include "CustomPolarChart.h"
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtWidgets/QGraphicsScene>
#include <QtCore/QDebug>
#include <QtMath>

static constexpr int SECTORS = 8;

CustomPolarChart::CustomPolarChart(const QVector<QVector<double>>& datasets,
	const QVector<QStringList>& labelGroups,
	const QVector<double>& standardValues,
	const QStringList& legendNames,
	const QStringList& unitList,
	QGraphicsItem* parent)
	: QPolarChart(parent),
	m_datasets(datasets),
	m_labelGroups(labelGroups),
	m_legendNames(legendNames),
	m_unitList(unitList)
{
	m_colors = { QColor(0, 122, 255), QColor(255, 80, 80), QColor(34, 255, 76), QColor(255, 255, 0) };

	setupChart();
	buildSeries();
	drawOctagonOverlay();
	drawStandardCircle();
	setActiveDataset(0);
}

void CustomPolarChart::setupChart()
{
	m_angleAxis = new QCategoryAxis(this);
	m_angleAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	m_angleAxis->setRange(0, 360);
	m_angleAxis->setLabelsColor(Qt::white);
	

	m_radialAxis = new QValueAxis(this);
	m_radialAxis->setRange(0, 100);
	m_radialAxis->setTickCount(5);  // 设置刻度数量（包括0点）
	m_radialAxis->setGridLineVisible(false);
	m_radialAxis->setLineVisible(false);
	m_radialAxis->setLabelsVisible(false);
	m_radialAxis->setLabelsColor(Qt::white);

	this->addAxis(m_angleAxis, QPolarChart::PolarOrientationAngular);
	this->addAxis(m_radialAxis, QPolarChart::PolarOrientationRadial);

	this->legend()->setVisible(true);
	this->legend()->setAlignment(Qt::AlignBottom);
	this->legend()->setLabelColor(Qt::white);
	this->setBackgroundVisible(false);

	this->setTitle("跌落试验");
	this->setTitleBrush(Qt::white);

	updateChartTitle();

	// 初始化右上角单位文本控件
	m_unitTextItem = new QGraphicsTextItem(this);
	QFont unitFont = m_unitTextItem->font();
	unitFont.setPointSize(9);    // 字号适中
	unitFont.setBold(true);      // 加粗突出
	m_unitTextItem->setFont(unitFont);
	m_unitTextItem->setDefaultTextColor(Qt::white); // 白色和图表配色一致
	m_unitTextItem->setZValue(99); // 置顶层级，永远不被遮挡

	// 保证八边形在绘图区域变化时重绘（包含窗口/大小变化/坐标变换）
	connect(this, &QChart::plotAreaChanged, this, [this](const QRectF&) {
		drawOctagonOverlay();
		drawStandardCircle();
		updateUnitText();
		});
}

void CustomPolarChart::buildSeries()
{
	for (auto s : m_lineSeries) { this->removeSeries(s); delete s; }
	m_lineSeries.clear();

	for (auto a : m_areaSeries) { this->removeSeries(a); delete a; }
	m_areaSeries.clear();

	QLineSeries* baseline = new QLineSeries();
	for (int i = 0; i < SECTORS; ++i) baseline->append((360.0 / SECTORS) * i, 0.0);
	baseline->append(360.0, 0.0);

	int groups = qMin(4, m_datasets.size());
	for (int g = 0; g < groups; ++g) {
		QLineSeries* ls = new QLineSeries();
		const QVector<double>& data = m_datasets[g];
		for (int i = 0; i < SECTORS; ++i) {
			qreal angle = (360.0 / SECTORS) * i;
			qreal radius = (i < data.size()) ? data[i] : 0.0;
			ls->append(angle, radius);
		}
		qreal firstRadius = data.isEmpty() ? 0.0 : data[0];
		ls->append(360.0, firstRadius);

		QAreaSeries* area = new QAreaSeries(ls, baseline);
		area->setName(m_legendNames.value(g, QString("item%1").arg(g)));

		this->addSeries(area);
		this->addSeries(ls);

		// 隐藏线条图例
		for (QLegendMarker* marker : this->legend()->markers(ls))
			marker->setVisible(false);

		ls->attachAxis(m_angleAxis);
		ls->attachAxis(m_radialAxis);
		area->attachAxis(m_angleAxis);
		area->attachAxis(m_radialAxis);

		m_lineSeries.append(ls);
		m_areaSeries.append(area);

		applySeriesStyle(ls, m_colors[g % m_colors.size()], (g == m_activeIndex) ? 1.0 : 0.0);
		QColor fill = m_colors[g % m_colors.size()];
		fill.setAlpha((g == m_activeIndex) ? 110 : 0);
		area->setBrush(QBrush(fill, Qt::SolidPattern));
		area->setVisible(g == m_activeIndex);
	}

	for (QLegendMarker* marker : this->legend()->markers(baseline)) marker->setVisible(false);

	for (int i = 0; i < m_areaSeries.size(); ++i) {
		for (QLegendMarker* marker : this->legend()->markers(m_areaSeries[i])) {
			marker->setProperty("datasetIndex", i);
			connect(marker, &QLegendMarker::clicked, this, &CustomPolarChart::handleLegendClicked);
		}
	}
}

void CustomPolarChart::applySeriesStyle(QLineSeries* series, const QColor& color, qreal opacity)
{
	QPen pen(color);
	pen.setWidth(2);
	pen.setJoinStyle(Qt::RoundJoin);
	series->setPen(pen);
	series->setOpacity(opacity);
	series->setPointsVisible(false);
}

void CustomPolarChart::handleLegendClicked()
{
	QLegendMarker* marker = qobject_cast<QLegendMarker*>(sender());
	if (!marker) return;
	int idx = marker->property("datasetIndex").toInt();
	setActiveDataset(idx);
	emit datasetChanged(idx);
}

void CustomPolarChart::setActiveDataset(int idx)
{
	if (idx < 0 || idx >= m_areaSeries.size()) return;
	m_activeIndex = idx;

	for (int i = 0; i < m_lineSeries.size(); ++i) {
		QColor c = m_colors[i % m_colors.size()];
		if (i == idx) {
			applySeriesStyle(m_lineSeries[i], c, 1.0);
			QColor fill = c; 
			fill.setAlpha(110);
			m_areaSeries[i]->setBrush(QBrush(fill, Qt::SolidPattern));
			m_areaSeries[i]->setVisible(true);
		}
		else {
			applySeriesStyle(m_lineSeries[i], c, 0.0);
			m_areaSeries[i]->setVisible(false);
		}
	}
	// 图例全部显示，不隐藏任何 marker
	for (int i = 0; i < m_areaSeries.size(); ++i) {
		for (QLegendMarker* marker : this->legend()->markers(m_areaSeries[i])) {
			marker->setVisible(true);
			marker->setProperty("datasetIndex", i);
		}
	}
	updateRadialAxisRange(idx);
	// 更新标准值圈（切换数据集时更新为对应标准值）
	drawStandardCircle();
	updateAngleLabels(idx);

	updateChartTitle();
	updateUnitText();
}

void CustomPolarChart::setActiveDatasetVisible(int idx)
{
	if (idx < 0 || idx >= m_areaSeries.size()) return;
	m_activeIndex = idx;

	for (int i = 0; i < m_lineSeries.size(); ++i) {
		QColor c = m_colors[i % m_colors.size()];
		if (i == idx) {
			// 高亮线条
			applySeriesStyle(m_lineSeries[i], c, 1.0);

			// 阴影渐变
			QRadialGradient gradient(QPointF(0, 0), 1.0); // 中心为0,0，半径为1
			QColor startColor = c; startColor.setAlpha(150); // 内层颜色
			QColor endColor = c; endColor.setAlpha(0);       // 外层透明
			gradient.setColorAt(0.0, startColor);
			gradient.setColorAt(1.0, endColor);
			gradient.setCoordinateMode(QGradient::ObjectBoundingMode); // 相对坐标

			m_areaSeries[i]->setBrush(QBrush(gradient));
			m_areaSeries[i]->setVisible(true);
		}
		else {
			// 其他线条半透明
			applySeriesStyle(m_lineSeries[i], c, 0.0);
			// 隐藏阴影
			m_areaSeries[i]->setVisible(false);
		}
	}
	// 只显示当前激活数据集（温度）的图例，隐藏其他图例
	for (int i = 0; i < m_areaSeries.size(); ++i) {
		for (QLegendMarker* marker : this->legend()->markers(m_areaSeries[i])) {
			marker->setVisible(i == m_activeIndex);  // 仅激活的数据集图例可见
			marker->setProperty("datasetIndex", i);
		}
	}
	updateAngleLabels(idx);
	updateChartTitle();
	updateUnitText();
}

void CustomPolarChart::updateAngleLabels(int datasetIndex)
{
	if (m_angleAxis) { this->removeAxis(m_angleAxis); delete m_angleAxis; }

	m_angleAxis = new QCategoryAxis(this);
	m_angleAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	m_angleAxis->setRange(0, 360);

	const QStringList labels = (datasetIndex < m_labelGroups.size()) ? m_labelGroups[datasetIndex] : QStringList();
	// 获取当前激活数据集的数值
	const QVector<double>& currentData = (datasetIndex < m_datasets.size()) ? m_datasets[datasetIndex] : QVector<double>();

	for (int i = 0; i < SECTORS; ++i) {
		qreal angle = (360.0 / SECTORS) * i;
		QString lab = (i < labels.size()) ? labels[i] : QString("%1").arg(i);
		// 获取当前角度对应的数值，无数据时显示0
        double value = (i < currentData.size()) ? currentData[i] : 0.0;
		lab = lab + ":" + QString::number(value);
		m_angleAxis->append(lab, angle);
		m_angleAxis->setLabelsColor(Qt::white);
	}

	this->addAxis(m_angleAxis, QPolarChart::PolarOrientationAngular);
	for (QLineSeries* ls : m_lineSeries) ls->attachAxis(m_angleAxis);
	for (QAreaSeries* ar : m_areaSeries) ar->attachAxis(m_angleAxis);
}

void CustomPolarChart::drawOctagonOverlay()
{
	if (!scene()) return;
	if (m_octagonItem) { scene()->removeItem(m_octagonItem); delete m_octagonItem; m_octagonItem = nullptr; }

	QPolygonF polygon;
	qreal maxR = m_radialAxis ? m_radialAxis->max() : 100.0;
	for (int i = 0; i < SECTORS; ++i) {
		qreal angle = (360.0 / SECTORS) * i;
		QPointF pt = this->mapToPosition(QPointF(angle, maxR), nullptr);
		polygon << pt;
	}
	polygon << polygon.front();

	m_octagonItem = new QGraphicsPolygonItem(polygon);
	QPen pen(Qt::gray); pen.setWidth(1);
	m_octagonItem->setPen(pen);
	m_octagonItem->setBrush(Qt::NoBrush);
	m_octagonItem->setZValue(0);
	scene()->addItem(m_octagonItem);
}

void CustomPolarChart::drawStandardCircle()
{
	if (!scene()) return;
	// 移除旧的标准值圈
	if (m_standardCircleItem) {
		scene()->removeItem(m_standardCircleItem);
		delete m_standardCircleItem;
		m_standardCircleItem = nullptr;
	}

	// 获取当前激活数据集的标准值
	double standardValue = (m_activeIndex < m_standardValues.size()) ? m_standardValues[m_activeIndex] : 50.0;
	if (standardValue < 0) standardValue = 0;

	// 计算标准值圈的位置和大小
	QPointF center = this->mapToPosition(QPointF(0, 0), nullptr);
	QPointF edge = this->mapToPosition(QPointF(0, standardValue), nullptr);
	qreal radius = qSqrt(qPow(edge.x() - center.x(), 2) + qPow(edge.y() - center.y(), 2));

	// 创建圆形
	m_standardCircleItem = new QGraphicsEllipseItem(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
	QPen pen(Qt::yellow, 2);  // 黄色标准值圈，宽度2
	pen.setStyle(Qt::DashLine);  // 虚线样式
	m_standardCircleItem->setPen(pen);
	m_standardCircleItem->setBrush(Qt::NoBrush);
	m_standardCircleItem->setZValue(1);  // 层级高于八边形，低于数据系列
	scene()->addItem(m_standardCircleItem);
}

void CustomPolarChart::updateDatasets(const QVector<QVector<double>>& datasets,
	const QVector<QStringList>& labelGroups,
	const QVector<double>& standardValues,
	const QStringList& unitList)
{
	m_datasets = datasets;
	m_labelGroups = labelGroups;
	m_standardValues = standardValues;
	m_unitList = unitList;

	double globalMax = 1.0; 
	for (const auto& grp : m_datasets) 
		for (double v : grp) 
			globalMax = qMax(globalMax, v);
	for (double sv : m_standardValues)
		globalMax = qMax(globalMax, sv);
	if (m_radialAxis) m_radialAxis->setMax(globalMax * 1.1);

	for (int g = 0; g < m_lineSeries.size() && g < m_datasets.size(); ++g) {
		QLineSeries* ls = m_lineSeries[g]; 
		ls->clear();
		const QVector<double>& data = m_datasets[g];
		for (int i = 0; i < SECTORS; ++i) {
			qreal angle = (360.0 / SECTORS) * i;
			qreal radius = (i < data.size()) ? data[i] : 0.0;
			ls->append(angle, radius);
		}
		qreal firstRadius = data.isEmpty() ? 0.0 : data[0];
		ls->append(360.0, firstRadius);
	}
	drawOctagonOverlay();
	drawStandardCircle();
	updateRadialAxisRange(m_activeIndex);
	updateAngleLabels(m_activeIndex);
	updateChartTitle();
	updateUnitText();
}

void CustomPolarChart::renameLegend(int index, const QString& newName)
{
	if (index < 0 || index >= m_areaSeries.size())
		return;

	// 方法1：直接修改系列名称（推荐）
	m_lineSeries[index]->setName(newName);
	m_areaSeries[index]->setName(newName);

	// 方法2：通过图例标记查找并修改（适用于动态查找场景）
	QLegend* legend = this->legend();
	if (!legend) return;

	// 遍历所有图例标记
	for (QLegendMarker* marker : legend->markers()) {
		// 找到对应数据集的标记
		if (marker->property("datasetIndex").toInt() == index) {
			// 修改关联系列的名称（会自动更新图例显示）
			marker->series()->setName(newName);
			break;
		}
	}
}

void CustomPolarChart::setStandardValue(int idx, double value)
{
	if (idx < 0 || idx >= m_standardValues.size()) return;
	m_standardValues[idx] = value;
	if (idx == m_activeIndex) {
		drawStandardCircle();
		updateRadialAxisRange(idx); // 修改标准值后也同步更新坐标轴
		updateChartTitle();
		updateUnitText();
	}
}

void CustomPolarChart::updateRadialAxisRange(int idx)
{
	if (idx < 0 || idx >= m_datasets.size() || m_radialAxis == nullptr) return;

	// 获取当前选中数据集的所有数据
	const QVector<double>& currentData = m_datasets[idx];
	if (currentData.isEmpty())
	{
		m_radialAxis->setRange(0, 100);
		return;
	}

	// 计算当前数据集的最大值
	double dataMax = 0.0;
	for (double val : currentData)
	{
		if (val > dataMax) dataMax = val;
	}

	// 取【数据最大值】和【当前数据集标准值】的较大值，保证标准圈一定在坐标轴内
	double standardVal = (idx < m_standardValues.size()) ? m_standardValues[idx] : 0.0;
	double finalMax = qMax(dataMax, standardVal);

	// 乘以1.1倍，给图表留边距，避免数据点贴到图表边缘
	m_radialAxis->setRange(0, finalMax * 1.1);
}

// 更新图表标题（包含标准值）
void CustomPolarChart::updateChartTitle()
{
	// 获取当前激活数据集的标准值
	double standardValue = (m_activeIndex < m_standardValues.size()) ? m_standardValues[m_activeIndex] : 0.0;
	if (standardValue > 0.0)
	{
		
		// 拼接标题文本（格式可自定义，例如：跌落试验 - 标准值：85.0）
		QString title = QString(this->title() + " - 阈值：") + QString::number(standardValue);
		this->setTitle(title);
		this->setTitleBrush(Qt::white); // 保持标题颜色为白色
	}
	
}

// 更新右上角单位文本（位置+内容）
void CustomPolarChart::updateUnitText()
{
	if (!m_unitTextItem) return;
	// 1. 获取当前激活数据集对应的单位，无单位则显示空字符串
	QString currUnit = (m_activeIndex < m_unitList.size()) ? m_unitList[m_activeIndex] : "no";
	if (currUnit == "no")
	{
		currUnit = " ";
		
	}
	else
	{
		currUnit = "单位：" + currUnit;
	}
	m_unitTextItem->setPlainText(currUnit);
	// 3. 计算图表右上角的精准坐标：固定在绘图区右上角，向右偏移10px，向上偏移10px
	QRectF plotArea = this->plotArea();
	QPointF rightTopPoint = plotArea.topRight();
	// 文本左对齐，所以x坐标要减去文本宽度，保证贴紧右侧、不溢出
	QRectF textRect = m_unitTextItem->boundingRect();
	qreal x = rightTopPoint.x() - textRect.width() + 200 ;
	qreal y = rightTopPoint.y() - 60;
	m_unitTextItem->setPos(x, y);
	m_unitTextItem->setDefaultTextColor(Qt::white);
}