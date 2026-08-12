#pragma once

#include <QtDataVisualization>
#include <QValue3DAxis>
#include <QCategory3DAxis>
#include <QWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEvent>

using namespace QtDataVisualization;

class GraphicWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphicWidget(QWidget* parent = nullptr);
    ~GraphicWidget();

    /**
     * @brief create3DSurfaceGraph 创建三维曲面图
     * @return 返回三维曲面图指针
     */
    QAbstract3DGraph* create3DSurfaceGraph();

    /**
     * @brief createValue3DAxis 创建数值坐标轴
     * @param axisTitle 坐标轴标题
     * @param titleVisible 是否显示标题
     * @param min 坐标轴最小值
     * @param max 坐标轴最大值
     * @return 返回数值坐标轴指针
     */
    QValue3DAxis* createValue3DAxis(QString axisTitle, bool titleVisible = true, float min = 0, float max = 100);

    /**
     * @brief createCategory3DAxis 创建文本坐标轴
     * @param axisTitle 坐标轴标题
     * @param titleVisible 是否显示标题
     * @param labList 坐标轴标签
     * @return 返回文本坐标轴指针
     */
    QCategory3DAxis* createCategory3DAxis(QString axisTitle, bool titleVisible = true, QStringList labList = QStringList());

    /**
     * @brief on_angleValueChange 视角改变槽函数
     * @param type 类型（0:水平, 1:垂直）
     * @param val 角度值
     */
    void on_angleValueChange(int type, int val);

    // 修改坐标轴标题
    void axisTitleChange(QString xName, QString yName, QString zName);

    // 修改数据
    /**
     * @brief 数据更新接口，输入精确 X/Y 坐标数组与 Z 值矩阵，更新 3D 曲面
     * @param xCoords 精确 X 坐标值数组（长度 = rowCount，每行数据的 X 坐标）
     * @param yCoords 精确 Y 坐标值数组（长度 = columnCount，每列数据的 Y 坐标）
     * @param newData 二维 Z 值数组（维度：rowCount x columnCount），对应 (x[i], y[j]) 处的高度
     * @param rowCount 行数（X 方向数据点数量，与 xCoords 长度一致）
     * @param columnCount 列数（Y 方向数据点数量，与 yCoords 长度一致）
     * @param xMin X 轴最小值
     * @param xMax X 轴最大值
     * @param yMin Y 轴最小值
     * @param yMax Y 轴最大值
     */
    void dataUpdate(const QVector<double>& xCoords,
        const QVector<double>& yCoords,
        const QVector<QVector<double>>& newData,
        int rowCount,
        int columnCount,
        double xMin,
        double xMax,
        double yMin,
        double yMax);

    /**
     * @brief 设置坐标轴是否自动调整范围（默认关闭，设置后可能失效）
     * @param xAuto X 轴是否自动调整
     * @param yAuto Y 轴是否自动调整
     * @param zAuto Z 轴是否自动调整
     */
    void setAxisAutoAdjust(bool xAuto, bool yAuto, bool zAuto);

    // 计算 Z 轴最小值和最大值（用于手动设置坐标范围）
    QPair<double, double> calculateZRange(const QVector<QVector<double>>& newData);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void on_scaleSlider_sliderMoved(int position);

private:
    void clampCameraAngle();

    Q3DSurface* m_surface = nullptr; // 三维曲面图对象
    QSurface3DSeries* m_series = nullptr; // 序列
    QAbstract3DGraph* m_graph = nullptr;    // 图表基类指针
    QSurfaceDataArray* m_array = nullptr;  // 数据集

    QValue3DAxis* m_axisX = nullptr;  // X轴
    QValue3DAxis* m_axisY = nullptr;  // Y轴（高度）
    QValue3DAxis* m_axisZ = nullptr;  // Z轴
};