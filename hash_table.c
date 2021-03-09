#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "hash_table.h"
#include "string_builder.h"
#include "number_util.h"

// ================================================================================
// Private Properties
// ================================================================================
// 實作雜湊表的私有屬性
struct HashTable_Private
{
    // 雜湊映射表的容器大小
    int bucket_size;
    /*
     * 實際存在的映射物件(HT_DELETED_ITEM 不算在內)
     * 會隨著刪除方法而減少
     */
    int item_count;
    /*
     * 成功新增、刪除的計數
     * 不會隨著刪除方法而減少
     */
    int modified_count;
    // 承裝映射物件的容器
    HashTableItem** items;
};

// 作為雜湊運算用的常數
static int HASH_ARG_PRIME_1 = 5;
static int HASH_ARG_PRIME_2 = 11;
static int DEFAULT_SIZE = 0XF0;

// 作為判定已刪除的物件
static HashTableItem HT_DELETED_ITEM = {NULL, NULL};

static const char *QUOTE = "\"";
static const char *EQUAL = "=";
static const char *DELIMITER = ",";
static const char *BEGIN = "{";
static const char *END = "}";

static HashTableItem* New_HashTableItem(const char *k, const char *v) 
{
    HashTableItem* item = malloc(sizeof(HashTableItem));
    item->key = strdup(k);
    item->value = strdup(v);
    return item;
}

static void Delete_HashTableItem(HashTableItem* item) 
{
    free(item->key);
    free(item->value);
    free(item);
}

static int Calculate_StringHash(const char *str, const int hash_arg, const int max) 
{
    long hash = 0;
    const int str_len = strlen(str);
    for (size_t i = 0; i < str_len; i++)
    {
        hash += (long) pow(hash_arg, str_len - (i + 1)) * str[i];
        if (hash > 0XFFFFFFFF / 2)
        {
            hash >>= 2;
        }
        hash %= max;
    }
    return (int) hash;
}

static int Get_HashValue(const char *str, const int num_buckets, const int attempt)
{
    const int hash_a = Calculate_StringHash(str, HASH_ARG_PRIME_1, num_buckets);
    const int hash_b = Calculate_StringHash(str, HASH_ARG_PRIME_2, num_buckets);
    int result = (hash_a + (attempt * (hash_b))) + (hash_a ^ hash_b) % num_buckets;
    return result;
}

static inline int HashTableItem_IsAbandoned(HashTableItem *item)
{
    return item == &HT_DELETED_ITEM;
}

static inline int HashTableItem_IsValid(HashTableItem *item)
{
    return item != NULL && !HashTableItem_IsAbandoned(item);
}

static HashTable* _New_HashTable(int size)
{
    HashTable* table = malloc(sizeof(HashTable));
    HashTable_Private *priv = malloc(sizeof(HashTable_Private));
    priv->bucket_size = size;
    priv->item_count = 0;
    priv->modified_count = 0;
    priv->items = calloc((size_t) priv->bucket_size, sizeof(HashTableItem*));
    table->priv = priv;
    return table;
}

static void HashTable_Resize(HashTable *table)
{
    int next_pow_of_2 = (int) NumberUtil_NextPowerOf_2(table->priv->item_count);
    int new_size = next_pow_of_2 + (next_pow_of_2 >> 1);
    if (DEFAULT_SIZE >= new_size) new_size = DEFAULT_SIZE;
    HashTable *temp_table = _New_HashTable(new_size);
    if (!temp_table)
    {
        printf("malloc new HashTable failed...\n");
    }
    HashTable_Private *curr_priv = table->priv;
    for (size_t i = 0; i < curr_priv->bucket_size; i++)
    {
        HashTableItem *item = curr_priv->items[i];
        if (item != NULL)
        {
            if (!HashTableItem_IsAbandoned(item))
            {
                HashTable_Put(temp_table, item->key, item->value);
            }
        }
    }
    HashTable_Private *temp_priv = temp_table->priv;
    table->priv = temp_priv;
    free(curr_priv->items);
    if (curr_priv)
    {
        free(curr_priv);
    }
}

static int HashTable_NeedResize(HashTable *table)
{
    HashTable_Private *priv = table->priv;
    return priv->modified_count * 100 / priv->bucket_size > 70;
}

// ================================================================================
// Public properties
// ================================================================================
HashTable* New_HashTable() 
{
    return _New_HashTable(DEFAULT_SIZE);
}

void Delete_HashTable(HashTable **p_to_table) 
{
    HashTable *table = *p_to_table;
    HashTable_Private *priv = table->priv;
    for (int i = 0; i < priv->bucket_size; i++) 
    {
        HashTableItem *item = priv->items[i];
        if (HashTableItem_IsValid(item)) Delete_HashTableItem(item);
    }
    free(priv->items);
    free(priv);
    free(table);
    *p_to_table = NULL;
} 

void HashTable_Put(HashTable *table, const char *key, const char *value)
{
    if (HashTable_NeedResize(table))
    {
        HashTable_Resize(table);
    }
    HashTable_Private *priv = table->priv;
    HashTableItem *new_item = New_HashTableItem(key, value);
    int index = Get_HashValue(new_item->key, priv->bucket_size, 0);
    HashTableItem *curr_item = priv->items[index];
    int i = 1;
    while (curr_item != NULL)
    {
        if (!HashTableItem_IsAbandoned(curr_item) && strcmp(key, curr_item->key) == 0)
        {
            Delete_HashTableItem(curr_item);
            priv->items[index] = new_item;
            return;
        }
        printf("key '%s' collide\n", key);
        index = Get_HashValue(new_item->key, priv->bucket_size, i);
        curr_item = priv->items[index];
        i++;
    }
    priv->items[index] = new_item;
    priv->item_count++;
    priv->modified_count++;
}

char* HashTable_Find(HashTable *table, const char *key)
{
    HashTable_Private *priv = table->priv;
    int index = Get_HashValue(key, priv->bucket_size, 0);
    HashTableItem *item = priv->items[index];
    int i = 1;
    while (item != NULL)
    {
        if (!HashTableItem_IsAbandoned(item))
        {
            if (strcmp(item->key, key) == 0)
            {
                return item->value;
            }
        }
        index = Get_HashValue(key, priv->bucket_size, i);
        item = priv->items[index];
        i++;
    }
    return NULL;
}

void HashTable_Delete(HashTable *table, const char *key)
{
    HashTable_Private *priv = table->priv;
    int index = Get_HashValue(key, priv->bucket_size, 0);
    HashTableItem *item = priv->items[index];
    int i = 1;
    while (item != NULL)
    {
        if (!HashTableItem_IsAbandoned(item))
        {
            if (strcmp(item->key, key) == 0)
            {
                Delete_HashTableItem(item);
                priv->items[index] = &HT_DELETED_ITEM;
                priv->item_count--;
                priv->modified_count++;
                break;
            }
        }
        index = Get_HashValue(key, priv->bucket_size, i);
        item = priv->items[index];
        i++;
    }
    if (HashTable_NeedResize(table))
    {
        HashTable_Resize(table);
    }
}

inline int HashTable_Size(HashTable *table)
{
    return table->priv->item_count;
}

inline int HashTable_IsEmpty(HashTable *table)
{
    return HashTable_Size(table) == 0;
}

char* HashTable_ToString(HashTable *table)
{
    int counter = 0;
    StringBuilder *builder = New_StringBuilder();
    StringBuilder_Append(builder, BEGIN);
    HashTable_Private *priv = table->priv;
    for (size_t i = 0; i < priv->bucket_size; i++)
    {
        HashTableItem *item = priv->items[i];
        if (HashTableItem_IsValid(item))
        {
            if (counter > 0)
            {
                StringBuilder_Append(builder, DELIMITER);
            }
            StringBuilder_Append(builder, QUOTE);
            StringBuilder_Append(builder, item->key);
            StringBuilder_Append(builder, QUOTE);
            StringBuilder_Append(builder, EQUAL);
            StringBuilder_Append(builder, QUOTE);
            StringBuilder_Append(builder, item->value);
            StringBuilder_Append(builder, QUOTE);
            
            counter++;
        }
    }
    
    StringBuilder_Append(builder, END);
    char *str = StringBuilder_Value(builder);
    Delete_StringBuilder(&builder);
    return str;
}

