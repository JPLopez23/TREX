#!/usr/bin/env bash
# Corre la simulacion T-Rex en modo secuencial y paralelo (1,2,4,8 hilos) y guarda results.csv
set -e

N=${1:-20000}
M=${2:-4000}
SEED=${3:-12345}
REPS=${4:-3}
BIN=./trex_omp
OUT=results.csv

if [ ! -x "$BIN" ]; then
    echo "Compilando $BIN ..."
    gcc -O2 -fopenmp -Wall trex_omp.c -o trex_omp -lm
fi

echo "Parametros: N=$N frames, M=$M obstaculos, seed=$SEED, repeticiones=$REPS"
echo "modo,threads,N,M,tiempo_seg,puntaje,frame_final,colision" > "$OUT"

# Calcula la mediana de una lista de numeros (uno por linea) por stdin
mediana() {
    sort -n | awk '{a[NR]=$1} END{if(NR%2==1) print a[(NR+1)/2]; else print (a[NR/2]+a[NR/2+1])/2}'
}

echo "-> Corriendo version SECUENCIAL ($REPS repeticiones)..."
tseq_list=""
for r in $(seq 1 "$REPS"); do
    line=$($BIN seq "$N" "$M" "$SEED" 0)
    t=$(echo "$line" | cut -d, -f5)
    tseq_list="$tseq_list$t"$'\n'
    echo "   rep $r: $t s"
done
T_SEQ=$(echo "$tseq_list" | grep -v '^$' | mediana)
echo "   mediana T_sec = $T_SEQ s"
puntaje=$(echo "$line" | cut -d, -f6)
frame_final=$(echo "$line" | cut -d, -f7)
colision=$(echo "$line" | cut -d, -f8)
echo "seq,1,$N,$M,$T_SEQ,$puntaje,$frame_final,$colision" >> "$OUT"

for T in 1 2 4 8; do
    echo "-> Corriendo version PARALELA con $T hilo(s) ($REPS repeticiones)..."
    tpar_list=""
    for r in $(seq 1 "$REPS"); do
        line=$($BIN par "$N" "$M" "$T" "$SEED" 0)
        t=$(echo "$line" | cut -d, -f5)
        tpar_list="$tpar_list$t"$'\n'
        echo "   rep $r: $t s"
    done
    T_PAR=$(echo "$tpar_list" | grep -v '^$' | mediana)
    echo "   mediana T_par($T) = $T_PAR s"
    puntaje=$(echo "$line" | cut -d, -f6)
    frame_final=$(echo "$line" | cut -d, -f7)
    colision=$(echo "$line" | cut -d, -f8)
    echo "par,$T,$N,$M,$T_PAR,$puntaje,$frame_final,$colision" >> "$OUT"
done

echo ""
echo "Resultados guardados en $OUT"
echo "Ahora corre: python3 plot_results.py"
