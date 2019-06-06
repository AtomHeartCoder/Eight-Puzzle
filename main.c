/***********************************************************
 *                                                         *
 * A solution of A Star (A*) algorithm to 8-puzzle problem *
 *                                                         *
 * Author: LU Sitong                                       *
 *                                                         *
 * Date: 2019.5.29                                         *
 *                                                         *
 ***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <sys/timeb.h>

#define  MAXLIST     10000
#define  TARGETLIST  1500

typedef enum movement {                                              /* describe the movement of the blank */
    up, down, left, right, none
} MOVEMENT;

typedef struct NODE {                                                /* information of the nodes */
    int num;                                                         /* recording the number of the node */
    int prior_num;                                                   /* recording the number 0f the prior node for debugging */
    char board[3][4];                                                /* main information of the board of every node */
    int depth;                                                       /* the depth of the node in the tree */
    int value;                                                       /* this describe the possibility of this node and its subnodes */
    struct NODE *prior;                                              /* pointer to the prior node */
    MOVEMENT avoid;                                                  /* the movement that should be avoided to move back to its prior node in its subnodes */
} NODE;

void gotoxy(int x, int y);
void makeboard(NODE *node, char *name);
int search(NODE *begin, int maxlist, int mode);
//int cut(int min[]);
int boundary(NODE *node, int movement);
int evaluate(NODE *node, int mode);
int judge(NODE *node1, NODE *node2);
void output(NODE *node, int mode);

NODE start = {0, 0, {"123", "456", "78 "}, 0, 0, NULL, none}, final_target = {0, 0, {"123", "456", "78 "}, 0, 0, NULL, none};
NODE *open[MAXLIST], *close[MAXLIST], *path[MAXLIST], *target[TARGETLIST + 1];
FILE *fp = NULL;

void gotoxy(int x, int y)
{
    COORD pos = {x, y};
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOut, pos);
}

int boundary(NODE *node, int movement)                               /* in case of moving out of the boundary of the board */
{
    int i, j, sign = 1;
    for (i = 0; i < 3;) {
        for (j = 0; j < 3;) {
            if (node->board[i][j] == ' ') {
                sign = 0;
                break;
            }
            else
                j++;
        }
        if (sign == 0)
            break;
        else
            i++;
    }
    if ((i == 0 && movement == up) || (i == 2 && movement == down) || (j == 0 && movement == left) || (j == 2 && movement == right))
        return 1;
    return 0;
}

int search(NODE *begin, int maxlist, int mode)                                                     /* A* algorithm */
{
    int m = 0, sign, sign2, i, j, k, x = 0, y = 0, weight = 1, max_depth = 0, min[maxlist];
    static int num = 1;
    MOVEMENT movement;
    open[0] = begin;
    while (m < maxlist) {
        for (i = 0, j = 0, min[0] = 0, sign = 1, sign2 = 1; i < maxlist; i++) {
            if (sign && open[i] != NULL)                             /* in case that the open list is full or empty */
                sign = 0;
            if (sign2 && open[i] == NULL)
                sign2 = 0;
            if (open[i] != NULL && open[min[j]]->value * weight + open[min[j]]->depth > open[i]->value * weight + open[i]->depth) {               /* find the open nodes with the least value */
                j = 0;
                min[j] = i;
            }
            else if (open[i] != NULL && min[i] != i && open[min[j]]->value * weight + open[min[j]]->depth > open[i]->value * weight + open[i]->depth)
                min[++j] = i;
        }
        if (sign || sign2)
            return -1;
        //if (open[min[0]]->depth >= 18) {
        //    sign2 = cut(min);
        //    j--;
        //}
        for (i = 0; i <= j; i++) {                                   /* judge for success */
            if (mode == 0) {
                if (judge(open[min[i]], &start)) {
                    start.prior = open[min[i]];
                    return 1;
                }
            }
            else if (mode == 1)
                for (k = 0; target[k] != NULL && k < TARGETLIST; k++) {
                    if (judge(open[min[i]], target[k])) {
                        target[TARGETLIST] = open[min[i]];
                        return k;
                    }
                }
        }
        if (j != 0) {                                                 /* randomly pick up one of the open nodes with the least value */
            srand(time(NULL));
            i = rand() % (j + 1);
        }
        else
            i = 0;
        if (open[min[i]]->depth > 25 && (rand() % 100) > 60) {
            int mark;
            for (i = 0, mark = min[0]; i <= j; i++) {
                if (open[min[i]]->value < open[mark]->value)
                    mark = min[i];
            }
            min[i] = mark;
        }
        close[m] = open[min[i]];                                     /* move the node from open list to close list */
        open[min[i]] = NULL;
        for (movement = up; movement <= right; movement++) {         /* try every movement of different direction */
            if (movement == close[m]->avoid || boundary(close[m], movement))             /* check whether the movements are legal */
                continue;
            NODE *node = (NODE *)malloc(sizeof(NODE));
            node->prior = close[m];
            node->depth = close[m]->depth + 1;
            if (node->depth > max_depth)
                max_depth = node->depth;
            node->num = num++;
            node->prior_num = close[m]->num;
            char temp;
            for (i = 0; i < 3; i++)
                for (j = 0; j < 3; j++)
                    node->board[i][j] = close[m]->board[i][j];
            sign = 1, i = 0;
            do {                                                     /* find the blank */
                j = 0;
                do {
                    if (node->board[i][j] == ' ')
                        sign = 0;
                } while (sign && (j++ < 3));
            } while (sign && (i++ < 3));
            switch (movement) {                                      /* make movements */
                case up:
                    x = -1, y = 0;
                    break;
                case down:
                    x = 1, y = 0;
                    break;
                case left:
                    x = 0, y = -1;
                    break;
                case right:
                    x = 0, y = 1;
                    break;
                default:
                    x = 0, y = 0;
            }
            temp = node->board[i][j];
            node->board[i][j] = node->board[i + x][j + y];
            node->board[i + x][j + y] = temp;
            node->value = evaluate(node, mode);                      /* evaluate the value of the node */
            if (movement == up)
                node->avoid = down;
            else if (movement == down)
                node->avoid = up;
            else if (movement == left)
                node->avoid = right;
            else if (movement == right)
                node->avoid = left;
            output(node, 1);                                         /* put the information to the log */
            for (i = 0, sign = 1; i < maxlist; i++) {                /* check whether the node is already in open or close list */
                if (open[i] != NULL && judge(node, open[i]) && node->depth < open[i]->depth) {
                    open[i]->depth = node->depth;
                    open[i]->prior = node->prior;
                    sign = 0;
                    break;
                }
                if (close[i] != NULL && judge(node, close[i]) && node->depth < close[i]->depth) {
                    close[i]->depth = node->depth;
                    close[i]->prior = node->prior;
                    for (j = 0; j < maxlist; j++)
                        if (open[j] == NULL) {
                            open[j] = close[i];
                            close[i] = NULL;
                            break;
                        }
                    sign = 0;
                    break;
                }
            }
            if (sign == 0)
                free(node);
            else                                                     /* add the node to open list */
                for (i = 0; i < maxlist; i++)
                    if (open[i] == NULL) {
                        open[i] = node;
                        break;
                    }
            if (max_depth >= 10)
                weight = 2;
            else if (max_depth >= 20)
                weight = 3;
        }
        m++;
    }
    return -1;
}

int evaluate(NODE *node, int mode)                                   /* evaluate the nodes */
{
    int value = 0, i, j, k, min_value = 999;
    if (mode == 0) {
        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++)
                if (node->board[i][j] != start.board[i][j])
                    value++;
        min_value = value;
    } else if (mode == 1)
        for (k = 0; target[k] != NULL; k++) {
            for (i = 0; i < 3; i++)
                for (j = 0; j < 3; j++)
                    if (node->board[i][j] != target[k]->board[i][j])
                        value++;
            if (min_value > value)
                min_value = value;
        }
    else {
        for (i = 0; i < 3; i++)
            for (j = 0; j < 3; j++)
                if (node->board[i][j] != target[0]->board[i][j])
                    value++;
        min_value = value;
    }
    return min_value;
}

int judge(NODE *node1, NODE *node2)                                  /* judge whether satisfied */
{
    int i, j, sign;
    for (i = 0, sign = 1; sign && i < 3; i++)
        for (j = 0; sign && j < 3; j++)
            if (node1->board[i][j] != node2->board[i][j])
                sign = 0;
    if(sign)
        return 1;
    return 0;
}

void output(NODE *node, int mode)
{
    int m, n;
    fprintf(fp, "No.%d: prior number: %d, depth = %d, value = %d, avoid = %d\n", node->num, node->prior_num, node->depth, node->value, node->avoid);
    for (m = 0; m < 3; m++) {
        for (n = 0; n < 3; n++)
            fprintf(fp, "%c", node->board[m][n]);
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
    if (mode == 1) {
        gotoxy(0, 8);
        printf("Amount of nodes: %d\n", node->num);
    }
}

/*int cut(int min[])
{
    int i, max;
    NODE *maxnode = NULL;
    for (i = 0, maxnode = open[min[0]]; (open[min[i]]->value == maxnode->value) && i < MAXLIST; i++)
        if ((open[min[i]]->value - open[min[i]]->depth) > (maxnode->value - maxnode->depth)) {
            maxnode = open[i];
            max = i;
        }
    if (i > 5) {
        open[min[max]] = NULL;
        free(maxnode);
    }
    return max;
}*/

void makeboard(NODE *node, char *name)
{
    MOVEMENT move = none;
    char ctrl, temp;
    int i, j, sign, x, y;
    system("cls");
    for (i = 0; i < 3; i++) {
        printf("%c%c%c\n", node->board[i][0], node->board[i][1], node->board[i][2]);
    }
    printf("\nPress 'p' to save the %s board...", name);
    while((ctrl = _getch())) {
        for (i = 0, sign = 1, x = y = 0; i < 3;) {
            for (j = 0; j < 3;) {
                if (node->board[i][j] == ' ') {
                    sign = 0;
                    break;
                }
                else
                    j++;
            }
            if (sign == 0)
                break;
            else
                i++;
        }
        switch(ctrl) {
            case 'w':
                move = down;
                x = 1, y = 0;
                break;
            case 's':
                move = up;
                x = -1, y = 0;
                break;
            case 'a':
                move = right;
                x = 0, y = 1;
                break;
            case 'd':
                move = left;
                x = 0, y = -1;
                break;
            case 'p':
                system("cls");
                return;
        }
        if ((i == 0 && move == up) || (i == 2 && move == down) || (j == 0 && move == left) || (j == 2 && move == right))
            continue;
        else {
            temp = node->board[i][j];
            node->board[i][j] = node->board[i + x][j + y];
            node->board[i + x][j + y] = temp;
        }
        system("cls");
        for (i = 0; i < 3; i++) {
            printf("%c%c%c\n", node->board[i][0], node->board[i][1], node->board[i][2]);
        }
        printf("\nPress 'p' to save the %s board...", name);
    }
    return;
}

int main(void)
{
    int i, j, m, n, mark;
    long cost_s, cost_ms;
    struct timeb t1, t2;
    char option;
    NODE *node, *list[50];
    fp = fopen("log1.txt", "w");
    target[0] = &final_target;
    for (i = 0; i < MAXLIST; i++) {
        open[i] = NULL;
        close[i] = NULL;
        path[i] = NULL;
    }
    for (i = 1; i < TARGETLIST; i++)
        target[i] = NULL;
    printf("Function:\n1. Input mode\n2. Game mode");
    if ((option = _getch()) == '1') {
        system("cls");
        for (i = 0; i < 3; i++) {
            scanf("%c%c%c%*c", &start.board[i][0], &start.board[i][1], &start.board[i][2]);
        }
        putchar('\n');
        for (i = 0; i < 3; i++) {
            scanf("%c%c%c%*c", &target[0]->board[i][0], &target[0]->board[i][1], &target[0]->board[i][2]);
        }
    } else {
        system("cls");
        makeboard(&start, "start");
        makeboard(&final_target, "target");
        for (i = 0; i < 3; i++) {
            printf("%c%c%c\n", start.board[i][0], start.board[i][1], start.board[i][2]);
        }
        putchar('\n');
        for (i = 0; i < 3; i++) {
            printf("%c%c%c\n", target[0]->board[i][0], target[0]->board[i][1], target[0]->board[i][2]);
        }
    }
    target[0]->value = evaluate(target[0], 0);
    ftime(&t1);
    if ((search(target[0], TARGETLIST, 0)) != -1) {
        ftime(&t2);
        cost_s = t2.time - t1.time;
        if((cost_ms = t2.millitm - t1.millitm) < 0) {
            cost_ms += 1000;
            cost_s--;
        }
        for (node = start.prior, i = 0; node != NULL; node = node->prior, i++)
            path[i] = node;
        for (j = 0; path[j] != NULL; j++)
            path[j]->depth = i - 1 - path[j]->depth;
        printf("\nSuccess! Cost %ld s %ld ms. Took %d step(s).\n\nPress any key to watch the process...\n\n", cost_s, cost_ms, path[i - 1]->depth);
        freopen("output.txt", "w", fp);
        system("pause > nul");
        gotoxy(0, 12);
        printf("                                     ");
        for (j = 0; path[j] != NULL; j++) {
            gotoxy(0, 12);
            for (m = 0; m < 3; m++) {
                for (n = 0; n < 3; n++){
                    printf("%c", path[j]->board[m][n]);
                }
                putchar('\n');
            }
            printf("\nStep: %d\n\n", path[j]->depth);
            output(path[j], 0);
            Sleep(500);
        }
        fclose(fp);
        system("pause");
        return 0;
    }
    for (i = 0, j = 1; j < TARGETLIST && i < MAXLIST; i++)
        if (open[i] != NULL)
            target[j++] = open[i];
    for (i = 1; i < j; i++)
        for (m = i + 1; m < j; m++)
            if (target[i]->depth > target[m]->depth)
                node = target[i], target[i] = target[m], target[m] = node;
    for (i = 0; i < MAXLIST; i++) {
        open[i] = NULL;
        close[i] = NULL;
    }
    freopen("target.txt", "w", fp);
    for (i = 0; target[i] != NULL && i < TARGETLIST; i++)
        output(target[i], 0);
    freopen("log2.txt", "w", fp);
    start.value = evaluate(&start, 1);
    output(&start, 0);
    output(&final_target, 0);
    if ((mark = search(&start, MAXLIST, 1)) != -1) {
        ftime(&t2);
        cost_s = t2.time - t1.time;
        if((cost_ms = t2.millitm - t1.millitm) < 0) {
            cost_ms += 1000;
            cost_s--;
        }
        for (node = target[mark]->prior, i = 0; node != NULL; node = node->prior, i++)
            list[i] = node;
        for (j = i - 1; j > 0; j--)
            list[j]->prior = list[j - 1];
        list[0]->prior = target[TARGETLIST];
        for (node = target[0], i = 0; node != NULL; node = node->prior, i++)
            path[i] = node;
        printf("\nSuccess! Cost %ld s %ld ms. Took %d step(s).\n\nPress any key to watch the process...\n\n", cost_s, cost_ms, i - 1);
        freopen("output.txt", "w", fp);
        system("pause > nul");
        gotoxy(0, 12);
        printf("                                     ");
        for (j = i - 1; path[j] != NULL && j >= 0; j--) {
            if (path[j]->depth != i - 1 - j) {
                path[j]->depth = i - 1 - j;
                path[j]->value = evaluate(path[j], 2);
            }
            gotoxy(0, 12);
            for (m = 0; m < 3; m++) {
                for (n = 0; n < 3; n++){
                    printf("%c", path[j]->board[m][n]);
                }
                putchar('\n');
            }
            printf("\nStep: %d\n\n", path[j]->depth);
            output(path[j], 0);
            Sleep(500);
        }
    }
    else {
        ftime(&t2);
        cost_s = t2.time - t1.time;
        if((cost_ms = t2.millitm - t1.millitm) < 0) {
            cost_ms += 1000;
            cost_s--;
        }
        printf("\nFail! Cost %ld s %ld ms.\n\n", cost_s, cost_ms);
    }
    fclose(fp);
    system("pause");
    return 0;
}
