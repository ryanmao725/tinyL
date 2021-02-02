#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstrUtils.h"
#include "Utils.h"

#define EMPTY_FIELD 0xFFFFF

/* LinkedList of Critical field ints */
typedef struct CField {
    int fieldValue;
    struct CField* next;
} CField;

/* Utilities */
static void markReadCritical(Instruction* head);
static void markWriteCritical(Instruction* head);
static void markUpwardsCritical(Instruction* write);
static int isFieldCritical(int value, CField* first);
static void addCField(CField* first, int fieldValue);
static void removeCField(CField* first, int fieldValue);
static void freeCFieldList(CField* first);
static Instruction* removeNoncritical(Instruction* head);

/* Misc */
static void printCriticalInstructions(Instruction *head);

/* Utility functions */

static void markReadCritical(Instruction* head) {
    Instruction* temp = head;
    while (temp != NULL) {
        if (temp->opcode == READ) {
            temp->critical = 'y';
        }
        temp = temp->next;
    }
    free(temp);
}

static void markWriteCritical(Instruction* head) {
    // First must find a write function. Easier to iterate down to up.
    Instruction* temp = LastInstruction(head);
    while (temp != NULL) {
        if (temp->opcode == WRITE) {
            temp->critical = 'y';
            // After finding a write, we use markUpwardsCritical to upwards traverse and mark all critical lines.
            markUpwardsCritical(temp);
        }
        temp = temp->prev;
    }
    free(temp);
}

static void markUpwardsCritical(Instruction* write) {
    Instruction* temp = write;
    CField* firstCField = malloc(sizeof(CField));
    addCField(firstCField, temp->field1);
    // printf("First field value: %d\n", firstCField->fieldValue);
    // firstCField->fieldValue = temp->field1;
    // firstCField->next = NULL;
    temp = temp->prev;
    while (temp != NULL) {
        if (isFieldCritical(temp->field1, firstCField)) {
            // Set current instruction to be critical
            if (!temp->critical) temp->critical = 'y';
            // if it is a LOADI instruction, we don't want to store the other fields.
            if (temp->opcode == LOADI) {
                temp = temp->prev;
                continue;
            }
            // Add current instruction's fields to CField linked list.
            addCField(firstCField, temp->field2);
            addCField(firstCField, temp->field3);
        }
        temp = temp->prev;
    }
    // After reaching the top, we must free this write instruction's critical field values linked list.
    freeCFieldList(firstCField);
    free(temp);
}

static int isFieldCritical(int value, CField* first) {
    CField* temp = first;
    while (temp != NULL) {
        if (value == temp->fieldValue) {
            // printf("found %d\n", value);
            removeCField(first, value);
            return 1;
        }
        temp = temp->next;
    }
    free(temp);
    return 0;
}

static void addCField(CField* first, int fieldValue) {
    if (fieldValue == EMPTY_FIELD) return;
    CField* temp = first;
    while (temp->next != NULL) {
        if (temp->fieldValue == fieldValue) {
            temp = NULL;
            free(temp);
            return;
        }
        temp = temp->next;
    }
    CField* newCField = malloc(sizeof(CField));
    newCField->fieldValue = fieldValue;
    newCField->next = NULL;
    temp->next = newCField;
}

static void removeCField(CField* first, int fieldValue) {
    if (fieldValue == EMPTY_FIELD) return;
    CField* temp = first;
    CField* prev = NULL;
    // printf("remove called");

    //Check if it is the first
    while (temp != NULL) {
        // printf("looping");
        if (temp->fieldValue == fieldValue) {
            // printf("found");
            if (prev == NULL) {
                // The value is the first.
                // More than one element in list
                if (first->next == NULL) {
                    free(temp);
                    first = malloc(sizeof(CField));
                    return;
                }
                free(temp);
                first = first->next;
                return;
            } else {
                free(temp);
                prev->next = temp->next;
                return;
            }
        }
        prev = temp;
        temp = temp->next;
    }
    free(temp);
    return;
}


static void freeCFieldList(CField* first) {
    CField* temp = first;
    CField* freeTemp = first;
    while (temp != NULL) {
        freeTemp = temp;
        temp = temp->next;
        free(freeTemp);
    }
    free(temp);
}

static Instruction* removeNoncritical(Instruction* head) {
    Instruction* temp = head;
    while (temp != NULL) {
        if (!temp->critical) {
            if (temp->prev == NULL && temp->next == NULL) return NULL;
            if (temp->prev == NULL) {
                // That means this is the head.
                head = head->next;
                head->prev = NULL;
                free(temp);
                temp = head;
                continue;
            }
            if (temp->next == NULL) {
                // That means this is the tail.
                temp->prev->next = NULL;
                temp = temp->next;
                continue;
            }
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
        }
        temp = temp->next;
    }
    free(temp);
    return head;
}

/* Misc functions */
static void printCriticalInstructions(Instruction *head) {
    Instruction* temp = head;
    while (temp != NULL) {
        if (temp->critical) {
            PrintInstruction(stdout, temp);
        }
        temp = temp->next;
    }
    free(temp);
}

int main()
{
	Instruction *head;

	head = ReadInstructionList(stdin);
	if (!head) {
		WARNING("No instructions\n");
		exit(EXIT_FAILURE);
	}
	/* CODE */
    markReadCritical(head);
    markWriteCritical(head);
    head = removeNoncritical(head);
    // printCriticalInstructions(head);
    
	if (head) {
		PrintInstructionList(stdout, head);
		DestroyInstructionList(head);
    }
	return EXIT_SUCCESS;
    
}

