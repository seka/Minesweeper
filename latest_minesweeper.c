#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define TRUE      (1)
#define FALSE     (0)

#define BOMB      (-1)
#define SPACE     (0)

#define VISUAL_UNOPEN ('?')
#define VISUAL_BOMB   ('x')
#define VISUAL_FLAG   ('f')

#define OPENED    (1)
#define UNOPEN    (2)
#define FLAG      (3)

#define TRUE      (1)
#define FALSE     (0)

typedef struct{
    int bomb;
    int state;
    int count;
} Cell;

int g_count_opened;
int g_col;
int g_row;
int g_max_bomb;

static int *g_visited;  // 近傍探索用
Cell **g_map;         // マップの実体

void init_game          (void);
void create_map         (void);
void input_definition   (int *num, char *str, int (*check_func)(int num));
int  check_map_size     (int num);
int  check_bomb_size    (int num);
void end_game           (void);
void show_map           (int game_flag);
void input_point        (int *x, int *y);
int  check_point        (int x, int y);
int  is_limit           (int limit, int n);
void set_bomb           (int x, int y);
void adapter_around     (int x, int y, void (*func)(int x, int y));
void increment_around   (int x, int y);
void open_around        (int x, int y);
char input_action       (void);
int  check_action       (char c);
void switch_flag        (int x, int y);
int  is_bomb            (int x, int y);
int  check_cell         (int x, int y);
int  is_clear           (void);

void display_debug(void);

int main(void)
{
    int selected_x, selected_y;
    int game_flag = TRUE;

    init_game();

    input_point(&selected_x, &selected_y);
    set_bomb(selected_x, selected_y);
    game_flag = check_cell(selected_x, selected_y);

    while (is_clear() != TRUE && game_flag == TRUE){
        show_map(FALSE);
        input_point(&selected_x, &selected_y);
        if (input_action() == 'f'){
            switch_flag(selected_x, selected_y);
        }
        else {
            game_flag = check_cell(selected_x, selected_y);
        }
    }

    end_game();

    return (0);
}

void init_game()
{
    input_definition(&g_col, "マップの縦の長さ", check_map_size);
    input_definition(&g_row, "マップの横の長さ", check_map_size);
    input_definition(&g_max_bomb, "爆弾", check_bomb_size);
    create_map();
    show_map(FALSE);
}

void create_map(void)
{
    int i, j;
    Cell c = {FALSE, UNOPEN, 0}; // bomb, state, count

    g_visited = (int *)calloc(g_col * g_row, sizeof(int));
    g_map = (Cell **)malloc(g_col * sizeof(Cell *));

    if (g_map == NULL){
        puts("メモリの確保に失敗しました");
        exit(1);
    }

    for (i = g_col - 1; 0 <= i; i--){
        *(g_map + i) = (Cell *)malloc(g_row * sizeof(Cell));

        if (*(g_map + i) == NULL){
            puts("メモリの確保に失敗しました2");
            exit(1);
        }

        for (j = g_row - 1; 0 <= j; j--){
            g_map[i][j] = c;
        }
    }
}

void input_definition(int *num, char *str, int (*check_func)(int num))
{
    char s[20];

    do {
        printf("%sの数を設定して下さい:", str);
        gets(s);
        fflush(stdin);
        *num = atoi(s);
    }while (check_func(*num) != TRUE);
}

int check_map_size(int num)
{
    if (num <= 0 || 26 < num){
        puts("範囲外の数値が入力されました");
        return (FALSE);
    }

    return (TRUE);
}

int check_bomb_size(int num)
{
    if (num < 0 || g_col * g_row < num){
        puts("マップの数を超える数値が入力されました");
        return (FALSE);
    }

    return (TRUE);
}

void end_game(void)
{
    int i, j;

    puts("解答 -------------------------------");
    for (i = g_col - 1; 0 <= i; i--){
        for (j = g_row - 1; 0 <= j; j--){
            g_map[i][j].state = OPENED;
        }
    }
    show_map(TRUE);

    for (i = g_col - 1; 0 <= i; i--){
        free(*(g_map + i));
    }

    free(g_visited);
    free(g_map);
}

void show_map(int game_flag)
{
    int i, j;
    static char *colors[] = {"\x1b[37m", "\e[36m", "\e[34m", "\e[32m"
      , "\e[33m", "\e[31m", "\e[31m", "\e[31m", "\e[31m"};

    if (game_flag != TRUE){
       // printf("\e[2J\e[1;1H");
    }

    printf("\n%5c", ' ');

    for (i = 0; i < g_row; i++){
      printf("%4c ", 'a' + i);
    }
    putchar('\n');

    for (i = 0; i < g_col; i++){
        printf("%4c", i + '0');
        for (j = 0; j < g_row; j++){
            printf("\x1b[m%4c", ' ');

            switch (g_map[i][j].state){
              case FLAG:
                printf("\x1b[47m");
                printf("%c", VISUAL_FLAG);
                break;
              case UNOPEN:
                printf("\x1b[37m");
                printf("%c", VISUAL_UNOPEN);
                break;
              case OPENED:
                if (g_map[i][j].bomb == TRUE){
                  printf("\x1b[41m");
                  printf("%c", VISUAL_BOMB);
                  break;
                }
                printf("%s", colors[g_map[i][j].count]);
                printf("%d", g_map[i][j].count);
                break;
            }
        }
        printf("\x1b[m\n");
    }
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

    puts("マップを超える数値が入力されました");
    return (FALSE);
}

int is_limit(int limit, int n)
{
    if (-1 < n && n < limit){
        return (TRUE);
    }

    return (FALSE);
}

void set_bomb(int selected_x, int selected_y)
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

        if (g_map[y][x].bomb == TRUE || (x == selected_x && y == selected_y)){
            continue;
        }

        g_map[y][x].bomb = TRUE;
        adapter_around(x, y, increment_around);
        count++;
    }
}

void adapter_around(int x, int y, void (*func)(int x, int y))
{
    int i, j;

    for (i = y - 1; i < y + 2; i++){
        for (j = x - 1; j < x + 2; j++){
            if (is_limit(g_col, i) == FALSE || is_limit(g_row, j) == FALSE
                || g_map[i][j].bomb == TRUE){
                continue;
            }
            func(j, i);
        }
    }
}

void increment_around(int x, int y)
{
    g_map[y][x].count++;
}

void open_around(int x, int y)
{
    if (g_visited[y * g_row + x] == TRUE || g_map[y][x].state == FLAG){
        return;
    }

    g_visited[y * g_row + x] = TRUE;
    g_map[y][x].state = OPENED;
    g_count_opened++;

    if (0 < g_map[y][x].count){
        return;
    }

    adapter_around(x, y, open_around);
}

char input_action(void)
{
    char c;

    do {
        printf("(f)lag か (o)pen を選択して下さい:");
        scanf(" %c", &c);
        fflush(stdin);
    }while (check_action(c) != TRUE);

    return (c);
}

int check_action(char c)
{
    if (c == 'o' || c == 'f'){
        return (TRUE);
    }

    puts("fかo以外の文字が入力されました");
    return (FALSE);
}

void switch_flag(int x, int y)
{
    if (g_map[y][x].state != OPENED && g_map[y][x].state != FLAG){
        g_map[y][x].state = FLAG;
        return;
    }
    g_map[y][x].state = UNOPEN;
}

int check_cell(int x, int y)
{
    if (is_bomb(x, y) == TRUE){
        return (FALSE);
    }

    if (g_map[y][x].state == FLAG){
        return (TRUE);
    }

    g_map[y][x].state = OPENED;
    g_count_opened++;
    adapter_around(x, y, open_around);

    return (TRUE);
}

int is_bomb(int x, int y)
{
    if (g_map[y][x].bomb == FALSE){
        return (FALSE);
    }

    puts("爆弾です!");
    return (TRUE);
}

int is_clear(void)
{
    if (g_count_opened < g_col * g_row - g_max_bomb){
        return (FALSE);
    }

    puts("ゲームクリア！");
    return (TRUE);
}

void display_debug(void)
{
    printf("\n%3c", ' ');

    for (int i = 0; i < g_row; i++){
      printf("%5c", 'a' + i);
    }
    putchar('\n');

    for (int i = 0; i < g_col; i++){
        printf("%4c", i + '0');
        for (int j = 0; j < g_row; j++){
            if (g_map[i][j].bomb == TRUE){
                printf("%4c ", VISUAL_BOMB);
            }
            else {
                printf("%4d ", g_map[i][j].count);
            }
        }
        putchar('\n');
    }
}
