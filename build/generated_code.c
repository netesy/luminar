#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define MAX_STACK_SIZE 1024
int stack[MAX_STACK_SIZE];
int top = -1;
int constants[MAX_STACK_SIZE];
int variables[MAX_STACK_SIZE];
void* run_task(void* arg);

int main() {
// Load Constant
stack[++top] = constants[0];
// Load Constant
stack[++top] = constants[0];
// Binary Operation: ADD
stack[top-1] = stack[top-1] + stack[top];
top--;
// Print
printf("%d\n", stack[top--]);
