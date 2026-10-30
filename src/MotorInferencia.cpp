#include "MotorInferencia.hpp"
#include <QStringList>
#include <functional>

MotorInferencia::MotorInferencia() {}
MotorInferencia::~MotorInferencia() {}

static QString formulaATexto(const Formula* f)
{
    if (!f) return QString();
    switch (f->tipo) {
    case Formula::ATOMO:
        return f->textoAtomo;
    case Formula::NEGACION:
        return QString("¬%1").arg(formulaATexto(f->izquierda));
    case Formula::CONJUNCION:
        return QString("(%1 ∧ %2)").arg(formulaATexto(f->izquierda), formulaATexto(f->derecha));
    case Formula::DISYUNCION:
        return QString("(%1 ∨ %2)").arg(formulaATexto(f->izquierda), formulaATexto(f->derecha));
    case Formula::IMPLICACION:
        return QString("(%1 → %2)").arg(formulaATexto(f->izquierda), formulaATexto(f->derecha));
    case Formula::BICONDICIONAL:
        return QString("(%1 ↔ %2)").arg(formulaATexto(f->izquierda), formulaATexto(f->derecha));
    }
    return QString();
}

ResultadoAnalisis MotorInferencia::analizar(const QString& entrada)
{
    ResultadoAnalisis r;
    r.exitoso = false;
    r.gestor = nullptr;
    r.arbol = nullptr;
    r.ramasCerradas = 0;
    r.ramasAbiertas = 0;

    QStringList lineas = entrada.split('\n', Qt::SkipEmptyParts);
    if (lineas.isEmpty()) {
        r.mensajeError = "No se ingresó ninguna regla.";
        return r;
    }

    GestorAtomos* gestor = new GestorAtomos();
    QVector<Formula*> formulas;

    for (int i = 0; i < lineas.size(); ++i) {
        QString linea = lineas[i].trimmed();
        if (linea.isEmpty()) continue;

        QString err;
        Formula* f = analizador.analizarLinea(linea, err);
        if (!f) {
            r.mensajeError = QString("Línea %1: %2").arg(i + 1).arg(err);
            for (Formula* ff : formulas) delete ff;
            delete gestor;
            return r;
        }

        std::function<void(Formula*)> registrar = [&](Formula* ff) {
            if (!ff) return;
            if (ff->tipo == Formula::ATOMO) gestor->registrar(ff->textoAtomo);
            else { registrar(ff->izquierda); registrar(ff->derecha); }
        };
        registrar(f);

        formulas.append(f);
        r.lineasOriginales.append(linea);
        r.formulasComoTexto.append(formulaATexto(f));
    }

    if (formulas.isEmpty()) {
        r.mensajeError = "No se ingresó ninguna regla válida.";
        delete gestor;
        return r;
    }

    r.formulas = formulas;
    r.gestor = gestor;
    r.arbol = new ArbolSemantico(gestor);
    NodoArbol* raiz = r.arbol->construir(formulas);
    r.arbol->expandir(raiz);
    r.arbol->contarRamas(r.ramasCerradas, r.ramasAbiertas);
    r.modelos = r.arbol->obtenerModelos();
    r.exitoso = true;
    return r;
}