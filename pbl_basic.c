#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <conio.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#define getch() getchar()
#endif

#define MAP_WIDTH 15
#define MAP_HEIGHT 10
#define MAX_LEADERBOARD 10
#define SAVE_FILE "dungeon_save.dat"
#define LEADERBOARD_FILE "leaderboard.txt"

#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_CYAN    "\x1b[36m"

// --- Data Structure Definitions ---
typedef struct TreasureNode {
    char name[50];
    int value;
    struct TreasureNode* next;
} TreasureNode;

typedef struct MonsterNode {
    int id;
    char name[50];
    int power;
    struct MonsterNode *left, *right;
} MonsterNode;

typedef struct {
    int x, y, health, score;
} Player;

typedef struct {
    char name[50];
    int score;
} LeaderboardEntry;

typedef struct QNode {
    int x, y, dist;
    struct QNode* next;
} QNode;

typedef struct {
    QNode *front, *rear;
} Queue;

char map[MAP_HEIGHT][MAP_WIDTH + 1];

// --- Forward Declarations ---
void runGame(Player player, TreasureNode* inventory);
TreasureNode* loadGame(Player* player);
void displayLeaderboard();

// --- Map & Pathfinding (Graph & BFS) ---
void initializeMap(Player* player) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = (y == 0 || y == MAP_HEIGHT - 1 || x == 0 || x == MAP_WIDTH - 1) ? '#' : '.';
        }
        map[y][MAP_WIDTH] = '\0';
    }
    player->x = 1;
    player->y = 1;
    map[player->y][player->x] = 'P';

    char items[] = "ETTTTMMMMM";
    for (int i = 0; i < 10; i++) {
        int x, y;
        do {
            x = rand() % (MAP_WIDTH - 2) + 1;
            y = rand() % (MAP_HEIGHT - 2) + 1;
        } while (map[y][x] != '.');
        map[y][x] = items[i];
    }
}

int findShortestPathBFS(int startX, int startY) {
    int visited[MAP_HEIGHT][MAP_WIDTH] = {0};
    Queue q = {NULL, NULL};
    QNode* q_head = (QNode*)malloc(sizeof(QNode));
    q_head->x = startX; q_head->y = startY; q_head->dist = 0; q_head->next = NULL;
    q.front = q.rear = q_head;
    visited[startY][startX] = 1;

    int rowNum[] = {-1, 1, 0, 0};
    int colNum[] = {0, 0, -1, 1};

    while (q.front != NULL) {
        QNode* curr = q.front;
        q.front = q.front->next;
        if (map[curr->y][curr->x] == 'E') {
            int dist = curr->dist;
            free(curr);
            while(q.front) { QNode* temp = q.front; q.front = q.front->next; free(temp); }
            return dist;
        }
        for (int i = 0; i < 4; i++) {
            int newX = curr->x + colNum[i], newY = curr->y + rowNum[i];
            if (newY >= 0 && newY < MAP_HEIGHT && newX >= 0 && newX < MAP_WIDTH && map[newY][newX] != '#' && !visited[newY][newX]) {
                visited[newY][newX] = 1;
                QNode* newNode = (QNode*)malloc(sizeof(QNode));
                newNode->x = newX; newNode->y = newY; newNode->dist = curr->dist + 1; newNode->next = NULL;
                q.rear->next = newNode;
                q.rear = newNode;
            }
        }
        free(curr);
    }
    return -1;
}

// --- Data Structure Management (Linked List & BST) ---
TreasureNode* addTreasureToInventory(TreasureNode* head, const char* name, int value) {
    TreasureNode* newNode = (TreasureNode*)malloc(sizeof(TreasureNode));
    strcpy(newNode->name, name);
    newNode->value = value;
    newNode->next = head;
    printf(COLOR_YELLOW "You collected: %s! (+%d Score)\n" COLOR_RESET, name, value);
    return newNode;
}

MonsterNode* insertMonsterBST(MonsterNode* root, int id, const char* name, int power) {
    if (root == NULL) {
        MonsterNode* newNode = (MonsterNode*)malloc(sizeof(MonsterNode));
        newNode->id = id; strcpy(newNode->name, name); newNode->power = power;
        newNode->left = newNode->right = NULL;
        return newNode;
    }
    if (id < root->id) root->left = insertMonsterBST(root->left, id, name, power);
    else if (id > root->id) root->right = insertMonsterBST(root->right, id, name, power);
    return root;
}

MonsterNode* findMonsterInBST(MonsterNode* root, int id) {
    if (root == NULL || root->id == id) return root;
    return (id < root->id) ? findMonsterInBST(root->left, id) : findMonsterInBST(root->right, id);
}

void freeInventory(TreasureNode* head) {
    while (head != NULL) {
        TreasureNode* temp = head;
        head = head->next;
        free(temp);
    }
}

void freeBST(MonsterNode* root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    free(root);
}

// --- Display & UI ---
void displayMap() {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            switch (map[y][x]) {
                case 'P': printf("%s%c%s", COLOR_GREEN, 'P', COLOR_RESET); break;
                case 'M': printf("%s%c%s", COLOR_RED, 'M', COLOR_RESET); break;
                case 'T': printf("%s%c%s", COLOR_YELLOW, 'T', COLOR_RESET); break;
                case 'E': printf("%s%c%s", COLOR_BLUE, 'E', COLOR_RESET); break;
                case '#': printf("%s%c%s", COLOR_CYAN, '#', COLOR_RESET); break;
                default:  printf("%c", map[y][x]);
            }
        }
        printf("\n");
    }
}

void displayHUD(Player* player, TreasureNode* inventory) {
    printf("---------------------------------------\n");
    printf("Health: %s%d%s | Score: %s%d%s\n", COLOR_GREEN, player->health, COLOR_RESET, COLOR_YELLOW, player->score, COLOR_RESET);
    printf("Inventory: ");
    if (inventory == NULL) printf("Empty");
    else for (TreasureNode* curr = inventory; curr != NULL; curr = curr->next) printf("[%s] ", curr->name);
    printf("\n---------------------------------------\n");
}

// --- File Handling & Leaderboard (Quick Sort) ---
int saveGame(Player* player, TreasureNode* inventory) {
    FILE* f = fopen(SAVE_FILE, "wb");
    if (!f) return 0;
    fwrite(player, sizeof(Player), 1, f);
    fwrite(map, sizeof(char), sizeof(map), f);
    int itemCount = 0;
    for (TreasureNode* curr = inventory; curr; curr = curr->next) itemCount++;
    fwrite(&itemCount, sizeof(int), 1, f);
    for (TreasureNode* curr = inventory; curr; curr = curr->next) fwrite(curr, sizeof(TreasureNode), 1, f);
    fclose(f);
    return 1;
}

TreasureNode* loadGame(Player* player) {
    FILE* f = fopen(SAVE_FILE, "rb");
    if (!f) return NULL;
    fread(player, sizeof(Player), 1, f);
    fread(map, sizeof(char), sizeof(map), f);
    int itemCount = 0;
    fread(&itemCount, sizeof(int), 1, f);
    TreasureNode* head = NULL;
    for (int i = 0; i < itemCount; i++) {
        TreasureNode* temp = (TreasureNode*)malloc(sizeof(TreasureNode));
        fread(temp, sizeof(TreasureNode), 1, f);
        temp->next = head;
        head = temp;
    }
    fclose(f);
    return head;
}

void swap(LeaderboardEntry* a, LeaderboardEntry* b) { LeaderboardEntry t = *a; *a = *b; *b = t; }

int partition(LeaderboardEntry arr[], int low, int high) {
    int pivot = arr[high].score;
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j].score > pivot) { i++; swap(&arr[i], &arr[j]); }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSort(LeaderboardEntry arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void updateAndDisplayLeaderboard(Player* player) {
    LeaderboardEntry entries[MAX_LEADERBOARD + 1];
    int count = 0;
    FILE* f = fopen(LEADERBOARD_FILE, "r");
    if (f) {
        while (count < MAX_LEADERBOARD && fscanf(f, "%49s %d", entries[count].name, &entries[count].score) != EOF) count++;
        fclose(f);
    }
    printf("Enter your name: ");
    scanf("%49s", entries[count].name);
    entries[count].score = player->score;
    count++;
    quickSort(entries, 0, count - 1);

    f = fopen(LEADERBOARD_FILE, "w");
    if (!f) { printf("Error saving leaderboard!\n"); return; }
    printf("\n--- LEADERBOARD ---\n");
    for (int i = 0; i < count && i < MAX_LEADERBOARD; i++) {
        printf("%d. %s - %d\n", i + 1, entries[i].name, entries[i].score);
        fprintf(f, "%s %d\n", entries[i].name, entries[i].score);
    }
    printf("-------------------\n");
    fclose(f);
}

void displayLeaderboard() {
    system(CLEAR_SCREEN);
    printf("--- LEADERBOARD ---\n");
    FILE* f = fopen(LEADERBOARD_FILE, "r");
    if (!f) { printf("No leaderboard data found.\n"); }
    else {
        char name[50]; int score, rank = 1;
        while (fscanf(f, "%49s %d", name, &score) != EOF) printf("%d. %s - %d\n", rank++, name, score);
        fclose(f);
    }
    printf("-------------------\nPress any key to return...");
    getch(); getch();
}

// --- Gameplay & Main Loop ---
void fightMonster(Player* player, MonsterNode* monsterRoot) {
    MonsterNode* monster = findMonsterInBST(monsterRoot, (rand() % 3) + 1);
    if (!monster) { printf("An unknown entity fades away...\n"); return; }
    printf(COLOR_RED "A wild %s appeared! (Power: %d)\n" COLOR_RESET, monster->name, monster->power);
    int playerAttack = (rand() % 50) + 25;
    printf("Your Attack Power: %d\n", playerAttack);
    if (playerAttack >= monster->power) {
        printf(COLOR_GREEN "You defeated the %s!\n" COLOR_RESET, monster->name);
        player->score += 50;
    } else {
        printf(COLOR_RED "You were defeated! (-25 Health)\n" COLOR_RESET, monster->name);
        player->health -= 25;
    }
}

void runGame(Player player, TreasureNode* inventory) {
    MonsterNode* monsterDB = NULL;
    monsterDB = insertMonsterBST(monsterDB, 1, "Goblin", 30);
    monsterDB = insertMonsterBST(monsterDB, 2, "Orc", 50);
    monsterDB = insertMonsterBST(monsterDB, 3, "Dragon", 70);

    int gameWon = 0;
    while (player.health > 0) {
        system(CLEAR_SCREEN);
        displayMap();
        displayHUD(&player, inventory);
        printf("Enter move (W/A/S/D), 'h' for hint, 'q' to save/quit: ");
        char move = getch();
        
        if (move == 'q' || move == 'Q') {
            printf(saveGame(&player, inventory) ? "Game saved!\n" : "Error saving!\n");
            break;
        } else if (move == 'h' || move == 'H') {
            int dist = findShortestPathBFS(player.x, player.y);
            printf(dist != -1 ? COLOR_CYAN "HINT: Exit is %d steps away!\n" COLOR_RESET : COLOR_RED "HINT: Exit is unreachable!\n" COLOR_RESET);
            printf("Press any key to continue..."); getch(); continue;
        }

        int newX = player.x, newY = player.y;
        if (move == 'w' || move == 'W') newY--;
        else if (move == 's' || move == 'S') newY++;
        else if (move == 'a' || move == 'A') newX--;
        else if (move == 'd' || move == 'D') newX++;

        if (map[newY][newX] != '#') {
            map[player.y][player.x] = '.';
            player.x = newX; player.y = newY;
            char cell = map[player.y][player.x];

            if (cell == 'T') {
                inventory = addTreasureToInventory(inventory, "Gold Coin", 100);
                player.score += 100;
            } else if (cell == 'M') {
                fightMonster(&player, monsterDB);
                getch();
            } else if (cell == 'E') {
                gameWon = 1;
                break;
            }
            map[player.y][player.x] = 'P';
        }
    }
    system(CLEAR_SCREEN);
    if (gameWon) {
        printf(COLOR_GREEN "Congratulations! You escaped!\nFinal Score: %d\n" COLOR_RESET, player.score);
        updateAndDisplayLeaderboard(&player);
    } else if (player.health <= 0) {
        printf(COLOR_RED "GAME OVER! You ran out of health.\n" COLOR_RESET);
    } else { printf("Exiting game.\n"); }
    
    printf("\nPress any key to return to menu...");
    getch(); getch();
    freeInventory(inventory);
    freeBST(monsterDB);
}

int main() {
    srand(time(NULL));
    char choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf(COLOR_CYAN "=== DSA Quest: Dungeon Explorer ===\n" COLOR_RESET);
        printf("1. Start New Game\n2. Load Game\n3. View Leaderboard\n4. Exit\nChoose an option: ");
        scanf(" %c", &choice);

        if (choice == '1') {
            Player player = {0, 0, 100, 0};
            TreasureNode* inventory = NULL;
            initializeMap(&player);
            printf("Instructions:\n> W/A/S/D to move 'P'\n> Collect 'T', fight 'M', find 'E'\nPress any key...");
            getch(); getch();
            runGame(player, inventory);
        } else if (choice == '2') {
            Player player = {0};
            TreasureNode* inventory = loadGame(&player);
            if (inventory == NULL && player.x == 0) {
                 printf("No save file found!\nPress any key..."); getch(); getch();
            } else {
                runGame(player, inventory);
            }
        } else if (choice == '3') {
            displayLeaderboard();
        } else if (choice == '4') {
            printf("Thanks for playing!\n");
            return 0;
        }
    }
    return 0;
}