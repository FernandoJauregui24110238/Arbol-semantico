#ifndef VENTANAPRINCIPAL_HPP
#define VENTANAPRINCIPAL_HPP

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include "MotorInferencia.hpp"

class VentanaPrincipal : public QMainWindow {
    Q_OBJECT

public:
    VentanaPrincipal(QWidget* parent = nullptr);
    ~VentanaPrincipal();

private slots:
    void alAnalizar();
    void alCambiarPestana(int indice);

private:
    QTextEdit* cuadroReglas;
    QPushButton* botonAnalizar;
    QTabWidget* pestanas;

    QTableWidget* tablaAtomos;
    QLabel* resumenAtomos;

    QTableWidget* tablaFormulas;
    QLabel* resumenFormulas;

    QGraphicsView* vistaGrafica;
    QGraphicsScene* escenaGrafica;

    QScrollArea* areaRamas;
    QWidget* contenidoRamas;
    QVBoxLayout* layoutTarjetasRama;
    QLabel* resumenRamas;

    QLabel* areaResultados;
    QLabel* metricasResultados;
    QLabel* detalleErrorResultados;
    QTableWidget* tablaModelos;

    MotorInferencia motor;

    void configurarInterfaz();
    void precargarEjemplo();
    void limpiarVistas();
    void mostrarAtomos(const ResultadoAnalisis& resultado);
    void mostrarFormulas(const ResultadoAnalisis& resultado);
    void dibujarArbolGrafico(const ResultadoAnalisis& resultado);
    void mostrarDetalleRamas(const ResultadoAnalisis& resultado);
    void limpiarTarjetasRama();
    void mostrarResultados(const ResultadoAnalisis& resultado);
    void mostrarErrorResultados(const QString& mensaje);

    void dibujarNodoRecursivo(NodoArbol* nodo, qreal x, qreal y, qreal anchoTotal);
    qreal calcularAnchoSubarbol(NodoArbol* nodo) const;
};

#endif