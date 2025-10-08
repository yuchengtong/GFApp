#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QTableWidget>

class BasePropertyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BasePropertyWidget(QWidget* parent = nullptr);

    virtual void initWidget() = 0;

};