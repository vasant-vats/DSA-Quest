/*
 * data_structures.c (ANMOL'S MODULE)
 * NO CHANGES NEEDED.
 * This is pure data logic for lists and trees.
 */

#include "game.h"

// --- Inventory (Linked List) Functions ---

TreasureNode* add_treasure_to_inventory(TreasureNode* inventory_list, const char* name, int value) {
    TreasureNode* new_treasure = (TreasureNode*)malloc(sizeof(TreasureNode));
    strcpy(new_treasure->name, name);
    new_treasure->value = value;
    new_treasure->next = inventory_list;
    // We will print the message in the GUI, not here.
    return new_treasure;
}

void free_inventory_list(TreasureNode* inventory_list) {
    while (inventory_list != NULL) {
        TreasureNode* node_to_free = inventory_list;
        inventory_list = inventory_list->next;
        free(node_to_free);
    }
}

// --- Monster (Binary Search Tree) Functions ---

MonsterNode* insert_monster_into_bst(MonsterNode* monster_tree, int id, const char* name, int power) {
    if (monster_tree == NULL) {
        MonsterNode* new_monster = (MonsterNode*)malloc(sizeof(MonsterNode));
        new_monster->id = id;
        strcpy(new_monster->name, name);
        new_monster->power = power;
        new_monster->left = NULL;
        new_monster->right = NULL;
        return new_monster;
    }

    if (id < monster_tree->id) {
        monster_tree->left = insert_monster_into_bst(monster_tree->left, id, name, power);
    } 
    else if (id > monster_tree->id) {
        monster_tree->right = insert_monster_into_bst(monster_tree->right, id, name, power);
    }
    return monster_tree;
}

MonsterNode* find_monster_in_bst(MonsterNode* monster_tree, int id) {
    if (monster_tree == NULL || monster_tree->id == id) {
        return monster_tree;
    }
    if (id < monster_tree->id) {
        return find_monster_in_bst(monster_tree->left, id);
    } 
    else {
        return find_monster_in_bst(monster_tree->right, id);
    }
}

void free_monster_bst(MonsterNode* tree_node) {
    if (tree_node == NULL) return;
    free_monster_bst(tree_node->left);
    free_monster_bst(tree_node->right);
    free(tree_node);
}