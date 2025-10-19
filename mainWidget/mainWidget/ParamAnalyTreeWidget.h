#pragma once
using namespace std;
#include <iostream>
#include <vector>
#include <QTreeWidget>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QMessageBox>
#include <QTableWidget>

#include "GFTreeWidget.h"
#include "ModelDataManager.h"

class ParamAnalyTreeWidget :public QWidget
{
	Q_OBJECT
public:
	ParamAnalyTreeWidget(QWidget* parent = nullptr);
	~ParamAnalyTreeWidget();


protected:
	void contextMenuEvent(QContextMenuEvent* event) override;

signals:
	void itemClicked(const QString& itemData);

private slots:
	void onTreeItemClicked(QTreeWidgetItem* item, int column);
	// 计算
	void calculate();
	// 解析字符串为数字（支持分数和小数）
	double parseNumber(const QString& fractionStr);
	// 表格转二维数组
	vector<vector<double>> convertTableToVector(QTableWidget* tableWidget);
	// 矩阵乘法
	vector<vector<double>> matrixMultiply(const vector<vector<double>>& a, const vector<vector<double>>& b);
	// 列归一化计算
	vector<vector<double>> columnNormalize(const vector<vector<double>>& matrix);
	// 计算权重向量
	vector<double> calculateWeights(const vector<vector<double>>& matrix);
	// 一致性校验
	double consistencyCheck(const vector<vector<double>>& matrix, vector<double> weights);


private:
	QMenu* contextMenu; // 右键菜单对象

	GFTreeWidget* treeWidget = nullptr;
};

