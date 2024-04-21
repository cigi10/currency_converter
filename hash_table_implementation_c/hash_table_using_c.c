#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 3

// Define the structure for each entry in the hash table
typedef struct entry_t {
    char *key;
    char *value;
    struct entry_t *next; // Pointer to the next entry in case of collisions
} entry_t;

// Define the hash table structure
typedef struct {
    entry_t **entries; // Array of pointers to entry structures
} ht_t;

// Hash function to calculate the index for a given key
unsigned int hash(const char *key) {
    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    // Calculate the hash value using a multiplication method
    for (; i < key_len; ++i) {
        value = value * 37 + key[i];
    }

    // Ensure the hash value falls within the table size
    value = value % TABLE_SIZE;

    return value;
}

// Function to create a new entry with a key-value pair
entry_t *ht_pair(const char *key, const char *value) {
    // Allocate memory for the new entry
    entry_t *entry = malloc(sizeof(entry_t) * 1);
    entry->key = malloc(strlen(key) + 1);
    entry->value = malloc(strlen(value) + 1);

    // Copy the key and value into the entry
    strcpy(entry->key, key);
    strcpy(entry->value, value);

    // Initialize the next pointer to NULL
    entry->next = NULL;

    return entry;
}

// Function to create a new hash table
ht_t *ht_create(void) {
    // Allocate memory for the hash table structure
    ht_t *hashtable = malloc(sizeof(ht_t) * 1);

    // Allocate memory for the array of entry pointers
    hashtable->entries = malloc(sizeof(entry_t*) * TABLE_SIZE);

    // Initialize each entry pointer to NULL
    int i = 0;
    for (; i < TABLE_SIZE; ++i) {
        hashtable->entries[i] = NULL;
    }

    return hashtable;
}

// Function to set a key-value pair in the hash table
void ht_set(ht_t *hashtable, const char *key, const char *value) {
    unsigned int slot = hash(key);

    // Check if the slot is empty
    entry_t *entry = hashtable->entries[slot];

    // If slot is empty, insert the new entry
    if (entry == NULL) {
        hashtable->entries[slot] = ht_pair(key, value);
        return;
    }

    entry_t *prev;

    // Handle collisions by walking through the chain of entries in the slot
    while (entry != NULL) {
        // Check if the key already exists
        if (strcmp(entry->key, key) == 0) {
            // Update the value for the existing key
            free(entry->value);
            entry->value = malloc(strlen(value) + 1);
            strcpy(entry->value, value);
            return;
        }

        // Move to the next entry in the chain
        prev = entry;
        entry = prev->next;
    }

    // End of chain reached without finding the key, add a new entry at the end
    prev->next = ht_pair(key, value);
}

// Function to get the value for a given key from the hash table
char *ht_get(ht_t *hashtable, const char *key) {
    unsigned int slot = hash(key);

    // Check if the slot is empty
    entry_t *entry = hashtable->entries[slot];

    // If slot is empty, key does not exist
    if (entry == NULL) {
        return NULL;
    }

    // Search for the key in the chain of entries in the slot
    while (entry != NULL) {
        // Check if the current entry's key matches the search key
        if (strcmp(entry->key, key) == 0) {
            // Return the value if key is found
            return entry->value;
        }

        // Move to the next entry in the chain
        entry = entry->next;
    }

    // Key not found in the slot
    return NULL;
}

// Function to delete an entry with the given key from the hash table
void ht_del(ht_t *hashtable, const char *key) {
    unsigned int bucket = hash(key);

    // Check if the bucket (slot) is empty
    entry_t *entry = hashtable->entries[bucket];

    // If bucket is empty, key does not exist
    if (entry == NULL) {
        return;
    }

    entry_t *prev;
    int idx = 0;

    // Search for the key in the chain of entries in the bucket
    while (entry != NULL) {
        // Check if the current entry's key matches the search key
        if (strcmp(entry->key, key) == 0) {
            // Handle different cases for deleting the entry based on its position in the chain
            if (entry->next == NULL && idx == 0) { // First item and no next entry
                hashtable->entries[bucket] = NULL;
            } else if (entry->next != NULL && idx == 0) { // First item with a next entry
                hashtable->entries[bucket] = entry->next;
            } else if (entry->next == NULL && idx != 0) { // Last item
                prev->next = NULL;
            } else { // Middle item
                prev->next = entry->next;
            }

            // Free the memory allocated for the deleted entry
            free(entry->key);
            free(entry->value);
            free(entry);

            return;
        }

        // Move to the next entry in the chain
        prev = entry;
        entry = prev->next;

        ++idx;
    }
}

// Function to print the contents of the hash table
void ht_dump(ht_t *hashtable) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        entry_t *entry = hashtable->entries[i];

        if (entry == NULL) {
            continue;
        }

        printf("slot[%4d]: ", i);

        // Iterate through the chain of entries in the slot
        for (;;) {
            printf("%s=%s ", entry->key, entry->value);

            if (entry->next == NULL) {
                break;
            }

            entry = entry->next;
        }

        printf("\n");
    }
}

int main(int argc, char **argv) {
    ht_t *ht = ht_create();

    // Insert some key-value pairs into the hash table
    ht_set(ht, "name1", "em");
    ht_set(ht, "name2", "lmao");
    ht_set(ht, "name3", "pizza");
    ht_set(ht, "name4", "doge");
    ht_set(ht, "name5", "pyro");
    ht_set(ht, "name6", "hihi");
    ht_set(ht, "name7", "ggwp");
    ht_set(ht, "name8", "nawwww");
    ht_set(ht, "name9", "????");
    ht_set(ht, "name10", "hehe");


    ht_dump(ht);

    // Ask the user to search for an element
    char search_key[100];
    printf("\nEnter the key to search for: ");
    scanf("%s", search_key);

    // Get the value for the searched key
    char *search_result = ht_get(ht, search_key);

    if (search_result != NULL) {
        printf("Value found for key '%s': %s\n", search_key, search_result);
    } else {
        printf("Value not found for key '%s'\n", search_key);
    }

    // Delete an entry from the hash table
    char del_key[100];
    printf("\nEnter the key to delete: ");
    scanf("%s", del_key);

    ht_del(ht, del_key);

    // Print the updated hash table
    printf("\nUpdated Hash Table:\n");
    ht_dump(ht);

    return 0;
}

