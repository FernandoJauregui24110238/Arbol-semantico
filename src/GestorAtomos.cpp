#include "GestorAtomos.hpp"
#include "AnalizadorLexico.hpp"

GestorAtomos::GestorAtomos() : siguienteIndice(0) {}

QString GestorAtomos::registrar(const QString& proposicion)
{
    QString clave = AnalizadorLexico::normalizar(proposicion);
    if (proposicionALetra.contains(clave)) {
        return proposicionALetra.value(clave);
    }
    QString letra;
    int indice = siguienteIndice;
    if (indice < 26) {
        letra = QString(QChar('p' + indice));
    } else {
        int base = indice % 26;
        int rep = indice / 26;
        letra = QString(QChar('p' + base)) + QString::number(rep);
    }
    proposicionALetra.insert(clave, letra);
    letraAProposicion.insert(letra, clave);
    ++siguienteIndice;
    return letra;
}

QString GestorAtomos::letraDe(const QString& proposicion) const
{
    QString clave = AnalizadorLexico::normalizar(proposicion);
    return proposicionALetra.value(clave, QString());
}

QString GestorAtomos::significadoDe(const QString& letra) const
{
    return letraAProposicion.value(letra, QString());
}

QVector<QString> GestorAtomos::letras() const
{
    QVector<QString> resultado;
    for (int i = 0; i < siguienteIndice; ++i) {
        QString letra;
        if (i < 26) {
            letra = QString(QChar('p' + i));
        } else {
            int base = i % 26;
            int rep = i / 26;
            letra = QString(QChar('p' + base)) + QString::number(rep);
        }
        resultado.append(letra);
    }
    return resultado;
}

void GestorAtomos::limpiar()
{
    proposicionALetra.clear();
    letraAProposicion.clear();
    siguienteIndice = 0;
}