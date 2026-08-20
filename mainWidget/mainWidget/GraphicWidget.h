#pragma once

#include <QWidget>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/QCategory3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtDataVisualization/Q3DCamera>
#include <QtDataVisualization/Q3DInputHandler>
#include <QVector>
#include <QPair>
#include <vector>

QT_BEGIN_NAMESPACE
class QLabel;
class QSpinBox;
QT_END_NAMESPACE

using namespace QtDataVisualization;

class GraphicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphicWidget(QWidget* parent = nullptr);
    ~GraphicWidget();

    void dataUpdate(const QVector<double>& xCoords,
        const QVector<double>& yCoords,
        const QVector<QVector<double>>& newData,
        int rowCount,
        int columnCount,
        double xMin,
        double xMax,
        double yMin,
        double yMax);

    void axisTitleChange(QString xName, QString yName, QString zName);
    void setAxisAutoAdjust(bool xAuto, bool yAuto, bool zAuto);

public slots:
    void on_angleValueChange(int type, int val);
    void on_scaleSlider_sliderMoved(int position);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QAbstract3DGraph* create3DSurfaceGraph();
    QValue3DAxis* createValue3DAxis(QString axisTitle, bool titleVisible = true, float min = 0, float max = 100);
    QCategory3DAxis* createCategory3DAxis(QString axisTitle, bool titleVisible = true, QStringList labList = QStringList());

    QPair<double, double> calculateZRange(const QVector<QVector<double>>& newData);
    void clampCameraAngle();

    // ========== 新增：曲面拟合相关 ==========
    static std::vector<double> gaussSolve(std::vector<std::vector<double>> A, std::vector<double> b);
    std::vector<double> fitQuadraticSurface(const QVector<double>& xCoords,
        const QVector<double>& yCoords,
        const QVector<QVector<double>>& newData);
    QVector<QVector<double>> generateFittedGrid(const std::vector<double>& coeff,
        double xMin, double xMax,
        double yMin, double yMax,
        int gridSize);

private:
    QAbstract3DGraph* m_graph = nullptr;
    Q3DSurface* m_surface = nullptr;
    QValue3DAxis* m_axisX = nullptr;
    QValue3DAxis* m_axisY = nullptr;
    QValue3DAxis* m_axisZ = nullptr;
    QSurface3DSeries* m_series = nullptr;
    QSurfaceDataArray* m_array = nullptr;
};