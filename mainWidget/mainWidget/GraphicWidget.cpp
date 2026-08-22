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
    m_graph->activeTheme()->setGridEnabled(true);
    m_graph->activeTheme()->setBackgroundEnabled(true);
    m_graph->activeTheme()->setLabelBackgroundEnabled(true);

    m_graph->installEventFilter(this);

    QWidget* graphContainer = QWidget::createWindowContainer(m_graph);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(graphContainer);
}

GraphicWidget::~GraphicWidget()
{
    if (m_array) {
        qDeleteAll(*m_array);
        delete m_array;
    }
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

    for (int i = 0; i < 3; ++i) {
        QSurfaceDataRow* dataRow = new QSurfaceDataRow;
        for (int j = 0; j < 3; ++j) {
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

void GraphicWidget::on_scaleSlider_sliderMoved(int position)
{
    m_graph->scene()->activeCamera()->setZoomLevel(position);
}

void GraphicWidget::axisTitleChange(QString xName, QString yName, QString zName)
{
    if (!xName.isEmpty()) m_axisX->setTitle(xName);
    if (!yName.isEmpty()) m_axisZ->setTitle(yName);
    if (!zName.isEmpty()) m_axisY->setTitle(zName);
}

std::vector<double> GraphicWidget::gaussSolve(std::vector<std::vector<double>> A, std::vector<double> b, double lambda)
{
    int n = static_cast<int>(A.size());
    if (n == 0 || static_cast<int>(b.size()) != n) return {};

    for (int i = 0; i < n; ++i) A[i].push_back(b[i]);

    if (lambda > 0.0) {
        for (int i = 0; i < n; ++i) {
            A[i][i] += lambda;
        }
    }

    const double EPS = 1e-12;
    for (int col = 0; col < n; ++col) {
        int pivot = col;
        for (int row = col + 1; row < n; ++row) {
            if (std::abs(A[row][col]) > std::abs(A[pivot][col]))
                pivot = row;
        }
        std::swap(A[col], A[pivot]);

        if (std::abs(A[col][col]) < EPS) continue;

        for (int row = col + 1; row < n; ++row) {
            double factor = A[row][col] / A[col][col];
            for (int j = col; j <= n; ++j) {
                A[row][j] -= factor * A[col][j];
            }
        }
    }

    std::vector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        if (std::abs(A[i][i]) < EPS) {
            x[i] = 0.0;
            continue;
        }
        double sum = A[i][n];
        for (int j = i + 1; j < n; ++j) sum -= A[i][j] * x[j];
        x[i] = sum / A[i][i];
    }
    return x;
}

std::vector<double> GraphicWidget::fitQuadraticSurface(
    const QVector<double>& xCoords,
    const QVector<double>& yCoords,
    const QVector<QVector<double>>& newData)
{
    double S[6][6] = {};
    double R[6] = {};

    int rowCount = newData.size();
    int totalPoints = 0;
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
            ++totalPoints;
        }
    }

    std::vector<std::vector<double>> A(6, std::vector<double>(6));
    std::vector<double> b(6);
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 6; ++j) A[i][j] = S[i][j];
        b[i] = R[i];
    }

    auto coeff = gaussSolve(A, b, 0.0);

    bool needReg = false;
    int zeroCnt = 0;
    for (double c : coeff) {
        if (std::isnan(c) || std::isinf(c)) { needReg = true; break; }
        if (std::abs(c) < 1e-15) ++zeroCnt;
    }
    if (zeroCnt >= 6) needReg = true;

    if (totalPoints < 6) {
        qDebug() << "警告: 数据点仅" << totalPoints << "个，二次曲面欠定，启用正则化拟合";
        needReg = true;
    }

    if (needReg) {
        coeff = gaussSolve(A, b, 1e-3);
    }

    return coeff;
}

QVector<QVector<double>> GraphicWidget::generateFittedGrid(
    const std::vector<double>& coeff,
    double xMin, double xMax,
    double yMin, double yMax,
    int gridSize)
{
    QVector<QVector<double>> grid(gridSize, QVector<double>(gridSize));
    if (coeff.size() < 6) return grid;

    double a = coeff[0], b = coeff[1], c = coeff[2];
    double d = coeff[3], e = coeff[4], f = coeff[5];

    for (int i = 0; i < gridSize; ++i) {
        double xv = xMin + (xMax - xMin) * i / (gridSize - 1);
        for (int j = 0; j < gridSize; ++j) {
            double yv = yMin + (yMax - yMin) * j / (gridSize - 1);
            grid[i][j] = a * xv * xv + b * yv * yv + c * xv * yv + d * xv + e * yv + f;
        }
    }
    return grid;
}

//static void printMatrix(const QString& title, const QVector<QVector<double>>& mat, int rowCount, int colCount)
//{
//    qDebug() << "==========" << title << "==========";
//    for (int i = 0; i < rowCount; ++i) {
//        QString line;
//        for (int j = 0; j < colCount; ++j) {
//            line.append(QString::number(mat[i][j], 'f', 4)).append(" ");
//        }
//        qDebug() << "Row" << i << ":" << line;
//    }
//}

double GraphicWidget::dataUpdate(const QVector<double>& xCoords,
    const QVector<double>& yCoords,
    const QVector<QVector<double>>& newData,
    int rowCount,
    int columnCount,
    double xMin,
    double xMax,
    double yMin,
    double yMax)
{
    //printMatrix("原始数据 (newData)", newData, rowCount, columnCount);

    std::vector<double> coeff = fitQuadraticSurface(xCoords, yCoords, newData);
    double a = coeff[0], b = coeff[1], c = coeff[2];
    double d = coeff[3], e = coeff[4], f = coeff[5];

    double zMean = 0.0;
    int totalPoints = 0;
    for (int i = 0; i < rowCount; ++i)
        for (int j = 0; j < columnCount; ++j) { zMean += newData[i][j]; ++totalPoints; }
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
    //qDebug() << "曲面拟合 R² =" << r2;
    //qDebug() << "拟合系数: a=" << a << "b=" << b << "c=" << c
    //    << "d=" << d << "e=" << e << "f=" << f;

    const int gridSize = 100;
    QVector<double> fittedXCoords(gridSize);
    QVector<double> fittedYCoords(gridSize);
    for (int i = 0; i < gridSize; ++i) {
        fittedXCoords[i] = xMin + (xMax - xMin) * i / (gridSize - 1);
        fittedYCoords[i] = yMin + (yMax - yMin) * i / (gridSize - 1);
    }
    QVector<QVector<double>> fittedData = generateFittedGrid(coeff, xMin, xMax, yMin, yMax, gridSize);

    double fMin = fittedData[0][0], fMax = fittedData[0][0];
    for (int i = 0; i < gridSize; ++i)
        for (int j = 0; j < gridSize; ++j) {
            fMin = qMin(fMin, fittedData[i][j]);
            fMax = qMax(fMax, fittedData[i][j]);
        }
    //qDebug() << "fittedData 数值范围:" << fMin << "~" << fMax;

    qDeleteAll(*m_array);
    m_array->clear();

    //qDebug() << "========== 参与绘图的3D坐标（后10个点）==========";

    for (int i = 0; i < gridSize; ++i) {
        double x = fittedXCoords[i];
        auto* row = new QSurfaceDataRow(gridSize);

        for (int j = 0; j < gridSize; ++j) {
            double y = fittedYCoords[j];
            double z = fittedData[i][j];
            (*row)[j].setPosition(QVector3D(x, z, y));

            //if (i == gridSize - 1 && j >= gridSize - 10) {
            //    qDebug() << "QVector3D(" << x << "," << z << "," << y << ")";
            //}
        }
        m_array->append(row);
    }

    auto zRange = calculateZRange(fittedData);
    m_axisX->setRange(xMin-1, xMax);
    m_axisZ->setRange(yMin-1, yMax);
    m_axisY->setRange(zRange.first, zRange.second);


    m_surface->setHorizontalAspectRatio(1.0);



    QLinearGradient gradient;
    gradient.setColorAt(0.00, QColor(0, 0, 255));
    gradient.setColorAt(0.17, QColor(0, 128, 255));
    gradient.setColorAt(0.33, QColor(0, 255, 255));
    gradient.setColorAt(0.50, QColor(0, 255, 0));
    gradient.setColorAt(0.67, QColor(255, 255, 0));
    gradient.setColorAt(0.83, QColor(255, 128, 0));
    gradient.setColorAt(1.00, QColor(255, 0, 0));
    m_series->setBaseGradient(gradient);
    m_series->setColorStyle(Q3DTheme::ColorStyleRangeGradient);

    m_series->dataProxy()->resetArray(m_array);
    m_surface->show();

    return r2;
}

QPair<double, double> GraphicWidget::calculateZRange(const QVector<QVector<double>>& data)
{
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();

    for (const auto& row : data) {
        for (double z : row) {
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
    }

    if (std::abs(maxZ - minZ) < 1e-12) {
        double mid = (maxZ + minZ) * 0.5;
        return { mid - 1.0, mid + 1.0 };
    }

    double offset = (maxZ - minZ) * 0.05;
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
    if (!m_surface) return;

    Q3DCamera* camera = m_surface->scene()->activeCamera();
    float xRot = camera->xRotation();
    float yRot = camera->yRotation();

    bool changed = false;
    if (xRot < -180.0f) { xRot = -180.0f; changed = true; }
    if (xRot > -90.0f) { xRot = -90.0f;  changed = true; }
    if (yRot < 0.0f) { yRot = 0.0f;    changed = true; }
    if (yRot > 90.0f) { yRot = 90.0f;   changed = true; }

    if (changed) {
        camera->setXRotation(xRot);
        camera->setYRotation(yRot);
    }
}