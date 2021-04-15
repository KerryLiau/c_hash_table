#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hash_table.h"
#include "string_builder.h"

void HashTable_Simple_Test(void)
{
    printf("\n\nBegin test HashTable\n");
    HashTable* table = New_HashTable();
    printf("the hash table address: '%p' \n", table);
    
    printf("\nlet's try add key value in map\n");
    const char *key = "Hello";
    const char *value = "World";
    HashTable_Add(table, key, value);

    char *v = HashTable_Find_Str(table, key);
    printf("the key is '%s', and the value is '%s'\n", key, v);
    int count = HashTable_Size(table);
    printf("now the map size is: %d\n", count);

    char *table_str = HashTable_ToJsonStr(table);
    printf("the hash map now looks like: %s\n", table_str);

    printf("\nlet's try update value stored in key %s\n", key);
    const char *new_value = "I'm Kerry";
    HashTable_Add(table, key, new_value);
    v = HashTable_Find_Str(table, key);
    if (strcmp(v, new_value) == count)
    {
        printf("now the value stored in '%s' has change to '%s'\n", key, new_value);
    }
    if (HashTable_Size(table) == 1)
    {
        printf("and the map count is still the same\n");
    }

    if (strcmp(value, v) == 0)
    {
        printf("the value from hash table is the same as input \n");
    }

    table_str = HashTable_ToJsonStr(table);
    printf("the hash map now looks like: %s\n", table_str);
    free(table_str);

    printf("\nlet's try remove key %s\n", key);
    HashTable_Delete(table, key);
    printf("the map size is: %d\n", HashTable_Size(table));
    v = HashTable_Find_Str(table, key);
    if (v == NULL)
    {
        printf("the key %s is now removed \n", key);
    }
    
    printf("\nlet's try destruct table %p\n", table);
    Delete_HashTable(&table);
    if (table == NULL)
    {
        printf("the table was delete successfully, now the address is '%p'\n", table);
    }
    else 
    {
        printf("the table was failed to delete, now the address is: '%p'\n", table);
    }
}

void HashTable_Resize_Test()
{
    printf("\n\nNow, let's try put and delete more intensively\n");
    HashTable *table = New_HashTable_WithBucketSize(200);
    StringBuilder *key_builder = New_StringBuilder();
    StringBuilder *value_builder = New_StringBuilder();

    int count = 10;
    printf("First, let's put %d element in table\n", count);
    for (size_t i = 0; i < count; i++)
    {
        StringBuilder_Append(key_builder, (int) i);
        StringBuilder_Append(value_builder, (int) i);
        HashTable_Add_Str(
            table, StringBuilder_Value(key_builder), StringBuilder_Value(value_builder)
        );
        StringBuilder_Clear(key_builder);
        StringBuilder_Clear(value_builder);
    }
    printf("finished, now the table size is %d\n", HashTable_Size(table));
    char *table_str1 = HashTable_ToIndentJsonStr(table);
    printf("now the table looks like below:\n%s\n", table_str1);
    free(table_str1);
    printf("Second, let's delete all element in table\n");
    for (size_t i = 0; i < count; i++)
    {
        StringBuilder_Append(key_builder, (int) i);
        HashTable_Delete(
            table, StringBuilder_Value(key_builder)
        );
        StringBuilder_Clear(key_builder);
    }
    char *table_str;
    if (HashTable_IsEmpty(table))
    {
        table_str = HashTable_ToJsonStr(table);
        printf("now the table is empty, and looks like this: %s\n", table_str);
        free(table_str);
    }
    Delete_HashTable(&table);
    Delete_StringBuilder(&key_builder);
    Delete_StringBuilder(&value_builder);
}

void Time_Test(void)
{
    time_t begin, end;
    begin = clock();
    HashTable_Simple_Test();
    HashTable_Resize_Test();
    end = clock();
    double period = end - begin;
    printf("elapsed milli seconds: %f\n", period / CLOCKS_PER_SEC * 1000);
}

void Generic_Test(void)
{
    printf("\n\nBegin HashTable generic value test\n");
    HashTable *table = New_HashTable();
    HashTable_Add(table, "intVal", 26);
    HashTable_Add(table, "strVal", "foo");
    printf("strVal: %s\n", HashTable_Find_Str(table, "strVal"));
    printf("intVal: %d\n", *HashTable_Find_Int(table, "intVal"));
    int *ss = HashTable_Find_Int(table, "ss");
    if (!ss) printf("ss is NULL\n");
    else printf("intVal: %d\n", *ss);
    printf("%s\n", HashTable_ToJsonStr(table));
    Delete_HashTable(&table);
}

void Dynamic_Type(void)
{
    printf("\n\nBegin HashTable dynamic value type test\n");
    HashTable *table = New_HashTable();
    HashTable_Add(table, "val", 26);
    printf("%d\n", *HashTable_Find_Int(table, "val"));
    HashTable_Add(table, "val", "AAA");
    printf("%s\n", HashTable_Find_Str(table, "val"));
    HashTable_Add(table, "val", 5.56);
    printf("%f\n", *HashTable_Find_Double(table, "val"));
    printf("%s\n", HashTable_ToJsonStr(table));
    Delete_HashTable(&table);
}

int main(void) 
{
    Time_Test();
    Generic_Test();
    Dynamic_Type();
}
