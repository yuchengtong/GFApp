#pragma execution_character_set("utf-8")
#include "CustomPolarChart.h"
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>
#include <QtGui/QPen>
#include <QtGui/QBrush>
#include <QtWidgets/QGraphicsScene>
#include <QtCore/QDebug>

static constexpr int SECTORS = 8;

CustomPolarChart::CustomPolarChart(const QVector<QVector<double>>& datasets,
	const QVector<QStringList>& labelGroups,
	const QStringList& legendNames,
	QGraphicsItem* parent)
	: QPolarChart(parent),
	m_datasets(datasets),
	m_labelGroups(labelGroups),
	m_legendNames(legendNames)
{
	m_colors = { QColor(0, 122, 255), QColor(255, 80, 80), QColor(34, 255, 76), QColor(255, 255, 0) };

	setupChart();
	buildSeries();
	drawOctagonOverlay();
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
	this->legend()->setAlignment(Qt::AlignRight);
	this->legend()->setLabelColor(Qt::white);
	this->setBackgroundVisible(false);

	this->setTitle("跌落试验");
	this->setTitleBrush(Qt::white);
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

		applySeriesStyle(ls, m_colors[g % m_colors.size()], (g == m_activeIndex) ? 1.0 : 0.25);
		QColor fill = m_colors[g % m_colors.size()]; fill.setAlpha(80);
		area->setBrush(QBrush(fill, Qt::SolidPattern));
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
			/*applySeriesStyle(m_lineSeries[i], c, 1.0);
			QColor fill = c; fill.setAlpha(110);
			m_areaSeries[i]->setVisible(true);
			m_areaSeries[i]->setBrush(QBrush(fill, Qt::SolidPattern));*/
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
	// 图例全部显示，不隐藏任何 marker
	for (int i = 0; i < m_areaSeries.size(); ++i) {
		for (QLegendMarker* marker : this->legend()->markers(m_areaSeries[i])) {
			marker->setVisible(true);
			marker->setProperty("datasetIndex", i);
		}
	}
	updateAngleLabels(idx);
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
}

void CustomPolarChart::updateAngleLabels(int datasetIndex)
{
	if (m_angleAxis) { this->removeAxis(m_angleAxis); delete m_angleAxis; }

	m_angleAxis = new QCategoryAxis(this);
	m_angleAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	m_angleAxis->setRange(0, 360);

	const QStringList labels = (datasetIndex < m_labelGroups.size()) ? m_labelGroups[datasetIndex] : QStringList();
	for (int i = 0; i < SECTORS; ++i) {
		qreal angle = (360.0 / SECTORS) * i;
		QString lab = (i < labels.size()) ? labels[i] : QString("%1").arg(i);
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

void CustomPolarChart::updateDatasets(const QVector<QVector<double>>& datasets,
	const QVector<QStringList>& labelGroups)
{
	m_datasets = datasets;
	m_labelGroups = labelGroups;

	double globalMax = 1.0; for (const auto& grp : m_datasets) for (double v : grp) globalMax = qMax(globalMax, v);
	if (m_radialAxis) m_radialAxis->setMax(globalMax * 1.1);

	for (int g = 0; g < m_lineSeries.size() && g < m_datasets.size(); ++g) {
		QLineSeries* ls = m_lineSeries[g]; ls->clear();
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
	updateAngleLabels(m_activeIndex);
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