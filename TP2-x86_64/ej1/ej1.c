#include "ej1.h"
#include <stdlib.h>
#include <string.h>

string_proc_list* string_proc_list_create(void) {
    string_proc_list* list = (string_proc_list*) malloc(sizeof(string_proc_list));
    if (!list) return NULL;
    list->first = NULL;
    list->last = NULL;
    return list;
}

string_proc_node* string_proc_node_create(uint8_t type, char* hash) {
    string_proc_node* node = (string_proc_node*) malloc(sizeof(string_proc_node));
    if (!node) return NULL;
    node->type = type;
    node->hash = hash; 
    node->next = NULL;
    node->previous = NULL;
    return node;
}

void string_proc_list_add_node(string_proc_list* list, uint8_t type, char* hash) {
    if (!list) return;

    string_proc_node* new_node = string_proc_node_create(type, hash);
    if (!new_node) return;

    if (list->first == NULL) {
        list->first = new_node;
        list->last = new_node;
    } else {
        new_node->previous = list->last;
        list->last->next = new_node;
        list->last = new_node;
    }
}

char* string_proc_list_concat(string_proc_list* list, uint8_t type , char* hash) {
    if (!list) return NULL;

    char* result = (char*) malloc(strlen(hash) + 1);
	strcpy(result, hash);
    if (!result) return NULL;

    string_proc_node* current = list->first;

    while (current != NULL) {
        if (current->type == type) {
            char* new_result = str_concat(result, current->hash);
            free(result);
            result = new_result;
        }
        current = current->next;
    }

    return result; 
}


void string_proc_list_destroy(string_proc_list* list) {
    if (list == NULL) return;

    string_proc_node* current_node = list->first;
    string_proc_node* next_node = NULL;
    while(current_node != NULL){
        next_node = current_node->next;
        string_proc_node_destroy(current_node);
        current_node = next_node;
    }
    list->first = NULL;
    list->last  = NULL;
    free(list);
}

void string_proc_node_destroy(string_proc_node* node){
    if (node == NULL) return;

    node->next      = NULL;
    node->previous  = NULL;
    node->type      = 0;
    free(node);
}



char* str_concat(char* a, char* b) {
	int len1 = strlen(a);
    int len2 = strlen(b);
	int totalLength = len1 + len2;
    char *result = (char *)malloc(totalLength + 1); 
    strcpy(result, a);
    strcat(result, b);
    return result;  
}

void string_proc_list_print(string_proc_list* list, FILE* file){
        uint32_t length = 0;
        string_proc_node* current_node  = list->first;
        while(current_node != NULL){
                length++;
                current_node = current_node->next;
        }
        fprintf( file, "List length: %d\n", length );
		current_node    = list->first;
        while(current_node != NULL){
                fprintf(file, "\tnode hash: %s | type: %d\n", current_node->hash, current_node->type);
                current_node = current_node->next;
        }
}

