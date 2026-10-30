#include "MotorInferencia.hpp"
#include <iostream>

static bool comprobar(bool condicion, const char* mensaje)
{
    if (!condicion) {
        std::cerr << "FAIL: " << mensaje << '\n';
        return false;
    }
    return true;
}

int main()
{
    bool correcto = true;
    MotorInferencia motor;
    ResultadoAnalisis resultado = motor.analizar(QString::fromUtf8(
        "Si es de día, entonces voy a trabajar\n"
        "Si llueve, entonces no voy a trabajar\n"
        "Es de día"));

    correcto &= comprobar(resultado.exitoso, "el caso mínimo debe analizarse");
    if (!resultado.exitoso) {
        std::cerr << resultado.mensajeError.toStdString() << '\n';
        return 1;
    }

    correcto &= comprobar(resultado.formulasComoTexto.size() == 3, "debe haber tres reglas");
    correcto &= comprobar(resultado.formulasComoTexto[0] == QString::fromUtf8("(es de dia → voy a trabajar)"), "R1 debe ser p implica q");
    correcto &= comprobar(resultado.formulasComoTexto[1] == QString::fromUtf8("(llueve → ¬voy a trabajar)"), "R2 debe negar el mismo átomo q");
    correcto &= comprobar(resultado.formulasComoTexto[2] == "es de dia", "R3 debe ser un hecho simple");

    QVector<QString> letras = resultado.gestor->letras();
    correcto &= comprobar(letras.size() == 3, "debe identificar tres átomos simples");
    correcto &= comprobar(resultado.gestor->significadoDe("p") == "es de dia", "p debe representar es de día");
    correcto &= comprobar(resultado.gestor->significadoDe("q") == "voy a trabajar", "q debe reutilizarse bajo negación");
    correcto &= comprobar(resultado.gestor->significadoDe("r") == "llueve", "r debe representar llueve");
    correcto &= comprobar(resultado.ramasCerradas == 2, "debe cerrar dos ramas");
    correcto &= comprobar(resultado.ramasAbiertas == 1, "debe dejar una rama abierta");
    correcto &= comprobar(resultado.modelos.size() == 1, "debe producir un modelo");
    if (resultado.modelos.size() == 1) {
        const auto& modelo = resultado.modelos.first();
        correcto &= comprobar(modelo.value("p", false), "p debe ser verdadero");
        correcto &= comprobar(modelo.value("q", false), "q debe ser verdadero");
        correcto &= comprobar(!modelo.value("r", true), "r debe ser falso");
    }

    AnalizadorLexico analizador;
    const struct { const char* linea; Formula::Tipo tipo; const char* nombre; } conectivos[] = {
        {"llueve y hay viento", Formula::CONJUNCION, "y"},
        {"llueve o hay viento", Formula::DISYUNCION, "o"},
        {"no llueve", Formula::NEGACION, "no"},
        {"Si llueve, entonces hay viento", Formula::IMPLICACION, "si-entonces"},
        {"llueve si y solo si hay viento", Formula::BICONDICIONAL, "si-y-solo-si"}
    };
    for (const auto& caso : conectivos) {
        QString error;
        Formula* formula = analizador.analizarLinea(QString::fromUtf8(caso.linea), error);
        correcto &= comprobar(formula && formula->tipo == caso.tipo, caso.nombre);
        delete formula;
    }

    QString error;
    Formula* malformada = analizador.analizarLinea("Si llueve", error);
    correcto &= comprobar(malformada == nullptr, "debe rechazar una regla condicional incompleta");
    delete malformada;

    if (!correcto) return 1;
    std::cout << "PASS: caso mínimo, átomos, conectivos, ramas y modelo" << '\n';
    return 0;
}
