#include "NodoArbol.hpp"
#include <QSet>

NodoArbol::NodoArbol()
    : estado(PENDIENTE), padre(nullptr), profundidad(0)
{
}

NodoArbol::~NodoArbol()
{
    for (Formula* f : formulas) delete f;
    formulas.clear();
    for (NodoArbol* h : hijos) delete h;
    hijos.clear();
}

void NodoArbol::agregarFormula(Formula* f)
{
    formulas.append(f);
    aplicada.append(false);
}

void NodoArbol::marcarAplicada(int indice)
{
    if (indice >= 0 && indice < aplicada.size()) {
        aplicada[indice] = true;
    }
}

bool NodoArbol::todasAplicadas() const
{
    for (int i = 0; i < formulas.size(); ++i) {
        if (!aplicada[i]) {
            const Formula* f = formulas[i];
            bool esLiteral = (f->tipo == Formula::ATOMO) ||
                              (f->tipo == Formula::NEGACION &&
                               f->izquierda && f->izquierda->tipo == Formula::ATOMO);
            if (!esLiteral) return false;
        }
    }
    return true;
}

bool NodoArbol::detectarContradiccion(QString& descripcion) const
{
    QVector<QString> literales = literalesAcumulados();
    QSet<QString> positivos;
    QSet<QString> negativos;
    for (const QString& lit : literales) {
        if (lit.startsWith("!")) negativos.insert(lit.mid(1));
        else positivos.insert(lit);
    }
    for (const QString& p : positivos) {
        if (negativos.contains(p)) {
            descripcion = QString("Contradicción: «%1» y «no %1».").arg(p);
            return true;
        }
    }
    return false;
}

void NodoArbol::recolectarLiterales(QVector<QString>& lista) const
{
    for (int i = 0; i < formulas.size(); ++i) {
        const Formula* f = formulas[i];
        if (f->tipo == Formula::ATOMO) {
            lista.append(f->textoAtomo);
        } else if (f->tipo == Formula::NEGACION &&
                   f->izquierda && f->izquierda->tipo == Formula::ATOMO) {
            lista.append("!" + f->izquierda->textoAtomo);
        }
    }
    if (padre) padre->recolectarLiterales(lista);
}

QVector<QString> NodoArbol::literalesAcumulados() const
{
    QVector<QString> lista;
    recolectarLiterales(lista);
    return lista;
}