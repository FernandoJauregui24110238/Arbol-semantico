#include <QApplication>
#include "VentanaPrincipal.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Motor de Inferencia en Español");
    app.setOrganizationName("TareaIA");

    VentanaPrincipal ventana;
    ventana.show();

    return app.exec();
}