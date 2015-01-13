#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define COL       (5)
#define ROW       (5)
#define MAX_BOMB  (5)

#define BOMB      (-1)
#define SPACE     (0)
#define OPENED    (1)

void init_game(int ***map, char ***map_p);
void end_game(int ***map, char ***map_p);
void set_bomb(int **map);
int open(unsigned int x, unsigned int y);
int is_bomb(unsigned int x, unsigned int y);
int is_clear(void);
void display_state(int **map);

int main(void)
{
    int **map;
    char **map_p;

    init_game(&map, &map_p);

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

    for (i = COL - 1; 0 <= i; i--){
        *(*map + i)   = (int  *)calloc(ROW, sizeof(int));
        *(*map_p + i) = (char *)calloc(ROW, sizeof(char) + 1);
    }

    set_bomb(*map);
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
            count++;
        }
    }
}

void display_state(int **map)
{
    for (int i = 0; i < COL; i++){
        for (int j = 0; j < ROW; j++){
            printf("%4d ", map[i][j]);
        }
        puts("");
    }
}
