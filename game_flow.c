/*
 * game_flow.c (VASANT'S MODULE - PART 1)
 * MODIFIED: Added a text-entry screen.
 */

#include "game.h"
#include <string.h> // For sprintf

// --- Menu State ---
void DrawMainMenu() {
    ClearBackground(BLACK);
    DrawText("DSA Quest", SCREEN_WIDTH / 2 - MeasureText("DSA Quest", 40) / 2, 80, 40, YELLOW);
    
    DrawText("1. New Game", SCREEN_WIDTH / 2 - MeasureText("1. New Game", 20) / 2, 150, 20, WHITE);
    DrawText("2. Load Game", SCREEN_WIDTH / 2 - MeasureText("2. Load Game", 20) / 2, 180, 20, WHITE);
    DrawText("3. Leaderboard", SCREEN_WIDTH / 2 - MeasureText("3. Leaderboard", 20) / 2, 210, 20, WHITE);
    DrawText("4. Exit", SCREEN_WIDTH / 2 - MeasureText("4. Exit", 20) / 2, 240, 20, WHITE);
}

// MODIFIED: Added reset for name and letterCount
void UpdateMainMenu(GameState* state, Player* player, TreasureNode** inventory, char* name, int* letterCount) {
    if (IsKeyPressed(KEY_ONE)) {
        player->health = 100;
        player->score = 0;
        free_inventory_list(*inventory);
        *inventory = NULL;
        *letterCount = 0; // Reset name
        name[0] = '\0';   // Reset name

        initialize_map_and_player(player);
        *state = STATE_GAMEPLAY;
    }
    else if (IsKeyPressed(KEY_TWO)) {
        free_inventory_list(*inventory);
        *inventory = load_game_from_file(player);
        *letterCount = 0; // Reset name
        name[0] = '\0';   // Reset name
        
        if (player->health > 0) { 
            *state = STATE_GAMEPLAY;
        }
    }
    else if (IsKeyPressed(KEY_THREE)) {
        *state = STATE_LEADERBOARD;
    }
    else if (IsKeyPressed(KEY_FOUR) || IsKeyPressed(KEY_Q)) {
        *state = STATE_QUIT;
    }
}


// --- Gameplay State ---
void DrawGameplay(Player* player, TreasureNode* inventory, const char* message) {
    ClearBackground(BLACK);
    
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            DrawTexture(floor_sprite, x * TILE_SIZE, y * TILE_SIZE, WHITE);
            switch (map[y][x]) {
                case '#': DrawTexture(wall_sprite, x * TILE_SIZE, y * TILE_SIZE, WHITE); break;
                case 'P': DrawTexture(player_sprite, x * TILE_SIZE, y * TILE_SIZE, WHITE); break;
                case 'M': DrawTexture(monster_sprite, x * TILE_SIZE, y * TILE_SIZE, WHITE); break;
                case 'T': DrawTexture(treasure_sprite, x * TILE_SIZE, y * TILE_SIZE, WHITE); break;
                case 'E': DrawTexture(exit_sprite, x * TILE_SIZE, y * TILE_SIZE, WHITE); break;
            }
        }
    }
    
    char hud_text[100];
    sprintf(hud_text, "Health: %d | Score: %d", player->health, player->score);
    DrawRectangle(0, 0, SCREEN_WIDTH, TILE_SIZE - 10,Fade(BLACK, 0.8f));
    DrawText(hud_text, 10, 5, 20, GREEN);
    
    if (message[0] != '\0') {
        DrawRectangle(0, SCREEN_HEIGHT - 40, SCREEN_WIDTH, 40, Fade(BLACK, 0.8f));
        DrawText(message, 10, SCREEN_HEIGHT - 30, 20, YELLOW);
    }
}

void UpdateGameplay(Player* player, TreasureNode** inventory, MonsterNode* monsterDB, GameState* state, char* message, float* messageTimer) 
{
    int newX = player->x;
    int newY = player->y;
    bool key_pressed = false;

    if (*messageTimer > 0) {
        *messageTimer -= GetFrameTime();
    } else {
        message[0] = '\0';
    }

    if (IsKeyPressed(KEY_W)) { newY--; key_pressed = true; }
    else if (IsKeyPressed(KEY_S)) { newY++; key_pressed = true; }
    else if (IsKeyPressed(KEY_A)) { newX--; key_pressed = true; }
    else if (IsKeyPressed(KEY_D)) { newX++; key_pressed = true; }
    else if (IsKeyPressed(KEY_H)) {
        int dist = find_shortest_path_to_exit(player->x, player->y);
        sprintf(message, "HINT: Exit is %d steps away!", dist);
        *messageTimer = 2.0f;
        key_pressed = false;
    }
    else if (IsKeyPressed(KEY_Q)) {
        save_game_to_file(player, *inventory);
        *state = STATE_MENU;
        return;
    }

    if (key_pressed) {
        if (map[newY][newX] != '#') {
            map[player->y][player->x] = '.';
            player->x = newX;
            player->y = newY;
            
            char cell = map[player->y][player->x];
            
            if (cell == 'T') {
                *inventory = add_treasure_to_inventory(*inventory, "Gold Coin", 100);
                player->score += 100;
                sprintf(message, "You found a Gold Coin! +100 Score!");
                *messageTimer = 2.0f;
            }
            else if (cell == 'M') {
                MonsterNode* monster = find_monster_in_bst(monsterDB, (rand() % 3) + 1);
                int playerAttack = (rand() % 50) + 25;
                if (playerAttack >= monster->power) {
                    player->score += 50;
                    sprintf(message, "You beat a %s! +50 Score!", monster->name);
                    *messageTimer = 2.0f;
                } else {
                    player->health -= 25;
                    sprintf(message, "You lost to a %s! -25 Health.", monster->name);
                    *messageTimer = 2.0f;
                }
            }
            else if (cell == 'E') {
                // MODIFIED: Go to ENTER_NAME state, not GAME_OVER
                *state = STATE_ENTER_NAME; 
            }
            
            map[player->y][player->x] = 'P';
            
            // MODIFIED: Only go to GAME_OVER if health is 0 (a loss)
            if (player->health <= 0) {
                *state = STATE_GAME_OVER;
            }
        }
    }
}


// --- Leaderboard State ---
void DrawLeaderboard(LeaderboardEntry entries[], int count) {
    ClearBackground(BLACK);
    DrawText("Leaderboard", SCREEN_WIDTH / 2 - MeasureText("Leaderboard", 40) / 2, 20, 40, YELLOW);
    if (count == 0) {
        DrawText("No scores yet!", SCREEN_WIDTH / 2 - MeasureText("No scores yet!", 20) / 2, 100, 20, GRAY);
    }
    for (int i = 0; i < count; i++) {
        char entry_text[100];
        sprintf(entry_text, "%d. %s - %d", i + 1, entries[i].name, entries[i].score);
        DrawText(entry_text, 50, 80 + (i * 25), 20, WHITE);
    }
    DrawText("Press [Enter] to return", 50, SCREEN_HEIGHT - 30, 20, GRAY);
}

void UpdateLeaderboard(GameState* state) {
    if (IsKeyPressed(KEY_ENTER)) {
        *state = STATE_MENU;
    }
}

// --- Game Over State (Now only for Losing) ---
void DrawGameOver(Player* player) {
    ClearBackground(BLACK);
    // MODIFIED: Removed win condition text
    DrawText("GAME OVER", SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 40) / 2, 80, 40, RED);
    
    char score_text[50];
    sprintf(score_text, "Final Score: %d", player->score);
    DrawText(score_text, SCREEN_WIDTH / 2 - MeasureText(score_text, 20) / 2, 150, 20, YELLOW);
    
    DrawText("Press [Enter] to return to menu", SCREEN_WIDTH / 2 - MeasureText("Press [Enter] to return to menu", 20) / 2, 220, 20, GRAY);
}

void UpdateGameOver(GameState* state) {
    if (IsKeyPressed(KEY_ENTER)) {
        *state = STATE_MENU;
    }
}

// --- MODIFIED: New State Functions for Name Entry ---

void DrawEnterName(Player* player, char* name) {
    ClearBackground(BLACK);
    DrawText("YOU ESCAPED!", SCREEN_WIDTH / 2 - MeasureText("YOU ESCAPED!", 40) / 2, 40, 40, GREEN);
    
    char score_text[50];
    sprintf(score_text, "Final Score: %d", player->score);
    DrawText(score_text, SCREEN_WIDTH / 2 - MeasureText(score_text, 20) / 2, 100, 20, YELLOW);

    DrawText("Enter Your Name:", SCREEN_WIDTH / 2 - MeasureText("Enter Your Name:", 20) / 2, 150, 20, WHITE);

    // Draw the text box
    Rectangle textBox = { SCREEN_WIDTH/2 - 100, 180, 200, 40 };
    DrawRectangleRec(textBox, LIGHTGRAY);
    DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);

    // Draw the text being typed
    DrawText(name, (int)textBox.x + 10, (int)textBox.y + 10, 20, BLACK);

    // Draw a blinking cursor
    // (int)(GetTime() * 2.0f) % 2 checks if the second is even or odd
    if (((int)(GetTime() * 2.0f) % 2) == 0) {
        int textWidth = MeasureText(name, 20);
        DrawText("_", (int)textBox.x + 10 + textWidth, (int)textBox.y + 12, 20, BLACK);
    }
    
    DrawText("Press [Enter] to save", SCREEN_WIDTH / 2 - MeasureText("Press [Enter] to save", 20) / 2, 240, 20, GRAY);
}

void UpdateEnterName(GameState* state, Player* player, char* name, int* letterCount) {
    
    // GetCharPressed() returns the ASCII value of the key
    int key = GetCharPressed();
    
    // Check if a printable character was pressed (e.g., a-z, A-Z, 0-9)
    while (key > 0) {
        if ((key >= 32) && (key <= 125) && (*letterCount < MAX_INPUT_NAME)) {
            name[*letterCount] = (char)key;
            name[*letterCount + 1] = '\0'; // Add null terminator
            (*letterCount)++;
        }
        key = GetCharPressed(); // Check for next character
    }

    // Check for Backspace
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (*letterCount > 0) {
            (*letterCount)--;
            name[*letterCount] = '\0';
        }
    }

    // Check for Enter
    if (IsKeyPressed(KEY_ENTER)) {
        if (*letterCount > 0) { // Only save if they typed a name
            save_new_leaderboard_entry(name, player->score);
            *state = STATE_LEADERBOARD;
        }
    }
}