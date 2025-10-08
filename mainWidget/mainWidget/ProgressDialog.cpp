#pragma execution_character_set("utf-8")
#include "ProgressDialog.h"
#include <QApplication>

// 构造函数：初始化进度条样式和行为
ProgressDialog::ProgressDialog(const QString& title, QWidget* parent)
    : QProgressDialog(parent)
{
    // 基础设置
    setWindowTitle(title);
    setLabelText("准备开始三角化...");
    setCancelButtonText("取消");
    setRange(0, 100);          // 进度范围0~100
    setValue(0);               // 初始进度0
    setWindowModality(Qt::WindowModal); // 完全模态（阻塞父窗口）
    setMinimumDuration(0);     // 立即显示（不延迟）
    setFixedSize(400, 120);    // 固定大小（避免UI抖动）

    // 连接取消按钮信号
    connect(this, &QProgressDialog::canceled, this, &ProgressDialog::OnCancelButtonClicked);
}

// 设置进度
void ProgressDialog::SetProgress(int progress)
{
    if (progress < 0)
    {
        progress = 0;
    }
    if (progress > 100)
    {
        progress = 100;
    }
    setValue(progress);
    QApplication::processEvents(); // 强制刷新UI（避免进度条卡顿）
}

// 设置状态文本
void ProgressDialog::SetStatusText(const QString& text)
{
    setLabelText(text);
    QApplication::processEvents(); // 强制刷新UI
}

// 检查是否取消
bool ProgressDialog::IsCanceled() const
{
    return wasCanceled();
}

// 内部取消按钮点击处理
void ProgressDialog::OnCancelButtonClicked()
{
    emit Canceled(); // 发送取消信号给Worker
    setLabelText("正在取消三角化...");
    QApplication::processEvents();
}