#ifndef GESTORATOMOS_HPP
#define GESTORATOMOS_HPP

#include <QString>
#include <QMap>
#include <QVector>

class GestorAtomos {
public:
    GestorAtomos();

    QString registrar(const QString& proposicion);

    QString letraDe(const QString& proposicion) const;

    QString significadoDe(const QString& letra) const;

    QVector<QString> letras() const;

    void limpiar();

private:
    QMap<QString, QString> proposicionALetra;
    QMap<QString, QString> letraAProposicion;
    int siguienteIndice;
};

#endif