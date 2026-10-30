#ifndef ANALIZADORLEXICO_HPP
#define ANALIZADORLEXICO_HPP

#include <QString>
#include <QStringList>
#include <QVector>

struct Formula {
    enum Tipo { ATOMO, NEGACION, CONJUNCION, DISYUNCION, IMPLICACION, BICONDICIONAL };
    Tipo tipo;
    QString textoAtomo;
    Formula* izquierda;
    Formula* derecha;

    Formula() : tipo(ATOMO), izquierda(nullptr), derecha(nullptr) {}
    ~Formula() { delete izquierda; delete derecha; }
};

class AnalizadorLexico {
public:
    AnalizadorLexico();

    Formula* analizarLinea(const QString& linea, QString& mensajeError);

    static QString normalizar(const QString& entrada);

private:
    QStringList tokens;
    int posicion;
    QString errorActual;

    void tokenizar(const QString& lineaNormalizada);
    Formula* parseBicondicional();
    Formula* parseImplicacion();
    Formula* parseDisyuncion();
    Formula* parseConjuncion();
    Formula* parseNegacion();
    Formula* parsePrimario();
    bool esConector(const QString& t) const;
    bool hayError() const { return !errorActual.isEmpty(); }
};

#endif