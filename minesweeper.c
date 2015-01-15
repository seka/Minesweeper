#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define COL       (5)
#define ROW       (5)
#define MAX_BOMB  (5)

#define VISUAL_UNOPEN ('?')
#define VISUAL_BOMB   ('x')
#define VISUAL_FLAG   ('f')

#define BOMB      (-1)
#define OPENED    (10)

#define TRUE      (1)
#define FALSE     (0)

void end_game          (int ***map, char ***map_p);
void increment_around  (int **map, int x, int y);
void init_board        (char **map_p);
void init_game         (int ***map, char ***map_p);
int  is_bomb           (int **map, int x, int y);
int  is_clear          (int **map);
int  is_limit          (int limit, int n);
int  open_cell         (int **map, char **map_p, unsigned int x, unsigned int y);
void put_flag          (char **map_p, unsigned int x, unsigned int y);
void set_bomb          (int **map);
void show_map          (char **map);

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

    init_game(&map, &map_p);

    while (is_clear(map) != TRUE && flag == TRUE){
        show_map(map_p);

        printf("Type x(space)y and press Enter:");
        scanf(" %c %c", &c2, &c1);

        selected_x = c2 - 'a';
        selected_y = c1 - '0';

        if (is_limit(ROW, selected_x) != TRUE || is_limit(COL, selected_y) != TRUE){
            puts("範囲外の数値が入力されました");
            continue;
        }
    }

    show_map(map_p);
    end_game(&map, &map_p);

    return (0);
}

void init_game(int ***map, char ***map_p)
{
    *map   = (int **) calloc(COL, sizeof(char *));
    *map_p = (char **)calloc(COL, sizeof(int *));

    if (*map == NULL || *map_p == NULL){
        puts("メモリの確保に失敗しました");
        exit(1);
    }

    for (int i = COL - 1; 0 <= i; i--){
        *(*map + i)   = (int  *)calloc(ROW, sizeof(int));
        *(*map_p + i) = (char *)calloc(ROW + 1, sizeof(char));

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
    for (int i = 0; i < COL; i++){
        for (int j = 0; j < ROW; j++){
            map[i][j] = VISUAL_UNOPEN;
        }
        map[i][ROW] = '\0';
    }
}

void end_game(int ***map, char ***map_p)
{
    for (int i = 0; i < COL; i++){
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
    while (count < MAX_BOMB){
        rand_num = rand() % (COL * ROW);

        if (map[rand_num / ROW][rand_num % ROW] != BOMB){
            map[rand_num / ROW][rand_num % ROW] = BOMB;
            increment_around(map, rand_num % ROW, rand_num / ROW);
            count++;
        }
    }
}

void increment_around(int **map, int x, int y)
{
    // Cellの上側の重み付け
    if (is_limit(COL, y - 1) == TRUE){
        if (is_limit(ROW, x - 1) == TRUE && is_bomb(map, x - 1, y - 1) == FALSE){
            map[y - 1][x - 1]++;
        }

        if (is_bomb(map, x, y - 1) == FALSE){
            map[y - 1][x]++;
        }

        if (is_limit(ROW, x + 1) == TRUE && is_bomb(map, x + 1, y - 1) == FALSE){
            map[y - 1][x + 1]++;
        }
    }

    // Cellの左右の重み付け
    if (is_limit(ROW, x - 1) == TRUE && is_bomb(map, x - 1, y) == FALSE){
        map[y][x - 1]++;
    }
    if (is_limit(ROW, x + 1) == TRUE && is_bomb(map, x + 1, y) == FALSE){
        map[y][x + 1]++;
    }

    // Cellの下側の重み付け
    if (is_limit(COL, y + 1) == TRUE){
        if (is_limit(ROW, x - 1) == TRUE && is_bomb(map, x - 1, y + 1) == FALSE){
            map[y + 1][x - 1]++;
        }

        if (is_bomb(map, x, y + 1) == FALSE){
            map[y + 1][x]++;
        }

        if (is_limit(ROW, x + 1) == TRUE && is_bomb(map, x + 1, y + 1) == FALSE){
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

    for (i = COL - 1; 0 <= i; i--){
        for (j = ROW - 1; 0 <= j; j--){
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
        map_p[y][x] = 'x';
        return (FALSE);
    }

    if (map[y][x] != OPENED){
        map_p[y][x] = map[y][x] + '0';
        map[y][x] = OPENED;
    }

    return (TRUE);
}

void show_map(char **map)
{
    int i, j;

    printf("%4c", ' ');

    for (i = 0; i < ROW; i++){
      printf("%5c", 'a' + i);
    }
    puts("");

    for (i = 0; i < COL; i++){
        printf("%4c", i + '0');
        for (j = 0; j < ROW; j++){
            printf(" %4c", map[i][j]);
        }
        puts("");
    }
}

void put_flag(char **map_p, unsigned int x, unsigned int y)
{
    if (map_p[y][x] == '?' && map_p[y][x] != VISUAL_FLAG){
        map_p[y][x] = VISUAL_FLAG;
        return;
    }

    map_p[y][x] = '?';
}

/* デバッグ用関数 */
void display_state(int **map)
{
    for (int i = 0; i < COL; i++){
        for (int j = 0; j < ROW; j++){
            printf("%4d ", map[i][j]);
        }
        puts("");
    }
}
