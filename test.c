#include <stdio.h>

int main(){
    int a[10], b[10];
    for (int i = 0; i < 10; i ++){
        a[i] = i;
    }
    memcpy(a, b, 10);
    return 0;
}
