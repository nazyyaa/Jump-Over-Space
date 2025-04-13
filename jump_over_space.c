#include <time.h>
#include <curses.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEVELS 3
#define FRAME_WIDTH 80
#define FRAME_HEIGHT 24
#define GRAVITY_DELAY 100
#define MAX_BONUSES 10
#define TIME_LIMIT 60

typedef struct {
    int x, y;
    int color;
    int gravity;
    int falling;
} Player;

typedef struct {
    int x, y;
    int length;
    int color;
} Lane;

typedef struct {
    int x, y;
    int collected;
} Bonus;

typedef struct {
    int numLanes;
    Lane lanes[8];
    Player player;
    int countOfBonuses;
    Bonus bonuses[MAX_BONUSES];
} Level;

void init() {
    initscr();
    cbreak();
    curs_set(0);
    noecho();
    keypad(stdscr, TRUE);
    start_color();

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
}

void showGameOver() {
    clear();
    mvprintw(FRAME_HEIGHT / 2, (FRAME_WIDTH - 9) / 2, "Game Over");
    refresh();
    napms(2000);
}

void drawPlayer(Level *level) {
    Player *player = &level->player;
    char directionSymbol = (player->gravity > 0) ? 'v' : '^';

    attron(COLOR_PAIR(player->color));
    mvprintw(player->y + (player->gravity > 0 ? 1 : -1), player->x, "%c", directionSymbol);
    attroff(COLOR_PAIR(player->color));
}

void drawLane(Lane *lane) {
    attron(COLOR_PAIR(lane->color));
    for (int i = 0; i < lane->length; i++) {
        mvprintw(lane->y, lane->x + i, "#");
    }
    attroff(COLOR_PAIR(lane->color));
}

void drawLevel(Level *level) {
    for (int i = 0; i < level->numLanes; i++) {
        drawLane(&level->lanes[i]);
    }
}

void drawFrame() {
    attron(COLOR_PAIR(1));
    for (int i = 0; i < FRAME_HEIGHT; i++) {
        mvprintw(i, 0, "|");
        mvprintw(i, FRAME_WIDTH - 1, "|");
    }

    for (int i = 0; i < FRAME_WIDTH; i++) {
        mvprintw(0, i, "-");
        mvprintw(FRAME_HEIGHT - 1, i, "-");
    }
    attroff(COLOR_PAIR(1));
}

int showMenu() {
    int ch, highlight = 0;
    char *menuItems[] = {
            "Play Level 1",
            "Play Level 2",
            "Play Level 3",
            "Quit Game"
    };
    int numItems = sizeof(menuItems) / sizeof(char*);
    int maxWidth = 0;
    for (int i = 0; i < numItems; ++i) {
        int len = strlen(menuItems[i]);
        if (len > maxWidth) maxWidth = len;
    }
    int menuStartX = (FRAME_WIDTH - maxWidth) / 2;
    int menuStartY = (FRAME_HEIGHT - numItems * 2) / 2;
    while (1) {
        clear();
        mvprintw(2, (FRAME_WIDTH - strlen("Welcome to the Game Menu")) / 2, "Welcome to the Game Menu");
        attron(COLOR_PAIR(1));
        mvhline(3, 1, '-', FRAME_WIDTH - 2);
        attroff(COLOR_PAIR(1));
        drawFrame();
        for (int i = 0; i < numItems; ++i) {
            if (i == highlight) {
                attron(A_REVERSE);
            }
            mvprintw(menuStartY + i * 2, menuStartX, "%s", menuItems[i]);
            attroff(A_REVERSE);
        }

        refresh();
        ch = getch();
        switch (ch) {
            case KEY_UP:
                highlight = (highlight - 1 + numItems) % numItems;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % numItems;
                break;
            case 10:
                return highlight;
        }
    }
}

void drawBonus(Bonus *bonus) {
    if (!bonus->collected) {
        attron(COLOR_PAIR(5));
        mvprintw(bonus->y, bonus->x, "*");
        attroff(5);
    }
}

void drawBonuses(Level *level) {
    for (int i = 0; i < level->countOfBonuses; i++) {
        drawBonus(&level->bonuses[i]);
    }
}

void checkBonusCollection(Level *level) {
    for (int i = 0; i < level->countOfBonuses; i++) {
        if (level->player.x == level->bonuses[i].x && level->player.y + level->player.gravity == level->bonuses[i].y && !level->bonuses[i].collected) {
            level->bonuses[i].collected = 1;
        }
    }
}

int allBonusesCollected(Level *level) {
    for (int i = 0; i < level->countOfBonuses; i++) {
        if (!level->bonuses[i].collected) {
            return 0;
        }
    }
    return 1;
}

void showTitleScreen() {
    drawFrame();
    char *titleText = "Created by Nazar";
    char *instructions[] = {
            "How to Play:",
            "Use the arrow keys to move left or right.",
            "Press SPACE to switch direction.",
            "Collect all bonuses to win.",
            "Avoid falling off the platforms!",
            "Press 'q' to exit.",
            "Press any key to start..."
    };
    int numInstructions = sizeof(instructions) / sizeof(char*);
    for (int i = 0; i < numInstructions; ++i) {
        mvprintw((FRAME_HEIGHT / 4) + 2 + i, (FRAME_WIDTH - strlen(instructions[i])) / 2, "%s", instructions[i]);
    }
    refresh();
    getch(); 
    for (int i = 0; i < FRAME_WIDTH -  strlen(titleText) - 5; ++i) {
        clear();
        drawFrame();
        int startPos = FRAME_WIDTH -  strlen(titleText) - 2 - i;
        startPos = startPos < 0 ? 0 : startPos;

        mvprintw(FRAME_HEIGHT / 2 - 1, startPos, "%s", titleText);
        refresh();
        napms(20);

        if (startPos == 0) break;
    }
    refresh();
}
void generateLevel(Level *level, int levelNumber) {
    int minLaneLength = 5; 
    int maxLaneLength = 15;
    level->numLanes = 10 + (levelNumber * 5); 
    if (level->numLanes > sizeof(level->lanes) / sizeof(level->lanes[0])) {
        level->numLanes = sizeof(level->lanes) / sizeof(level->lanes[0]);
    }
    for (int i = 0; i < level->numLanes; ++i) {
        int laneLength = minLaneLength + rand() % (maxLaneLength - minLaneLength + 1);
        int laneX = rand() % (FRAME_WIDTH - laneLength - 2) + 1; 
        int laneY = rand() % (FRAME_HEIGHT - 2) + 1;
        level->lanes[i] = (Lane){laneX, laneY, laneLength, 1 + rand() % 4};
    }

    int randomLaneIndex = rand() % level->numLanes;
    Lane chosenLane = level->lanes[randomLaneIndex];
    level->player = (Player){
            .x = chosenLane.x + chosenLane.length / 2,
            .y = chosenLane.y,
            .color = 1,
            .gravity = 1,
            .falling = 0
    };
    srand(time(NULL));
    level->countOfBonuses = levelNumber * 3;
    for (int i = 0; i < level->countOfBonuses; i++) {
        int isValidPosition;
        do {
            isValidPosition = 1;
            level->bonuses[i].x = rand() % (FRAME_WIDTH - 2) + 1;
            level->bonuses[i].y = (rand() % (FRAME_HEIGHT - 6)) + 3;
            level->bonuses[i].collected = 0;
            for (int j = 0; j < level->numLanes; j++) {
                if (level->bonuses[i].y == level->lanes[j].y &&
                    level->bonuses[i].x >= level->lanes[j].x &&
                    level->bonuses[i].x < (level->lanes[j].x + level->lanes[j].length)) {
                    isValidPosition = 0;
                    break;
                }
            }
        } while (!isValidPosition);
    }
}

void showWinner() {
    char *winnerText = "Winner!";
    int textLength = strlen(winnerText);
    int baseCol = (FRAME_WIDTH - textLength) / 2;
    int colors[] = {1, 2, 3, 4};
    int numColors = sizeof(colors) / sizeof(colors[0]);
    for (int i = 0; i < 10; ++i) {
        clear();
        int colorIndex = i % numColors;
        attron(COLOR_PAIR(colors[colorIndex]));
        int direction = (i % 2 == 0) ? 1 : -1;
        int col = baseCol + direction * (i % 3);
        if (col < 0) col = 0;
        if (col > FRAME_WIDTH - textLength) col = FRAME_WIDTH - textLength;
        mvprintw(FRAME_HEIGHT / 2, col, "%s", winnerText);
        attroff(COLOR_PAIR(colors[colorIndex]));
        refresh();
        napms(200);
    }
    napms(2000);
    clear();
}

int main() {
    init();
    showTitleScreen();
    int selectedLevel = showMenu();
    if (selectedLevel == 3) {
        endwin();
        return 0;
    }
    Level level;
    generateLevel(&level, selectedLevel + 1);
    timeout(0); 
    refresh();
    bool isGameOver = false;
    time_t startTime = time(NULL);
    time_t currentTime;
    int remainingTime;
    int ch;
    while ((ch = getch()) != 'q') {
        clear();
        drawFrame();
        switch (ch) {
            case KEY_LEFT:
                if (level.player.x > 0) level.player.x--;
                break;
            case KEY_RIGHT:
                if (level.player.x < FRAME_WIDTH - 2) level.player.x++;
                break;
            case ' ':     
                level.player.gravity *= -1;
                break;
        }
        int onLane = 0;
        for (int i = 0; i < level.numLanes; i++) {
            if (level.player.x >= level.lanes[i].x &&
                level.player.x < level.lanes[i].x + level.lanes[i].length &&
                ((level.player.gravity > 0 && level.player.y == level.lanes[i].y) ||
                 (level.player.gravity < 0 && level.player.y == level.lanes[i].y))) {
                onLane = 1;
                break;
            }
        }
        currentTime = time(NULL);
        remainingTime = TIME_LIMIT - (currentTime - startTime); 
        if (remainingTime <= 0) {
            showGameOver();
            break;
        }
        if (remainingTime <= 10 && remainingTime % 2 == 0) {
            attron(COLOR_PAIR(1));
        }
        mvprintw(0, (FRAME_WIDTH / 2) - strlen("Time left: 00 s.") / 2, "Time left: %02d s", remainingTime);
        attroff(1);
        if (!onLane) {
            level.player.y += level.player.gravity;
            if (level.player.y <= 0 || level.player.y >= FRAME_HEIGHT - 1) {
                isGameOver = true; 
            }
        }
        checkBonusCollection(&level);
        if (!isGameOver) {
            if (allBonusesCollected(&level)) {
                showWinner(); 
                break;
            }
            drawLevel(&level); 
            drawPlayer(&level); 
            drawBonuses(&level); 
        } else {
            showGameOver(); 
            break;
        }
        refresh(); 
        napms(GRAVITY_DELAY); 
    }
    endwin();
    return 0;
}
