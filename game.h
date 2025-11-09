/* * game.h
 * This is the main header file for the entire game.
 * It now includes Raylib for GUI.
 */

#ifndef GAME_H
#define GAME_H

// --- Main GUI Library ---
#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Platform-Specific Includes (FOR CONSOLE) ---
#ifdef _WIN32
#include <conio.h>
#define CLEAR_SCREEN "cls"
#else
#include <unistd.h>
#define CLEAR_SCREEN "clear"
#define getch() getchar()
#endif

// --- Game Constants ---
#define MAP_WIDTH 15
#define MAP_HEIGHT 10
#define MAX_LEADERBOARD 10
#define SAVE_FILE "dungeon_save.dat"
#define LEADERBOARD_FILE "leaderboard.txt"

#define MAX_INPUT_NAME 10 // Max letters in player's name

// --- GUI Constants ---
// MODIFIED: Changed TILE_SIZE from 32 to 64
#define TILE_SIZE 64 // Each tile on our map will be 64x64 pixels
#define SCREEN_WIDTH (MAP_WIDTH * TILE_SIZE)  // 15 * 64 = 960
#define SCREEN_HEIGHT (MAP_HEIGHT * TILE_SIZE) // 10 * 64 = 640

// --- Data Structure Definitions (UNCHANGED) ---

// [ANMOL] Structure for the inventory (Linked List)
typedef struct TreasureNode {
    char name[50];
    int value;
    struct TreasureNode* next;
} TreasureNode;

// [ANMOL] Structure for the monsters (Binary Search Tree)
typedef struct MonsterNode {
    int id;
    char name[50];
    int power;
    struct MonsterNode *left, *right;
} MonsterNode;

// [VASANT] Structure for the player
typedef struct {
    int x, y, health, score;
} Player;

// [SHIWANI] Structure for a leaderboard entry
typedef struct {
    char name[50];
    int score;
} LeaderboardEntry;

// [BOOMIKA] Structure for the BFS pathfinding queue
typedef struct QNode {
    int x, y, dist;
    struct QNode* next;
} QNode;

// [BOOMIKA] Structure for the BFS queue
typedef struct {
    QNode *front, *rear;
} Queue;

// --- Global Variables ---
// [BOOMIKA] The main game map.
extern char map[MAP_HEIGHT][MAP_WIDTH + 1];

// [VASANT] Game state machine
typedef enum {
    STATE_MENU,
    STATE_GAMEPLAY,
    STATE_LEADERBOARD,
    STATE_GAME_OVER,  // This will now be for "You Lose"
    STATE_ENTER_NAME, // MODIFIED: New state for "You Win"
    STATE_QUIT
} GameState;

// [VASANT] Global Textures for Graphics
extern Texture2D wall_sprite;
extern Texture2D floor_sprite;
extern Texture2D player_sprite;
extern Texture2D monster_sprite;
extern Texture2D treasure_sprite;
extern Texture2D exit_sprite;

// --- Function Prototypes ---

// Module 1: Boomika (Map & BFS) - UNCHANGED
void initialize_map_and_player(Player* player_to_init);
int find_shortest_path_to_exit(int start_x, int start_y);
Queue create_queue();
void enqueue(Queue* q, int x, int y, int dist);
QNode* dequeue(Queue* q);
int is_queue_empty(Queue* q);

// Module 2: Anmol (Data Structures) - UNCHANGED
TreasureNode* add_treasure_to_inventory(TreasureNode* inventory_list, const char* name, int value);
MonsterNode* insert_monster_into_bst(MonsterNode* monster_tree, int id, const char* name, int power);
MonsterNode* find_monster_in_bst(MonsterNode* monster_tree, int id);
void free_inventory_list(TreasureNode* inventory_list);
void free_monster_bst(MonsterNode* tree_node);

// Module 3: Shiwani (File Handling & Leaderboard)
int save_game_to_file(Player* player, TreasureNode* inventory);
TreasureNode* load_game_from_file(Player* player);
void swap_entries(LeaderboardEntry* a, LeaderboardEntry* b);
int partition_scores(LeaderboardEntry scores_array[], int start_index, int end_index);
void quick_sort_scores(LeaderboardEntry scores_array[], int start_index, int end_index);
int load_leaderboard(LeaderboardEntry entries[]);
void save_new_leaderboard_entry(const char* name, int score);

// Module 4: Vasant (Game Flow & UI)
void DrawMainMenu();
void UpdateMainMenu(GameState* state, Player* player, TreasureNode** inventory, char* name, int* letterCount); // Modified

void DrawGameplay(Player* player, TreasureNode* inventory, const char* message);
void UpdateGameplay(Player* player, TreasureNode** inventory, MonsterNode* monsterDB, GameState* state, char* message, float* messageTimer);

void DrawLeaderboard(LeaderboardEntry entries[], int count);
void UpdateLeaderboard(GameState* state);

void DrawGameOver(Player* player);
void UpdateGameOver(GameState* state);

void DrawEnterName(Player* player, char* name);
void UpdateEnterName(GameState* state, Player* player, char* name, int* letterCount);


#endif // GAME_H