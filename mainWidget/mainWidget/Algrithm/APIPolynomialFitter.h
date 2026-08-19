#pragma once
#include <QVector>
#include <QPointF>
#include <QString>

struct FitResult
{
	double a = 0.0;
	double b = 0.0;
	double c = 0.0;
	double r2 = 0.0;
	bool   valid = false;
};

class APIPolynomialFitter
{
public:
	static FitResult fitLinear(const QVector<QPointF>& points);
	static FitResult fitQuadratic(const QVector<QPointF>& points);
	static QVector<QPointF> generateLinearCurve(const FitResult& fit,
		double xMin, double xMax, int samples = 50);
	static QVector<QPointF> generateQuadraticCurve(const FitResult& fit,
		double xMin, double xMax, int samples = 50);
	static QString formatLinear(const FitResult& fit);
	static QString formatQuadratic(const FitResult& fit);
};
