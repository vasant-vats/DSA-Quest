/*
 * file_handling.c (SHIWANI'S MODULE)
 * MODIFIED: Added a dedicated function to save a new leaderboard score.
 */

#include "game.h"

// --- Game Save/Load Functions (UNCHANGED) ---

int save_game_to_file(Player* player, TreasureNode* inventory) {
    FILE* save_file_pointer = fopen(SAVE_FILE, "wb");
    if (!save_file_pointer) return 0;

    fwrite(player, sizeof(Player), 1, save_file_pointer);
    fwrite(map, sizeof(char), sizeof(map), save_file_pointer);

    int num_items = 0;
    for (TreasureNode* current = inventory; current; current = current->next) {
        num_items++;
    }
    fwrite(&num_items, sizeof(int), 1, save_file_pointer);

    for (TreasureNode* current = inventory; current; current = current->next) {
        fwrite(current, sizeof(TreasureNode), 1, save_file_pointer);
    }
    fclose(save_file_pointer);
    return 1;
}

TreasureNode* load_game_from_file(Player* player) {
    FILE* load_file_pointer = fopen(SAVE_FILE, "rb");
    if (!load_file_pointer) return NULL;

    fread(player, sizeof(Player), 1, load_file_pointer);
    fread(map, sizeof(char), sizeof(map), load_file_pointer);

    int num_items = 0;
    fread(&num_items, sizeof(int), 1, load_file_pointer);

    TreasureNode* inventory_list = NULL;
    for (int i = 0; i < num_items; i++) {
        TreasureNode* new_item = (TreasureNode*)malloc(sizeof(TreasureNode));
        fread(new_item, sizeof(TreasureNode), 1, load_file_pointer);
        new_item->next = inventory_list;
        inventory_list = new_item;
    }
    fclose(load_file_pointer);
    return inventory_list;
}

// --- Leaderboard (Quick Sort) Functions (UNCHANGED) ---

void swap_entries(LeaderboardEntry* a, LeaderboardEntry* b) {
    LeaderboardEntry temp = *a;
    *a = *b;
    *b = temp;
}

int partition_scores(LeaderboardEntry scores_array[], int start_index, int end_index) {
    int pivot_score = scores_array[end_index].score;
    int i = (start_index - 1);
    for (int j = start_index; j <= end_index - 1; j++) {
        if (scores_array[j].score > pivot_score) {
            i++;
            swap_entries(&scores_array[i], &scores_array[j]);
        }
    }
    swap_entries(&scores_array[i + 1], &scores_array[end_index]);
    return (i + 1);
}

void quick_sort_scores(LeaderboardEntry scores_array[], int start_index, int end_index) {
    if (start_index < end_index) {
        int partition_index = partition_scores(scores_array, start_index, end_index);
        quick_sort_scores(scores_array, start_index, partition_index - 1);
        quick_sort_scores(scores_array, partition_index + 1, end_index);
    }
}

// --- Leaderboard File Functions ---

// This helper function is unchanged
int load_leaderboard(LeaderboardEntry entries[]) {
    int count = 0;
    FILE* file_ptr = fopen(LEADERBOARD_FILE, "r");
    if (file_ptr) {
        while (count < MAX_LEADERBOARD && 
               fscanf(file_ptr, "%49s %d", entries[count].name, &entries[count].score) != EOF) {
            count++;
        }
        fclose(file_ptr);
    }
    return count;
}

// MODIFIED: New function to handle saving the score from the GUI
void save_new_leaderboard_entry(const char* name, int score) {
    LeaderboardEntry leaderboard_list[MAX_LEADERBOARD + 1];
    int count = 0;

    // Read existing scores
    count = load_leaderboard(leaderboard_list);

    // Add the new score
    strcpy(leaderboard_list[count].name, name);
    leaderboard_list[count].score = score;
    count++;

    // Sort all entries
    quick_sort_scores(leaderboard_list, 0, count - 1);

    // Write top scores back to the file
    FILE* file_ptr = fopen(LEADERBOARD_FILE, "w");
    if (!file_ptr) {
        return; // Failed to save
    }

    for (int i = 0; i < count && i < MAX_LEADERBOARD; i++) {
        fprintf(file_ptr, "%s %d\n", leaderboard_list[i].name, leaderboard_list[i].score);
    }
    fclose(file_ptr);
}