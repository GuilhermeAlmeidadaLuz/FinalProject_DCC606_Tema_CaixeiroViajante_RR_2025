#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define MAX_NODES 500
#define MAX_EDGES 5000
#define TEMPO_PESO 1.0 
#define PASSAGEIROS_PESO 1.0
#define TEMP_INICIAL 1000000.0
#define RESFRIAMENTO 0.999
#define QTD_INTERMEDIARIOS 6
#define MAX_ITER_POR_TEMPERATURA 100

typedef struct {
    int origem;
    int destino;
    double tempo;
    double passageiros;
} Aresta;

Aresta arestas[MAX_EDGES];
int qtd_arestas = 0;
int vertices[MAX_NODES];
int qtd_vertices = 0;

int indice_por_id(int id) {
    for (int i = 0; i < qtd_vertices; i++)
        if (vertices[i] == id) return i;
    return -1;
}

void ler_arestas(const char *arquivo) {
    FILE *f = fopen(arquivo, "r");
    if (!f) { printf("Erro ao abrir arquivo.\n"); exit(1); }
    char linha[256];
    fgets(linha, sizeof(linha), f);
    while (fgets(linha, sizeof(linha), f)) {
        Aresta a;
        sscanf(linha, "%d,%d,%lf,%lf", &a.origem, &a.destino, &a.tempo, &a.passageiros);
        arestas[qtd_arestas++] = a;
        if (indice_por_id(a.origem) == -1) vertices[qtd_vertices++] = a.origem;
        if (indice_por_id(a.destino) == -1) vertices[qtd_vertices++] = a.destino;
    }
    fclose(f);
}

double calc_score(int rota[], int tamanho) {
    double tempo_total = 0.0;
    double passageiros_total = 0.0;
    for (int i = 0; i < tamanho - 1; i++) {
        int o = rota[i], d = rota[i + 1];
        double t = 1e9, p = 0.0;
        for (int j = 0; j < qtd_arestas; j++) {
            if (arestas[j].origem == o && arestas[j].destino == d) {
                t = arestas[j].tempo;
                p = arestas[j].passageiros;
                break;
            }
        }
        if (t == 1e9) return 1e9; 
        tempo_total += t;
        passageiros_total += p;
    }
    return (TEMPO_PESO * tempo_total) - (PASSAGEIROS_PESO * passageiros_total);
}

void imprimir_detalhes_rota(int rota[], int tamanho, const char* nome) {
    double tempo_total = 0.0;
    double passageiros_total = 0.0;
    for (int i = 0; i < tamanho - 1; i++) {
        int o = rota[i], d = rota[i + 1];
        double t = 1e9, p = 0.0;
        for (int j = 0; j < qtd_arestas; j++) {
            if (arestas[j].origem == o && arestas[j].destino == d) {
                t = arestas[j].tempo;
                p = arestas[j].passageiros;
                break;
            }
        }
        if (t == 1e9) {
            printf("Aresta inválida entre %d e %d\n", o, d);
            return;
        }
        tempo_total += t;
        passageiros_total += p;
    }
    printf("%s:\n", nome);
    printf("  Tempo total: %.2f minutos\n", tempo_total);
    printf("  Passageiros totais: %.2f\n\n", passageiros_total);
}


void gerar_rota_inicial(int origem, int destino, int rota[], int *tamanho_rota) {
    int intermediarios[MAX_NODES];
    int qtd_intermed = 0;
    for (int i = 0; i < qtd_vertices; i++) {
        if (vertices[i] != origem && vertices[i] != destino)
            intermediarios[qtd_intermed++] = vertices[i];
    }
    for (int i = 0; i < qtd_intermed; i++) {
        int j = rand() % qtd_intermed;
        int tmp = intermediarios[i];
        intermediarios[i] = intermediarios[j];
        intermediarios[j] = tmp;
    }
    rota[0] = origem;
    for (int i = 0; i < QTD_INTERMEDIARIOS && i < qtd_intermed; i++)
        rota[i + 1] = intermediarios[i];
    rota[QTD_INTERMEDIARIOS + 1] = destino;
    *tamanho_rota = QTD_INTERMEDIARIOS + 2;
}

void swap(int rota[], int tamanho) {
    int num_swaps = 2 + rand() % 3;
    for (int s = 0; s < num_swaps; s++) {
        int i = 1 + rand() % (tamanho - 2);
        int j = 1 + rand() % (tamanho - 2);
        if (i != j) {
            int tmp = rota[i];
            rota[i] = rota[j];
            rota[j] = tmp;
        }
    }
}

void copiar_rota(int src[], int dst[], int tamanho) {
    for (int i = 0; i < tamanho; i++) dst[i] = src[i];
}

void simulated_annealing(int origem, int destino, int melhor_rota[], int *melhor_tam) {
    int rota_atual[MAX_NODES], nova_rota[MAX_NODES];
    double temp = TEMP_INICIAL;
    gerar_rota_inicial(origem, destino, rota_atual, melhor_tam);
    copiar_rota(rota_atual, melhor_rota, *melhor_tam);
    double melhor_score = calc_score(melhor_rota, *melhor_tam);

    while (temp > 1e-3) {
        for (int iter = 0; iter < MAX_ITER_POR_TEMPERATURA; iter++) {
            copiar_rota(rota_atual, nova_rota, *melhor_tam);
            swap(nova_rota, *melhor_tam);

            double score_novo = calc_score(nova_rota, *melhor_tam);
            double score_atual = calc_score(rota_atual, *melhor_tam);

            if (score_novo < score_atual || (rand() / (double)RAND_MAX) < exp((score_atual - score_novo) / temp)) {
                copiar_rota(nova_rota, rota_atual, *melhor_tam);

                if (score_novo < melhor_score) {
                    copiar_rota(nova_rota, melhor_rota, *melhor_tam);
                    melhor_score = score_novo;
                }
            }
        }
        temp *= RESFRIAMENTO;
    }
}


void salvar_rota(const char *arquivo, int rota1[], int tam1, int rota2[], int tam2) {
    FILE *f = fopen(arquivo, "w");
    if (!f) { printf("Erro ao salvar arquivo.\n"); exit(1); }
    fprintf(f, "id\n");
    for (int i = 0; i < tam1; i++)
        fprintf(f, "%d\n", rota1[i]);
    for (int i = 1; i < tam1; i++)
        fprintf(f, "%d\n", rota2[i]);
    fclose(f);
};

int main() {
    srand(time(NULL));
    ler_arestas("../pythonUtils/arestas.csv");

    int origem = vertices[0];
    int destino = vertices[qtd_vertices - 1];

    int melhor_ida[MAX_NODES], tam_ida;
    int melhor_volta[MAX_NODES], tam_volta;

    simulated_annealing(origem, destino, melhor_ida, &tam_ida);
    simulated_annealing(destino, origem, melhor_volta, &tam_volta);
    imprimir_detalhes_rota(melhor_ida, tam_ida, "Rota de ida");
    imprimir_detalhes_rota(melhor_volta, tam_volta, "Rota de volta");

    salvar_rota("../Resultados/equilibrio.csv", melhor_ida, tam_ida, melhor_volta, tam_volta);
    return 0;
}
