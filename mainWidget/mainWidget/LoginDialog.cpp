#pragma execution_character_set("utf-8")
#include "LoginDialog.h"
#include "ModelDataManager.h"

#include "xlsxdocument.h"
#include <QDir>



LoginDialog::LoginDialog(QWidget*parent)
	:QDialog(parent)
{
	setWindowTitle("用户登录");
	setWindowIcon(QIcon(":/icons/login.png"));
	setFixedSize(600, 400);
	setStyleSheet("QDialog { background-color: #f5f7fa; }");


	// 创建主布局
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(30, 30, 30, 20);
	mainLayout->setSpacing(20);

	// 添加标题
	QLabel *titleLabel = new QLabel("欢迎登录", this);
	titleLabel->setStyleSheet(
		"QLabel {"
		"   font-size: 24px;"
		"   font-weight: bold;"
		"   color: #2c3e50;"
		"   padding-bottom: 10px;"
		"}");
	mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);

	// 添加表单布局
	QGridLayout *formLayout = new QGridLayout;
	formLayout->setContentsMargins(10, 10, 10, 10);
	formLayout->setSpacing(15);

	// 用户名输入
	QLabel *usernameLabel = new QLabel("用户名:", this);
	usernameEdit = new QLineEdit(this);
	usernameEdit->setPlaceholderText("请输入用户名");
	usernameEdit->setMinimumHeight(40);
	usernameEdit->setStyleSheet(
		"QLineEdit {"
		"   padding: 8px 12px;"
		"   border: 1px solid #d1d8e0;"
		"   border-radius: 4px;"
		"   font-size: 14px;"
		"}"
		"QLineEdit:focus {"
		"   border: 1px solid #3498db;"
		"}");

	// 密码输入
	QLabel *passwordLabel = new QLabel("密码:", this);
	passwordEdit = new QLineEdit(this);
	passwordEdit->setPlaceholderText("请输入密码");
	passwordEdit->setEchoMode(QLineEdit::Password);
	passwordEdit->setMinimumHeight(40);
	passwordEdit->setStyleSheet(
		"QLineEdit {"
		"   padding: 8px 12px;"
		"   border: 1px solid #d1d8e0;"
		"   border-radius: 4px;"
		"   font-size: 14px;"
		"}"
		"QLineEdit:focus {"
		"   border: 1px solid #3498db;"
		"}");

	// 添加到表单布局
	formLayout->addWidget(usernameLabel, 0, 0);
	formLayout->addWidget(usernameEdit, 0, 1);
	formLayout->addWidget(passwordLabel, 1, 0);
	formLayout->addWidget(passwordEdit, 1, 1);

	// 添加显示密码按钮
	showPasswordButton = new QPushButton("显示密码", this);
	showPasswordButton->setCheckable(true);
	showPasswordButton->setStyleSheet(
		"QPushButton {"
		"   background-color: transparent;"
		"   color: #3498db;"
		"   border: none;"
		"   font-size: 12px;"
		"   text-align: right;"
		"   padding: 0;"
		"}"
		"QPushButton:hover {"
		"   color: #2980b9;"
		"   text-decoration: underline;"
		"}");
	formLayout->addWidget(showPasswordButton, 1, 2);

	mainLayout->addLayout(formLayout);



	// 添加按钮布局
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->setSpacing(15);

	// 注册按钮
	registerButton = new QPushButton("一键注册登录", this);
	registerButton->setMinimumHeight(40);
	registerButton->setStyleSheet(
		"QPushButton {"
		"   background-color: #3498db;"
		"   color: white;"
		"   border-radius: 4px;"
		"   font-size: 14px;"
		"   font-weight: bold;"
		"}"
		"QPushButton:hover {"
		"   background-color: #2980b9;"
		"}"
		"QPushButton:pressed {"
		"   background-color: #1c6ea4;"
		"}");

	// 登录按钮
	loginButton = new QPushButton("登录", this);
	loginButton->setMinimumHeight(40);
	loginButton->setStyleSheet(
		"QPushButton {"
		"   background-color: #3498db;"
		"   color: white;"
		"   border-radius: 4px;"
		"   font-size: 14px;"
		"   font-weight: bold;"
		"}"
		"QPushButton:hover {"
		"   background-color: #2980b9;"
		"}"
		"QPushButton:pressed {"
		"   background-color: #1c6ea4;"
		"}");

	buttonLayout->addWidget(registerButton);
	buttonLayout->addWidget(loginButton);

	mainLayout->addLayout(buttonLayout);



	// 添加底部间距
	mainLayout->addSpacerItem(new QSpacerItem(20, 20,
		QSizePolicy::Minimum, QSizePolicy::Expanding));

	// 连接信号槽
	connect(loginButton, &QPushButton::clicked,
		this, &LoginDialog::onLoginClicked);
	connect(registerButton, &QPushButton::clicked,
		this, &LoginDialog::onRegisterClicked);
	connect(showPasswordButton, &QPushButton::toggled,
		this, &LoginDialog::togglePasswordVisibility);
}


void LoginDialog::onLoginClicked()
{
	QString username = usernameEdit->text().trimmed();
	QString password = passwordEdit->text().trimmed();

	// 表单验证
	if (username.isEmpty()) {
		QMessageBox::warning(this, "登录错误", "用户名不能为空");
		usernameEdit->setFocus();
		return;
	}

	if (password.isEmpty()) {
		QMessageBox::warning(this, "登录错误", "密码不能为空");
		passwordEdit->setFocus();
		return;
	}

	bool loginCheck = false;
	QString filepath = nullptr;
	QDir dir;
	filepath = dir.absoluteFilePath("src/database/账号密码.xlsx");
	if (!filepath.isEmpty()) {
		QXlsx::Document xlsx(filepath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数

		for (int row = 1; row <= rowcount; ++row) {
			QString my_username = xlsx.read(row, 2).toString();

			if (username == my_username)
			{
				QString my_password = xlsx.read(row, 3).toString();
				if (password == my_password)
				{
					// 保存账号信息
					auto ins = ModelDataManager::GetInstance();
					UserInfo info;
					info.username = my_username;
					info.password = my_password;
					ins->SetUserInfo(info);

					loginCheck = true;
					QMessageBox::information(this, "登录成功",
						"欢迎回来, " + username + "!");
					// 关闭对话框并返回QDialog::Accepted
					accept();
					break;
				}
				continue;
			}
		}
	}

	if (!loginCheck) {
		QMessageBox::warning(this, "登录错误", "用户名或密码错误");
		passwordEdit->clear();
		passwordEdit->setFocus();
	}

}

void LoginDialog::onRegisterClicked()
{
	QString username = usernameEdit->text().trimmed();
	QString password = passwordEdit->text().trimmed();

	// 表单验证
	if (username.isEmpty()) {
		QMessageBox::warning(this, "注册失败", "用户名不能为空");
		usernameEdit->setFocus();
		return;
	}

	if (password.isEmpty()) {
		QMessageBox::warning(this, "注册失败", "密码不能为空");
		passwordEdit->setFocus();
		return;
	}

	bool loginCheck = false;
	QString filepath = nullptr;
	QDir dir;
	filepath = dir.absoluteFilePath("src/database/账号密码.xlsx");
	if (!filepath.isEmpty()) {
		QXlsx::Document xlsx(filepath);
		int rowcount = xlsx.dimension().lastRow(); // 获取总行数
		int colcount = xlsx.dimension().lastColumn(); // 获取总列数

		for (int row = 1; row <= rowcount; ++row) {
			QString my_username = xlsx.read(row, 2).toString();

			if (username == my_username)
			{
				QMessageBox::warning(this, "注册失败", "该用户名已存在");
				return;
			}
		}
		// 保存账号信息
		auto ins = ModelDataManager::GetInstance();
		UserInfo info;
		info.username = username;
		info.password = password;
		ins->SetUserInfo(info);

		// 记录账号密码
		xlsx.write(rowcount + 1, 1, rowcount);
		xlsx.write(rowcount + 1, 2, username);
		xlsx.write(rowcount + 1, 3, password);


		if (xlsx.save()) {
			QMessageBox::information(this, "注册登录成功",
				"欢迎登录, " + username + "!");
			// 关闭对话框并返回QDialog::Accepted
			accept();
		}
		else {
			QMessageBox::critical(this, "标题", "保存失败");
		}
	}
}

void LoginDialog::togglePasswordVisibility(bool checked)
{
	if (checked) {
		passwordEdit->setEchoMode(QLineEdit::Normal);
		showPasswordButton->setText("隐藏密码");
	}
	else {
		passwordEdit->setEchoMode(QLineEdit::Password);
		showPasswordButton->setText("显示密码");
	}
}

