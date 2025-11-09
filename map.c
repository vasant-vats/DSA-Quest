/*
 * map.c (BOOMIKA'S MODULE)
 * NO CHANGES NEEDED.
 * This is pure logic. It doesn't care how the map is drawn.
 */

#include "game.h"

// --- Global Map Definition ---
char map[MAP_HEIGHT][MAP_WIDTH + 1];

// --- Private Function ---
static int get_random_number(int max_value) {
    return rand() % max_value;
}

// --- Map (Graph) Functions ---
void initialize_map_and_player(Player* player_to_init) {
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            if (row == 0 || row == MAP_HEIGHT - 1 || col == 0 || col == MAP_WIDTH - 1) {
                map[row][col] = '#';
            } else {
                map[row][col] = '.'; // Empty space
            }
        }
        map[row][MAP_WIDTH] = '\0';
    }

    player_to_init->x = 1;
    player_to_init->y = 1;
    map[player_to_init->y][player_to_init->x] = 'P';

    char items[] = "ETTTTMMMMM";
    int num_items = 10;

    for (int i = 0; i < num_items; i++) {
        int item_x, item_y;
        do {
            item_x = get_random_number(MAP_WIDTH - 2) + 1;
            item_y = get_random_number(MAP_HEIGHT - 2) + 1;
        } while (map[item_y][item_x] != '.');
        map[item_y][item_x] = items[i];
    }
}

// --- BFS (Pathfinding) Functions ---

Queue create_queue() {
    Queue q;
    q.front = NULL;
    q.rear = NULL;
    return q;
}

void enqueue(Queue* q, int x, int y, int dist) {
    QNode* new_node = (QNode*)malloc(sizeof(QNode));
    new_node->x = x;
    new_node->y = y;
    new_node->dist = dist;
    new_node->next = NULL;

    if (q->rear == NULL) {
        q->front = new_node;
        q->rear = new_node;
        return;
    }
    q->rear->next = new_node;
    q->rear = new_node;
}

QNode* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    QNode* temp_node = q->front;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    return temp_node;
}

int is_queue_empty(Queue* q) {
    return (q->front == NULL);
}

int find_shortest_path_to_exit(int start_x, int start_y) {
    int visited[MAP_HEIGHT][MAP_WIDTH] = {0};
    Queue path_queue = create_queue();
    enqueue(&path_queue, start_x, start_y, 0);
    visited[start_y][start_x] = 1;

    int move_row[] = {-1, 1, 0, 0};
    int move_col[] = {0, 0, -1, 1};

    while (!is_queue_empty(&path_queue)) {
        QNode* current_node = dequeue(&path_queue);

        if (map[current_node->y][current_node->x] == 'E') {
            int distance = current_node->dist;
            free(current_node);
            while(!is_queue_empty(&path_queue)) {
                QNode* temp = dequeue(&path_queue);
                free(temp);
            }
            return distance;
        }

        for (int i = 0; i < 4; i++) {
            int next_row = current_node->y + move_row[i];
            int next_col = current_node->x + move_col[i];

            if (next_row >= 0 && next_row < MAP_HEIGHT &&
                next_col >= 0 && next_col < MAP_WIDTH &&
                map[next_row][next_col] != '#' &&
                !visited[next_row][next_col]) 
            {
                visited[next_row][next_col] = 1;
                enqueue(&path_queue, next_col, next_row, current_node->dist + 1);
            }
        }
        free(current_node);
    }
    return -1;
}