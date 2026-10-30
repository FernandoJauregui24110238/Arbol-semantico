#include "VentanaPrincipal.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QHeaderView>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QWheelEvent>
#include <QPainter>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QScrollBar>
#include <QScrollArea>
#include <QMenuBar>
#include <QSet>
#include <cmath>
#include <QTabBar>

class VistaArbol : public QGraphicsView {
public:
    VistaArbol(QGraphicsScene* escena, QWidget* padre = nullptr)
        : QGraphicsView(escena, padre), zoom(1.0)
    {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setRenderHint(QPainter::Antialiasing, true);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }

    void ajustarVista() {
        if (!scene() || scene()->items().isEmpty()) return;
        fitInView(scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        zoom = transform().m11();
    }

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override {
        painter->fillRect(rect, QColor("#080f0a"));
        QPen pen(QColor(33, 54, 39));
        pen.setWidthF(0.6);
        painter->setPen(pen);

        const qreal paso = 32.0;
        qreal izquierda = std::floor(rect.left() / paso) * paso;
        qreal arriba = std::floor(rect.top() / paso) * paso;
        for (qreal x = izquierda; x < rect.right(); x += paso) {
            for (qreal y = arriba; y < rect.bottom(); y += paso) {
                painter->drawPoint(QPointF(x, y));
            }
        }
    }

    void wheelEvent(QWheelEvent* evento) override {
        if (evento->angleDelta().y() > 0) zoom *= 1.15;
        else zoom /= 1.15;
        if (zoom < 0.1) zoom = 0.1;
        if (zoom > 5.0) zoom = 5.0;
        setTransform(QTransform().scale(zoom, zoom));
        evento->accept();
    }

private:
    qreal zoom;
};

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

VentanaPrincipal::VentanaPrincipal(QWidget* padre)
    : QMainWindow(padre)
{
    configurarInterfaz();
    precargarEjemplo();
}

VentanaPrincipal::~VentanaPrincipal() {}

void VentanaPrincipal::configurarInterfaz()
{
    setWindowTitle("Motor de Inferencia Semántica");
    resize(1400, 850);
    setMinimumSize(1100, 700);

    setStyleSheet(R"(
        QMainWindow {
            background: #070d09;
            color: #f4fff6;
        }
        QWidget {
            background: #070d09;
            color: #f4fff6;
        }
        QWidget#navigationPanel {
            background: #0c1710;
            border-right: 1px solid #23412d;
        }
        QLabel#navigationBrand {
            color: #f4fff6;
            font-size: 18px;
            font-weight: 800;
        }
        QLabel#navigationStatus {
            color: #69d98d;
            font-size: 11px;
            font-weight: 600;
        }
        QPushButton#navigationHeader {
            background: #111f16;
            color: #f4fff6;
            border: 1px solid #294b34;
            border-radius: 3px;
            text-align: left;
            min-height: 38px;
            padding: 4px 12px;
            font-size: 11px;
            font-weight: 800;
        }
        QPushButton#navigationHeader:hover,
        QPushButton#navigationHeader:checked {
            background: #183b25;
            border-color: #43a862;
        }
        QPushButton#navigationItem {
            background: transparent;
            color: #f0fff3;
            border: none;
            border-radius: 3px;
            text-align: left;
            min-height: 34px;
            padding: 3px 14px;
            font-weight: 500;
        }
        QPushButton#navigationItem:hover {
            background: #173522;
            color: #ffffff;
        }
        QPushButton#navigationItem[active="true"] {
            background: #1b7a3d;
            color: #ffffff;
            font-weight: 700;
        }
        QLabel#viewTitle {
            color: #f4fff6;
            font-size: 22px;
            font-weight: 700;
        }
        QLabel#viewDescription {
            color: #9cbaa4;
            font-size: 12px;
        }
        QLabel#viewSummary {
            color: #8ce2a4;
            font-size: 13px;
            font-weight: 600;
            padding: 8px 0;
        }
        QLabel#resultVerdict {
            color: #8ce2a4;
            font-size: 21px;
            font-weight: 700;
            padding: 10px 0;
        }
        QLabel#resultMetrics {
            color: #f4fff6;
            background: #102017;
            border-left: 3px solid #35a85a;
            padding: 12px 14px;
            font-weight: 600;
        }
        QLabel#resultError {
            color: #ff9a91;
            padding: 8px 0;
        }
        QGroupBox {
            border: 1px solid #294b34;
            border-radius: 14px;
            margin-top: 12px;
            padding-top: 12px;
            background: #0d1710;
            color: #f4fff6;
            font-weight: 600;
        }
        QLabel {
            color: #f0fff3;
        }
        QTextEdit, QTableWidget, QTreeWidget, QGraphicsView {
            background: #090f0b;
            border: 1px solid #294b34;
            border-radius: 12px;
            color: #f4fff6;
            selection-background-color: #176b36;
        }
        QTextEdit {
            padding: 10px;
        }
        QHeaderView::section {
            background: #14251a;
            color: #ffffff;
            border: 1px solid #294b34;
            padding: 6px;
            font-weight: 600;
        }
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #176b36, stop:1 #22a653);
            border: none;
            border-radius: 10px;
            color: white;
            font-weight: 700;
            min-height: 38px;
            padding: 8px 20px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #218847, stop:1 #35c46a);
        }
        QTabWidget::pane {
            border: 1px solid #294b34;
            border-radius: 14px;
            background: #0d1710;
        }
        QTabBar::tab {
            background: #14251a;
            color: #f0fff3;
            border: 1px solid #294b34;
            border-bottom: none;
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            padding: 10px 18px;
            margin-right: 4px;
        }
        QTabBar::tab:selected {
            background: #1b7a3d;
            color: #ffffff;
        }
        QTreeWidget::item:selected,
        QTableWidget::item:selected {
            background: #176b36;
            color: #ffffff;
        }
    )");

    menuBar()->hide();

    QWidget* central = new QWidget(this);
    QHBoxLayout* layoutPrincipal = new QHBoxLayout(central);
    layoutPrincipal->setContentsMargins(0, 0, 0, 0);
    layoutPrincipal->setSpacing(0);

    QWidget* menuLateral = new QWidget(central);
    menuLateral->setObjectName("navigationPanel");
    menuLateral->setFixedWidth(250);
    QVBoxLayout* layoutNavegacion = new QVBoxLayout(menuLateral);
    layoutNavegacion->setContentsMargins(16, 20, 16, 16);
    layoutNavegacion->setSpacing(7);

    QPushButton* encabezadoMenu = new QPushButton("Menú principal     −", menuLateral);
    encabezadoMenu->setObjectName("navigationHeader");
    encabezadoMenu->setCheckable(true);
    encabezadoMenu->setChecked(true);

    QScrollArea* areaMenu = new QScrollArea(menuLateral);
    areaMenu->setObjectName("navigationScroll");
    areaMenu->setWidgetResizable(true);
    areaMenu->setFrameShape(QFrame::NoFrame);

    QWidget* contenidoMenu = new QWidget(areaMenu);
    contenidoMenu->setObjectName("navigationItems");
    QVBoxLayout* layoutOpciones = new QVBoxLayout(contenidoMenu);
    layoutOpciones->setContentsMargins(4, 3, 0, 5);
    layoutOpciones->setSpacing(1);

    const QVector<QString> nombresOpciones = {
        "Rama",
        "Fórmula",
        "Átomo",
        "Resultados",
        "Árbol gráfico",
        "Salir"
    };

    QVector<QPushButton*> botonesMenu;
    auto agregarOpcion = [contenidoMenu, layoutOpciones, &botonesMenu](const QString& texto) {
        QPushButton* opcion = new QPushButton(texto);
        opcion->setObjectName("navigationItem");
        opcion->setCursor(Qt::PointingHandCursor);
        layoutOpciones->addWidget(opcion);
        botonesMenu.append(opcion);
        return opcion;
    };

    for (const QString& nombre : nombresOpciones) {
        agregarOpcion(nombre);
    }
    layoutOpciones->addStretch();
    areaMenu->setWidget(contenidoMenu);
    layoutNavegacion->addWidget(encabezadoMenu);
    layoutNavegacion->addWidget(areaMenu, 1);

    connect(encabezadoMenu, &QPushButton::toggled, this, [encabezadoMenu, areaMenu](bool abierto) {
        encabezadoMenu->setText(QString("Menú principal     ") + (abierto ? "−" : "+"));
        areaMenu->setVisible(abierto);
    });

    QWidget* panelIzquierdo = new QWidget(central);
    panelIzquierdo->setFixedWidth(340);
    QVBoxLayout* layoutIzquierdo = new QVBoxLayout(panelIzquierdo);
    layoutIzquierdo->setContentsMargins(20, 22, 18, 18);
    layoutIzquierdo->setSpacing(14);

    QLabel* titulo = new QLabel("Motor de inferencia");
    titulo->setStyleSheet("font-size: 24px; font-weight: 700; color: #f8fafc; margin-bottom: 2px;");
    QLabel* subtitulo = new QLabel("Árbol semántico • análisis lógico");
    subtitulo->setStyleSheet("font-size: 12px; color: #69d98d; letter-spacing: 1.2px; text-transform: uppercase;");

    QGroupBox* grupoEntrada = new QGroupBox("Base de conocimiento");
    QVBoxLayout* layoutEntrada = new QVBoxLayout(grupoEntrada);
    layoutEntrada->setContentsMargins(12, 14, 12, 12);
    layoutEntrada->setSpacing(12);

    cuadroReglas = new QTextEdit(grupoEntrada);
    cuadroReglas->setFont(QFont("Consolas", 11));
    cuadroReglas->setMinimumHeight(180);
    cuadroReglas->setPlaceholderText("Ejemplo:\nSi es de dia, entonces voy a trabajar\nEs de dia");
    layoutEntrada->addWidget(cuadroReglas);

    botonAnalizar = new QPushButton("Analizar reglas", grupoEntrada);
    botonAnalizar->setMinimumWidth(170);
    botonAnalizar->setCursor(Qt::PointingHandCursor);
    layoutEntrada->addWidget(botonAnalizar, 0, Qt::AlignRight);

    QLabel* info = new QLabel(
        "• Detecta inconsistencias\n"
        "• Expande las ramas del árbol\n"
        "• Muestra modelos abiertos");
    info->setStyleSheet("background: #0d1710; border: 1px solid #294b34; border-radius: 12px; padding: 14px; color: #f0fff3; line-height: 1.6;");
    info->setWordWrap(true);

    layoutIzquierdo->addWidget(titulo);
    layoutIzquierdo->addWidget(subtitulo);
    layoutIzquierdo->addWidget(grupoEntrada);
    layoutIzquierdo->addWidget(info);
    layoutIzquierdo->addStretch();

    QWidget* panelDerecho = new QWidget(central);
    QVBoxLayout* layoutDerecho = new QVBoxLayout(panelDerecho);
    layoutDerecho->setContentsMargins(0, 0, 0, 0);
    layoutDerecho->setSpacing(0);

    pestanas = new QTabWidget(panelDerecho);
    pestanas->setDocumentMode(true);
    pestanas->tabBar()->hide();

    auto crearEncabezadoPagina = [](QVBoxLayout* layout, const QString& tituloPagina,
                                    const QString& descripcion) {
        QLabel* tituloVista = new QLabel(tituloPagina);
        tituloVista->setObjectName("viewTitle");
        QLabel* descripcionVista = new QLabel(descripcion);
        descripcionVista->setObjectName("viewDescription");
        layout->addWidget(tituloVista);
        layout->addWidget(descripcionVista);
    };

    QWidget* paginaAtomos = new QWidget(pestanas);
    QVBoxLayout* layoutAtomos = new QVBoxLayout(paginaAtomos);
    layoutAtomos->setContentsMargins(26, 24, 26, 24);
    layoutAtomos->setSpacing(10);
    crearEncabezadoPagina(layoutAtomos, "Átomos identificados",
                          "Proposiciones simples y sus letras asignadas");
    resumenAtomos = new QLabel("0 átomos únicos");
    resumenAtomos->setObjectName("viewSummary");
    tablaAtomos = new QTableWidget(0, 2, paginaAtomos);
    tablaAtomos->setHorizontalHeaderLabels({"Letra", "Proposición simple"});
    tablaAtomos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaAtomos->verticalHeader()->hide();
    tablaAtomos->setShowGrid(false);
    tablaAtomos->setAlternatingRowColors(true);
    tablaAtomos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layoutAtomos->addWidget(resumenAtomos);
    layoutAtomos->addWidget(tablaAtomos, 1);
    pestanas->addTab(paginaAtomos, "Átomos");

    QWidget* paginaFormulas = new QWidget(pestanas);
    QVBoxLayout* layoutFormulas = new QVBoxLayout(paginaFormulas);
    layoutFormulas->setContentsMargins(26, 24, 26, 24);
    layoutFormulas->setSpacing(10);
    crearEncabezadoPagina(layoutFormulas, "Fórmulas lógicas",
                          "Cada regla en lenguaje natural junto a su representación formal");
    resumenFormulas = new QLabel("0 reglas analizadas");
    resumenFormulas->setObjectName("viewSummary");
    tablaFormulas = new QTableWidget(0, 3, paginaFormulas);
    tablaFormulas->setHorizontalHeaderLabels({"Regla", "Texto original", "Fórmula lógica"});
    tablaFormulas->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tablaFormulas->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tablaFormulas->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tablaFormulas->verticalHeader()->hide();
    tablaFormulas->setShowGrid(false);
    tablaFormulas->setAlternatingRowColors(true);
    tablaFormulas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layoutFormulas->addWidget(resumenFormulas);
    layoutFormulas->addWidget(tablaFormulas, 1);
    pestanas->addTab(paginaFormulas, "Fórmulas");

    QWidget* paginaGrafica = new QWidget(pestanas);
    QVBoxLayout* layoutGrafica = new QVBoxLayout(paginaGrafica);
    layoutGrafica->setContentsMargins(26, 24, 26, 24);
    layoutGrafica->setSpacing(10);
    crearEncabezadoPagina(layoutGrafica, "Árbol gráfico",
                          "Expansión de fórmulas, bifurcaciones y estado de cada rama");
    escenaGrafica = new QGraphicsScene(this);
    vistaGrafica = new VistaArbol(escenaGrafica, paginaGrafica);
    layoutGrafica->addWidget(vistaGrafica, 1);
    pestanas->addTab(paginaGrafica, "Árbol gráfico");

    QWidget* paginaRamas = new QWidget(pestanas);
    QVBoxLayout* layoutRamas = new QVBoxLayout(paginaRamas);
    layoutRamas->setContentsMargins(26, 24, 26, 24);
    layoutRamas->setSpacing(10);
    crearEncabezadoPagina(layoutRamas, "Desarrollo de ramas",
                          "Secuencia de fórmulas aplicadas y literales en cada rama");
    resumenRamas = new QLabel("Sin ramas analizadas");
    resumenRamas->setObjectName("viewSummary");
    areaRamas = new QScrollArea(paginaRamas);
    areaRamas->setWidgetResizable(true);
    areaRamas->setFrameShape(QFrame::NoFrame);
    contenidoRamas = new QWidget(areaRamas);
    layoutTarjetasRama = new QVBoxLayout(contenidoRamas);
    layoutTarjetasRama->setContentsMargins(2, 4, 8, 8);
    layoutTarjetasRama->setSpacing(12);
    layoutTarjetasRama->addStretch();
    areaRamas->setWidget(contenidoRamas);
    layoutRamas->addWidget(resumenRamas);
    layoutRamas->addWidget(areaRamas, 1);
    pestanas->addTab(paginaRamas, "Detalle de ramas");

    QWidget* paginaResultados = new QWidget(pestanas);
    QVBoxLayout* layoutResultados = new QVBoxLayout(paginaResultados);
    layoutResultados->setContentsMargins(26, 24, 26, 24);
    layoutResultados->setSpacing(12);
    crearEncabezadoPagina(layoutResultados, "Resultados del análisis",
                          "Consistencia de la base de conocimiento y asignaciones que la satisfacen");
    areaResultados = new QLabel("Sin análisis");
    areaResultados->setObjectName("resultVerdict");
    areaResultados->setWordWrap(true);
    metricasResultados = new QLabel("Ejecuta el análisis para ver sus resultados.");
    metricasResultados->setObjectName("resultMetrics");
    detalleErrorResultados = new QLabel;
    detalleErrorResultados->setObjectName("resultError");
    detalleErrorResultados->setWordWrap(true);
    tablaModelos = new QTableWidget(0, 0, paginaResultados);
    tablaModelos->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tablaModelos->verticalHeader()->hide();
    tablaModelos->setShowGrid(false);
    tablaModelos->setAlternatingRowColors(true);
    tablaModelos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layoutResultados->addWidget(areaResultados);
    layoutResultados->addWidget(metricasResultados);
    layoutResultados->addWidget(detalleErrorResultados);
    layoutResultados->addWidget(tablaModelos, 1);
    pestanas->addTab(paginaResultados, "Resultados");

    layoutDerecho->addWidget(pestanas);
    layoutPrincipal->addWidget(menuLateral);
    layoutPrincipal->addWidget(panelIzquierdo);
    layoutPrincipal->addWidget(panelDerecho, 1);

    setCentralWidget(central);

    connect(botonAnalizar, &QPushButton::clicked, this, &VentanaPrincipal::alAnalizar);
    connect(botonesMenu[0], &QPushButton::clicked, this, [this]() { pestanas->setCurrentIndex(3); });
    connect(botonesMenu[1], &QPushButton::clicked, this, [this]() { pestanas->setCurrentIndex(1); });
    connect(botonesMenu[2], &QPushButton::clicked, this, [this]() { pestanas->setCurrentIndex(0); });
    connect(botonesMenu[3], &QPushButton::clicked, this, [this]() { pestanas->setCurrentIndex(4); });
    connect(botonesMenu[4], &QPushButton::clicked, this, [this]() { pestanas->setCurrentIndex(2); });
    connect(botonesMenu[5], &QPushButton::clicked, this, &VentanaPrincipal::close);
    connect(pestanas, &QTabWidget::currentChanged, this, &VentanaPrincipal::alCambiarPestana);
}

void VentanaPrincipal::precargarEjemplo()
{
    cuadroReglas->setPlainText(
        "Si es de día, entonces voy a trabajar\n"
        "Si llueve, entonces no voy a trabajar\n"
        "Es de día");
}

void VentanaPrincipal::alAnalizar()
{
    limpiarVistas();

    QString entrada = cuadroReglas->toPlainText();
    if (entrada.trimmed().isEmpty()) {
        mostrarErrorResultados("No se ingresó ninguna regla.");
        pestanas->setCurrentIndex(4);
        return;
    }

    ResultadoAnalisis resultado = motor.analizar(entrada);

    if (!resultado.exitoso) {
        mostrarErrorResultados(resultado.mensajeError);
        pestanas->setCurrentIndex(4);
        return;
    }

    mostrarAtomos(resultado);
    mostrarFormulas(resultado);
    dibujarArbolGrafico(resultado);
    mostrarDetalleRamas(resultado);
    mostrarResultados(resultado);

    static_cast<VistaArbol*>(vistaGrafica)->ajustarVista();
}

void VentanaPrincipal::alCambiarPestana(int indice)
{
    if (indice == 2) {
        static_cast<VistaArbol*>(vistaGrafica)->ajustarVista();
    }
}

void VentanaPrincipal::limpiarVistas()
{
    tablaAtomos->setRowCount(0);
    resumenAtomos->setText("0 proposiciones simples únicas");
    tablaFormulas->setRowCount(0);
    resumenFormulas->setText("0 reglas analizadas");
    escenaGrafica->clear();
    limpiarTarjetasRama();
    resumenRamas->setText("Sin ramas analizadas");
    areaResultados->setText("Sin análisis");
    metricasResultados->setText("Ejecuta el análisis para ver sus resultados.");
    detalleErrorResultados->clear();
    tablaModelos->setRowCount(0);
    tablaModelos->setColumnCount(0);
}

void VentanaPrincipal::mostrarAtomos(const ResultadoAnalisis& resultado)
{
    if (!resultado.gestor) return;

    QVector<QString> letras = resultado.gestor->letras();
    tablaAtomos->setRowCount(letras.size());
    resumenAtomos->setText(QString("%1 proposiciones simples únicas").arg(letras.size()));

    for (int i = 0; i < letras.size(); ++i) {
        QString letra = letras[i];
        QString significado = resultado.gestor->significadoDe(letra);

        QTableWidgetItem* itemLetra = new QTableWidgetItem(letra);
        QTableWidgetItem* itemSignificado = new QTableWidgetItem(significado);
        itemLetra->setTextAlignment(Qt::AlignCenter);

        tablaAtomos->setItem(i, 0, itemLetra);
        tablaAtomos->setItem(i, 1, itemSignificado);
    }
}

void VentanaPrincipal::mostrarFormulas(const ResultadoAnalisis& resultado)
{
    int n = resultado.lineasOriginales.size();
    tablaFormulas->setRowCount(n);
    resumenFormulas->setText(QString("%1 reglas analizadas").arg(n));

    for (int i = 0; i < n; ++i) {
        tablaFormulas->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        tablaFormulas->setItem(i, 1, new QTableWidgetItem(resultado.lineasOriginales[i]));
        tablaFormulas->setItem(i, 2, new QTableWidgetItem(resultado.formulasComoTexto[i]));
    }
}

void VentanaPrincipal::limpiarTarjetasRama()
{
    while (layoutTarjetasRama->count() > 0) {
        QLayoutItem* item = layoutTarjetasRama->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    layoutTarjetasRama->addStretch();
}

qreal VentanaPrincipal::calcularAnchoSubarbol(NodoArbol* nodo) const
{
    if (!nodo) return 0.0;
    if (nodo->hijos.isEmpty()) return 1.0;
    qreal total = 0.0;
    for (NodoArbol* h : nodo->hijos) {
        total += calcularAnchoSubarbol(h);
    }
    return qMax(1.0, total);
}

void VentanaPrincipal::dibujarNodoRecursivo(NodoArbol* nodo, qreal x, qreal y,
                                            qreal anchoTotal)
{
    if (!nodo) return;

    const qreal ANCHO_NODO = 300.0;
    const qreal PASO_HORIZONTAL = 390.0;
    const qreal ALTO_LINEA = 19.0;

    QStringList lineas;
    for (int i = 0; i < nodo->formulas.size(); ++i) {
        QString texto = formulaATexto(nodo->formulas[i]);
        texto = QString(nodo->aplicada[i] ? "✓  " : "·  ") + texto;
        lineas << texto;
    }

    if (lineas.isEmpty()) lineas << "Sin fórmulas en este nivel";

    QColor colorFondo(13, 24, 16);
    QColor colorCabecera(27, 52, 34);
    QColor colorBorde(55, 96, 63);
    QColor colorTexto(237, 250, 239);
    QString estado = "EN DESARROLLO";

    if (nodo->estado == NodoArbol::CERRADA) {
        colorFondo = QColor(42, 29, 16);
        colorCabecera = QColor(132, 75, 22);
        colorBorde = QColor(238, 151, 54);
        colorTexto = QColor(255, 238, 214);
        estado = "CERRADA";
    } else if (nodo->estado == NodoArbol::ABIERTA) {
        colorFondo = QColor(15, 29, 47);
        colorCabecera = QColor(27, 76, 130);
        colorBorde = QColor(81, 163, 245);
        colorTexto = QColor(226, 240, 255);
        estado = "ABIERTA";
    }

    QString textoNodo = lineas.join("\n");
    if (nodo->estado == NodoArbol::CERRADA && !nodo->contradiccion.isEmpty()) {
        textoNodo += "\n" + nodo->contradiccion;
    }

    qreal alturaCuerpo = (textoNodo.count('\n') + 1) * ALTO_LINEA + 18.0;
    qreal alturaNodo = 38.0 + alturaCuerpo;
    qreal centroX = x + (anchoTotal * PASO_HORIZONTAL) / 2.0;
    qreal izquierda = centroX - ANCHO_NODO / 2.0;

    QPainterPath rutaNodo;
    rutaNodo.addRoundedRect(QRectF(izquierda, y, ANCHO_NODO, alturaNodo), 8.0, 8.0);
    QGraphicsPathItem* rect = escenaGrafica->addPath(
        rutaNodo, QPen(colorBorde, 1.5), QBrush(colorFondo));

    rect->setZValue(1.0);

    QPainterPath rutaCabecera;
    rutaCabecera.addRoundedRect(
        QRectF(izquierda + 1.0, y + 1.0, ANCHO_NODO - 2.0, 34.0), 7.0, 7.0);
    QGraphicsPathItem* cabecera = escenaGrafica->addPath(
        rutaCabecera, Qt::NoPen, QBrush(colorCabecera));
    cabecera->setZValue(2.0);

    QGraphicsTextItem* tituloNodo = escenaGrafica->addText(
        QString("NODO  %1").arg(nodo->profundidad + 1));
    tituloNodo->setFont(QFont("Segoe UI", 9, QFont::Bold));
    tituloNodo->setDefaultTextColor(Qt::white);
    tituloNodo->setPos(izquierda + 12.0, y + 8.0);
    tituloNodo->setZValue(3.0);

    QGraphicsTextItem* etiquetaEstado = escenaGrafica->addText(estado);
    etiquetaEstado->setFont(QFont("Segoe UI", 8, QFont::DemiBold));
    etiquetaEstado->setDefaultTextColor(Qt::white);
    etiquetaEstado->setTextWidth(130.0);
    etiquetaEstado->setPos(izquierda + ANCHO_NODO - 142.0, y + 8.0);
    etiquetaEstado->setZValue(3.0);

    QGraphicsTextItem* texto = escenaGrafica->addText(textoNodo);
    texto->setFont(QFont("Consolas", 9));
    texto->setDefaultTextColor(colorTexto);
    texto->setTextWidth(ANCHO_NODO - 24.0);
    texto->setPos(izquierda + 12.0, y + 43.0);
    texto->setZValue(2.0);

    qreal xHijo = x;
    qreal nuevoNivelY = y + alturaNodo + 62.0;
    for (NodoArbol* hijo : nodo->hijos) {
        qreal anchoHijo = calcularAnchoSubarbol(hijo);
        qreal centroHijoX = xHijo + (anchoHijo * PASO_HORIZONTAL) / 2.0;

        QGraphicsLineItem* linea = escenaGrafica->addLine(
            centroX, y + alturaNodo,
            centroHijoX, nuevoNivelY,
            QPen(QColor(66, 117, 78), 2.0));
        linea->setZValue(0.0);

        dibujarNodoRecursivo(hijo, xHijo, nuevoNivelY, anchoHijo);
        xHijo += anchoHijo * PASO_HORIZONTAL;
    }
}

void VentanaPrincipal::dibujarArbolGrafico(const ResultadoAnalisis& resultado)
{
    if (!resultado.arbol) return;

    NodoArbol* raiz = resultado.arbol->getRaiz();
    if (!raiz) return;

    qreal ancho = calcularAnchoSubarbol(raiz);
    dibujarNodoRecursivo(raiz, 0.0, 0.0, ancho);
    escenaGrafica->setSceneRect(escenaGrafica->itemsBoundingRect().adjusted(-50, -35, 50, 50));
}

static void recolectarHojas(NodoArbol* nodo, QVector<NodoArbol*>& hojas)
{
    if (!nodo) return;
    if (nodo->hijos.isEmpty()) {
        hojas.append(nodo);
        return;
    }
    for (NodoArbol* hijo : nodo->hijos) {
        recolectarHojas(hijo, hojas);
    }
}

void VentanaPrincipal::mostrarDetalleRamas(const ResultadoAnalisis& resultado)
{
    if (!resultado.arbol) return;
    limpiarTarjetasRama();
    resumenRamas->setText(QString("%1 ramas abiertas     %2 ramas cerradas")
                              .arg(resultado.ramasAbiertas)
                              .arg(resultado.ramasCerradas));

    QVector<NodoArbol*> hojas;
    recolectarHojas(resultado.arbol->getRaiz(), hojas);
    if (hojas.isEmpty()) {
        QLabel* vacio = new QLabel("No se generaron ramas para esta base de conocimiento.");
        vacio->setObjectName("viewDescription");
        layoutTarjetasRama->insertWidget(0, vacio);
        return;
    }

    QVector<QString> letras = resultado.gestor->letras();
    for (int i = 0; i < hojas.size(); ++i) {
        NodoArbol* hoja = hojas[i];
        const bool cerrada = hoja->estado == NodoArbol::CERRADA;
        const QString colorEstado = cerrada ? "#f2a13d" : "#65adff";
        const QString fondo = cerrada ? "#292013" : "#101d2d";
        const QString borde = cerrada ? "#754819" : "#294c73";

        QFrame* tarjeta = new QFrame(contenidoRamas);
        tarjeta->setObjectName("branchCard");
        tarjeta->setStyleSheet(QString(
            "QFrame#branchCard { background: %1; border: 1px solid %2; border-left: 4px solid %3; border-radius: 6px; }")
            .arg(fondo, borde, colorEstado));
        QVBoxLayout* layoutTarjeta = new QVBoxLayout(tarjeta);
        layoutTarjeta->setContentsMargins(16, 14, 16, 14);
        layoutTarjeta->setSpacing(9);

        QHBoxLayout* filaTitulo = new QHBoxLayout;
        QLabel* tituloRama = new QLabel(QString("Rama %1").arg(i + 1), tarjeta);
        tituloRama->setStyleSheet("font-size: 16px; font-weight: 700; color: #f4fff6; border: none;");
        QLabel* etiquetaEstado = new QLabel(cerrada ? "CERRADA" : "ABIERTA", tarjeta);
        etiquetaEstado->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        etiquetaEstado->setStyleSheet(QString("font-size: 11px; font-weight: 800; color: %1; border: none;").arg(colorEstado));
        filaTitulo->addWidget(tituloRama);
        filaTitulo->addStretch();
        filaTitulo->addWidget(etiquetaEstado);
        layoutTarjeta->addLayout(filaTitulo);

        QLabel* resumenCamino = new QLabel(
            QString("%1 pasos de expansión").arg(hoja->profundidad), tarjeta);
        resumenCamino->setStyleSheet("font-size: 11px; color: #a4b8a8; border: none;");
        layoutTarjeta->addWidget(resumenCamino);

        QSet<QString> positivos;
        QSet<QString> negativos;
        for (const QString& literal : hoja->literalesAcumulados()) {
            if (literal.startsWith("!")) negativos.insert(literal.mid(1));
            else positivos.insert(literal);
        }

        QStringList asignaciones;
        for (const QString& letra : letras) {
            QString atom = resultado.gestor->significadoDe(letra);
            if (positivos.contains(atom)) asignaciones << QString("%1 = V · %2").arg(letra, atom);
            else if (negativos.contains(atom)) asignaciones << QString("%1 = F · %2").arg(letra, atom);
            else asignaciones << QString("%1 = libre · %2").arg(letra, atom);
        }

        QLabel* detalleAsignaciones = new QLabel(
            asignaciones.isEmpty() ? "Sin literales asignados" : asignaciones.join("     |     "), tarjeta);
        detalleAsignaciones->setWordWrap(true);
        detalleAsignaciones->setStyleSheet("font-size: 12px; color: #e8f4ea; background: #080f0a; border: 1px solid #284330; border-radius: 4px; padding: 10px;");
        layoutTarjeta->addWidget(detalleAsignaciones);

        if (cerrada && !hoja->contradiccion.isEmpty()) {
            QLabel* detalleCierre = new QLabel(hoja->contradiccion, tarjeta);
            detalleCierre->setWordWrap(true);
            detalleCierre->setStyleSheet("font-size: 11px; color: #ffaaa3; border: none;");
            layoutTarjeta->addWidget(detalleCierre);
        }

        QSet<QString> formulasAplicadas;
        for (NodoArbol* actual = hoja; actual; actual = actual->padre) {
            for (int j = 0; j < actual->formulas.size(); ++j) {
                if (actual->aplicada[j]) formulasAplicadas.insert(formulaATexto(actual->formulas[j]));
            }
        }
        QStringList formulasOrdenadas = formulasAplicadas.values();
        formulasOrdenadas.sort();
        QLabel* detalleExpansion = new QLabel(
            formulasOrdenadas.isEmpty()
                ? "No se requirieron expansiones"
                : "Desarrolladas: " + formulasOrdenadas.join("   ·   "), tarjeta);
        detalleExpansion->setWordWrap(true);
        detalleExpansion->setStyleSheet("font-size: 11px; color: #9cbaa4; border: none;");
        layoutTarjeta->addWidget(detalleExpansion);

        layoutTarjetasRama->insertWidget(layoutTarjetasRama->count() - 1, tarjeta);
    }
}

void VentanaPrincipal::mostrarResultados(const ResultadoAnalisis& resultado)
{
    const bool consistente = resultado.ramasAbiertas > 0;
    areaResultados->setText(consistente
                                ? "Base de conocimiento consistente"
                                : "Base de conocimiento inconsistente");
    areaResultados->setStyleSheet(consistente
        ? "color: #8ce2a4; font-size: 21px; font-weight: 700; padding: 10px 0;"
        : "color: #ff9a91; font-size: 21px; font-weight: 700; padding: 10px 0;");
    metricasResultados->setText(
        QString("%1 reglas     %2 átomos     %3 ramas abiertas     %4 ramas cerradas")
            .arg(resultado.lineasOriginales.size())
            .arg(resultado.gestor->letras().size())
            .arg(resultado.ramasAbiertas)
            .arg(resultado.ramasCerradas));
    detalleErrorResultados->clear();

    QVector<QString> letras = resultado.gestor->letras();
    QStringList encabezados{"Modelo"};
    for (const QString& letra : letras) {
        encabezados << QString("%1 · %2").arg(letra, resultado.gestor->significadoDe(letra));
    }
    tablaModelos->setColumnCount(encabezados.size());
    tablaModelos->setHorizontalHeaderLabels(encabezados);
    tablaModelos->setRowCount(resultado.modelos.size());

    for (int i = 0; i < resultado.modelos.size(); ++i) {
        tablaModelos->setItem(i, 0, new QTableWidgetItem(QString("M%1").arg(i + 1)));
        const QMap<QString, bool>& modelo = resultado.modelos[i];
        for (int j = 0; j < letras.size(); ++j) {
            QString valor = modelo.contains(letras[j])
                ? (modelo.value(letras[j]) ? "V" : "F")
                : "Libre";
            QTableWidgetItem* item = new QTableWidgetItem(valor);
            item->setTextAlignment(Qt::AlignCenter);
            tablaModelos->setItem(i, j + 1, item);
        }
    }
}

void VentanaPrincipal::mostrarErrorResultados(const QString& mensaje)
{
    areaResultados->setText("No se pudo completar el análisis");
    areaResultados->setStyleSheet("color: #ff9a91; font-size: 21px; font-weight: 700; padding: 10px 0;");
    metricasResultados->setText("Revisa la entrada y vuelve a ejecutar el análisis.");
    detalleErrorResultados->setText(mensaje);
    tablaModelos->setRowCount(0);
    tablaModelos->setColumnCount(0);
}