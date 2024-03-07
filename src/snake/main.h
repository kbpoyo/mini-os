/**
 * 贪吃蛇游戏
 *
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CMD_H
#define CMD_H

#define ESC_CMD2(Pn, cmd) "\x1b[" #Pn #cmd
#define ESC_CLEAR_SCREEN ESC_CMD2(2, J)  // 擦除整屏幕

#define PLAYER1_KEY_UP 'w'
#define PLAYER1_KEY_DOWN 's'
#define PLAYER1_KEY_LEFT 'a'
#define PLAYER1_KEY_RIGHT 'd'
#define PLAYER1_KEY_QUITE 'q'

// clang-format off
#define ESC_COLOR_SNAKE ESC_CMD2(38;2;255;255;0, m) 
#define ESC_COLOR_FOOD ESC_CMD2(38;2;0;255;0, m)
#define ESC_COLOR_WALL	ESC_CMD2(38;2;0;255;255, m)
#define ESC_COLOR_DEFAULT ESC_CMD2(0, m)  // 更换前景色为默认白色
// clang-format on

/**
 * 蛇身的一个节点
 */
typedef struct _body_part_t {
  int row;
  int col;
  struct _body_part_t *next;
} body_part_t;

/*
 * 蛇结构
 */
typedef struct _snake_t {
  body_part_t *head;

  enum {
    SNAKE_BIT_NONE,
    SNAKE_BIT_ITSELF,
    SNAKE_BIT_WALL,
    SNAKE_BIT_FOOD,
  } status;

  int dir;
} snake_t;

#endif
