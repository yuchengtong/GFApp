#pragma execution_character_set("utf-8")
#include "GraphicWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>

#include "GraphicWidget.h"

GraphicWidget::GraphicWidget(QWidget* parent)
	: QWidget(parent)
{
    m_graph = create3DSurfaceGraph();

    // 设置显示网格
    m_graph->activeTheme()->setGridEnabled(true);
    // 设置背景可用
    m_graph->activeTheme()->setBackgroundEnabled(true);
    // 标签可用
    m_graph->activeTheme()->setLabelBackgroundEnabled(true);


    // 创建三维曲面图容器
    QWidget* graphContainer = QWidget::createWindowContainer(m_graph);

    // 设置布局（让3D图表占满整个GraphicWidget）
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(graphContainer);
}

GraphicWidget::~GraphicWidget()
{

}

QAbstract3DGraph* GraphicWidget::create3DSurfaceGraph()
{
    // 创建三维曲面图对象
    surface = new Q3DSurface;
    
    //! 创建三维散点图的坐标轴
    //! 因为是三维散点图，所以包括X、Y、Z三个方向的坐标轴(并且三个坐标轴类型都为值坐标轴哦)
    // 创建X、Y、Z轴并添加 q
    m_axisX = createValue3DAxis("壳体厚度");
    m_axisY = createValue3DAxis("跌落高度");
    m_axisZ = createValue3DAxis("壳体最大应力");
    surface->setAxisX(m_axisX);
    surface->setAxisY(m_axisY);
    surface->setAxisZ(m_axisZ);


    // 创建三维曲面图系列对象
    series = new QSurface3DSeries;
    // 将系列添加到三维曲面图中
    surface->addSeries(series);

    // 创建三维曲面图数据容器
    m_array = new QSurfaceDataArray;
    // 创建三维曲面图数据
    for (int index = 0; index != 3; ++index)
    {
        // 创建三维曲面图行数据容器
        QSurfaceDataRow* dataRow = new QSurfaceDataRow;
        // 遍历添加数据到行容器
        for (int valIdx = 0; valIdx != 3; ++valIdx)
        {
            // 随机数添加数据
            dataRow->append(QVector3D(0, 0, 0));
        }
        // 将行容器添加到array中
        m_array->append(dataRow);
    }
    // 将数据添加到系列对象中
    series->dataProxy()->resetArray(m_array);

    // 返回三维曲面图对象
    return surface;
}


QValue3DAxis* GraphicWidget::createValue3DAxis(QString axisTitle, bool titleVisible, float min, float max)
{
    // 创建值坐标轴对象
    QValue3DAxis* axis = new QValue3DAxis;
    axis->setTitle(axisTitle);  // 设置坐标轴标题
    axis->setTitleVisible(titleVisible); // 设置标题是否显示
    axis->setRange(min, max);   // 设置坐标轴取值范围
    // 返回坐标轴对象
    return axis;
}

QCategory3DAxis* GraphicWidget::createCategory3DAxis(QString axisTitle, bool titleVisible, QStringList labList)
{
    // 创建文本坐标轴对象
    QCategory3DAxis* axis = new QCategory3DAxis;
    axis->setTitle(axisTitle);  // 设置坐标轴标题
    axis->setTitleVisible(titleVisible); // 设置标题是否显示
    axis->setLabels(labList);   // 设置坐标轴标签
    // 返回坐标轴对象
    return axis;
}


void GraphicWidget::on_angleValueChange(int type, int val)
{
    // 判断当前方向类型，并将角度赋到对应位置
    if (0 == type)
    {
        // 获取图表的视图->活动摄像头->设置角度
        m_graph->scene()->activeCamera()->setXRotation(val);
    }
    else if (1 == type)
    {       
        // 获取图表的视图->活动摄像头->设置角度
        m_graph->scene()->activeCamera()->setYRotation(val);
    }
}


void GraphicWidget::on_scaleSlider_sliderMoved(int position)
{
    // 设置图表缩放
    m_graph->scene()->activeCamera()->setZoomLevel(position);
}



template<class T>
void setSeriesStyle(T graphi, int index)
{
    // 循环设置图表样式
    foreach(QAbstract3DSeries * series, graphi->seriesList())
    {
        //! 设置样式
        //! 索引值加1是防止设置值为0的Mesh，未作出对应操作设置该值的样式会导致程序崩溃
        //! 帮助中这样形容（翻译）：用户定义网格，通过QAbstract3DSeries:：userDefinedMesh属性设置。
        series->setMesh(QAbstract3DSeries::Mesh(index + 1));
    }
}

void GraphicWidget::axisTitleChange(QString xName, QString yName, QString zName)
{
    if (xName != "")
    {
        m_axisX->setTitle(xName);
    }
    if (yName != "")
    {
        m_axisZ->setTitle(yName);
    }
    if (zName != "")
    {
        m_axisY->setTitle(zName);
    }
}

void GraphicWidget::dataUpdate(const QVector<double>& xCoords,
    const QVector<double>& yCoords,
    const QVector<QVector<double>>& newData,
    int rowCount,
    int columnCount,
    double xMin,
    double xMax,
    double yMin,
    double yMax)
{
    // 清空旧数据（保留容器，避免重复创建）
    m_array->clear();

    double m_xScaleFactor = 50.0; // 缩放系数（150~300可调，越大X轴越宽）
    double scaledXMin = xMin * m_xScaleFactor;
    double scaledXMax = xMax * m_xScaleFactor;

    for (int i = 0; i < rowCount; ++i) {
        double x = xCoords[i] * m_xScaleFactor; // 精确 X 坐标（直接从入参获取，不做任何计算）
        auto* row = new QSurfaceDataRow(columnCount);

        for (int j = 0; j < columnCount; ++j) {
            double y = yCoords[j]; // 精确 Y 坐标（直接从入参获取）
            double z = newData[i][j];// 对应 (x,y) 处的 Z 值
            (*row)[j].setPosition(QVector3D(x, z, y)); // 精确三维坐标
        }

        m_array->append(row);
    }

    auto zRange = calculateZRange(newData);
    auto zMin = zRange.first;
    auto zMax = zRange.second;

    m_axisX->setRange(scaledXMin, scaledXMax);
    m_axisZ->setRange(yMin, yMax);
    m_axisY->setRange(zMin, zMax);

    QLinearGradient gradient;
    gradient.setColorAt(0.0, Qt::green);
    gradient.setColorAt(0.8, Qt::red);
    gradient.setColorAt(1.0, Qt::red);
    series->setBaseGradient(gradient);
    series->setColorStyle(Q3DTheme::ColorStyleRangeGradient);

    series->dataProxy()->resetArray(m_array);
    surface->show();
}

void GraphicWidget::setAxisAutoAdjust(bool xAuto, bool yAuto, bool zAuto)
{
    if (!surface) return;
    // 禁用自动调整后，轴范围由手动控制（避免 Qt 自动缩放覆盖比例）
    surface->axisX()->setAutoAdjustRange(xAuto);
    surface->axisY()->setAutoAdjustRange(yAuto);
    surface->axisZ()->setAutoAdjustRange(zAuto);
}

QPair<double, double> GraphicWidget::calculateZRange(const QVector<QVector<double>>& newData)
{
    double minZ = std::numeric_limits<float>::max();
    double maxZ = std::numeric_limits<float>::min();

    for (const auto& row : newData) {
        for (double z : row) {
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
    }

    // 扩大 5% 的范围，避免数据紧贴轴边界
    double offset = (maxZ - minZ) * 0.05f;
    return { minZ - offset, maxZ + offset };
}