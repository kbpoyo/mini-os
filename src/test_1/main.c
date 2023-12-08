#include <stdio.h>

int add(int a, int b) {
    int c = 10;
    int d = a + b + c;
    return d;
}

extern int fun;

int main() {

    int a = 10;
    int b = 20;

    char *str = "this rest = ";

    printf("%s %d\n", str, add(a, b));

    int ret = ((int(*)(void))&fun)();


    printf("fun = %d, ret = %d\n", &fun, ret);
    



    return 0;
}