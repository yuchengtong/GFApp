#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

class StartDialog : public QDialog
{
    Q_OBJECT
public:
    // 构造函数
    explicit StartDialog(QWidget* parent = nullptr);

    // 获取选择的文件夹路径
    QString getSelectedPath() const;

private slots:
    // 选择文件夹按钮点击事件
    void onSelectFolderClicked();
    // 确定按钮点击事件
    void onConfirmClicked();
  

private:
    // 控件声明
    QLineEdit* m_pathEdit = nullptr;      // 显示选中的路径
    QString m_selectedPath;     // 存储选中的路径
};