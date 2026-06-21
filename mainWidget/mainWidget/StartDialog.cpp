#pragma execution_character_set("utf-8")
#include "StartDialog.h"
#include "ModelDataManager.h"

StartDialog::StartDialog(QWidget* parent)
    : QDialog(parent)
{
    // 窗口基础设置
    setWindowTitle("选择数据存储路径");
    setWindowIcon(QIcon(":/src/engine.svg"));
    setFixedSize(500, 180);
    setStyleSheet("QDialog { background-color: #f5f7fa; }");

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 20);
    mainLayout->setSpacing(20);

    // 标题标签
    QLabel* titleLabel = new QLabel("请选择数据存储路径", this);
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   padding-bottom: 10px;"
        "}"
    );
    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);

    // 路径选择行布局
    QHBoxLayout* pathLayout = new QHBoxLayout;
    pathLayout->setSpacing(10);

    // 路径显示框
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText("请选择数据存储路径...");
    m_pathEdit->setMinimumHeight(40);
    m_pathEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 12px;"
        "   border: 1px solid #d1d8e0;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #3498db;"
        "}"
    );
    m_pathEdit->setReadOnly(true); // 只读，防止手动输入

    // 选择文件夹按钮
    QPushButton* selectBtn = new QPushButton("选择文件夹", this);
    selectBtn->setMinimumHeight(40);
    selectBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #3498db;"
        "   color: white;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "   padding: 0 15px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1c6ea4;"
        "}"
    );

    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(selectBtn);
    mainLayout->addLayout(pathLayout);

    // 按钮行布局
    QHBoxLayout* btnLayout = new QHBoxLayout;

    // 确定按钮
    QPushButton* confirmBtn = new QPushButton("确定", this);
    confirmBtn->setMinimumHeight(40);
    confirmBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #2ecc71;"
        "   color: white;"
        "   border-radius: 4px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   padding: 0 20px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #27ae60;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #1e8449;"
        "}"
    );

    
    btnLayout->addWidget(confirmBtn, 0, Qt::AlignHCenter);
    mainLayout->addLayout(btnLayout);

    // 信号槽连接
    connect(selectBtn, &QPushButton::clicked, this, &StartDialog::onSelectFolderClicked);
    connect(confirmBtn, &QPushButton::clicked, this, &StartDialog::onConfirmClicked);
}

// 获取选中的文件夹路径
QString StartDialog::getSelectedPath() const
{
    return m_selectedPath;
}

// 选择文件夹按钮点击事件
void StartDialog::onSelectFolderClicked()
{
    // 打开文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        "选择数据存储路径",
        QDir::currentPath(), // 默认起始路径
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!folderPath.isEmpty())
    {
        m_selectedPath = folderPath;
        m_pathEdit->setText(folderPath); // 显示选中的路径
    }
}

// 确定按钮点击事件
void StartDialog::onConfirmClicked()
{
    // 校验路径是否选择
    if (m_selectedPath.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请先选择有效的文件夹路径！");
        return;
    }

    // 路径有效，发送 accept 信号并关闭弹窗
    QMessageBox::information(this, "成功", "数据存储路径设置完成：\n" + m_selectedPath);
    // 保存账号信息
    auto ins = ModelDataManager::GetInstance();
    UserInfo info;
    info.username = "";
    info.password = "";
    info.workdir = m_selectedPath;
    ins->SetUserInfo(info);
    accept();
}

