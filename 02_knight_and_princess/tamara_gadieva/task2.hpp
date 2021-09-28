#include <cstdlib>
#include <iostream>

#define VERTEX_NUM 14
#define HELP_STR_LEN 2

//матрица связности данного графа
int adj_matrix[VERTEX_NUM][VERTEX_NUM] = {
    {0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0}};

int* edge_ids_init(void);
//функция для создания вспомогательной матрицы
//и внесения в неё информации о номерах вершин

void edge_ids_free(int*);
//функция очистки динамической памяти
//вспомогательной матрицы

void vertices_json_output(FILE*, int*);
//функция заполнения в формате json информации
//о вершинах данного графа

void edges_json_output(FILE*, int*);
//функция заполнения в формате json информации
//о рёбрах данного графа