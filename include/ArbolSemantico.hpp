#ifndef ARBOLSEMANTICO_HPP
#define ARBOLSEMANTICO_HPP

#include <QVector>
#include <QString>
#include "NodoArbol.hpp"
#include "GestorAtomos.hpp"

class ArbolSemantico {
public:
    ArbolSemantico(GestorAtomos* gestor);
    ~ArbolSemantico();

    NodoArbol* construir(const QVector<Formula*>& formulas);

    void expandir(NodoArbol* raiz);

    NodoArbol* getRaiz() const { return raiz; }

    void contarRamas(int& cerradas, int& abiertas) const;

    QVector<QMap<QString, bool>> obtenerModelos() const;

    QVector<QString> todasLasLetras() const;

private:
    GestorAtomos* gestor;
    NodoArbol* raiz;

    bool aplicarFormula(NodoArbol* nodo, int indiceFormula);

    void propagarCierre(NodoArbol* nodo);

    int siguienteFormulaPendiente(NodoArbol* nodo) const;

    void recolectarRamasAbiertas(NodoArbol* nodo, QVector<NodoArbol*>& hojas) const;
};

#endif