#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define COL       (5)
#define ROW       (5)
#define MAX_BOMB  (5)

#define VISUAL_UNOPEN ('?')
#define VISUAL_BOMB   ('x')

#define BOMB      (-1)
#define OPENED    (10)

#define TRUE      (1)
#define FALSE     (0)

void init_game         (int ***map, char ***map_p);
void end_game          (int ***map, char ***map_p);
void set_bomb          (int **map);
void init_board        (char **map_p);
void increment_around  (int **map, int x, int y);
int  check_limit       (int limit, int n);
int  open_map          (int **map, char **map_p, unsigned int x, unsigned int y);
int  is_bomb           (int **map, int x, int y);
int  is_clear          (int **map);
void show_map          (char **map);
void display_state     (int **map);

int main(void)
{
    int **map;
    char **map_p;
    int flag;

    char c1, c2;
    unsigned int selected_x;
    unsigned int selected_y;

    init_game(&map, &map_p);

    while (is_clear(map) != TRUE){
        show_map(map_p);

        printf("opend(x, y):");
        scanf(" %d %d", &selected_x, &selected_y);

        flag = open_map(map, map_p, selected_x, selected_y);
        if (flag == FALSE){
            break;
        }
    }

    display_state(map);

    end_game(&map, &map_p);

    return (0);
}

// TODO:callocの失敗の時のことを考えていない
void init_game(int ***map, char ***map_p)
{
    int i;

    /* mapと仮mapの領域の確保 */
    *map   = (int **) calloc(COL, sizeof(char *));
    *map_p = (char **)calloc(COL, sizeof(int *));

    if (*map == NULL || *map_p == NULL){
        puts("メモリの確保に失敗しました");
        exit(1);
    }

    for (i = COL - 1; 0 <= i; i--){
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
    int i;
    int j;

    for (i = 0; i < COL; i++){
        for (j = 0; j < ROW; j++){
            map[i][j] = VISUAL_UNOPEN;
        }
        map[i][j] = '\0';
    }
}

void end_game(int ***map, char ***map_p)
{
    int i;

    for (i = 0; i < COL; i++){
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
    if (check_limit(COL, y - 1) == TRUE && check_limit(ROW, x - 1) == TRUE && is_bomb(map, x - 1, y - 1) == FALSE){
        map[y - 1][x - 1]++;
    }

    if (check_limit(COL, y - 1) == TRUE && is_bomb(map, x, y - 1) == FALSE){
        map[y - 1][x]++;
    }

    if (check_limit(COL, y - 1) == TRUE && check_limit(ROW, x + 1) == TRUE && is_bomb(map, x + 1, y - 1) == FALSE){
        map[y - 1][x + 1]++;
    }

    if (check_limit(ROW, x - 1) == TRUE && is_bomb(map, x - 1, y) == FALSE){
        map[y][x - 1]++;
    }

    if (check_limit(ROW, x + 1) == TRUE && is_bomb(map, x + 1, y) == FALSE){
        map[y][x + 1]++;
    }

    if (check_limit(COL, y + 1) == TRUE && check_limit(ROW, x - 1) == TRUE && is_bomb(map, x - 1, y + 1) == FALSE){
        map[y + 1][x - 1]++;
    }

    if (check_limit(COL, y + 1) == TRUE && is_bomb(map, x, y + 1) == FALSE){
        map[y + 1][x]++;
    }

    if (check_limit(COL, y + 1) == TRUE && check_limit(ROW, x + 1) == TRUE && is_bomb(map, x + 1, y + 1) == FALSE){
        map[y + 1][x + 1]++;
    }
}

int check_limit(int limit, int n)
{
    if (0 <= n && n < limit){
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
    int i;
    int j;

    for (i = COL - 1; 0 <= i; i--){
        for (j = ROW - 1; 0 <= j; j--){
            if (map[i][j] != OPENED && map[i][j] != BOMB){
                return (FALSE);
            }
        }
    }

    return (TRUE);
}

int open_map(int **map, char **map_p, unsigned int x, unsigned int y)
{
    if (map[y][x] == OPENED){
        return (TRUE);
    }

    if (is_bomb(map, x, y) == TRUE){
        puts("爆弾です!");
        return (FALSE);
    }

    map_p[y][x] = map[y][x] + '0';
    map[y][x] = OPENED;

    return (TRUE);
}

void show_map(char **map)
{
    int i;
    int j;

    for (i = 0; i < COL; i++){
        for (j = 0; j < ROW; j++){
            printf("%4c", map[i][j]);
        }
        puts("");
    }
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
