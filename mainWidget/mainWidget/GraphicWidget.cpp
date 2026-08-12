#pragma execution_character_set("utf-8")
#include "GraphicWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>
#include <Q3DInputHandler>
#include <QMouseEvent>
#include <QTimer>

GraphicWidget::GraphicWidget(QWidget* parent)
    : QWidget(parent)
{
    m_graph = create3DSurfaceGraph();
    auto* inputHandler = qobject_cast<Q3DInputHandler*>(m_surface->activeInputHandler());

    m_graph->activeTheme()->setGridEnabled(true);
    m_graph->activeTheme()->setBackgroundEnabled(true);
    m_graph->activeTheme()->setLabelBackgroundEnabled(true);

    // 安装事件过滤器，用于右键拖拽后限制视角角度
    m_graph->installEventFilter(this);

    QWidget* graphContainer = QWidget::createWindowContainer(m_graph);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(graphContainer);
}

GraphicWidget::~GraphicWidget()
{

}

QAbstract3DGraph* GraphicWidget::create3DSurfaceGraph()
{
    m_surface = new Q3DSurface;

    m_axisX = createValue3DAxis("壳体厚度");
    m_axisY = createValue3DAxis("壳体最大应力");
    m_axisZ = createValue3DAxis("跌落高度");
    m_surface->setAxisX(m_axisX);
    m_surface->setAxisY(m_axisY);
    m_surface->setAxisZ(m_axisZ);

    m_series = new QSurface3DSeries;
    m_surface->addSeries(m_series);
    m_array = new QSurfaceDataArray;

    for (int index = 0; index != 3; ++index)
    {
        QSurfaceDataRow* dataRow = new QSurfaceDataRow;
        for (int valIdx = 0; valIdx != 3; ++valIdx)
        {
            dataRow->append(QVector3D(0, 0, 0));
        }
        m_array->append(dataRow);
    }
    m_series->dataProxy()->resetArray(m_array);

    Q3DCamera* camera = m_surface->scene()->activeCamera();
    camera->setXRotation(-125.0f);
    camera->setYRotation(30.0f);
    camera->setZoomLevel(70.0f);

    return m_surface;
}


QValue3DAxis* GraphicWidget::createValue3DAxis(QString axisTitle, bool titleVisible, float min, float max)
{
    QValue3DAxis* axis = new QValue3DAxis;
    axis->setTitle(axisTitle);
    axis->setTitleVisible(titleVisible);
    axis->setRange(min, max);
    return axis;
}

QCategory3DAxis* GraphicWidget::createCategory3DAxis(QString axisTitle, bool titleVisible, QStringList labList)
{
    QCategory3DAxis* axis = new QCategory3DAxis;
    axis->setTitle(axisTitle);
    axis->setTitleVisible(titleVisible);
    axis->setLabels(labList);
    return axis;
}


void GraphicWidget::on_angleValueChange(int type, int val)
{
    if (0 == type)
    {
        m_graph->scene()->activeCamera()->setXRotation(val);
    }
    else if (1 == type)
    {
        m_graph->scene()->activeCamera()->setYRotation(val);
    }
}


void GraphicWidget::on_scaleSlider_sliderMoved(int position)
{
    m_graph->scene()->activeCamera()->setZoomLevel(position);
}



template<class T>
void setSeriesStyle(T graphi, int index)
{
    foreach(QAbstract3DSeries * series, graphi->seriesList())
    {
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
    m_array->clear();

    double m_xScaleFactor = 10.0;
    double scaledXMin = xMin * m_xScaleFactor;
    double scaledXMax = xMax * m_xScaleFactor;

    for (int i = 0; i < rowCount; ++i)
    {
        double x = xCoords[i] * m_xScaleFactor;
        auto* row = new QSurfaceDataRow(columnCount);

        for (int j = 0; j < columnCount; ++j)
        {
            double y = yCoords[j];
            double z = newData[i][j];
            (*row)[j].setPosition(QVector3D(x, z, y));
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
    gradient.setColorAt(0.0, Qt::blue);
    gradient.setColorAt(0.5, Qt::green);
    gradient.setColorAt(1.0, Qt::red);
    m_series->setBaseGradient(gradient);
    m_series->setColorStyle(Q3DTheme::ColorStyleRangeGradient);

    m_series->dataProxy()->resetArray(m_array);
    m_surface->show();
}

void GraphicWidget::setAxisAutoAdjust(bool xAuto, bool yAuto, bool zAuto)
{
    if (!m_surface)
    {
        return;
    }

    m_surface->axisX()->setAutoAdjustRange(xAuto);
    m_surface->axisY()->setAutoAdjustRange(yAuto);
    m_surface->axisZ()->setAutoAdjustRange(zAuto);
}

QPair<double, double> GraphicWidget::calculateZRange(const QVector<QVector<double>>& newData)
{
    double minZ = std::numeric_limits<float>::max();
    double maxZ = std::numeric_limits<float>::min();

    for (const auto& row : newData)
    {
        for (double z : row)
        {
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
    }

    double offset = (maxZ - minZ) * 0.05f;
    return { minZ - offset, maxZ + offset };
}

bool GraphicWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_graph && event->type() == QEvent::MouseButtonRelease) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            QTimer::singleShot(0, this, &GraphicWidget::clampCameraAngle);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void GraphicWidget::clampCameraAngle()
{
    if (!m_surface) {
        return;
    }

    Q3DCamera* camera = m_surface->scene()->activeCamera();
    float xRot = camera->xRotation();
    float yRot = camera->yRotation();

    bool changed = false;
    if (xRot < -180.0f) { xRot = -180.0f; changed = true; }
    if (xRot > -90.0f) { xRot = -90.0f;  changed = true; }
    if (yRot < 0) { yRot = 0;   changed = true; }
    if (yRot > 90) { yRot = 90;  changed = true; }

    if (changed) {
        camera->setXRotation(xRot);
        camera->setYRotation(yRot);
    }
}