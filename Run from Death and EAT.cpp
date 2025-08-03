#include <Arduino.h>
#include <IRremote.hpp>
#include <LiquidCrystal.h>

#define IR_REC_PIN 8
#define LCD_ROWS 16
#define LCD_COLUMNS 2
#define UP 0x18
#define DOWN 0x52
#define LEFT 0x8
#define RIGHT 0x5A
#define ONE 0x45

void player_up();
void player_down();
void player_right();
void player_left();
void print_player();
void tile_clear(int x, int y); // clears the given tile, leaves cursor at the same spot
int near_player(int x, int y); // near means within 2 tiles in any direction
void food_spawn(); // set food pos to a random tile !near_player
void print_food(int x, int y);
void defeat_screen();
void death_ai();
void print_death();

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int player_x, player_y;
int death_x, death_y;
int food_pos[2];
unsigned long t1, t0;
int ai_speed = 850, score = 0;

void setup()
{
    lcd.begin(LCD_ROWS, LCD_COLUMNS);
    IrReceiver.begin(IR_REC_PIN, ENABLE_LED_FEEDBACK);
    lcd.print("Run From Death");
    lcd.setCursor(4, 1);
    lcd.print("And EAT");
    player_x = 0, player_y = 0;
    death_x = LCD_ROWS - 1, death_y = LCD_COLUMNS - 1;
    food_pos[0] = LCD_ROWS/2;
    food_pos[1] = LCD_COLUMNS/2;
    ai_speed = 850, score = 0;
    randomSeed(analogRead(A0));

    delay(5000);
    t0 = millis();
    lcd.clear();
    lcd.print("*");
    print_food(food_pos[0], food_pos[1]);
    print_death();
}

void loop()
{
    t1 = millis();
    if ((t1 - t0)%ai_speed == 0) {
        tile_clear(death_x, death_y);
        death_ai();
        print_death();
    }
    if (death_x == player_x && death_y == player_y) defeat_screen();
    if(IrReceiver.decode()) {
        uint16_t command = IrReceiver.decodedIRData.command;
        switch (command) {
        case UP:
            player_up();
            break;
        case DOWN:
            player_down();
            break;
        case LEFT:
            player_left();
            break;
        case RIGHT:
            player_right();
            break;
        default:
            break;
        }
        print_player();
        if (food_pos[0] == player_x && food_pos[1] == player_y) {
            score++;
            food_spawn();
        }
        print_food(food_pos[0], food_pos[1]);
        print_death();
        

        delay(50);
        IrReceiver.resume();
    }
}

void player_up() {
    tile_clear(player_x, player_y);
    if (player_y) player_y -= 1;
    else player_y = LCD_COLUMNS - 1;
}

void player_down() {
    tile_clear(player_x, player_y);
    if (player_y != LCD_COLUMNS - 1) player_y += 1;
    else player_y = 0;
}

void player_right() {
    tile_clear(player_x, player_y);
    if (player_x != LCD_ROWS - 1) player_x += 1;
    else player_x = 0;
}

void player_left() {
    tile_clear(player_x, player_y);
    if (player_x) player_x -= 1;
    else player_x = LCD_ROWS - 1;
}

void tile_clear(int x, int y) {
    lcd.setCursor(x, y);
    lcd.print(" ");
    lcd.setCursor(player_x, player_y);
}

void print_player() {
    lcd.setCursor(player_x, player_y);
    lcd.print("*");
}

void print_death() {
    lcd.setCursor(death_x, death_y);
    lcd.print("D");
    lcd.setCursor(player_x, player_y);
}

void print_food(int x, int y) {
    lcd.setCursor(x, y);
    lcd.print("F");
    lcd.setCursor(player_x, player_y);
}

int near_player(int x, int y) {
    // if distance from x AND y to the player is less than 2, return 1; else 0;
    // 0 <= x < ROWS; range of [0, 16)
    // 0 <= y < COLUMNS; range of y [0, 2)
    int diff_x = player_x > x ? (player_x - x) : (x - player_x);
    int diff_y = player_y > y ? (player_y - y) : (y - player_y);
    if (diff_x <= 2 && diff_y <= 2) return 1;
    else return 0; 
}

void food_spawn() {
    for (;;) {
        int x = random(0, LCD_ROWS); // is exclusive to RANGE_MAX
        int y = random(0, LCD_COLUMNS);
        int deaths_condition = (death_x == x) && (death_y == y);
        if (!near_player(x, y) && !deaths_condition) {
            food_pos[0] = x;
            food_pos[1] = y;
            return;
        }
    }
    return;
}

void death_ai() {
    if (death_x > player_x) death_x--;
    else if (death_x < player_x) death_x++;
    if (death_y > player_y) death_y--; // change if to else if to not allow diagonal movement
    else if(death_y < player_y) death_y++;
    
    if (death_x == food_pos[0] && death_y == food_pos[1]) {
        food_spawn();
        ai_speed -= 85;
    }
}

void defeat_screen() {
    lcd.clear();
    lcd.setCursor((LCD_ROWS-12)/2, LCD_COLUMNS/2 - 1);
    lcd.print("DEFEAT");
    lcd.setCursor((LCD_ROWS-12)/2, LCD_COLUMNS/2);
    lcd.print("F Eaten: ");
    lcd.print(score);
    for(;;) {
        IrReceiver.resume();
        if (IrReceiver.decode()) {
            if (IrReceiver.decodedIRData.command == ONE) {
                lcd.clear();
                player_x = 0, player_y = 0;
                death_x = LCD_ROWS - 1, death_y = LCD_COLUMNS - 1;
                food_pos[0] = LCD_ROWS/2;
                food_pos[1] = LCD_COLUMNS/2;
                ai_speed = 850, score = 0;
                delay(500);
                return;
            }
        }
    }
}
