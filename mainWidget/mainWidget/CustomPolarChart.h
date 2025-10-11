#pragma once

#include <QtCharts/QPolarChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QGraphicsPolygonItem>

using namespace QtCharts;

class CustomPolarChart : public QPolarChart
{
    Q_OBJECT
public:
    explicit CustomPolarChart(const QVector<QVector<double>>& datasets,
        const QVector<QStringList>& labelGroups,
        const QStringList& legendNames = { "应力","温度","超压","应变" },
        QGraphicsItem* parent = nullptr);

    void updateDatasets(const QVector<QVector<double>>& datasets,
        const QVector<QStringList>& labelGroups);

    void setActiveDatasetVisible(int idx);

    // 修改图例名称
    void renameLegend(int index, const QString& newName);

signals:
    void datasetChanged(int index);

private slots:
    void handleLegendClicked();

private:
    void setupChart();
    void buildSeries();
    void applySeriesStyle(QLineSeries* series, const QColor& color, qreal opacity);
    void setActiveDataset(int idx);
    void drawOctagonOverlay();
    void updateAngleLabels(int datasetIndex);

    QVector<QVector<double>> m_datasets;
    QVector<QStringList> m_labelGroups;
    QStringList m_legendNames;

    QVector<QLineSeries*> m_lineSeries;
    QVector<QAreaSeries*> m_areaSeries;
    QVector<QColor> m_colors;

    QCategoryAxis* m_angleAxis = nullptr;
    QValueAxis* m_radialAxis = nullptr;

    QGraphicsPolygonItem* m_octagonItem = nullptr;
    int m_activeIndex = 0;
};