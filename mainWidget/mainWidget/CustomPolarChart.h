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
        const QVector<double>& standardValues,// 每个数据集的标准值
        const QStringList& legendNames = { "应力","温度","超压","应变" },
        const QStringList& unitList = { "MPa", "℃", "MPa", " " },
        QGraphicsItem* parent = nullptr);

    void updateDatasets(const QVector<QVector<double>>& datasets,
        const QVector<QStringList>& labelGroups,
        const QVector<double>& standardValues,
        const QStringList& unitList);

    void setActiveDatasetVisible(int idx);

    // 设置单个数据集标准值的接口
    void setStandardValue(int idx, double value);

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
    void drawStandardCircle();  // 绘制标准值参考圈
    void updateRadialAxisRange(int idx); //更新坐标轴范围
    void updateChartTitle();// 修改标题添加标准值
    void updateUnitText();//修改单位显示

    QVector<QVector<double>> m_datasets;
    QVector<QStringList> m_labelGroups;
    QStringList m_legendNames;
    QVector<double> m_standardValues;  // 存储每个数据集的标准值
    QStringList m_unitList;//单位集合

    QVector<QLineSeries*> m_lineSeries;
    QVector<QAreaSeries*> m_areaSeries;
    QVector<QColor> m_colors;

    QCategoryAxis* m_angleAxis = nullptr;
    QValueAxis* m_radialAxis = nullptr;

    QGraphicsPolygonItem* m_octagonItem = nullptr;
    QGraphicsEllipseItem* m_standardCircleItem = nullptr;  // 标准值参考圈
    QGraphicsTextItem* m_unitTextItem; // 右上角单位文本控件
    int m_activeIndex = 0;
};