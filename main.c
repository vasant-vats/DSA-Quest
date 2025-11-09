/*
 * main.c (VASANT'S MODULE - PART 2)
 * MODIFIED: Added variables and state for name entry.
 */

#include "game.h"

// --- Define Global Textures ---
Texture2D wall_sprite;
Texture2D floor_sprite;
Texture2D player_sprite;
Texture2D monster_sprite;
Texture2D treasure_sprite;
Texture2D exit_sprite;

// --- Asset Loading Functions ---
void LoadGameTextures() {
    wall_sprite = LoadTexture("sprites/wall.png");
    floor_sprite = LoadTexture("sprites/floor.png");
    player_sprite = LoadTexture("sprites/player.png");
    monster_sprite = LoadTexture("sprites/monster.png");
    treasure_sprite = LoadTexture("sprites/treasure.png");
    exit_sprite = LoadTexture("sprites/exit.png");
}

void UnloadGameTextures() {
    UnloadTexture(wall_sprite);
    UnloadTexture(floor_sprite);
    UnloadTexture(player_sprite);
    UnloadTexture(monster_sprite);
    UnloadTexture(treasure_sprite);
    UnloadTexture(exit_sprite);
}


// --- Main Game Entry Point ---
int main(void)
{
    // 1. Initialize Window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "DSA Quest - GUI");
    SetTargetFPS(60);
    srand(time(NULL));

    // 2. Load Assets
    LoadGameTextures();

    // 3. Initialize Game Variables
    GameState current_state = STATE_MENU;
    Player player = {0, 0, 100, 0};
    TreasureNode* inventory = NULL;
    
    MonsterNode* monsterDB = NULL;
    monsterDB = insert_monster_into_bst(monsterDB, 1, "Goblin", 30);
    monsterDB = insert_monster_into_bst(monsterDB, 2, "Orc", 50);
    monsterDB = insert_monster_into_bst(monsterDB, 3, "Dragon", 70);
    
    LeaderboardEntry leaderboard[MAX_LEADERBOARD];
    int leaderboard_count = 0;
    
    char game_message[100] = {0};
    float messageTimer = 0.0f; 

    // MODIFIED: Add variables for name input
    char playerName[MAX_INPUT_NAME + 1] = {0}; // +1 for null terminator
    int letterCount = 0;

    // 4. Main Game Loop
    while (current_state != STATE_QUIT && !WindowShouldClose())
    {
        // --- UPDATE (Logic) ---
        switch (current_state) {
            case STATE_MENU:
                // MODIFIED: Pass name variables to reset them
                UpdateMainMenu(&current_state, &player, &inventory, playerName, &letterCount);
                break;
            case STATE_GAMEPLAY:
                UpdateGameplay(&player, &inventory, monsterDB, &current_state, game_message, &messageTimer);
                break;
            case STATE_LEADERBOARD:
                leaderboard_count = load_leaderboard(leaderboard);
                UpdateLeaderboard(&current_state);
                break;
            case STATE_GAME_OVER:
                UpdateGameOver(&current_state);
                break;
            case STATE_ENTER_NAME: // MODIFIED: Added new state
                UpdateEnterName(&current_state, &player, playerName, &letterCount);
                break;
            default: break;
        }

        // --- DRAW (Graphics) ---
        BeginDrawing();
        
        switch (current_state) {
            case STATE_MENU:
                DrawMainMenu();
                break;
            case STATE_GAMEPLAY:
                DrawGameplay(&player, inventory, game_message);
                break;
            case STATE_LEADERBOARD:
                DrawLeaderboard(leaderboard, leaderboard_count);
                break;
            case STATE_GAME_OVER:
                DrawGameOver(&player);
                break;
            case STATE_ENTER_NAME: // MODIFIED: Added new state
                DrawEnterName(&player, playerName);
                break;
            default: break;
        }
        
        EndDrawing();
    }

    // 5. De-Initialize
    UnloadGameTextures();
    free_inventory_list(inventory);
    free_monster_bst(monsterDB);
    CloseWindow();

    return 0;
}