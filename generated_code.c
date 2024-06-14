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
// Declare Variable
variables[0] = 0;
