#include <iostream>


void sleak() {
    int *a = new int [100];
}

void leak() {
    int *a = new int [100];
    sleak();
}


int main() {
    leak();
}