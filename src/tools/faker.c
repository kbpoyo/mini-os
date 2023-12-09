/**
 * @file faker.c
 * @author your name (you@domain.com)
 * @brief 欺骗编译器，通过编译
 * @version 0.1
 * @date 2023-12-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

/**
 * @brief 为libgcc定义符号raise
 * 
 * @param arg 
 * @return int 
 */
int raise(int arg) {
    return 0;
}