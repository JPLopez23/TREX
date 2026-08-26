#!/usr/bin/env python3
"""Lee results.csv y genera las graficas de tiempo y speedup."""
import sys
import csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

path = sys.argv[1] if len(sys.argv) > 1 else "results.csv"

rows = []
with open(path, newline="") as f:
    reader = csv.DictReader(f)
    for r in reader:
        rows.append(r)

seq_row = next(r for r in rows if r["modo"] == "seq")
T_sec = float(seq_row["tiempo_seg"])
N = seq_row["N"]
M = seq_row["M"]

par_rows = sorted(
    (r for r in rows if r["modo"] == "par"),
    key=lambda r: int(r["threads"]),
)

threads = [int(r["threads"]) for r in par_rows]
tiempos = [float(r["tiempo_seg"]) for r in par_rows]
speedups = [T_sec / t for t in tiempos]
eficiencias = [s / p for s, p in zip(speedups, threads)]

# Tabla de resultados
print(f"Carga: N={N} frames, M={M} obstaculos")
print(f"T_secuencial = {T_sec:.6f} s\n")
print(f"{'hilos':>6} | {'T_paralelo (s)':>15} | {'Speedup':>8} | {'Eficiencia':>10}")
print("-" * 50)
for p, t, s, e in zip(threads, tiempos, speedups, eficiencias):
    print(f"{p:>6} | {t:>15.6f} | {s:>8.3f} | {e:>10.3f}")

# Grafica de tiempos: secuencial vs paralelo
fig, ax = plt.subplots(figsize=(7, 5))
labels = ["Secuencial"] + [f"Paralelo\n{p} hilos" for p in threads]
tiempos_all = [T_sec] + tiempos
colors = ["#888888"] + ["#2e7dd6"] * len(threads)
bars = ax.bar(labels, tiempos_all, color=colors)
ax.set_ylabel("Tiempo de ejecucion (s)")
ax.set_title(f"Tiempo de ejecucion: Secuencial vs Paralelo\n(N={N} frames, M={M} obstaculos)")
for b, t in zip(bars, tiempos_all):
    ax.text(b.get_x() + b.get_width() / 2, b.get_height(), f"{t:.3f}s",
            ha="center", va="bottom", fontsize=9)
fig.tight_layout()
fig.savefig("execution_time.png", dpi=150)
print("\nGuardado: execution_time.png")

# Grafica de speedup real vs ideal
fig2, ax2 = plt.subplots(figsize=(7, 5))
ax2.plot(threads, speedups, marker="o", color="#2e7dd6", label="Speedup real")
ax2.plot(threads, threads, linestyle="--", color="#999999", label="Speedup ideal (lineal)")
ax2.set_xlabel("Numero de hilos (p)")
ax2.set_ylabel("Speedup (S = T_sec / T_par)")
ax2.set_title(f"Curva de Speedup\n(N={N} frames, M={M} obstaculos)")
ax2.set_xticks(threads)
ax2.grid(True, alpha=0.3)
ax2.legend()
fig2.tight_layout()
fig2.savefig("speedup_curve.png", dpi=150)
print("Guardado: speedup_curve.png")
