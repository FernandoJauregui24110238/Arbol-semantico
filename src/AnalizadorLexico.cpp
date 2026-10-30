#include "AnalizadorLexico.hpp"

AnalizadorLexico::AnalizadorLexico() : posicion(0) {}

QString AnalizadorLexico::normalizar(const QString& entrada)
{
    QString s = entrada;
    s.replace(QChar(0x00E1), "a");
    s.replace(QChar(0x00E9), "e");
    s.replace(QChar(0x00ED), "i");
    s.replace(QChar(0x00F3), "o");
    s.replace(QChar(0x00FA), "u");
    s.replace(QChar(0x00C1), "A");
    s.replace(QChar(0x00C9), "E");
    s.replace(QChar(0x00CD), "I");
    s.replace(QChar(0x00D3), "O");
    s.replace(QChar(0x00DA), "U");
    s.replace(QChar(0x00FC), "u");
    s.replace(QChar(0x00DC), "U");
    s.replace(QChar(0x00F1), "n");
    s.replace(QChar(0x00D1), "N");
    s = s.toLower();
    s.remove(',');
    s.remove('.');
    s = s.simplified();
    return s;
}

void AnalizadorLexico::tokenizar(const QString& lineaNormalizada)
{
    tokens.clear();
    posicion = 0;
    errorActual.clear();

    QString resto = lineaNormalizada.trimmed();
    if (resto.isEmpty()) {
        errorActual = "La línea está vacía.";
        return;
    }

    int i = 0;
    const int n = resto.size();
    while (i < n) {
        QChar c = resto.at(i);
        if (c.isSpace()) { ++i; continue; }
        if (c == '(' || c == ')') { tokens << QString(c); ++i; continue; }
        if (c == '&' || c == '|' || c == '!' || c == '~') { tokens << QString(c); ++i; continue; }
        if (c == '<' && i + 2 < n && resto.at(i+1) == '-' && resto.at(i+2) == '>') {
            tokens << QString("<->"); i += 3; continue;
        }
        if (c == '-' && i + 1 < n && resto.at(i+1) == '>') {
            tokens << QString("->"); i += 2; continue;
        }
        int inicio = i;
        while (i < n && !resto.at(i).isSpace() &&
               resto.at(i) != '(' && resto.at(i) != ')' &&
               resto.at(i) != '&' && resto.at(i) != '|' &&
               resto.at(i) != '!' && resto.at(i) != '~' &&
               resto.at(i) != '<' && resto.at(i) != '-') {
            ++i;
        }
        tokens << resto.mid(inicio, i - inicio);
    }

    for (int k = 0; k + 3 < tokens.size(); ++k) {
        if (tokens[k] == "si" && tokens[k + 1] == "y" &&
            tokens[k + 2] == "solo" && tokens[k + 3] == "si") {
            tokens[k] = "<->";
            tokens.removeAt(k + 1);
            tokens.removeAt(k + 1);
            tokens.removeAt(k + 1);
        }
    }

    if (!tokens.isEmpty() && tokens.first() == "si") {
        tokens.removeFirst();
        if (!tokens.contains("entonces")) {
            errorActual = "La palabra «si» requiere un «entonces» correspondiente.";
            return;
        }
    }

    for (int k = 0; k < tokens.size(); ++k) {
        if (tokens[k] == "no") {
            if (k + 1 >= tokens.size() ||
                tokens[k+1] == ")" || tokens[k+1] == "y" ||
                tokens[k+1] == "o" || tokens[k+1] == "entonces") {
                errorActual = "La palabra «no» debe ir seguida de una proposición.";
                return;
            }
        }
    }
}

bool AnalizadorLexico::esConector(const QString& t) const
{
    return t == "y" || t == "o" || t == "no" ||
           t == "entonces" || t == "si" || t == "solo" ||
           t == "bicondicional" ||
           t == "(" || t == ")" ||
           t == "->" || t == "<->" || t == "&" || t == "|" || t == "!" || t == "~";
}

Formula* AnalizadorLexico::analizarLinea(const QString& linea, QString& mensajeError)
{
    QString norm = normalizar(linea);
    tokenizar(norm);
    if (hayError()) { mensajeError = errorActual; return nullptr; }
    if (tokens.isEmpty()) { mensajeError = "La línea está vacía."; return nullptr; }

    Formula* f = parseBicondicional();
    if (hayError()) { mensajeError = errorActual; delete f; return nullptr; }
    if (f == nullptr) { mensajeError = "No se pudo interpretar la regla."; return nullptr; }
    if (posicion != tokens.size()) {
        mensajeError = QString("Token inesperado: «%1».").arg(tokens[posicion]);
        delete f;
        return nullptr;
    }
    return f;
}

Formula* AnalizadorLexico::parseBicondicional()
{
    Formula* izq = parseImplicacion();
    if (hayError()) return nullptr;
    while (posicion < tokens.size() &&
           (tokens[posicion] == "solo" || tokens[posicion] == "<->" || tokens[posicion] == "bicondicional")) {
        if (tokens[posicion] == "solo") posicion += 2;
        else posicion += 1;
        Formula* der = parseImplicacion();
        if (hayError()) { delete izq; return nullptr; }
        if (der == nullptr) {
            errorActual = "Falta la proposición derecha del «si y solo si».";
            delete izq;
            return nullptr;
        }
        Formula* f = new Formula();
        f->tipo = Formula::BICONDICIONAL;
        f->izquierda = izq;
        f->derecha = der;
        izq = f;
    }
    return izq;
}

Formula* AnalizadorLexico::parseImplicacion()
{
    Formula* izq = parseDisyuncion();
    if (hayError()) return nullptr;
    while (posicion < tokens.size() &&
           (tokens[posicion] == "entonces" || tokens[posicion] == "->")) {
        ++posicion;
        Formula* der = parseDisyuncion();
        if (hayError()) { delete izq; return nullptr; }
        if (der == nullptr) {
            errorActual = "Falta la proposición derecha del «entonces».";
            delete izq;
            return nullptr;
        }
        Formula* f = new Formula();
        f->tipo = Formula::IMPLICACION;
        f->izquierda = izq;
        f->derecha = der;
        izq = f;
    }
    return izq;
}

Formula* AnalizadorLexico::parseDisyuncion()
{
    Formula* izq = parseConjuncion();
    if (hayError()) return nullptr;
    while (posicion < tokens.size() && (tokens[posicion] == "o" || tokens[posicion] == "|")) {
        ++posicion;
        Formula* der = parseConjuncion();
        if (hayError()) { delete izq; return nullptr; }
        if (der == nullptr) {
            errorActual = "Falta la proposición derecha de la disyunción «o».";
            delete izq;
            return nullptr;
        }
        Formula* f = new Formula();
        f->tipo = Formula::DISYUNCION;
        f->izquierda = izq;
        f->derecha = der;
        izq = f;
    }
    return izq;
}

Formula* AnalizadorLexico::parseConjuncion()
{
    Formula* izq = parseNegacion();
    if (hayError()) return nullptr;
    while (posicion < tokens.size() && (tokens[posicion] == "y" || tokens[posicion] == "&")) {
        ++posicion;
        Formula* der = parseNegacion();
        if (hayError()) { delete izq; return nullptr; }
        if (der == nullptr) {
            errorActual = "Falta la proposición derecha de la conjunción «y».";
            delete izq;
            return nullptr;
        }
        Formula* f = new Formula();
        f->tipo = Formula::CONJUNCION;
        f->izquierda = izq;
        f->derecha = der;
        izq = f;
    }
    return izq;
}

Formula* AnalizadorLexico::parseNegacion()
{
    if (posicion < tokens.size() &&
        (tokens[posicion] == "no" || tokens[posicion] == "!" || tokens[posicion] == "~")) {
        ++posicion;
        Formula* interior = parseNegacion();
        if (hayError()) return nullptr;
        if (interior == nullptr) {
            errorActual = "La negación «no» requiere una proposición.";
            return nullptr;
        }
        Formula* f = new Formula();
        f->tipo = Formula::NEGACION;
        f->izquierda = interior;
        f->derecha = nullptr;
        return f;
    }
    return parsePrimario();
}

Formula* AnalizadorLexico::parsePrimario()
{
    if (posicion >= tokens.size()) {
        errorActual = "Se esperaba una proposición y se llegó al final.";
        return nullptr;
    }
    QString t = tokens[posicion];
    if (t == "(") {
        ++posicion;
        Formula* interior = parseBicondicional();
        if (hayError()) { delete interior; return nullptr; }
        if (posicion >= tokens.size() || tokens[posicion] != ")") {
            errorActual = "Paréntesis desbalanceados: falta «)».";
            delete interior;
            return nullptr;
        }
        ++posicion;
        return interior;
    }
    if (t == ")") {
        errorActual = "Paréntesis desbalanceados: «)» inesperado.";
        return nullptr;
    }
    if (t == "y" || t == "o" || t == "entonces" || t == "si" ||
        t == "solo" || t == "bicondicional" || t == "->" || t == "<->") {
        errorActual = QString("Se esperaba una proposición, se encontró «%1».").arg(t);
        return nullptr;
    }
    QStringList partes;
    while (posicion < tokens.size()) {
        QString tk = tokens[posicion];
        if (tk == "y" || tk == "o" || tk == "no" || tk == "si" ||
            tk == "entonces" || tk == "solo" || tk == "bicondicional" ||
            tk == "(" || tk == ")" || tk == "->" || tk == "<->" ||
            tk == "&" || tk == "|" || tk == "!" || tk == "~") {
            break;
        }
        partes << tk;
        ++posicion;
    }
    if (partes.isEmpty()) {
        errorActual = QString("Se esperaba una proposición, se encontró «%1».").arg(t);
        return nullptr;
    }
    Formula* f = new Formula();
    f->tipo = Formula::ATOMO;
    f->textoAtomo = partes.join(' ');
    return f;
}