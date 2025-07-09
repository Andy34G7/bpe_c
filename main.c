#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define VOCABULARY (256+25) // 256 ASCII chars + 25 merges, increase it for higher compression

typedef struct{
    int left;
    int right;
    int left_pos;
} byte_pair;

typedef struct{
    int pair[2];
    int token;
} token; //merged pair replaced by a token

typedef struct{
    token tokens[VOCABULARY-256];
    int num_tokens;
} state;

void get_pairs(const int* tokens, int len, int **pair_count){
    for (int i=0; i <  VOCABULARY; ++i){
        for (int j=0; j <VOCABULARY; ++j){
            pair_count[i][j] = 0;
        }
    }
    for (int i = 0; i < len - 1; ++i){
        pair_count[tokens[i]][tokens[i+1]]++;
    }
}

byte_pair most_freq(int **pair_count){
    byte_pair best = {-1,-1,-1};
    int max_count =0;
    for (int i=0; i<VOCABULARY; ++i){
        for (int j=0; j<VOCABULARY; ++j){
            if (pair_count[i][j] > max_count){
                max_count = pair_count[i][j];
                best.left = i;
                best.right = j;
            }
        }
    }
    return best;
}

int * merge_pair(int *tokens, int* len, byte_pair pair, int new_token){
    int new_len = *len;
    for (int i = 0; i < *len -1; ++i){
        if (tokens[i] == pair.left && tokens[i+1] == pair.right){
            new_len--;
        }
    }
    int * encoded_output = malloc(new_len * sizeof(int));
    if (!encoded_output) {
        perror("Failed to allocate memory");
        return tokens;
    }

    int current_pos = 0;
    for (int i=0; i< *len;){
        if (i < *len - 1 && tokens[i] == pair.left && tokens[i+1] == pair.right) {
            encoded_output[current_pos++] = new_token;
            i += 2;
        } else {
            encoded_output[current_pos++] = tokens[i];
            i++;
        }
    }
    free(tokens);
    *len = new_len;
    return encoded_output;
}

int * byte_pair_encode(const char * text, state * curr_state, int * encoded_length){
    if (!text) return NULL;
    
    int text_length = strlen(text);
    int * tokens = (int*) malloc(text_length * sizeof(int));
    if (!tokens) {
        perror("Failed to allocate memory for tokens");
        return NULL;
    }
    for (int i = 0; i < text_length; ++i) {
        tokens[i] = (int)(unsigned char)text[i];
    }

    int current_length = text_length;
    int **pair_count = (int **)malloc(VOCABULARY * sizeof(int *));
    
    if (!pair_count) {
        perror("Failed to allocate memory for pair_count");
        free(tokens);
        return NULL;
    }
    
    for (int i = 0; i< VOCABULARY; ++i){
        pair_count[i] = (int*)calloc(VOCABULARY, sizeof(int));
        if (!pair_count[i]) {
            perror("Failed to allocate memory for pair_count[i]");
            for (int j = 0; j < i; ++j) {
                free(pair_count[j]);
            }
            free(pair_count);
            free(tokens);
            return NULL;
        }
    }
    
    int merges_todo = VOCABULARY - 256;
    curr_state->num_tokens = 0;
    
    for (int i = 0; i<merges_todo; ++i){
        if (current_length <2){
            printf("Not enough tokens to merge.\n");
            break;
        }
        get_pairs(tokens, current_length, pair_count);

        byte_pair best_pair = most_freq(pair_count);
        if (best_pair.left == -1 || best_pair.right == -1) {
            printf("No pairs to merge.\n");
            break;
        }

        int new_token = 256 + i;
        int * temp_token = merge_pair(tokens, &current_length, best_pair, new_token);
        if (!temp_token) {
            free(tokens);
            for(int k = 0; k < VOCABULARY; ++k) {
                free(pair_count[k]);
            }
            free(pair_count);
            return NULL;
        }
        tokens = temp_token;

        curr_state->tokens[curr_state->num_tokens].pair[0] = best_pair.left;
        curr_state->tokens[curr_state->num_tokens].pair[1] = best_pair.right;
        curr_state->tokens[curr_state->num_tokens].token = new_token;
        curr_state->num_tokens++;

        printf("Merged pair (%d, %d) into %d\n", best_pair.left, best_pair.right, new_token);
    }

    for (int i = 0; i < VOCABULARY; ++i) {
        free(pair_count[i]);
    }
    free(pair_count);
    *encoded_length = current_length;
    return tokens;
}

int main(){
    char text[1000];
    printf("Enter text: ");
    scanf("%999[^\n]", text);
    state curr_state;
    int encoded_length = 0;
    byte_pair_encode(text, &curr_state, &encoded_length);
    printf("Encoded length: %d\n", encoded_length);
    printf("Number of tokens: %d\n", curr_state.num_tokens);
    for (int i = 0; i < curr_state.num_tokens; ++i){
        printf("Token %d: (%d, %d) -> %d\n", i, curr_state.tokens[i].pair[0], curr_state.tokens[i].pair[1], curr_state.tokens[i].token);
    }
    return 0;
}
