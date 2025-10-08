#ifndef TRIANGULATION_PROGRESS_DIALOG_H
#define TRIANGULATION_PROGRESS_DIALOG_H

#include <QProgressDialog>
#include <QString>

class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    // 构造函数：初始化进度条标题和父窗口
    explicit ProgressDialog(const QString& title, QWidget* parent = nullptr);

    // 设置进度（0~100）
    void SetProgress(int progress);

    // 设置状态文本（如“处理节点1000/5000”）
    void SetStatusText(const QString& text);

    // 检查是否被用户取消
    bool IsCanceled() const;

signals:
    // 取消操作信号（转发给Worker）
    void Canceled();

private slots:
    // 内部取消按钮点击处理
    void OnCancelButtonClicked();
};

#endif // TRIANGULATION_PROGRESS_DIALOG_H