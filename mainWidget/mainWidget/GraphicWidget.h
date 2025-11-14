#pragma once

#include <QtDataVisualization>
#include <QValue3DAxis>
#include <QCategory3DAxis>
#include <QWidget>
#include <QAbstractButton>
#include <QLabel>
#include <QHBoxLayout>

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
     * @brief createValue3DAxis 创建值坐标轴
     * @param axisTitle 坐标轴标题
     * @param titleVisible 是否显示标题
     * @param min 坐标轴最小值
     * @param max 坐标轴最大值
     * @return 返回值坐标轴指针
     */
    QValue3DAxis* createValue3DAxis(QString axisTitle, bool titleVisible = true, float min = 0, float max = 100);

    /**
     * @brief createCategory3DAxis 创建文本坐标轴
     * @param axisTitle 坐标轴标题
     * @param titleVisible 是否显示坐标轴
     * @param labList 坐标轴标签
     * @return 返回值坐标轴指针
     */
    QCategory3DAxis* createCategory3DAxis(QString axisTitle, bool titleVisible = true, QStringList labList = QStringList());

    /**
     * @brief on_angleValueChange 视角改变槽函数
     * @param val 角度值
     */
    void on_angleValueChange(int type, int val);

    // 修改坐标轴名称
    void axisTitleChange(QString xName, QString yName, QString zName);

    // 修改数据
    /**
     * @brief 核心更新接口：传入精确 X/Y 坐标和 Z 值，更新 3D 曲面
     * @param xCoords 精确 X 轴坐标数组（长度 = rowCount，每行数据的 X 坐标）
     * @param yCoords 精确 Y 轴坐标数组（长度 = columnCount，每列数据的 Y 坐标）
     * @param zData 二维 Z 值数组（维度：rowCount x columnCount，对应 (x[i], y[j]) 处的高度）
     * @param rowCount 数据行数（X 轴数据点数量，需与 xCoords 长度一致）
     * @param columnCount 数据列数（Y 轴数据点数量，需与 yCoords 长度一致）
     * @param zMin Z 轴最小值（默认自动计算）
     * @param zMax Z 轴最大值（默认自动计算）
     * @return 是否更新成功
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
     * @brief 设置轴是否自动调整范围（默认关闭，避免缩放失效）
     * @param autoAdjust X/Y/Z 轴是否自动调整
     */
    void setAxisAutoAdjust(bool xAuto, bool yAuto, bool zAuto);

    // 计算 Z 轴的最小值和最大值（用于手动设置轴范围）
    QPair<double, double> calculateZRange(const QVector<QVector<double>>& newData);

private slots:
    
    /**
     * @brief on_scaleSlider_sliderMoved 缩放功能槽函数
     * @param position 缩放值
     */
    void on_scaleSlider_sliderMoved(int position);

 


private:
    Q3DSurface* surface; //三维曲面图对象
    QSurface3DSeries* series; // 数据
    QAbstract3DGraph* m_graph;    // 图表容器指针
    QSurfaceDataArray* m_array;  //数据集

    QValue3DAxis* m_axisX;  // 坐标轴
    QValue3DAxis* m_axisY;
    QValue3DAxis* m_axisZ;



};

