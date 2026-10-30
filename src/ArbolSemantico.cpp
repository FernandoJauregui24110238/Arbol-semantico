#include "ArbolSemantico.hpp"
#include <QSet>
#include <functional>

ArbolSemantico::ArbolSemantico(GestorAtomos* gestor)
    : gestor(gestor), raiz(nullptr)
{
}

ArbolSemantico::~ArbolSemantico()
{
    delete raiz;
}

static Formula* clonarFormula(const Formula* origen)
{
    if (!origen) return nullptr;
    Formula* c = new Formula();
    c->tipo = origen->tipo;
    c->textoAtomo = origen->textoAtomo;
    c->izquierda = clonarFormula(origen->izquierda);
    c->derecha = clonarFormula(origen->derecha);
    return c;
}

static NodoArbol* crearHijoClonado(NodoArbol* padre, const QVector<Formula*>& nuevas)
{
    NodoArbol* hijo = new NodoArbol();
    hijo->padre = padre;
    hijo->profundidad = padre->profundidad + 1;
    for (Formula* fp : padre->formulas) {
        hijo->agregarFormula(clonarFormula(fp));
    }
    for (int k = 0; k < padre->aplicada.size() && k < hijo->aplicada.size(); ++k) {
        hijo->aplicada[k] = padre->aplicada[k];
    }
    for (Formula* nf : nuevas) {
        hijo->agregarFormula(nf);
    }
    padre->hijos.append(hijo);
    return hijo;
}

NodoArbol* ArbolSemantico::construir(const QVector<Formula*>& formulas)
{
    delete raiz;
    raiz = new NodoArbol();
    raiz->profundidad = 0;
    for (Formula* f : formulas) {
        raiz->agregarFormula(clonarFormula(f));
    }
    return raiz;
}

int ArbolSemantico::siguienteFormulaPendiente(NodoArbol* nodo) const
{
    for (int i = 0; i < nodo->formulas.size(); ++i) {
        if (nodo->aplicada[i]) continue;
        const Formula* f = nodo->formulas[i];
        if (f->tipo == Formula::ATOMO) continue;
        if (f->tipo == Formula::NEGACION &&
            f->izquierda && f->izquierda->tipo == Formula::ATOMO) continue;

        bool noRamifica = false;
        if (f->tipo == Formula::CONJUNCION) noRamifica = true;
        else if (f->tipo == Formula::NEGACION && f->izquierda) {
            if (f->izquierda->tipo == Formula::DISYUNCION ||
                f->izquierda->tipo == Formula::IMPLICACION ||
                f->izquierda->tipo == Formula::NEGACION) {
                noRamifica = true;
            }
        }
        if (noRamifica) return i;
    }
    for (int i = 0; i < nodo->formulas.size(); ++i) {
        if (nodo->aplicada[i]) continue;
        const Formula* f = nodo->formulas[i];
        if (f->tipo == Formula::ATOMO) continue;
        if (f->tipo == Formula::NEGACION &&
            f->izquierda && f->izquierda->tipo == Formula::ATOMO) continue;
        return i;
    }
    return -1;
}

void ArbolSemantico::expandir(NodoArbol* nodo)
{
    if (!nodo) return;

    QString contradiccion;
    if (nodo->detectarContradiccion(contradiccion)) {
        nodo->estado = NodoArbol::CERRADA;
        nodo->contradiccion = contradiccion;
        return;
    }

    int idx = siguienteFormulaPendiente(nodo);
    if (idx < 0) {
        nodo->estado = NodoArbol::ABIERTA;
        return;
    }

    nodo->marcarAplicada(idx);
    aplicarFormula(nodo, idx);
}

bool ArbolSemantico::aplicarFormula(NodoArbol* nodo, int indiceFormula)
{
    const Formula* f = nodo->formulas[indiceFormula];
    if (!f) return false;

    switch (f->tipo) {
    case Formula::CONJUNCION: {
        NodoArbol* hijo = crearHijoClonado(nodo, {});
        hijo->agregarFormula(clonarFormula(f->izquierda));
        hijo->agregarFormula(clonarFormula(f->derecha));
        expandir(hijo);
        return true;
    }
    case Formula::NEGACION: {
        const Formula* in = f->izquierda;
        if (!in) return false;
        if (in->tipo == Formula::NEGACION) {
            NodoArbol* hijo = crearHijoClonado(nodo, {});
            hijo->agregarFormula(clonarFormula(in->izquierda));
            expandir(hijo);
            return true;
        }
        if (in->tipo == Formula::DISYUNCION) {
            Formula* n1 = new Formula(); n1->tipo = Formula::NEGACION; n1->izquierda = clonarFormula(in->izquierda);
            Formula* n2 = new Formula(); n2->tipo = Formula::NEGACION; n2->izquierda = clonarFormula(in->derecha);
            NodoArbol* hijo = crearHijoClonado(nodo, {});
            hijo->agregarFormula(n1);
            hijo->agregarFormula(n2);
            expandir(hijo);
            return true;
        }
        if (in->tipo == Formula::IMPLICACION) {
            Formula* a = clonarFormula(in->izquierda);
            Formula* nb = new Formula(); nb->tipo = Formula::NEGACION; nb->izquierda = clonarFormula(in->derecha);
            NodoArbol* hijo = crearHijoClonado(nodo, {});
            hijo->agregarFormula(a);
            hijo->agregarFormula(nb);
            expandir(hijo);
            return true;
        }
        if (in->tipo == Formula::CONJUNCION) {
            Formula* n1 = new Formula(); n1->tipo = Formula::NEGACION; n1->izquierda = clonarFormula(in->izquierda);
            Formula* n2 = new Formula(); n2->tipo = Formula::NEGACION; n2->izquierda = clonarFormula(in->derecha);
            NodoArbol* h1 = crearHijoClonado(nodo, { n1 });
            NodoArbol* h2 = crearHijoClonado(nodo, { n2 });
            expandir(h1);
            expandir(h2);
            return true;
        }
        if (in->tipo == Formula::BICONDICIONAL) {
            Formula* a1 = clonarFormula(in->izquierda);
            Formula* nb1 = new Formula(); nb1->tipo = Formula::NEGACION; nb1->izquierda = clonarFormula(in->derecha);
            Formula* na2 = new Formula(); na2->tipo = Formula::NEGACION; na2->izquierda = clonarFormula(in->izquierda);
            Formula* b2 = clonarFormula(in->derecha);
            NodoArbol* h1 = crearHijoClonado(nodo, { a1, nb1 });
            NodoArbol* h2 = crearHijoClonado(nodo, { na2, b2 });
            expandir(h1);
            expandir(h2);
            return true;
        }
        return false;
    }
    case Formula::DISYUNCION: {
        Formula* a = clonarFormula(f->izquierda);
        Formula* b = clonarFormula(f->derecha);
        NodoArbol* h1 = crearHijoClonado(nodo, { a });
        NodoArbol* h2 = crearHijoClonado(nodo, { b });
        expandir(h1);
        expandir(h2);
        return true;
    }
    case Formula::IMPLICACION: {
        Formula* na = new Formula(); na->tipo = Formula::NEGACION; na->izquierda = clonarFormula(f->izquierda);
        Formula* b = clonarFormula(f->derecha);
        NodoArbol* h1 = crearHijoClonado(nodo, { na });
        NodoArbol* h2 = crearHijoClonado(nodo, { b });
        expandir(h1);
        expandir(h2);
        return true;
    }
    case Formula::BICONDICIONAL: {
        Formula* a1 = clonarFormula(f->izquierda);
        Formula* b1 = clonarFormula(f->derecha);
        Formula* na2 = new Formula(); na2->tipo = Formula::NEGACION; na2->izquierda = clonarFormula(f->izquierda);
        Formula* nb2 = new Formula(); nb2->tipo = Formula::NEGACION; nb2->izquierda = clonarFormula(f->derecha);
        NodoArbol* h1 = crearHijoClonado(nodo, { a1, b1 });
        NodoArbol* h2 = crearHijoClonado(nodo, { na2, nb2 });
        expandir(h1);
        expandir(h2);
        return true;
    }
    default:
        return false;
    }
}

void ArbolSemantico::contarRamas(int& cerradas, int& abiertas) const
{
    cerradas = 0;
    abiertas = 0;
    if (!raiz) return;
    std::function<void(NodoArbol*)> recorrer = [&](NodoArbol* n) {
        if (!n) return;
        if (n->hijos.isEmpty()) {
            if (n->estado == NodoArbol::CERRADA) ++cerradas;
            else if (n->estado == NodoArbol::ABIERTA) ++abiertas;
        } else {
            for (NodoArbol* h : n->hijos) recorrer(h);
        }
    };
    recorrer(raiz);
}

void ArbolSemantico::recolectarRamasAbiertas(NodoArbol* nodo, QVector<NodoArbol*>& hojas) const
{
    if (!nodo) return;
    if (nodo->hijos.isEmpty()) {
        if (nodo->estado == NodoArbol::ABIERTA) hojas.append(nodo);
        return;
    }
    for (NodoArbol* h : nodo->hijos) recolectarRamasAbiertas(h, hojas);
}

QVector<QMap<QString, bool>> ArbolSemantico::obtenerModelos() const
{
    QVector<QMap<QString, bool>> modelos;
    QVector<NodoArbol*> hojas;
    recolectarRamasAbiertas(raiz, hojas);
    for (NodoArbol* hoja : hojas) {
        QMap<QString, bool> modelo;
        QVector<QString> literales = hoja->literalesAcumulados();
        for (const QString& lit : literales) {
            if (lit.startsWith("!")) {
                modelo.insert(lit.mid(1), false);
            } else if (!modelo.contains(lit)) {
                modelo.insert(lit, true);
            }
        }
        modelos.append(modelo);
    }
    return modelos;
}

QVector<QString> ArbolSemantico::todasLasLetras() const
{
    return gestor->letras();
}