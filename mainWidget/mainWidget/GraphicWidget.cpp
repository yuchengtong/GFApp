#pragma execution_character_set("utf-8")
#include "GraphicWidget.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QSpinBox>
#include <Q3DInputHandler>
#include <QMouseEvent>
#include <QTimer>
#include <cmath>

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
    m_series->setDrawMode(QSurface3DSeries::DrawSurface);

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

// ========== 新增：高斯消元法 ==========
std::vector<double> GraphicWidget::gaussSolve(std::vector<std::vector<double>> A, std::vector<double> b)
{
    int n = A.size();
    for (int i = 0; i < n; i++) A[i].push_back(b[i]);

    for (int col = 0; col < n; col++) {
        int pivot = col;
        for (int row = col + 1; row < n; row++)
            if (std::abs(A[row][col]) > std::abs(A[pivot][col])) pivot = row;
        std::swap(A[col], A[pivot]);

        if (std::abs(A[col][col]) < 1e-12) continue;

        for (int row = col + 1; row < n; row++) {
            double factor = A[row][col] / A[col][col];
            for (int j = col; j <= n; j++) A[row][j] -= factor * A[col][j];
        }
    }

    std::vector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; i--) {
        double sum = A[i][n];
        for (int j = i + 1; j < n; j++) sum -= A[i][j] * x[j];
        x[i] = (std::abs(A[i][i]) < 1e-12) ? 0.0 : sum / A[i][i];
    }
    return x;
}

// ========== 新增：二次曲面拟合 ==========
// 拟合方程: z = a*x^2 + b*y^2 + c*x*y + d*x + e*y + f
std::vector<double> GraphicWidget::fitQuadraticSurface(
    const QVector<double>& xCoords,
    const QVector<double>& yCoords,
    const QVector<QVector<double>>& newData)
{
    double S[6][6] = {};
    double R[6] = {};

    int rowCount = newData.size();
    for (int i = 0; i < rowCount; ++i) {
        int colCount = newData[i].size();
        for (int j = 0; j < colCount; ++j) {
            double xv = xCoords[i];
            double yv = yCoords[j];
            double zv = newData[i][j];

            double phi[6] = { xv * xv, yv * yv, xv * yv, xv, yv, 1.0 };
            for (int p = 0; p < 6; ++p) {
                for (int q = 0; q < 6; ++q) {
                    S[p][q] += phi[p] * phi[q];
                }
                R[p] += phi[p] * zv;
            }
        }
    }

    std::vector<std::vector<double>> A(6, std::vector<double>(6));
    std::vector<double> b(6);
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) A[i][j] = S[i][j];
        b[i] = R[i];
    }

    return gaussSolve(A, b);
}

// ========== 新增：生成拟合网格 ==========
QVector<QVector<double>> GraphicWidget::generateFittedGrid(
    const std::vector<double>& coeff,
    double xMin, double xMax,
    double yMin, double yMax,
    int gridSize)
{
    QVector<QVector<double>> grid(gridSize, QVector<double>(gridSize));
    double a = coeff[0];
    double b = coeff[1];
    double c = coeff[2];
    double d = coeff[3];
    double e = coeff[4];
    double f = coeff[5];

    for (int i = 0; i < gridSize; ++i) {
        double xv = xMin + (xMax - xMin) * i / (gridSize - 1);
        for (int j = 0; j < gridSize; ++j) {
            double yv = yMin + (yMax - yMin) * j / (gridSize - 1);
            grid[i][j] = a * xv * xv + b * yv * yv + c * xv * yv + d * xv + e * yv + f;
        }
    }
    return grid;
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
    // ========== 打印原始输入数据 ==========
    qDebug() << "=== 原始输入数据 ===";
    qDebug() << "xCoords (行坐标):" << xCoords;
    qDebug() << "yCoords (列坐标):" << yCoords;
    qDebug() << "数据矩阵大小:" << rowCount << "行 ×" << columnCount << "列";
    qDebug() << "x轴范围: [" << xMin << ", " << xMax << "]";
    qDebug() << "y轴范围: [" << yMin << ", " << yMax << "]";

    for (int i = 0; i < rowCount; ++i) {
        QString rowStr;
        for (int j = 0; j < columnCount; ++j) {
            rowStr += QString::number(newData[i][j]) + " ";
        }
        qDebug() << "Row" << i << ":" << rowStr;
    }

    // ========== 1. 二次曲面拟合 ==========
    std::vector<double> coeff = fitQuadraticSurface(xCoords, yCoords, newData);
    double a = coeff[0], b = coeff[1], c = coeff[2];
    double d = coeff[3], e = coeff[4], f = coeff[5];

    // 计算R²（可选，调试用）
    double zMean = 0.0;
    int totalPoints = 0;
    for (int i = 0; i < rowCount; ++i)
        for (int j = 0; j < columnCount; ++j) { zMean += newData[i][j]; totalPoints++; }
    zMean /= totalPoints;

    double ssTot = 0.0, ssRes = 0.0;
    for (int i = 0; i < rowCount; ++i) {
        for (int j = 0; j < columnCount; ++j) {
            double xv = xCoords[i];
            double yv = yCoords[j];
            double zPred = a * xv * xv + b * yv * yv + c * xv * yv + d * xv + e * yv + f;
            double zReal = newData[i][j];
            ssTot += (zReal - zMean) * (zReal - zMean);
            ssRes += (zReal - zPred) * (zReal - zPred);
        }
    }
    double r2 = (ssTot < 1e-12) ? 1.0 : 1.0 - ssRes / ssTot;
    qDebug() << "曲面拟合 R² =" << r2;

    const int gridSize = 100;
    QVector<double> fittedXCoords(gridSize);
    QVector<double> fittedYCoords(gridSize);
    for (int i = 0; i < gridSize; ++i) {
        fittedXCoords[i] = xMin + (xMax - xMin) * i / (gridSize - 1);
        fittedYCoords[i] = yMin + (yMax - yMin) * i / (gridSize - 1);
    }
    QVector<QVector<double>> fittedData = generateFittedGrid(coeff, xMin, xMax, yMin, yMax, gridSize);

    // ========== 3. 用拟合网格更新曲面 ==========
    m_array->clear();

    double m_xScaleFactor = 10.0;
    double scaledXMin = xMin * m_xScaleFactor;
    double scaledXMax = xMax * m_xScaleFactor;

    for (int i = 0; i < gridSize; ++i)
    {
        double x = fittedXCoords[i] * m_xScaleFactor;
        auto* row = new QSurfaceDataRow(gridSize);

        for (int j = 0; j < gridSize; ++j)
        {
            double y = fittedYCoords[j];
            double z = fittedData[i][j];
            (*row)[j].setPosition(QVector3D(x, z, y));
        }

        m_array->append(row);
    }

    auto zRange = calculateZRange(fittedData);
    auto zMin = zRange.first;
    auto zMax = zRange.second;

    m_axisX->setRange(scaledXMin, scaledXMax);
    m_axisZ->setRange(yMin, yMax);
    m_axisY->setRange(zMin, zMax);

    QLinearGradient gradient;
    gradient.setColorAt(0.00, QColor(0, 0, 255)); // 深蓝
    gradient.setColorAt(0.17, QColor(0, 128, 255)); // 浅蓝
    gradient.setColorAt(0.33, QColor(0, 255, 255)); // 青色
    gradient.setColorAt(0.50, QColor(0, 255, 0)); // 绿色
    gradient.setColorAt(0.67, QColor(255, 255, 0)); // 黄色
    gradient.setColorAt(0.83, QColor(255, 128, 0)); // 橙色
    gradient.setColorAt(1.00, QColor(255, 0, 0)); // 红色
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