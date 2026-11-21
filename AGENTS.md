# AGENTS.md — Papa's Louie Recomp

Guía del proyecto para personas y agentes. **Este archivo tiene prioridad sobre los valores por defecto de
cualquier skill** (`orchestrator`, `matcher`, `decomp-code-rules`, `native-platform-porting`, …).

## Qué es

Port **nativo** (C++20 + SDL3) de un juego Flash de Papa Louie a **Windows** y **Android**, reconstruido a partir
de la copia privada del juego del propietario. No es un emulador de Flash.

## Los juegos

Copias del propietario en `C:\Users\alfre\Proyectos\datos-papas\` (fuera del repo):

| juego | archivo | AS | escenario | FPS | código del juego (deofuscado) | sha256 (inicio) |
|---|---|---|---|---|---|---|
| Papa Louie 2: When Burgers Attack! | `PapaLouie2.swf` | AS3 | 700×416 | 30 | `papaGame.*`: 229 clases, ~42,7K líneas | `8b3ac80fe6dbbaac` |
| Papa Louie 3: When Sundaes Attack! | `PapaLouie3.swf` | AS3 | 700×416 | 30 | `papaGame.*`: 330 clases, ~57,7K líneas | `3a349ecf7b874b89` |
| Papa Louie: When Pizzas Attack! | `papalouie.swf` | AS2 | 550×400 | 24 | ~10,8K líneas en 660 scripts de línea de tiempo | `666e2e935ccecbc7` |

**Orden del port: PL2 → PL3 → PL1.** PL2 y PL3 comparten el motor `papaGame` (179 clases comunes; el 74 % de
las líneas de PL3 en esas clases es idéntico a PL2), así que el motor C++ se diseña para ambos con las
diferencias por juego aisladas. PL1 es otro motor (AS2, programado sobre la línea de tiempo) y va al final.

**Se descarta del port:** sistema de anuncios y API de Flipline (`adsystem_*`, `failsafe_*`, `flipline.api`,
`FliplineAPI_and_Branding_fla`), Mochi (`mochi.*`) y Playtomic (`Playtomic.*`): son servicios en línea
desaparecidos. Las clases `*_fla.*` y `customer*MC` son scripts de línea de tiempo (animación): se convierten
en datos de animación, no en código C++.

### Descompilación y ofuscación

PL2 y PL3 están ofuscados (control de flujo con predicados opacos + renombrado parcial de identificadores).
El código de juego conserva casi todos sus nombres reales. **Usar siempre el deofuscador de FFDec:**

```bash
J="/c/Program Files/Microsoft/jdk-21.0.13.21-hotspot/bin/java"
FF="C:/Users/alfre/Proyectos/tools/ffdec/app/ffdec-cli.jar"   # JPEXS FFDec 26.4.0, local, fuera del repo
D="C:/Users/alfre/Proyectos/datos-papas"
"$J" -Xmx4g -jar "$FF" -config autoDeobfuscate=1,simplifyExpressions=1 -export script "$D/extracted/pl2-deob" "$D/PapaLouie2.swf"
```

- Salida en `datos-papas/extracted/<juego>-deob/` (fuera del repo; **nunca copiar código descompilado al repo**).
- Quedan ~120 identificadores ofuscados por juego (`§_-94§`). Se nombran en C++ como `obf_94` hasta tener
  evidencia de su nombre (uso, cadenas, el mismo miembro sin ofuscar en el otro juego). Nunca inventar un nombre.
- Restos de `§§push/§§pop/§§goto` (4 archivos en PL2, 2 en PL3): traducir desde el P-code
  (`-format script:pcode`), no adivinar.

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

# Convertir un juego al formato propio (docs/asset-format.md). Salida SIEMPRE fuera del repo.
python -I tools/swf2assets.py --swf "C:/Users/alfre/Proyectos/datos-papas/PapaLouie2.swf" \
    --out "C:/Users/alfre/Proyectos/datos-papas/build/pl2" --scales 1,2

# Windows (VS Build Tools 2026 + Ninja/CMake del Android SDK)
tools\build_windows.bat Debug
build\windows\papalouie.exe --data "C:/Users/alfre/Proyectos/datos-papas/build"
#   --capture <png>: guarda el fotograma a los 3 s y sale (comprobaciones automáticas)
#   --input "R1-60,U61-90" (teclas LRUDJA por fotograma lógico), --trace (estado por fotograma), --frames N,
#   --level N, --room N, --at x,y (jugador en esa casilla): pruebas por texto, más baratas que las capturas
#   --mute: sin salida de sonido (con --trace, la línea S lista los sonidos de cada fotograma)
#   --customer N: jugar con el cliente N (orden de CustomerData; 0 Marty, 9 Kahuna, 26 Papa Louie)
#   Sin esas opciones de nivel la app empieza en el selector de juegos (propio del port; PL2 está en la primera
#   fila, en el escenario ~350,150); --game pl2 salta el selector y va a la pantalla de título. --lang es: idioma
#   de la interfaz; --tap "x,y@fotograma;...": toques simulados (coordenadas del escenario) para probar menús por
#   texto, también en el selector;
#   --capture-frame N: la captura en el fotograma N en vez de a los 3 s. Partidas guardadas de Windows:
#   %APPDATA%\SkibidiBoiler\PapaLouieRecomp\saves\PapaLouie2_<ranura>.json
# Progreso de la traducción (funciones y líneas del AS con su `// AS:` en src/game; solo local, sin tokens):
python -I tools/progress.py                 # --by-class: por clase; --missing <Clase>: funciones que faltan
# Leer una función del código original deofuscado (solo local):
python -I tools/as_func.py "C:/Users/alfre/Proyectos/datos-papas/extracted/pl2-deob/scripts" GameControls keyCheckPlayer

# Android (desde platform/android; JAVA_HOME = JDK 21)
./gradlew assembleDebug
adb -s a7e4babe install -r app/build/outputs/apk/debug/app-debug.apk
# En Git Bash: MSYS_NO_PATHCONV=1, o convierte /sdcard en una ruta de Windows.
export MSYS_NO_PATHCONV=1
R=/sdcard/Android/data/io.github.skibidiboiler.papalouie/files/data/pl2
adb -s a7e4babe shell mkdir -p $R/shapes
adb -s a7e4babe push .../build/pl2/symbols.bin .../build/pl2/manifest.json $R/
adb -s a7e4babe push .../build/pl2/shapes/2x $R/shapes/
```

Visor de símbolos (modo de desarrollo actual): toca la mitad derecha/izquierda de la pantalla (o ←/→) para
pasar de símbolo; el nombre sale en `adb logcat -s SDL/APP`.

Dispositivo de pruebas: POCO X5 Pro 5G (Snapdragon 778G, Adreno 642L, arm64-v8a), serie `a7e4babe`.
