#ifndef NODOARBOL_HPP
#define NODOARBOL_HPP

#include <QString>
#include <QVector>
#include "AnalizadorLexico.hpp"

class NodoArbol {
public:
    NodoArbol();
    ~NodoArbol();

    QVector<Formula*> formulas;
    QVector<bool> aplicada;

    enum Estado { PENDIENTE, ABIERTA, CERRADA };
    Estado estado;

    QString contradiccion;

    QVector<NodoArbol*> hijos;

    NodoArbol* padre;

    int profundidad;

    void agregarFormula(Formula* f);

    void marcarAplicada(int indice);

    bool todasAplicadas() const;

    bool detectarContradiccion(QString& descripcion) const;

    QVector<QString> literalesAcumulados() const;

private:
    void recolectarLiterales(QVector<QString>& lista) const;
};

#endif