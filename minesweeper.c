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

void adapt_around       (int **map, int x, int y, void (*func)(int **map, int x, int y));
int  check_cell         (int **map, char **vmap, int x, int y);
void end_game           (int ***map, char ***vmap);
void increment_around   (int **map, int x, int y);
void init_vmap          (char **vmap);
void init_game          (int ***map, char ***vmap);
void input_definision   (int *num, char *str);
int  is_bomb            (int **map, int x, int y);
int  is_clear           (int **map);
int  is_limit           (int limit, int n);
void open_around        (int **map, char **vmap, int x, int y);
void set_bomb           (int **map);
void show_map           (char **map);
void switch_flag        (char **vmap, int x, int y);

// デバッグ用関数
void display_state(int **map);

int main(void)
{
    char **vmap;    // ユーザに表示するマップ
    int **map;      // マップの実体

    char c1, c2, c3;
    int selected_x, selected_y;

    int game_flag = TRUE;

    input_definision(&g_col, "マップの縦の長さ");
    input_definision(&g_row, "マップの横の長さ");
    input_definision(&g_max_bomb, "爆弾の数");

    init_game(&map, &vmap);

    display_state(map);

    while (is_clear(map) != TRUE && game_flag == TRUE){
        show_map(vmap);

        printf("x(space)y を入力して下さい:");
        scanf(" %c %c", &c2, &c1);

        selected_x = c2 - 'a';
        selected_y = c1 - '0';

        if (is_limit(g_row, selected_x) != TRUE || is_limit(g_col, selected_y) != TRUE){
            puts("範囲外の数値が入力されました");
            continue;
        }

        printf("(f)lag か (o)pen を選択して下さい:");
        scanf(" %c", &c3);

        switch (c3){
          case 'f':
            switch_flag(vmap, selected_x, selected_y);
            break;
          case 'o':
            game_flag = check_cell(map, vmap, selected_x, selected_y);
            break;
          default:
            puts("f と o 以外の文字が入力されました");
            break;
        }
    }

    show_map(vmap);
    end_game(&map, &vmap);

    return (0);
}

// TODO: 爆弾はマップの中に入る個数しか入力できないようにするべきでは？
void input_definision(int *num, char *str)
{
    while (1){
        printf("%sの数を設定して下さい:", str);
        scanf("%d", num);

        if (0 <= *num && *num < 128){
            break;
        }

        puts("範囲外の整数が入力されました");
    }
}

void init_game(int ***map, char ***vmap)
{
    *map   = (int **) calloc(g_col, sizeof(char *));
    *vmap = (char **)calloc(g_col, sizeof(int *));

    if (*map == NULL || *vmap == NULL){
        puts("メモリの確保に失敗しました");
        exit(1);
    }

    for (int i = g_col - 1; 0 <= i; i--){
        *(*map + i)   = (int  *)calloc(g_row, sizeof(int));
        *(*vmap + i) = (char *)calloc(g_row + 1, sizeof(char));

        if (*map == NULL || *vmap == NULL){
            puts("メモリの確保に失敗しました");
            exit(1);
        }
    }

    init_vmap(*vmap);
    set_bomb(*map);
}

void init_vmap(char **vmap)
{
    int i, j;

    for (i = g_col - 1; 0 <= i; i--){
        for (j = g_row - 1; 0 <= j; j--){
            vmap[i][j] = VISUAL_UNOPEN;
        }
        vmap[i][g_row] = '\0';
    }
}

void end_game(int ***map, char ***vmap)
{
    int i, j;

    for (i = g_col - 1; 0 <= i; i--){
        for (j = g_row - 1; 0 <= j; j--){
            if (is_bomb(*map, i, j) == TRUE){
                *vmap[i][j] = VISUAL_BOMB;
            }
        }
    }

    show_map(*vmap);

    for (i = g_col - 1; 0 <= i; i--){
        free(*(*map + i));
        free(*(*vmap + i));
    }

    free(*map);
    free(*vmap);
}

void set_bomb(int **map)
{
    int count;
    int rand_num;
    int x, y;

    srand((unsigned int)time(NULL));

    count = 0;
    while (count < g_max_bomb){
        rand_num = rand() % (g_col * g_row);
        x = rand_num % g_row;
        y = rand_num / g_row;

        if (map[y][x] == BOMB){
            continue;
        }

        map[y][x] = BOMB;
        increment_around(map, x, y);
        count++;
    }
}

void increment_around(int **map, int x, int y)
{
    // 爆弾を設置したCellの上側のCellへ重み付け
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

    // 爆弾を設置したCellの左右のCellへ重み付け
    if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y) == FALSE){
        map[y][x - 1]++;
    }
    if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y) == FALSE){
        map[y][x + 1]++;
    }

    // 爆弾を設置したCellの下側のCellへ重み付け
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

int check_cell(int **map, char **vmap, int x, int y)
{
    if (vmap[y][x] == VISUAL_FLAG || map[y][x] == OPENED){
        return (TRUE);
    }

    if (is_bomb(map, x, y) == TRUE){
        puts("爆弾です!");
        vmap[y][x] = VISUAL_BOMB;
        return (FALSE);
    }

    open_around(map, vmap, x, y);

    return (TRUE);
}

void open_around(int **map, char **vmap, int x, int y)
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
    vmap[y][x] = map[y][x] + '0';
    map[y][x] = OPENED;

    if (flag == TRUE){
        return;
    }

    // Cellの上側の探索
    if (is_limit(g_col, y - 1) == TRUE){
        if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y - 1) == FALSE){
            open_around(map, vmap, x - 1, y - 1);
        }
        if (is_bomb(map, x, y - 1) == FALSE){
            open_around(map, vmap, x, y - 1);
        }
        if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y - 1) == FALSE){
            open_around(map, vmap, x + 1, y - 1);
        }
    }

    // Cellの左右の探索
    if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y) == FALSE){
        open_around(map, vmap, x - 1, y);
    }
    if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y) == FALSE){
        open_around(map, vmap, x + 1, y);
    }

    // Cellの下側の探索
    if (is_limit(g_col, y + 1) == TRUE){
        if (is_limit(g_row, x - 1) == TRUE && is_bomb(map, x - 1, y + 1) == FALSE){
            open_around(map, vmap, x - 1, y + 1);
        }
        if (is_bomb(map, x, y + 1) == FALSE){
            open_around(map, vmap, x, y + 1);
        }
        if (is_limit(g_row, x + 1) == TRUE && is_bomb(map, x + 1, y + 1) == FALSE){
            open_around(map, vmap, x + 1, y + 1);
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
    putchar('\n');

    for (i = 0; i < g_col; i++){
        printf("%4c", i + '0');
        for (j = 0; j < g_row; j++){
            printf(" %4c", map[i][j]);
        }
        putchar('\n');
    }
}

void switch_flag(char **vmap, int x, int y)
{
    if (vmap[y][x] == VISUAL_UNOPEN && vmap[y][x] != VISUAL_FLAG){
        vmap[y][x] = VISUAL_FLAG;
        return;
    }

    vmap[y][x] = VISUAL_UNOPEN;
}

/* デバッグ用関数 */
void display_state(int **map)
{
    printf("%3c", ' ');

    for (int i = 0; i < g_row; i++){
      printf("%5c", 'a' + i);
    }
    putchar('\n');

    for (int i = 0; i < g_col; i++){
        printf("%4c", i + '0');
        for (int j = 0; j < g_row; j++){
            printf("%4d ", map[i][j]);
        }
        putchar('\n');
    }
}
