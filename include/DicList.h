#pragma once

#include <Arduino.h>

struct Item
{
    const char *id; // ключ
    const char *data;     // какие-то данные
    Item *next;
};

class DicList
{
private:
    Item *head;
    int count;

public:
    DicList()
    {
        count = 0;
        head = NULL;
    }

    int Count()
    {
        return count;
    }

    Item *find(const char *id)
    {
        Item *curr = head;
        while (curr != NULL)
        {
            if (strcmp(curr->id, id) == 0)
            {
                return curr;
            }
            curr = curr->next;
        }
        return NULL;
    }

    bool exists(const char *id)
    {
        return find(id) != NULL;
    }

    const char *get(const char *id)
    {
        Item * item = find(id);
        return item == NULL ? NULL : item->data;
    }

    Item *set(const char *id, const char *data)
    {
        Item *item = find(id);

        // если не найден, то создать
        if (item == NULL)
        {
            // создание узла
            count++;
            item = new Item;
            item->id = id;
            item->next = NULL;

            // пустой список? тогда в самое начало
            if (head == NULL)
            {
                head = item;
            }
            else
            {
                // в конец
                Item *curr = head;
                while (curr->next != NULL)
                {
                    curr = curr->next;
                }
                curr->next = item;
            }
        }
        item->data = data;
        return item;
    }

    void forEach(void (*callback)(const char *id, const char *data, int index))
    {
        int i = 0;
        Item *curr = head;
        while (curr != NULL)
        {
            callback(curr->id, curr->data, i);
            curr = curr->next;
            i++;
        }
    }
};