#ifndef MOTORINFERENCIA_HPP
#define MOTORINFERENCIA_HPP

#include <QString>
#include <QVector>
#include <QMap>
#include "AnalizadorLexico.hpp"
#include "GestorAtomos.hpp"
#include "ArbolSemantico.hpp"

struct ResultadoAnalisis {
    bool exitoso;
    QString mensajeError;
    QVector<QString> lineasOriginales;
    QVector<Formula*> formulas;
    QVector<QString> formulasComoTexto;
    GestorAtomos* gestor;
    ArbolSemantico* arbol;
    int ramasCerradas;
    int ramasAbiertas;
    QVector<QMap<QString, bool>> modelos;
};

class MotorInferencia {
public:
    MotorInferencia();
    ~MotorInferencia();

    ResultadoAnalisis analizar(const QString& entrada);

private:
    AnalizadorLexico analizador;
};

#endif