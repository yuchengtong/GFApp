#pragma once
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QCheckBox>

class LoginDialog :public QDialog
{
	Q_OBJECT
public:
	LoginDialog(QWidget*parent = nullptr);


	void onLoginClicked();

	void onRegisterClicked();

	void togglePasswordVisibility(bool checked);


private:

	QLineEdit *usernameEdit = nullptr;
	QLineEdit *passwordEdit = nullptr;
	QPushButton *loginButton = nullptr;
	QPushButton *registerButton = nullptr;
	QPushButton *showPasswordButton = nullptr;
	QCheckBox *rememberMeCheckBox = nullptr;
};
