#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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

int  check_action       (char c);
int  check_cell         (int **map, char **vmap, int x, int y);
int  check_map_size     (int num);
int  check_bomb_size    (int num);
int  check_point        (int x, int y);
void end_game           (int ***map, char ***vmap);
void increment_around   (int **map, int x, int y);
char input_action       (void);
void init_vmap          (char **vmap);
void init_game          (int ***map, char ***vmap);
void input_definition   (int *num, char *str, int (*check_func)(int num));
void input_point        (int *x, int *y);
int  is_bomb            (int **map, int x, int y);
int  is_clear           (int **map);
int  is_limit           (int limit, int n);
void open_around        (int **map, char **vmap, int x, int y);
void set_bomb           (int **map, int selected_x, int selected_y);
void show_map           (char **map, int clear_flag);
void switch_flag        (char **vmap, int x, int y);

// デバッグ用関数
void display_state(int **map);

int main(void)
{
    char **vmap;    // ユーザに表示するマップ
    int **map;      // マップの実体

    int selected_x, selected_y;
    int game_flag;

    input_definition(&g_col, "マップの縦の長さ", check_map_size);
    input_definition(&g_row, "マップの横の長さ", check_map_size);
    input_definition(&g_max_bomb, "爆弾", check_bomb_size);

    init_game(&map, &vmap);
    show_map(vmap, FALSE);
    input_point(&selected_x, &selected_y);
    set_bomb(map, selected_x, selected_y);
    open_around(map, vmap, selected_x, selected_y);

    game_flag = TRUE;
    while (is_clear(map) != TRUE && game_flag == TRUE){
        show_map(vmap, FALSE);
        input_point(&selected_x, &selected_y);

        if (input_action() == 'f'){
            switch_flag(vmap, selected_x, selected_y);
        }
        else {
            game_flag = check_cell(map, vmap, selected_x, selected_y);
        }
    }

    end_game(&map, &vmap);

    return (0);
}

void input_definition(int *num, char *str, int (*check_func)(int num))
{
    do {
        printf("%sの数を設定して下さい:", str);
        fflush(stdin);
        scanf(" %d", num);
    }while (check_func(*num) != TRUE);
}

int check_map_size(int num)
{
    if (0 < num && num < 128){
        return (TRUE);
    }
    puts("範囲外の数値が入力されました");
    return (FALSE);
}

int check_bomb_size(int num)
{
    if (0 < num && num < g_col * g_row){
        return (TRUE);
    }
    puts("マップの数を超える数値が入力されました");
    return (FALSE);
}

void input_point(int *x, int *y)
{
    char c1, c2;

    do {
        printf("xy を入力して下さい:");
        scanf(" %c %c", &c1, &c2);
        fflush(stdin);

        *x = c1 - 'a';
        *y = c2 - '0';
    }while (check_point(*x, *y) != TRUE);
}

int check_point(int x, int y)
{
    if (is_limit(g_row, x) == TRUE && is_limit(g_col, y) == TRUE){
        return (TRUE);
    }

    puts("マップの数を超える数値が入力されました");
    return (FALSE);
}

char input_action(void)
{
    char c;

    do {
        printf("(f)lag か (o)pen を選択して下さい:");
        scanf(" %c", &c);
        fflush(stdin);
    } while (check_action(c) != TRUE);

    return (c);
}

int check_action(char c)
{
    if (c == 'o' || c == 'f'){
        return (TRUE);
    }
    puts("oかf以外の文字が入力されました");
    return (FALSE);
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

    puts("解答 -------------------------------");
    for (i = 0; i < g_col; i++){
        for (j = 0; j < g_row; j++){
            if (is_bomb(*map, j, i) == TRUE){
                (*vmap)[i][j] = VISUAL_BOMB;
            }
        }
    }

    show_map(*vmap, TRUE);

    for (i = g_col - 1; 0 <= i; i--){
        free(*(*map + i));
        free(*(*vmap + i));
    }

    free(*map);
    free(*vmap);
}

void set_bomb(int **map, int selected_x, int selected_y)
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

        if (map[y][x] == BOMB || (x == selected_x && y == selected_y)){
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
    static int *visited = (void *)NULL;

    if (visited == (void *)NULL){
        visited = (int *)calloc(g_col * g_row, sizeof(int));
    }

    if (visited[y * g_row + x] == TRUE || vmap[y][x] == VISUAL_FLAG){
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

void show_map(char **map, int clear_flag)
{
    int i, j;
    static char *colors[] = {"\x1b[37m", "\e[36m", "\e[34m", "\e[32m"
      , "\e[33m", "\e[31m", "\e[31m", "\e[31m", "\e[31m"};

    if (clear_flag != TRUE){
        printf("\e[2J\e[1;1H");
    }

    printf("%4c", ' ');

    for (i = 0; i < g_row; i++){
      printf("%5c", 'a' + i);
    }
    putchar('\n');

    for (i = 0; i < g_col; i++){
        printf("%4c", i + '0');

        for (j = 0; j < g_row; j++){
          printf("\x1b[m");
          printf("%4c", ' ');

          if (map[i][j] == VISUAL_FLAG){
              printf("\x1b[47m");
          }
          else if (map[i][j] == VISUAL_UNOPEN){
              printf("\x1b[37m");
          }
          else if (map[i][j] == VISUAL_BOMB){
              printf("\x1b[41m");
          }
          else {
              printf("%s", colors[map[i][j] - '0']);
          }
          printf("%c", map[i][j]);
        }

        printf("\x1b[m");
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
