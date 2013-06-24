//// Autor: Ivan Sevcik (xsevci50)
//// Datum vytvorenia: 27.10.2012
//// Popis:
//// Subor obsahujuci deklaracie
//// matematickych funkcii.

#ifndef PROJ2MATH_H_INCLUDED
#define PROJ2MATH_H_INCLUDED

#include <math.h>
#include <stdbool.h>

/**
 * @brief Porovna 2 desatinne cisla na rovnost.
 * @param x Porovnavane cislo.
 * @param y Porovnavane cislo.
 * @return True ak boli cisla rovnake.
 */
bool m_equal(double x, double y);

/**
 * @brief Vypocita absolutnu hodnotu cisla.
 * @param x Cislo.
 * @return Abslutna hodnota.
 */
double m_abs(double x);

/**
 * @brief Vypocita druhu odmocninu cisla.
 * @param x Cislo.
 * @param eps Percentualna presnost vypoctu.
 * @return Druha odmocnina.
 */
double m_sqrt(double x, double eps);

/**
 * @brief Vypocita logaritmus cisla pri zaklade 2.
 * @param x Cislo.
 * @param eps Percentualna presnost vypoctu.
 * @return Logaritmus pri zaklade 2.
 */
double m_log2(double x, double eps);

/**
 * @brief Vypocita prirodzeny logaritmus cisla.
 * @param x Cislo.
 * @param eps Percentualna presnost vypoctu.
 * @return Prirodzeny logaritmus.
 */
double m_ln(double x, double eps);

/**
 * @brief Vypocita mocninu cisla.
 * @param x Cislo.
 * @param exp Exponent.
 * @param eps Percentualna presnost vypoctu.
 * @return Mocnina.
 */
double m_pow(double x, double exp, double eps);

/**
 * @brief Vypocita arcus tangens (inverzny tangens) z cisla.
 * @param x Cislo.
 * @param eps Percentualna presnost vypoctu.
 * @return Arcus tangens.
 */
double m_atan(double x, double eps);

/**
 * @brief Vypocita argument sinusu hyperbolickeho z cisla.
 * @param x Cislo.
 * @param eps Percentualna presnost vypoctu.
 * @return Argument sinusu hyperbolickeho.
 */
double m_argsinh(double x, double eps);

#endif // PROJ2MATH_H_INCLUDED
