extern "C" {
#include <assert.h>
#include <stdlib.h>
#include <math.h>
}
#include <poincare/power.h>
#include <poincare/multiplication.h>
#include "layout/baseline_relative_layout.h"

namespace Poincare {

float Power::privateApproximate(Context& context, AngleUnit angleUnit) const {
  assert(angleUnit != AngleUnit::Default);
  return powf(m_operands[0]->approximate(context, angleUnit), m_operands[1]->approximate(context, angleUnit));
}

Expression::Type Power::type() const {
  return Type::Power;
}

Expression * Power::cloneWithDifferentOperands(Expression** newOperands,
    int numberOfOperands, bool cloneOperands) const {
  assert(numberOfOperands == 2);
  return new Power(newOperands, cloneOperands);
}

ExpressionLayout * Power::privateCreateLayout(FloatDisplayMode floatDisplayMode, ComplexFormat complexFormat) const {
  assert(floatDisplayMode != FloatDisplayMode::Default);
  assert(complexFormat != ComplexFormat::Default);
  Expression * indiceOperand = m_operands[1];
  // Delete eventual parentheses of the indice in the pretty print
  if (m_operands[1]->type() == Type::Parenthesis) {
    indiceOperand = (Expression *)m_operands[1]->operand(0);
  }
  return new BaselineRelativeLayout(m_operands[0]->createLayout(floatDisplayMode, complexFormat),indiceOperand->createLayout(floatDisplayMode, complexFormat), BaselineRelativeLayout::Type::Superscript);
}

Expression * Power::evaluateOnComplex(Complex * c, Complex * d, Context& context, AngleUnit angleUnit) const {
  if (d->b() != 0.0f) {
    /* First case c and d is complex */
    if (c->b() != 0.0f || c->a() <= 0) {
      return new Complex(Complex::Float(NAN));
    }
    /* Second case only d is complex */
    float radius = powf(c->a(), d->a());
    float theta = d->b()*logf(c->a());
    return new Complex(Complex::Polar(radius, theta));
  }
  /* Third case only c is complex */
  float radius = powf(c->r(), d->a());
  if (c->b() == 0 && d->a() == roundf(d->a())) {
    /* We handle the case "c float and d integer" separatly to avoid getting
     * complex result due to float representation: a float power an integer is
     * always real. */
    return new Complex(Complex::Cartesian(radius, 0.0f));
  }
  if (c->a() < 0 && c->b() == 0 && d->a() == 0.5f) {
    /* We handle the case "c negative float and d = 1/2" separatly to avoid
     * getting wrong result due to float representation: the squared root of
     * a negative float is always a pure imaginative. */
    return new Complex(Complex::Cartesian(0.0f, radius));
  }
  /* Third case only c is complex */
  float theta = d->a()*c->th();
  return new Complex(Complex::Polar(radius, theta));
}

Expression * Power::evaluateOnMatrixAndComplex(Matrix * m, Complex * c, Context& context, AngleUnit angleUnit) const {
  if (isnan(m_operands[1]->approximate(context, angleUnit)) || m_operands[1]->approximate(context, angleUnit) != (int)m_operands[1]->approximate(context, angleUnit)) {
    return new Complex(Complex::Float(NAN));
  }
 if (m->numberOfColumns() != m->numberOfRows()) {
    return new Complex(Complex::Float(NAN));
  }
  float power = c->approximate(context, angleUnit);
  if (isnan(power) || isinf(power) || power != (int)power) {
    return new Complex(Complex::Float(NAN));
  }
  if (power == 0.0f) {
    /* Build the identity matrix with the same dimensions as m */
    Expression ** operands = (Expression **)malloc(m->numberOfOperands()*sizeof(Expression *));
    for (int i = 0; i < m->numberOfRows(); i++) {
      for (int j = 0; j < m->numberOfColumns(); j++) {
        if (i == j) {
          operands[i*m->numberOfColumns()+j] = new Complex(Complex::Float(1.0f));
        } else {
          operands[i*m->numberOfColumns()+j] = new Complex(Complex::Float(0.0f));
        }
      }
    }
    Expression * matrix = new Matrix(new MatrixData(operands, m->numberOfOperands(), m->numberOfColumns(), m->numberOfRows(), false));
    free(operands);
    return matrix;
  }
  if (power < 0.0f) {
    Expression * inverse = m->createInverse(context, angleUnit);
    Expression * operands[2];
    operands[0] = inverse;
    operands[1] = new Complex(Complex::Float(-power));;
    Expression * power = new Power(operands, true);
    delete operands[0];
    delete operands[1];
    Expression * result = power->evaluate(context, angleUnit);
    delete power;
    return result;
  }
  Expression * result = new Complex(Complex::Float(1.0f));
  for (int k = 0; k < (int)power; k++) {
    Expression * operands[2];
    operands[0] = result;
    operands[1] = m;
    Expression * multiplication = new Multiplication(operands, true);
    Expression * newResult = multiplication->evaluate(context, angleUnit);
    delete result;
    result = newResult;
    delete multiplication;
  }
  return result;
}

Expression * Power::evaluateOnComplexAndMatrix(Complex * c, Matrix * m, Context& context, AngleUnit angleUnit) const {
  return new Complex(Complex::Float(NAN));
}

Expression * Power::evaluateOnMatrices(Matrix * m, Matrix * n, Context& context, AngleUnit angleUnit) const {
  return new Complex(Complex::Float(NAN));
}

}
