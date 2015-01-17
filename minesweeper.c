#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#define VISUAL_UNOPEN ('?')
#define VISUAL_BOMB   ('x')
#define VISUAL_FLAG   ('f')

#define BOMB      (-1)
#define OPENED    (10)
#define SPACE     (0)

#define TRUE      (1)
#define FALSE     (0)

int g_col;
int g_row;
int g_max_bomb;

void end_game           (int ***map, char ***map_p);
void increment_around   (int **map, int x, int y);
void init_board         (char **map_p);
void init_game          (int ***map, char ***map_p);
void input_definision   (int *num, char *str);
int  is_bomb            (int **map, int x, int y);
int  is_clear           (int **map);
int  is_limit           (int limit, int n);
int  open_cell          (int **map, char **map_p, unsigned int x, unsigned int y);
void open_around        (int **map, char **map_p, int x, int y);
void set_bomb           (int **map);
void show_map           (char **map);
void switch_flag        (char **map_p, unsigned int x, unsigned int y);

// デバッグ用関数
void display_state     (int **map);

int main(void)
{
    int **map;
    char **map_p;

    char c1, c2, c3;
    int selected_x;
    int selected_y;

    int flag = TRUE;

    input_definision(&g_col, "マップの縦の長さ");
    input_definision(&g_row, "マップの横の長さ");
    input_definision(&g_max_bomb, "爆弾の数");

    init_game(&map, &map_p);

    while (is_clear(map) != TRUE && flag == TRUE){
        show_map(map_p);

        printf("Type x(space)y and press Enter:");
        scanf(" %c %c", &c2, &c1);

        selected_x = c2 - 'a';
        selected_y = c1 - '0';

        if (is_limit(g_row, selected_x) != TRUE || is_limit(g_col, selected_y) != TRUE){
            puts("範囲外の数値が入力されました");
            continue;
        }

        printf("(f)lag or (o)pen and press to Enter:");
        scanf(" %c", &c3);

        if (c3 == 'f'){
            switch_flag(map_p, selected_x, selected_y);
        }
        else if (c3 == 'o'){
            flag = open_cell(map, map_p, selected_x, selected_y);
        }
        else {
            puts("f と o 以外の数値が入力されました");
        }
    }

    show_map(map_p);
    end_game(&map, &map_p);

    return (0);
}

// TODO: 爆弾はマップの中に入る個数しか入力できないようにするべきでは？
void input_definision(int *num, char *str)
{
    int x = 0;

    while (x == 0){
        printf("%sの設定:", str);
        scanf(" %d", &x);
        *num = x;

        if (x <= 0 && x > 16384){
            puts("範囲外の整数が入力されました");
            x = 0;
        }
    }
}

void init_game(int ***map, char ***map_p)
{
    *map   = (int **) calloc(g_col, sizeof(char *));
    *map_p = (char **)calloc(g_col, sizeof(int *));

    if (*map == NULL || *map_p == NULL){
        puts("メモリの確保に失敗しました");
        exit(1);
    }

    for (int i = g_col - 1; 0 <= i; i--){
        *(*map + i)   = (int  *)calloc(g_row, sizeof(int));
        *(*map_p + i) = (char *)calloc(g_row + 1, sizeof(char));

        if (*map == NULL || *map_p == NULL){
            puts("メモリの確保に失敗しました");
            exit(1);
        }
    }

    init_board(*map_p);
    set_bomb(*map);
}

void init_board(char **map)
{
    for (int i = 0; i < g_col; i++){
        for (int j = 0; j < g_row; j++){
            map[i][j] = VISUAL_UNOPEN;
        }
        map[i][g_row] = '\0';
    }
}

void end_game(int ***map, char ***map_p)
{
    for (int i = 0; i < g_col; i++){
        free(*(*map + i));
        free(*(*map_p + i));
    }

    free(*map);
    free(*map_p);
}

void set_bomb(int **map)
{
    int count;
    int rand_num;

    srand((unsigned int)time(NULL));

    count = 0;
    while (count < g_max_bomb){
        rand_num = rand() % (g_col * g_row);

        if (map[rand_num / g_row][rand_num % g_row] != BOMB){
            map[rand_num / g_row][rand_num % g_row] = BOMB;
            increment_around(map, rand_num % g_row, rand_num / g_row);
            count++;
        }
    }
}

void increment_around(int **map, int x, int y)
{
    // Cellの上側の重み付け
    if (is_limit(g_col, y - 1) == TRUE){
        if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y - 1) == FALSE){
            map[y - 1][x - 1]++;
        }
        if (is_bomb(map, x, y - 1) == FALSE){
            map[y - 1][x]++;
        }
        if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y - 1) == FALSE){
            map[y - 1][x + 1]++;
        }
    }

    // Cellの左右の重み付け
    if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y) == FALSE){
        map[y][x - 1]++;
    }
    if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y) == FALSE){
        map[y][x + 1]++;
    }

    // Cellの下側の重み付け
    if (is_limit(g_col, y + 1) == TRUE){
        if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y + 1) == FALSE){
            map[y + 1][x - 1]++;
        }
        if (is_bomb(map, x, y + 1) == FALSE){
            map[y + 1][x]++;
        }
        if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y + 1) == FALSE){
            map[y + 1][x + 1]++;
        }
    }
}

int is_limit(int limit, int n)
{
    if (-1 < n && n < limit){
        return (TRUE);
    }

    return (FALSE);
}

int is_bomb(int **map, int x, int y)
{
    if (map[y][x] == BOMB){
        return (TRUE);
    }

    return (FALSE);
}

int is_clear(int **map)
{
    int i, j;

    for (i = g_col - 1; 0 <= i; i--){
        for (j = g_row - 1; 0 <= j; j--){
            if (map[i][j] != OPENED && map[i][j] != BOMB){
                return (FALSE);
            }
        }
    }

    puts("ゲームクリア！");
    return (TRUE);
}

int open_cell(int **map, char **map_p, unsigned int x, unsigned int y)
{
    if (map_p[y][x] == VISUAL_FLAG){
        return (TRUE);
    }

    if (is_bomb(map, x, y) == TRUE){
        puts("爆弾です!");
        map_p[y][x] = VISUAL_BOMB;
        return (FALSE);
    }

    if (map[y][x] != OPENED){
        open_around(map, map_p, x, y);
    }

    return (TRUE);
}

void open_around(int **map, char **map_p, int x, int y)
{
    int flag = FALSE;
    static int visited[1000];

    if (visited[y * g_row + x] == TRUE){
        return;
    }

    if (map[y][x] != SPACE){
        flag = TRUE;
    }

    visited[y * g_row + x] = TRUE;
    map_p[y][x] = map[y][x] + '0';
    map[y][x] = OPENED;

    if (flag == TRUE){
        return;
    }

    // Cellの上側の探索
    if (is_limit(g_col, y - 1) == TRUE){
        if (is_limit(g_row, x - 1) == TRUE && map[y - 1][x - 1] != BOMB){
            open_around(map, map_p, x - 1, y - 1);
        }
        if (is_bomb(map, x, y - 1) != BOMB){
            open_around(map, map_p, x, y - 1);
        }
        if (is_limit(g_row, x + 1) == TRUE && map[y - 1][x + 1] != BOMB){
            open_around(map, map_p, x + 1, y - 1);
        }
    }

    // Cellの左右の探索
    if (is_limit(g_row, x - 1) == TRUE && map[y][x - 1] != BOMB){
        open_around(map, map_p, x - 1, y);
    }
    if (is_limit(g_row, x + 1) == TRUE && map[y][x + 1] != BOMB){
        open_around(map, map_p, x + 1, y);
    }

    // Cellの下側の探索
    if (is_limit(g_col, y + 1) == TRUE){
        if (is_limit(g_row, x - 1) == TRUE && map[y + 1][x - 1] != BOMB){
            open_around(map, map_p, x - 1, y + 1);
        }
        if (map[y + 1][x] != BOMB){
            open_around(map, map_p, x, y + 1);
        }
        if (is_limit(g_row, x + 1) == TRUE && map[y + 1][x + 1] != BOMB){
            open_around(map, map_p, x + 1, y + 1);
        }
    }
}

void show_map(char **map)
{
    int i, j;

    printf("%4c", ' ');

    for (i = 0; i < g_row; i++){
      printf("%5c", 'a' + i);
    }
    puts("");

    for (i = 0; i < g_col; i++){
        printf("%4c", i + '0');
        for (j = 0; j < g_row; j++){
            printf(" %4c", map[i][j]);
        }
        puts("");
    }
}

void switch_flag(char **map_p, unsigned int x, unsigned int y)
{
    if (map_p[y][x] == VISUAL_UNOPEN && map_p[y][x] != VISUAL_FLAG){
        map_p[y][x] = VISUAL_FLAG;
        return;
    }

    map_p[y][x] = VISUAL_UNOPEN;
}

/* デバッグ用関数 */
void display_state(int **map)
{
    printf("%3c", ' ');

    for (int i = 0; i < g_row; i++){
      printf("%5c", 'a' + i);
    }
    puts("");

    for (int i = 0; i < g_col; i++){
        printf("%4c", i + '0');
        for (int j = 0; j < g_row; j++){
            printf("%4d ", map[i][j]);
        }
        puts("");
    }
}
