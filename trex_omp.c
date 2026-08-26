/* Simulacion T-Rex: version secuencial y version paralela con OpenMP */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#define JUGADOR_X        50.0
#define SALTO_DURACION   30
#define SALTO_ALTURA_MAX 80.0
#define RECICLO_MOD      4200

typedef struct {
    int    N;
    int    M;
    double *pos_x;
    double *prev_pos_x;
    double *vel;
    int    *tipo;
    double *altura_obs;
    double *ancho_obs;
    long   *recycle_count;
} Mundo;

/* Inicializa el mundo: reserva memoria y genera obstaculos iniciales */
static void inicializar_mundo(Mundo *w, int N, int M, unsigned seed) {
    w->N = N;
    w->M = M;
    w->pos_x         = malloc(sizeof(double) * M);
    w->prev_pos_x    = malloc(sizeof(double) * M);
    w->vel           = malloc(sizeof(double) * M);
    w->tipo          = malloc(sizeof(int)    * M);
    w->altura_obs    = malloc(sizeof(double) * M);
    w->ancho_obs     = malloc(sizeof(double) * M);
    w->recycle_count = malloc(sizeof(long)   * M);

    for (int i = 0; i < M; i++) {
        w->pos_x[i]      = 800.0 + (double)((i * 131u + seed) % RECICLO_MOD);
        w->prev_pos_x[i] = w->pos_x[i];
        w->vel[i]        = -(2.0 + (double)((i + (int)seed) % 4) * 0.7);
        w->tipo[i]       = (i + (int)seed) % 2;
        w->altura_obs[i] = w->tipo[i] ? (30.0 + (double)(i % 30)) : 0.0;
        w->ancho_obs[i]  = 10.0 + (double)(i % 20);
        w->recycle_count[i] = 0;
    }
}

/* Libera la memoria reservada por el mundo */
static void liberar_mundo(Mundo *w) {
    free(w->pos_x); free(w->prev_pos_x); free(w->vel);
    free(w->tipo); free(w->altura_obs); free(w->ancho_obs);
    free(w->recycle_count);
}

/* Indica si el jugador debe saltar en este frame */
static inline int toca_saltar(int frame) {
    return (frame % 45) == 0;
}

/* Simulacion secuencial */
static double simular_secuencial(Mundo *w, int stop_on_collision, long *puntaje_out, int *frame_final_out, int *colision_out) {
    int N = w->N, M = w->M;
    double jugador_y = 0.0;
    int en_salto = 0, frame_salto = 0;
    long puntaje = 0;
    int colision = 0;
    int frame_final = N;
    double checksum = 0.0;

    double t0 = omp_get_wtime();

    for (int frame = 0; frame < N; frame++) {
        if (colision && stop_on_collision) { frame_final = frame; break; }

        for (int i = 0; i < M; i++) {
            double pos_old = w->pos_x[i];
            w->pos_x[i] = pos_old + w->vel[i];
            if (w->pos_x[i] < 0.0) {
                w->recycle_count[i]++;
                w->pos_x[i] = 800.0 + (double)((i * 131u + w->recycle_count[i] * 977u) % RECICLO_MOD);
            }
        }

        if (en_salto) {
            frame_salto++;
            jugador_y = SALTO_ALTURA_MAX * sin(M_PI * frame_salto / (double)SALTO_DURACION);
            if (frame_salto >= SALTO_DURACION) { jugador_y = 0.0; en_salto = 0; }
        } else if (toca_saltar(frame)) {
            en_salto = 1; frame_salto = 0;
        }

        for (int i = 0; i < M; i++) {
            double dx = fabs(JUGADOR_X - w->pos_x[i]);
            double umbral = JUGADOR_X - w->ancho_obs[i];
            if (dx < w->ancho_obs[i] && jugador_y <= w->altura_obs[i]) {
                colision = 1;
            }

            if (w->pos_x[i] < umbral && (w->pos_x[i] - w->vel[i]) >= umbral) {
                puntaje += 10;
            }
        }

        checksum = 0.0;
        for (int i = 0; i < M; i++) checksum += w->pos_x[i];

        frame_final = frame + 1;
    }

    double t1 = omp_get_wtime();
    (void)checksum;
    *puntaje_out = puntaje;
    *frame_final_out = frame_final;
    *colision_out = colision;
    return t1 - t0;
}

/* Simulacion paralela con OpenMP */
static double simular_paralela(Mundo *w, int threads, int stop_on_collision, long *puntaje_out, int *frame_final_out, int *colision_out) {
    int N = w->N, M = w->M;
    double jugador_y = 0.0;
    int en_salto = 0, frame_salto = 0;
    long puntaje = 0;
    int colision = 0;
    int frame_final = N;
    double checksum = 0.0;

    omp_set_num_threads(threads);
    omp_set_max_active_levels(2);

    double t0 = omp_get_wtime();

    for (int frame = 0; frame < N; frame++) {
        if (colision && stop_on_collision) { frame_final = frame; break; }

        #pragma omp parallel sections num_threads(3) shared(w, jugador_y, en_salto, frame_salto, checksum)
        {
            #pragma omp section
            {
                #pragma omp parallel for num_threads(threads) schedule(static)
                for (int i = 0; i < M; i++) {
                    w->pos_x[i] += w->vel[i];
                    if (w->pos_x[i] < 0.0) {
                        w->recycle_count[i]++;
                        w->pos_x[i] = 800.0 + (double)((i * 131u + w->recycle_count[i] * 977u) % RECICLO_MOD);
                    }
                }
            }

            #pragma omp section
            {
                if (en_salto) {
                    frame_salto++;
                    jugador_y = SALTO_ALTURA_MAX * sin(M_PI * frame_salto / (double)SALTO_DURACION);
                    if (frame_salto >= SALTO_DURACION) { jugador_y = 0.0; en_salto = 0; }
                } else if (toca_saltar(frame)) {
                    en_salto = 1; frame_salto = 0;
                }
            }

            #pragma omp section
            {
                double local_checksum = 0.0;
                for (int i = 0; i < M; i++) local_checksum += w->prev_pos_x[i];
                checksum = local_checksum;
            }
        }

        int col_f = 0;
        long pts_f = 0;
        #pragma omp parallel for num_threads(threads) reduction(||:col_f) reduction(+:pts_f) schedule(static)
        for (int i = 0; i < M; i++) {
            double dx = fabs(JUGADOR_X - w->pos_x[i]);
            double umbral = JUGADOR_X - w->ancho_obs[i];
            if (dx < w->ancho_obs[i] && jugador_y <= w->altura_obs[i]) {
                col_f = 1;
            }
            if (w->pos_x[i] < umbral && (w->pos_x[i] - w->vel[i]) >= umbral) {
                pts_f += 10;
            }
        }
        colision = col_f;
        puntaje += pts_f;

        memcpy(w->prev_pos_x, w->pos_x, sizeof(double) * M);

        frame_final = frame + 1;
    }

    double t1 = omp_get_wtime();
    (void)checksum;
    *puntaje_out = puntaje;
    *frame_final_out = frame_final;
    *colision_out = colision;
    return t1 - t0;
}

/* Punto de entrada: parsea argumentos y corre el modo elegido */
int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Uso:\n  %s seq <N> <M> [seed]\n  %s par <N> <M> <threads> [seed]\n", argv[0], argv[0]);
        return 1;
    }

    const char *modo = argv[1];
    int N = atoi(argv[2]);
    int M = atoi(argv[3]);

    Mundo w;
    long puntaje; int frame_final; int colision;
    double t;

    if (strcmp(modo, "seq") == 0) {
        unsigned seed = (argc > 4) ? (unsigned)atoi(argv[4]) : 12345u;
        int stop_on_collision = (argc > 5) ? atoi(argv[5]) : 1;
        inicializar_mundo(&w, N, M, seed);
        t = simular_secuencial(&w, stop_on_collision, &puntaje, &frame_final, &colision);
        printf("seq,1,%d,%d,%.6f,%ld,%d,%d\n", N, M, t, puntaje, frame_final, colision);
        liberar_mundo(&w);
    } else if (strcmp(modo, "par") == 0) {
        if (argc < 5) { fprintf(stderr, "Falta <threads>\n"); return 1; }
        int threads = atoi(argv[4]);
        unsigned seed = (argc > 5) ? (unsigned)atoi(argv[5]) : 12345u;
        int stop_on_collision = (argc > 6) ? atoi(argv[6]) : 1;
        inicializar_mundo(&w, N, M, seed);
        t = simular_paralela(&w, threads, stop_on_collision, &puntaje, &frame_final, &colision);
        printf("par,%d,%d,%d,%.6f,%ld,%d,%d\n", threads, N, M, t, puntaje, frame_final, colision);
        liberar_mundo(&w);
    } else {
        fprintf(stderr, "Modo desconocido: %s (usa 'seq' o 'par')\n", modo);
        return 1;
    }

    return 0;
}
