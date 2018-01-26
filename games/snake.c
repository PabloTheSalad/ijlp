#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define COORDERR -1
#define DELAY 3
#define STARTSNAKE 5

typedef struct Point {
    int x;
    int y;
    struct Point *next;
} Point;

typedef struct {
    Point *h; //head
    int l; //length
    int d; //direction
} Snake;

typedef struct {
    int M;
    int N;
    Snake s;
} Field;

int stdy, stdx, maxy, maxx, end, score = 0;
//Point *food[FOODMAX];
Point *food = NULL;

Point setpoint(int x, int y, Point *next);
Field setfield(int M, int N, Snake s);
Snake setsnake(Point p, int l);
int chpoint(Field *f, char key);
void printfield(Field *f);
void step(Field *f, int dy, int dx);
int startgame(Field *f);
int printsnake(Field *f);
int menu(void);
void setfood(Field *f);
void printfood(Field *f);
Point *checkpoint(Field *f, int x, int y);
void eatfood(Field *f, Point *p);
int issnake(Field *f, int x, int y);
int chpointdirect(Field *f, char key);
void directstep(Field *f);

int main() {
    srand(time(0));
    char M, N;
    int err, numbof = 0;
    M = N = 10;
    initscr(); //переход в ncurses режим
    curs_set(0); //отключение курсора
    getmaxyx(stdscr, maxy, maxx);
    stdy = (maxy-M)/2;
    stdx = (maxx-N)/2;
    noecho();
    halfdelay(DELAY);
    Point p; Snake s; Field f; 
    while(true) {
        int m;
        p = setpoint(5, 5, NULL);
        s = setsnake(p, STARTSNAKE);
        f = setfield(M, N, s);
        end = 0;
        if ((m = menu()) == 1)
            startgame(&f);
        else if (m == -1) break;
    }
    endwin(); //выход из curses режима
    return 0;
}

int startgame(Field *f) {
    while(true) {
        int err, numbof;
        char key;
        clear();
        //err = chpoint(f, key);
        if (key != ERR) {
            sleep(0.5);
        }
        err = chpointdirect(f, key);
        directstep(f);
        Point *pnt = f->s.h;
        move(stdy+f->M, 0);
        printw("%d %d %d %d %d %d\n", f->M, f->N, f->s.h->x, f->s.h->y, maxy, maxx);
        if (err == COORDERR) printw("bad key!\n");
        else printw("last key:%c\n", key);
        printw("snake: %d direct:%d\n", numbof, f->s.d);
        printw("time: %d\n", time(0));
        int numb = 0;
            while (true) {
                printw("x:%d y:%d\n", pnt->x, pnt->y);
                if (pnt->next != NULL) {
                    pnt = pnt->next;
                    numb++;
                    numbof = numb;
                }
                else
                    break;
            }
        printw("food\n");
        if (food == NULL) {
            food = malloc(sizeof(Point));
            *food = setpoint(10,10,NULL);
            for(int i = 0; i < 4; i++) {
                setfood(f);
            }
        }
        printfield(f);
        if (end != 0) {
            score = numbof-STARTSNAKE;
            return 0;
        }
        refresh();
        key = getch();
    }
    return 0;
}

Field setfield(int M, int N, Snake s) {
    Field f;
    f.M = M;
    f.N = N;
    f.s = s;
    return f;
}

Point setpoint(int x, int y, Point *next) {
    Point p;
    p.y = x;
    p.x = y;
    p.next = next;
    return p;
}

Snake setsnake(Point h, int l) {
    Snake s;
    Point *p;
    s.h = malloc(sizeof(Point));
    *s.h = setpoint(h.x, h.y, NULL);
    s.l = l;
    p = s.h;
    for (int i = 0; i < l-1; i++) {
         p->next = malloc(sizeof(Point));
         if (p->next == NULL) exit(1);
         *(p->next) = setpoint(p->y,p->x-1, NULL);
         p = p->next;
    }
    return s;
}

int chpoint(Field *f, char key) {
    switch (key) {
        case 'w': step(f, 0, -1); break;
        case 's': step(f, 0, 1); break;
        case 'a': step(f, -1, 0); break;
        case 'd': step(f, 1, 0); break;
        default: return COORDERR; break;
    }
    return 0;
}

int chpointdirect(Field *f, char key) {
    switch (key) {
        case 'w': f->s.d = 1; break;
        case 's': f->s.d = 3; break;
        case 'a': f->s.d = 4; break;
        case 'd': f->s.d = 2; break;
        default: return COORDERR; break;
    }
    return 0;
}

void directstep(Field *f) {
    switch(f->s.d) {
        case 1: step(f, 0, -1); break;
        case 3: step(f, 0, 1); break;
        case 4: step(f, -1, 0); break;
        case 2: step(f, 1, 0); break;
        default: return; break;
    }
}

void step(Field *f, int dy, int dx) {
    Point *p, *pred;
    p = f->s.h;
    Point *cp = checkpoint(f, f->s.h->x+dx, f->s.h->y+dy);
    if ( cp == -1) {
        end = 1;
        return;
    } else if (cp == NULL) 
        while(true) {
            if (p->next != NULL) {
                pred = p;
                p = p->next;
            }
            else {
                pred->next = NULL;
                p->next = f->s.h;
                p->x = f->s.h->x + dx;
                p->y = f->s.h->y + dy;
                f->s.h = p;
                break;
            }
    } else eatfood(f, cp);
}

void eatfood(Field *f, Point *p) {
    Point *new = p->next;
    p->next = p->next->next;
    new->next = f->s.h;
    f->s.h = new;
    setfood(f);
}

Point *checkpoint(Field *f, int x, int y) {
    if (x < 0 || x >= f->M || y < 0 || y >= f->N)
        return -1;
    Point *h = f->s.h;
    do {
        if(x == h->x && y == h->y) return -1;
    } while((h = h->next) != NULL);
    Point *fo = food, *pred;
    pred = food;
    do {
        if(x == fo->x && y == fo->y) return pred;
        pred = fo;
    } while((fo = fo->next) != NULL);

    return NULL;
}

void printfield(Field *f) {
    int i, j;
    move(stdy,stdx);
    for (i = 0; i < f->M; i++) {
        for (j = 0; j < f->N; j++) {
            //if (i == f->s.h->x && j == f->s.h->y)
               //addch('#'|A_BOLD);
            //else
                addch('0');
            addch(' ');
        }
        addch('\n');
        move(stdy+i+1,stdx);
    }
    printfood(f);
    printsnake(f);
}

int printsnake(Field *f) {
    Snake *s = &(f->s);
    Point *p = s->h;
    while(true) {
        if (p->x >= 0 && p->x < f->M && p->y >= 0 && p->y < f->N) {
            move(stdy+(p->x),stdx+(p->y)*2);
            addch('#'|A_BOLD);
        } else {
            end = 1;
            return 0;
        }
        if (p->next != NULL) p = p->next;
        else break;
    }
    return 0;
}

void setfood(Field *f) {
    Point *p, *pf;
    int x, y;
    p = malloc(sizeof(Point));
    x = rand()%(f->N);
    y = rand()%(f->M);
    while (issnake(f, x, y)) {
        move(11,10);
        x = rand()%(f->N);
        y = rand()%(f->M);
    }
    *p = setpoint(x, y, NULL);
    pf = food;
    if (pf != NULL) {
        while(pf->next != NULL) pf = pf->next;
        pf->next = p;
    }
    else
        food = p;
}

int issnake(Field *f, int x, int y) {
    Point *p = f->s.h;
    do {
        if (y == p->x && x == p->y) return 1;
    } while((p = p->next) != NULL);
    return 0;
}

void printfood(Field *f) {
    Point *pf;
    pf = food;
    while (pf != NULL) {
        int x, y;
        x = stdx+(pf->y)*2;
        y = stdy+pf->x;
        if (x >= 0 || x < f->M || y >= 0 || y < f->N) {
            move(y, x);
            addch('*');
        }
        pf = pf->next;
    }
}

int menu(void) {
    char *m_items[] = {"Game", "Settings", "Exit"};
    int minum = 3, choice = 0;
    keypad(stdscr, true);
    while(true) {
        clear();
        if (score != 0) {
            move(0,0);
            printw("Score: %d", score);
        }
        for(int i = 0; i < minum; i++) {
            int x, y;
            x = (maxx-strlen(m_items[i]))/2;
            y = i+maxy/2;
            move(y,x);
            if (choice == i) addch('>');
            else addch(' ');
            printw("%s\n", m_items[i]);
        }
        switch(getch()) {
            case KEY_UP: if (choice > 0) choice--; break;
            case KEY_DOWN: if (choice < minum-1) choice++; break;
            case '\n':
                switch(choice) {
                    case 0: keypad(stdscr, false); return 1;
                    case 1: move(0,0); printw("Soon..."); break;
                    case 2: endwin(); return -1;
                }
                break;
            default:
                break;
        }
    }
}
