# Motor de Inferencia en Español 

Aplicación de escritorio en **C++17** con **Qt 6 Widgets** que convierte reglas escritas en español a un árbol semántico (tableau semántico) y determina si una base de conocimiento es consistente, mostrando los modelos que la satisfacen.

## ¿Qué hace?

1. **Lee reglas en español** (una por línea) y las transforma en fórmulas bien formadas de lógica proposicional.
2. **Registra los átomos** (proposiciones simples) asignándoles letras en orden: `p`, `q`, `r`, `s`, `t`, …
3. **Construye un árbol semántico** aplicando reglas de expansión α (no ramifican) y β (ramifican).
4. **Cierra ramas** que contienen una contradicción (un átomo y su negación).
5. **Muestra** el árbol gráfico, el detalle de cada nodo y las asignaciones de verdad (modelos) de las ramas abiertas.

## Requisitos

- Windows 10/11
- [MSYS2](https://www.msys2.org/) con el entorno **UCRT64**
- Paquetes necesarios (instalar en la terminal MSYS2 UCRT64):
  ```bash
  pacman -S mingw-w64-ucrt-x86_64-gcc \
            mingw-w64-ucrt-x86_64-qt6-base \
            mingw-w64-ucrt-x86_64-qt6-tools \
            make

# C:\Users\ferna\OneDrive\Desktop\Arbolsemantico\bin\Arbolsemantico.exe