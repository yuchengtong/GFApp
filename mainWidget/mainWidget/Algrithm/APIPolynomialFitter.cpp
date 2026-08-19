#include "APIPolynomialFitter.h"
#include <QtMath>
#include <algorithm>
#include <cmath>

FitResult APIPolynomialFitter::fitLinear(const QVector<QPointF>& points)
{
	FitResult result;
	int n = points.size();
	if (n < 2) return result;
	double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
	for (const QPointF& p : points)
	{
		sum_x += p.x();
		sum_y += p.y();
		sum_xy += p.x() * p.y();
		sum_xx += p.x() * p.x();
	}
	double denom = n * sum_xx - sum_x * sum_x;
	if (qAbs(denom) < 1e-12) return result;
	result.a = (n * sum_xy - sum_x * sum_y) / denom;
	result.b = (sum_y - result.a * sum_x) / n;
	result.valid = true;
	double y_mean = sum_y / n;
	double ss_tot = 0.0, ss_res = 0.0;
	for (const QPointF& p : points)
	{
		double y_pred = result.a * p.x() + result.b;
		ss_res += (p.y() - y_pred) * (p.y() - y_pred);
		ss_tot += (p.y() - y_mean) * (p.y() - y_mean);
	}
	result.r2 = (ss_tot > 1e-12) ? (1.0 - ss_res / ss_tot) : 0.0;
	return result;
}

FitResult APIPolynomialFitter::fitQuadratic(const QVector<QPointF>& points)
{
	FitResult result;
	int n = points.size();
	if (n < 3) return result;
	double s1 = 0.0, s2 = 0.0, s3 = 0.0, s4 = 0.0;
	double sy = 0.0, sxy = 0.0, sx2y = 0.0;
	for (const QPointF& p : points)
	{
		double x = p.x(), y = p.y();
		double x2 = x * x, x3 = x2 * x, x4 = x3 * x;
		s1 += x; s2 += x2; s3 += x3; s4 += x4;
		sy += y; sxy += x * y; sx2y += x2 * y;
	}
	double A[3][4] = {
		{ s4, s3, s2, sx2y },
		{ s3, s2, s1, sxy  },
		{ s2, s1, (double)n, sy }
	};
	for (int col = 0; col < 3; ++col)
	{
		int maxRow = col;
		for (int row = col + 1; row < 3; ++row)
		{
			if (qAbs(A[row][col]) > qAbs(A[maxRow][col]))
				maxRow = row;
		}
		if (maxRow != col)
		{
			for (int k = 0; k < 4; ++k)
				std::swap(A[col][k], A[maxRow][k]);
		}
		if (qAbs(A[col][col]) < 1e-12) return result;
		for (int row = col + 1; row < 3; ++row)
		{
			double factor = A[row][col] / A[col][col];
			for (int k = col; k < 4; ++k)
				A[row][k] -= factor * A[col][k];
		}
	}
	double x[3];
	for (int row = 2; row >= 0; --row)
	{
		x[row] = A[row][3];
		for (int k = row + 1; k < 3; ++k)
			x[row] -= A[row][k] * x[k];
		x[row] /= A[row][row];
	}
	result.a = x[0]; result.b = x[1]; result.c = x[2];
	result.valid = true;
	double y_mean = sy / n;
	double ss_tot = 0.0, ss_res = 0.0;
	for (const QPointF& p : points)
	{
		double y_pred = result.a * p.x() * p.x() + result.b * p.x() + result.c;
		ss_res += (p.y() - y_pred) * (p.y() - y_pred);
		ss_tot += (p.y() - y_mean) * (p.y() - y_mean);
	}
	result.r2 = (ss_tot > 1e-12) ? (1.0 - ss_res / ss_tot) : 0.0;
	return result;
}

QVector<QPointF> APIPolynomialFitter::generateLinearCurve(const FitResult& fit,
	double xMin, double xMax, int samples)
{
	QVector<QPointF> curve;
	if (!fit.valid || samples < 2 || xMax <= xMin) return curve;
	double step = (xMax - xMin) / (samples - 1);
	for (int i = 0; i < samples; ++i)
	{
		double x = xMin + i * step;
		curve.append(QPointF(x, fit.a * x + fit.b));
	}
	return curve;
}

QVector<QPointF> APIPolynomialFitter::generateQuadraticCurve(const FitResult& fit,
	double xMin, double xMax, int samples)
{
	QVector<QPointF> curve;
	if (!fit.valid || samples < 2 || xMax <= xMin) return curve;
	double step = (xMax - xMin) / (samples - 1);
	for (int i = 0; i < samples; ++i)
	{
		double x = xMin + i * step;
		curve.append(QPointF(x, fit.a * x * x + fit.b * x + fit.c));
	}
	return curve;
}

QString APIPolynomialFitter::formatLinear(const FitResult& fit)
{
	if (!fit.valid) return QStringLiteral("ÄâºÏÊ§°Ü");
	return QString("y = %1*x + %2  (R?=%3)")
		.arg(fit.a, 0, 'f', 6).arg(fit.b, 0, 'f', 6).arg(fit.r2, 0, 'f', 6);
}

QString APIPolynomialFitter::formatQuadratic(const FitResult& fit)
{
	if (!fit.valid) return QStringLiteral("ÄâºÏÊ§°Ü");
	return QString("y = %1*x? + %2*x + %3  (R?=%4)")
		.arg(fit.a, 0, 'f', 6).arg(fit.b, 0, 'f', 6)
		.arg(fit.c, 0, 'f', 6).arg(fit.r2, 0, 'f', 6);
}
