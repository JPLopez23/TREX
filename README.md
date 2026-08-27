# Simulación T-Rex — Secuencial vs Paralelo (OpenMP)

## Requisitos

- Compilador C con soporte OpenMP (`gcc` recomendado).
- `bash`, `sort`, `awk` para `run_benchmark.sh`.
- Python 3 con `matplotlib` para las gráficas.

---

## Uso del binario

```
trex_omp seq <N> <M> [seed] [parar_en_colision]
trex_omp par <N> <M> <hilos> [seed] [parar_en_colision]
```

- `N` — número de frames a simular (carga de trabajo). Por defecto en el script: `20000`.
- `M` — número de obstáculos. Por defecto: `4000`.
- `hilos` — número de hilos OpenMP (solo modo `par`).
- `seed` — semilla del mundo (por defecto `12345`).
- `parar_en_colision` — `1` (por defecto) detiene la simulación al primer choque; `0` la corre completa.

## Opción A — Linux / WSL

En Windows: `wsl -d Ubuntu-22.04` y luego `cd /mnt/c/Users/jplop/Downloads/Paralela/simopen/TREX`.

### 1. Compilar

```bash
gcc -O2 -fopenmp -Wall trex_omp.c -o trex_omp -lm
```

### 2. Ejecutar

```bash
./trex_omp seq 20000 4000 12345
./trex_omp par 20000 4000 8 12345
```

### 3. Benchmark completo (genera `results.csv`)

```bash
bash run_benchmark.sh
# o con parámetros:  bash run_benchmark.sh <N> <M> <SEED> <REPS>
# valores por defecto:            20000   4000   12345    3
```

### 4. Gráficas

```bash
python3 -m pip install matplotlib     # si hace falta (o: sudo apt install python3-matplotlib)
python3 plot_results.py
```

Genera `execution_time.png` (barras seq vs par) y `speedup_curve.png` (speedup real vs ideal),
además de una tabla con speedup y eficiencia por número de hilos.

---

## Opción B — Windows nativo

Windows no trae compilador de C. Instala MinGW-w64 (incluye OpenMP):

```powershell
winget install BrechtSanders.WinLibs.POSIX.UCRT.LLVM
```

Cierra y reabre PowerShell para que `gcc` entre al `PATH` (`gcc --version` para verificar).

### 1. Compilar

```powershell
gcc -O2 -fopenmp -Wall .\trex_omp.c -o trex_omp.exe -lm
```

### 2. Ejecutar

```powershell
.\trex_omp.exe seq 20000 4000 12345
.\trex_omp.exe par 20000 4000 8 12345
```

### 3. Benchmark completo

El script necesita `bash`, así que se corre con **Git Bash**:

```powershell
& 'C:\Program Files\Git\bin\bash.exe' run_benchmark.sh
```

Si falla porque no encuentra `./trex_omp`, cambia la línea 9 de `run_benchmark.sh` a:

```
BIN=./trex_omp.exe
```

### 4. Gráficas

```powershell
python -m pip install matplotlib
python plot_results.py
```

---

## Opción C — macOS

El `clang` de Xcode no trae OpenMP. La vía más simple es instalar GCC con Homebrew:

```bash
brew install gcc
```

Homebrew instala un binario versionado (`gcc-13`, `gcc-14`, …). Comprueba cuál con `ls /opt/homebrew/bin/gcc-*` (Apple Silicon) o `/usr/local/bin/gcc-*` (Intel).

### 1. Compilar

```bash
gcc-14 -O2 -fopenmp -Wall trex_omp.c -o trex_omp -lm
```

### 2. Ejecutar

```bash
./trex_omp seq 20000 4000 12345
./trex_omp par 20000 4000 8 12345
```

### 3. Benchmark completo

El script usa `#!/usr/bin/env bash`, pero invoca `gcc` (sin versión). Edita la línea 14 de
`run_benchmark.sh` y cambia `gcc` por `gcc-14`, o compila antes a mano (paso 1) para que el
script use el binario ya existente. Luego:

```bash
bash run_benchmark.sh
```

### 4. Gráficas

```bash
python3 -m pip install matplotlib
python3 plot_results.py
```

> Alternativa con clang: `brew install libomp` y compilar con
> `clang -O2 -Xpreprocessor -fopenmp -I$(brew --prefix libomp)/include -L$(brew --prefix libomp)/lib -lomp -lm trex_omp.c -o trex_omp`.

---
