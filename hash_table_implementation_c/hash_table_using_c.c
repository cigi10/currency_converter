#include <stdlib.h> // includers 
#include <stdio.h>
#include <stddef.h>
#include <string.h> 
#include "oa_hash.h" // hash table related definition

enum oa_ret_ops { DELETE, PUT, GET };
// DELETE - represents an operation that deletes a key-value pair from the hash table
// PUT - represents an operation that inserts or updates a key-value pair in the hash table
// GET - operation that retrieves the value associated with a specific key from the hash table

static size_t oa_hash_getidx(oa_hash *htable, size_t idx, uint32_t hash_val, const void *key, enum oa_ret_ops op); 
//  oa_hash_getidx - responsible for determining the index where an operation (PUT, GET, or DELETE) 
// should be performed in the hash table
static inline void oa_hash_lp_idx(oa_hash *htable, size_t *idx);
// oa_hash_lp_idx - function is used for linear probing in hash tables
inline static void oa_hash_grow(oa_hash *htable);
// oa_hash_grow - responsible for growing the hash table when its load factor exceeds a certain threshold
static inline bool oa_hash_should_grow(oa_hash *htable);
// oa_hash_should_grow - checks if the hash table should be grown based on its current load factor ;if the load factor exceeds a predefined threshold, 
// it indicates that the table is becoming too full, and oa_hash_grow should be called to resize the table
static inline bool oa_hash_is_tombstone(oa_hash *htable, size_t idx);
//function checks if the slot at a given index in the hash table is a tombstone
// (a tombstone typically represents a deleted element, allowing the probing 
// process to distinguish between empty slots and deleted slots)
static inline void oa_hash_put_tombstone(oa_hash *htable, size_t idx); 
// function marks a slot in the hash table as a tombstone. 
// (it is used during deletion operations to indicate that an element was previously stored at this slot but has been deleted)

oa_hash* oa_hash_new(
    oa_key_ops key_ops, 
    oa_val_ops val_ops, 
    void (*probing_fct)(struct oa_hash_s *htable, size_t *from_idx)) 
{
    oa_hash *htable;
    
    htable = malloc(sizeof(*htable));
    if (NULL==htable) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }

/* oa_hash_new - function is responsible for creating and initializing a new hash table. 
It takes parameters for key and value operations, as well as a probing function pointer, 
and allocates memory for the hash table structure. 
If memory allocation fails, it prints an error message and exits the program.*/

    htable->size = 0;
    htable->capacity = OA_HASH_INIT_CAPACITY;
    htable->val_ops = val_ops;
    htable->key_ops = key_ops;
    htable->probing_fct = probing_fct;

/* these lines of code are part of initializing a hash table (htable) by setting its size, capacity,
value operations (val_ops), key operations (key_ops), and probing function (probing_fct)*/

    htable->buckets = malloc(sizeof(*(htable->buckets)) * htable->capacity);
    if (NULL==htable->buckets) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }

    for(int i = 0; i < htable->capacity; i++) {
        htable->buckets[i] = NULL;
    }

    return htable;
}

/* this code segment is responsible for dynamically allocating memory for the buckets of the hash table, initializing them to NULL, 
and handling errors in case memory allocation fails. It ensures that the hash table is properly set up and ready for use*/

oa_hash* oa_hash_new_lp(oa_key_ops key_ops, oa_val_ops val_ops) {
    return oa_hash_new(key_ops, val_ops, oa_hash_lp_idx);
}

void oa_hash_free(oa_hash *htable) {
    for(int i = 0; i < htable->capacity; i++) {
        if (NULL!=htable->buckets[i]) {
            htable->key_ops.free(htable->buckets[i]->key, htable->key_ops.arg);
            htable->val_ops.free(htable->buckets[i]->val, htable->val_ops.arg);
        }
        free(htable->buckets[i]);
    }
    free(htable->buckets);
    free(htable);
} 

/*  the oa_hash_free function is responsible for deallocating all the memory associated with a hash table. 
It iterates through each bucket, frees the keys and values stored in them using the
corresponding key and value operation's free functions, frees the buckets array, and then frees the hash table structure itself. 
This ensures that all dynamically allocated memory used by the hash table is properly released, preventing memory leaks.
*/

inline static void oa_hash_grow(oa_hash *htable) { // resizing the hash table basically
    uint32_t old_capacity;
    // unsigned 32-bit integer -  data type 
    oa_pair **old_buckets;
    oa_pair *crt_pair;

    uint64_t new_capacity_64 = (uint64_t) htable->capacity * OA_HASH_GROWTH_FACTOR;
    // unsigned 64-bit integer -  data type 
    if (new_capacity_64 > SIZE_MAX) { // checking for overflow
        fprintf(stderr, "re-size overflow in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }

    old_capacity = htable->capacity;
    old_buckets = htable->buckets;

    htable->capacity = (uint32_t) new_capacity_64; 
    htable->size = 0;
    htable->buckets = malloc(htable->capacity * sizeof(*(htable->buckets)));

    if (NULL == htable->buckets) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
/* stderr is correctly placed before the format string because it's part of the fprintf function's syntax,
specifying where the output should be directed*/
        
        exit(EXIT_FAILURE);   //memory allocation failure during resizing 
    }

    for(int i = 0; i < htable->capacity; i++) {
        htable->buckets[i] = NULL;
    };

    for(size_t i = 0; i < old_capacity; i++) {
        crt_pair = old_buckets[i];
        if (NULL!=crt_pair && !oa_hash_is_tombstone(htable, i)) {
            oa_hash_put(htable, crt_pair->key, crt_pair->val);
            htable->key_ops.free(crt_pair->key, htable->key_ops.arg);
            htable->val_ops.free(crt_pair->val, htable->val_ops.arg);
            free(crt_pair);
        }
    }

    free(old_buckets);
}

/* this code segment initializes the new buckets to NULL, rehashes elements from the old buckets into the resized hash table, 
and then frees the memory allocated for the old buckets. It's a crucial part of the resizing process for a hash table, 
ensuring that elements are correctly transferred to the resized structure and that no memory leaks occur*/

inline static bool oa_hash_should_grow(oa_hash *htable) {
    return (htable->size / htable->capacity) > OA_HASH_LOAD_FACTOR;
    /* conditional expression that checks if the current load factor of the hash table (htable) 
    exceeds a predefined load factor threshold (OA_HASH_LOAD_FACTOR)*/
}

void oa_hash_put(oa_hash *htable, const void *key, const void *val) {

    if (oa_hash_should_grow(htable)) {
        oa_hash_grow(htable);
    }

    uint32_t hash_val = htable->key_ops.hash(key, htable->key_ops.arg);
    size_t idx = hash_val % htable->capacity;
    /* Calculates the hash value for the given key using the hash function specified in key_ops.
    Computes the index where the key-value pair should be stored in the hash table based on the hash value and the table's capacity.*/

    if (NULL==htable->buckets[idx]) {
        // Key doesn't exist & we add it anew
        htable->buckets[idx] = oa_pair_new(
                hash_val, 
                htable->key_ops.cp(key, htable->key_ops.arg),
                htable->val_ops.cp(val, htable->val_ops.arg)
        ); 

/* If the bucket at the calculated index (idx) is empty, it means the key doesn't exist in the hash table. 
In this case, a new key-value pair (oa_pair) is created using oa_pair_new and added to the bucket at the index.*/

    } else { //Collision Handing 
        // // Probing for the next good index
        idx = oa_hash_getidx(htable, idx, hash_val, key, PUT);

        if (NULL==htable->buckets[idx]) {
            htable->buckets[idx] = oa_pair_new(
                hash_val, 
                htable->key_ops.cp(key, htable->key_ops.arg),
                htable->val_ops.cp(val, htable->val_ops.arg)
            ); 
        } else {
            // Update the existing value
            // Free the old values
            htable->val_ops.free(htable->buckets[idx]->val, htable->val_ops.arg);
            htable->key_ops.free(htable->buckets[idx]->key, htable->key_ops.arg);
            // Update the new values
            htable->buckets[idx]->val = htable->val_ops.cp(val, htable->val_ops.arg);
            htable->buckets[idx]->key = htable->val_ops.cp(key, htable->key_ops.arg);
            htable->buckets[idx]->hash = hash_val;
        }
   }

   /* If the bucket is not empty, it means there is a collision. 
The function then uses a probing mechanism (oa_hash_getidx) to find the next available index for insertion or update.
If the new index (idx) is empty, a new key-value pair is created and added to that index.
If the new index is not empty, it means the key already exists in the hash table. In this case, the existing value is updated with the new value. 
It also takes care of freeing memory for old values and updating hash-related information.*/
   
    htable->size++;
}

inline static bool oa_hash_is_tombstone(oa_hash *htable, size_t idx) {
    if (NULL==htable->buckets[idx]) {
        return false;
    }
    if (NULL==htable->buckets[idx]->key && 
        NULL==htable->buckets[idx]->val && 
        0 == htable->buckets[idx]->key) {
            return true;
    }        
    return false;
}


/* Checks if the bucket at the specified index in the hash table is a tombstone.
Returns true if the bucket is a tombstone (i.e., it was previously deleted), and false otherwise.
It checks for conditions where both the key and value are NULL and the hash is zero, indicating a tombstone.*/

inline static void oa_hash_put_tombstone(oa_hash *htable, size_t idx) {
    if (NULL != htable->buckets[idx]) {
        htable->buckets[idx]->hash = 0;
        htable->buckets[idx]->key = NULL;
        htable->buckets[idx]->val = NULL;
    }
}


/*Marks a bucket at the specified index in the hash table as a tombstone.
Sets the hash, key, and value of the bucket to NULL or zero, indicating that it's a tombstone.*/

void *oa_hash_get(oa_hash *htable, const void *key) {
    uint32_t hash_val = htable->key_ops.hash(key, htable->key_ops.arg);
    size_t idx = hash_val % htable->capacity;

    if (NULL==htable->buckets[idx]) {
        return NULL;
    }

    idx = oa_hash_getidx(htable, idx, hash_val, key, GET);

    return (NULL==htable->buckets[idx]) ?
         NULL : htable->buckets[idx]->val;
}

/* Retrieves the value associated with a given key from the hash table.
Calculates the hash value for the key and determines the index in the hash table.
If the bucket at the calculated index is empty (NULL), it means the key is not present, so it returns NULL.
Otherwise, it uses a probing mechanism (oa_hash_getidx) to find the actual index where the key-value pair is stored.
Returns the value associated with the key if found, or NULL if the key is not present.*/

void oa_hash_delete(oa_hash *htable, const void *key) {
    uint32_t hash_val = htable->key_ops.hash(key, htable->key_ops.arg);
    size_t idx = hash_val % htable->capacity;
    
    if (NULL==htable->buckets[idx]) {
        return;
    }

    idx = oa_hash_getidx(htable, idx, hash_val, key, DELETE);
    if (NULL==htable->buckets[idx]) {
        return;
    }

    htable->val_ops.free(htable->buckets[idx]->val, htable->val_ops.arg);
    htable->key_ops.free(htable->buckets[idx]->key, htable->key_ops.arg);

    oa_hash_put_tombstone(htable, idx);
}

/*Deletes an entry from the hash table based on the given key.
Calculates the hash value and determines the initial index in the hash table.
Checks if the bucket at the calculated index is empty, indicating that the key is not present, and returns.
Uses probing (oa_hash_getidx) to find the actual index of the key-value pair.
Frees the memory occupied by the value and key of the entry.
Marks the bucket as a tombstone using oa_hash_put_tombstone.*/

void oa_hash_print(oa_hash *htable, void (*print_key)(const void *k), void (*print_val)(const void *v)) {

    oa_pair *pair;

    printf("Hash Capacity: %lu\n", htable->capacity);
    printf("Hash Size: %lu\n", htable->size);

    printf("Hash Buckets:\n");
    for(int i = 0; i < htable->capacity; i++) {
        pair = htable->buckets[i];
        printf("\tbucket[%d]:\n", i);
        if (NULL!=pair) {
            if (oa_hash_is_tombstone(htable, i)) {
                printf("\t\t TOMBSTONE");
            } else {
                printf("\t\thash=%" PRIu32 ", key=", pair->hash);
                print_key(pair->key);
                printf(", value=");
                print_val(pair->val);
            }
        }
        printf("\n");
    }
}

/* Prints information about the hash table, including its capacity, size, and contents of each bucket.
Iterates through each bucket and prints details if it contains an entry.
Checks for tombstones and prints them as such.
Utilizes custom print functions for keys and values.*/

static size_t oa_hash_getidx(oa_hash *htable, size_t idx, uint32_t hash_val, const void *key, enum oa_ret_ops op) {
    do {
        if (op==PUT && oa_hash_is_tombstone(htable, idx)) {
            break;
        }
        if (htable->buckets[idx]->hash == hash_val && 
            htable->key_ops.eq(key, htable->buckets[idx]->key, htable->key_ops.arg)) {
            break;
        }
        htable->probing_fct(htable, &idx);
    } while(NULL!=htable->buckets[idx]);
    return idx;
}

/*Performs probing to find the index where an entry should be inserted or retrieved.
Handles cases such as tombstones (for insertion) and matching keys (for retrieval).
Calls the probing function (htable->probing_fct) to navigate through the hash table based on the probing strategy.*/

// Probing functions

static inline void oa_hash_lp_idx(oa_hash *htable, size_t *idx) {
    (*idx)++;
    if ((*idx)==htable->capacity) {
        (*idx) = 0;
    }
}

/* oa_hash_lp_idx is a probing function that implements linear probing by incrementing the index linearly 
and wrapping around at the end of the table if necessary, 
making sure that all slots in the table are checked during collision resolution.
*/

// Pair related

oa_pair *oa_pair_new(uint32_t hash, const void *key, const void *val) {
    oa_pair *p;
    p = malloc(sizeof(*p));
    if (NULL==p) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);  
    }
    p->hash = hash;
    p->val = (void*) val;
    p->key = (void*) key;
    return p;
}

/* oa_pair_new is a utility function used to dynamically allocate memory for a new oa_pair structure, 
initialize its fields with provided values (hash, key, and val), and return a pointer to the initialized pair.
 It includes error checking to handle cases where memory allocation fails.*/

// String operations

static uint32_t oa_hash_fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x3243f6a9U;
    h ^= h >> 16;
    return h;
}
/* This function performs mixing operations on a 32-bit hash value h.
h ^= h >> 16;: XORs h with its right-shifted value by 16 bits.
h *= 0x3243f6a9U;: Multiplies h by a constant.
h ^= h >> 16;: XORs h again with its right-shifted value by 16 bits.
Finally, it returns the modified hash value h.*/


uint32_t oa_string_hash(const void *data, void *arg) {
    
    //djb2
    uint32_t hash = (const uint32_t) 5381;
    const char *str = (const char*) data;
    char c;
    while((c=*str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return oa_hash_fmix32(hash);
}

/* This function calculates the hash value for a string (const void *data) using the DJB2 hash algorithm.
It initializes the hash variable to a constant value (5381).
It iterates through each character of the string (*str) and updates the hash value using a formula (((hash << 5) + hash) + c).
After processing all characters, it returns the hash value after applying oa_hash_fmix32 for final mixing.*/

void* oa_string_cp(const void *data, void *arg) {
    const char *input = (const char*) data;
    size_t input_length = strlen(input) + 1;
    char *result;
    result = malloc(sizeof(*result) * input_length);
    if (NULL==result) {
        fprintf(stderr,"malloc() failed in file %s at line # %d", __FILE__,__LINE__);
        exit(EXIT_FAILURE);
    }
    strcpy(result, input);
    return result;
}

/* This function is responsible for creating a copy of a string.
Parameters:
const void *data: Pointer to the string to be copied.
void *arg: Optional argument (not used in this implementation).
It allocates memory for a new string using malloc based on the length of the input string plus one (for the null terminator).
If memory allocation fails, it prints an error message and exits the program.
It then copies the input string into the newly allocated memory using strcpy and returns a pointer to the copied string.*/

bool oa_string_eq(const void *data1, const void *data2, void *arg) {
    const char *str1 = (const char*) data1;
    const char *str2 = (const char*) data2;
    return !(strcmp(str1, str2)) ? true : false;    
}

/* This function checks if two strings are equal.
Parameters:
const void *data1: Pointer to the first string.
const void *data2: Pointer to the second string.
void *arg: Optional argument (not used in this implementation).
It converts the void pointers to const char * pointers and compares the strings using strcmp.
If the strings are equal, it returns true; otherwise, it returns false*/

void oa_string_free(void *data, void *arg) {
    free(data);
}

/*This function frees the memory allocated for a string.
Parameter:
void *data: Pointer to the string to be freed.
void *arg: Optional argument (not used in this implementation).
It simply frees the memory using free.*/

void oa_string_print(const void *data) {    
    printf("%s", (const char*) data);
}

/*This function prints a string to the standard output.
Parameter:
const void *data: Pointer to the string to be printed.
It converts the void pointer to const char * and prints the string using printf.*/

oa_key_ops oa_key_ops_string = { oa_string_hash, oa_string_cp, oa_string_free, oa_string_eq, NULL};
oa_val_ops oa_val_ops_string = { oa_string_cp, oa_string_free, oa_string_eq, NULL};

/* oa_string_hash: Computes the hash value of a string.
oa_string_cp: Copies a string.
oa_string_free: Frees the memory allocated for a string.
oa_string_eq: Checks if two strings are equal.*/

#define WRITES 10
#define READS 10

int main(int argc, char *argv[]) {
    oa_hash *h = oa_hash_new(oa_key_ops_string, oa_val_ops_string, oa_hash_lp_idx);

    oa_hash_put(h, "Bucharest", "Romania");
    oa_hash_put(h, "Sofia", "Bulgaria");

    printf("%s\n", oa_hash_get(h, "Bucharest"));
    printf("%s\n", oa_hash_get(h, "Sofia"));

    return 0;
}   
