# AGENTS.md — Papa's Louie Recomp

Guía del proyecto para personas y agentes. **Este archivo tiene prioridad sobre los valores por defecto de
cualquier skill** (`orchestrator`, `matcher`, `decomp-code-rules`, `native-platform-porting`, …).

## Qué es

Port **nativo** (C++20 + SDL3) de un juego Flash de Papa Louie a **Windows** y **Android**, reconstruido a partir
de la copia privada del juego del propietario. No es un emulador de Flash.

## Reglas duras

1. **Los archivos del juego nunca entran en el repo ni en un APK/ejecutable.** El `.swf` original vive en
   `C:\Users\alfre\Proyectos\datos-papas\` (fuera del repo). Los recursos extraídos se generan en carpetas
   ignoradas (`build/`, `data/`) y se cargan en tiempo de ejecución. En Android se copian con `adb push`.
2. **Git:** commits solo en la rama de trabajo. **Nada de merge a `main`, push ni pull request sin permiso
   explícito del propietario.** Esto anula el procedimiento de merge de `orchestrator`.
3. **Nada se sube a servicios externos** sin permiso explícito.
4. **Sin emulación encubierta** (`native-platform-porting`): prohibido escribir un intérprete de bytecode
   AVM1/AVM2, un reproductor genérico de SWF o un "display list" de Flash interpretado en runtime. La lógica
   ActionScript se traduce a C++ a mano; los recursos se convierten a formatos propios con `tools/`.
5. **Las skills de `.claude/skills/` son de terceros sin licencia:** están ignoradas por git y no se commitean.

## Perfil de código (adaptación de `decomp-code-rules`)

No hay compilador original contra el que comparar bytes, así que la ley "the bytes win" se sustituye por
**paridad de comportamiento** con el juego original.

| tema | regla del proyecto |
|---|---|
| Lenguaje | C++20, sin excepciones en el bucle de juego, sin RTTI obligatoria. |
| Nombres (R7) | Evidencia primero: los nombres de clases, métodos y campos que conserva el SWF (ABC/SymbolClass, o identificadores AS2) se mantienen **tal cual**. Nunca inventar un nombre con aspecto seguro; si no hay evidencia, `unk_<offset>` o `sub_<id>` y se reporta como pendiente. |
| Archivos | Un archivo `.cpp/.h` por clase original, en `src/game/` reflejando su paquete (`com.flipline.Foo` → `src/game/com/flipline/Foo.cpp`). |
| Trazabilidad | Cada función traducida lleva encima `// AS: <Clase>.<método>` con el nombre original. |
| Tipos | `Number` → `double`, `int` → `int32_t`, `uint` → `uint32_t`, `Boolean` → `bool`. No pasar a `float` la lógica de juego (física, colisiones, temporizadores): solo en el límite del render. |
| Tiempo | Paso fijo a los FPS declarados en el SWF; el render puede interpolar. Nada de lógica dependiente del refresco de pantalla. |
| Bugs originales (R36) | Se conservan y se marcan con `// QUIRK: <qué hace el original>`. Arreglos opcionales detrás de una opción, nunca por defecto. |
| Código falso (R17) | Prohibido: temporales ficticios, bloques vacíos, `volatile` sin motivo, código "de prueba" olvidado. El código de andamiaje del esqueleto se marca `// SKELETON:` y se elimina al portar la escena real. |
| Formato | Cambios de formato en commits separados de los cambios de lógica. |

## Objetivos del port (mejoras sobre el original)

Todas son opcionales y configurables; con todo desactivado el juego debe verse y comportarse como el original.

| objetivo | cómo encaja en la arquitectura |
|---|---|
| **Altas tasas de fotogramas** (60/90/120 Hz+) | La lógica sigue a paso fijo a los FPS del SWF; el render **interpola** entre el estado anterior y el actual. Toda entidad traducida guarda posición/rotación/escala/frame anteriores (`prev*`) para poder interpolar. Nunca acelerar la lógica para subir FPS. |
| **Escala de resolución** | Render a una resolución interna = tamaño del escenario × escala (0.5×–4×, o nativa de la pantalla). Los gráficos vectoriales se rasterizan a la escala elegida (atlas generados por `tools/` a varias escalas, o teselado de vectores), no se escalan bitmaps de 1×. |
| **Widescreen** | Cámara con área visible ampliable más allá del escenario 4:3/original cuando la escena tiene arte fuera de cuadro; si no, pillarbox. Ajuste por escena. La interfaz se ancla a los bordes (izquierda/derecha/centro) en lugar de a coordenadas fijas del escenario. |
| **Traducciones** | Ningún texto visible en el código: todo pasa por una tabla de cadenas (`data/lang/<idioma>.json`, clave → texto). Los textos del SWF (incluido el texto estático, que Flash guarda como glifos) se extraen a la tabla base; las fuentes deben cubrir los idiomas añadidos (acentos, ñ, etc.). |
| **Controles táctiles editables (Android)** | Capa de entrada por **acciones** (izquierda, saltar, atacar…), no por teclas. Controles en pantalla con editor: posición, tamaño, opacidad/transparencia, visibilidad por botón; perfiles guardados en el almacenamiento de la app. También mando físico y teclado. |
| **Ajustes** | Menú de opciones propio del port (FPS, escala, widescreen, idioma, controles, vsync), guardado en un archivo de configuración. |

## Estructura

```
src/main.cpp            punto de entrada (callbacks de SDL3)
src/engine/             capa de plataforma: ventana, render, input, audio, rutas, reloj
src/game/               traducción del ActionScript (un archivo por clase original)
tools/                  scripts locales: inspección del SWF, extracción de recursos
platform/android/       proyecto Gradle (SDLActivity, arm64-v8a)
external/SDL            SDL3 (submódulo git)
```

## Comandos

Requisitos: CMake ≥ 3.24, Ninja, compilador C++20 (MSVC o clang) para Windows; Android SDK + NDK para Android.

```bash
# Inspeccionar la copia del juego (solo lectura, nada sale del equipo)
python -I tools/swf_info.py "C:/Users/alfre/Proyectos/datos-papas/<juego>.swf"

# Windows
cmake -S . -B build/windows -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/windows
build/windows/papalouie.exe --data <carpeta-de-recursos>

# Android (desde platform/android)
./gradlew assembleDebug
adb -s a7e4babe install -r app/build/outputs/apk/debug/app-debug.apk
adb -s a7e4babe push <carpeta-de-recursos>/. /sdcard/Android/data/io.github.skibidiboiler.papalouie/files/data/
```

Dispositivo de pruebas: POCO X5 Pro 5G (Snapdragon 778G, Adreno 642L, arm64-v8a), serie `a7e4babe`.
