#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//pins
const int UP_pin = 4;
const int DOWN_pin = 13;
const int LEFT_pin = 14;
const int RIGHT_pin = 27;
int pin[4]={UP_pin,DOWN_pin,LEFT_pin,RIGHT_pin};
//SSD1306
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1        // No reset pin
#define OLED_ADDR 0x3C       // Most common I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//enumerations
  enum GAME_STATES{
    MENU,
    PLAYING,
    GAME_OVER,
    GAMES_MENU
  };
  GAME_STATES game_state;
  enum GAMES{
    GAME1,
    GAME2
  };
  GAMES games;
  enum BTN_STATES{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SEL,
    NON
  };
  BTN_STATES btn_state;
  enum MENU_BTN{
  PLAY,
  EXIT
  };
  MENU_BTN menu_btn;
//classes
class Menu_class{

  public:
  void handleInput(){
    for (size_t i = 0; i < 5; i++)
    {
      if (touchRead(pin[i])<30)/* checks if any pin is touched. */
      {
        //if touched, define btn state.
        switch (i)
        {
        case 0:
          btn_state = UP;
          break;
        case 1:
          btn_state = DOWN;
          break;
        case 2:
          btn_state = LEFT;
          break;
        case 3:
          btn_state = RIGHT;
          break;
        case 4:
          btn_state = SEL;
          break;
        default:
          btn_state = NON;
          break;
        
        }
      }
    }
  }
  void drawMenu_play(){
    const unsigned char playBitmap[] = {
    0b11111111,
    0b10000001,
    0b10111101,
    0b10100101,
    0b10111101,
    0b10000001,
    0b11111111,
    0b00000000
    };
    display.clearDisplay();
    //create a rectangle above and put the high score before the bottom bit map.
    display.drawBitmap(0, 0, playBitmap, 8, 8, WHITE);
    display.display();
  }
  void drawMenu_exit(){
    const unsigned char exitBitmap[] = {
    0b11111111,
    0b10000001,
    0b10111101,
    0b10100101,
    0b10111101,
    0b10000001,
    0b11111111,
    0b00000000
    };
    display.clearDisplay();
    //create a rectangle above and put the high score before the bottom bit map.
    display.drawBitmap(0, 0, exitBitmap, 8, 8, WHITE);
    display.display();
  }
  void drawGame_over(){
    const unsigned char exitBitmap[] = {
    0b11111111,
    0b10000001,
    0b10111101,
    0b10100101,
    0b10111101,
    0b10000001,
    0b11111111,
    0b00000000
    };
    display.clearDisplay();
    //create a rectangle above and put the high score before the bottom bit map.
    display.drawBitmap(0, 0, exitBitmap, 8, 8, WHITE);
    display.display();
  }
};
Menu_class menu_obj;
class GAMES_MENU_class : public Menu_class{
  //render is a bit different here handleinput is still the same.
  public:
  void drawGame1(){
    const unsigned char playBitmap[] = {
    0b11111111,
    0b10000001,
    0b10111101,
    0b10100101,
    0b10111101,
    0b10000001,
    0b11111111,
    0b00000000
    };
    display.clearDisplay();
    //create a rectangle above and put the high score before the bottom bit map.
    display.drawBitmap(0, 0, playBitmap, 8, 8, WHITE);
    display.display();
  }

  void drawGame2(){
    const unsigned char playBitmap[] = {
    0b11111111,
    0b10000001,
    0b10111101,
    0b10100101,
    0b10111101,
    0b10000001,
    0b11111111,
    0b00000000
    };
    display.clearDisplay();
    //create a rectangle above and put the high score before the bottom bit map.
    display.drawBitmap(0, 0, playBitmap, 8, 8, WHITE);
    display.display();
  }

};
GAMES_MENU_class games_menu_obj;
class Play_DDR_class:public Menu_class{
  public:
  void updateGame(){
    //game logic is defined here
  }
};
Play_DDR_class play_ddr_obj;


//Global Graphics functions
void displayText(int a, int x, int y, char str[65]){
  //display.clearDisplay();
  display.setTextSize(a);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(str);
  display.display();
}
void getCenter(int *x0, int *y0,int textSize, char text[65]){
  unsigned int width  = strlen(text) * 6 * textSize;
  unsigned int height = 8 * textSize;
  *x0 = (SCREEN_WIDTH  - width) / 2;
  *y0 = (SCREEN_HEIGHT - height) / 2;

}

void setup(){
  Serial.begin(115200);
  Serial.println("Initialising Tiny_Arcade!");
  //init initial states
  game_state = GAMES_MENU;
  menu_btn = PLAY;
  btn_state = NON;
  games = GAME1;
  //init button pins
  for (size_t i = 0; i < 4; i++)pinMode(pin[i], INPUT);
  //init intro
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("SSD1306 NOT FOUND!");
    while (1); // Stop if OLED not found
  }
  //Draw logo
  int x,y;
  getCenter(&x, &y, 1, "Tiny_Arcade");
  display.clearDisplay();
  displayText(1, x, y, "Tiny_Arcade");
  delay(1500);
  //wait for btn press
  displayText(1, 50, 10, "Click any Button");
  for(;;){
    menu_obj.handleInput();
    if(btn_state!=NON) break;
  }
  display.clearDisplay();
  display.display();

}

void loop(){//main loop

  bool esc = false;
  switch (game_state)
  {
  case 0://MENU
    /* Handleinputs and Render the menu UI*/
    while (!esc)
    {
      switch (menu_btn)
      {
      case 0://PLAY
        /* what to display when on top of PLAY */
        //initial state.
        //renders the frame1 and then renders the highscore.
        menu_obj.drawMenu_play();
        displayText(2, 7, 51, "highscore"/*put highscore which is converted to string here*/);
        break;
      
      case 1://EXIT
        /* what to display when on top of EXIT */
        //renders the frame2 and then renders the highscore.
        menu_obj.drawMenu_exit();
        displayText(2, 7, 51, "highscore"/*put highscore which is converted to string here*/);
        break;
      }

      for(;;){
        menu_obj.handleInput();
        if(btn_state==DOWN){menu_btn=EXIT; break;}
        else if(btn_state==UP){menu_btn=PLAY; break;}
        else if(btn_state==SEL){
          if(menu_btn==EXIT){
            //sets gameState to GAMES_MENU
            game_state=GAMES_MENU;
            esc=true;
            break;
          }else if(menu_btn==PLAY){
            //sets gameState to PLAYING
            game_state=PLAYING;
            esc=true;
            break;
          }
        }
      }
    }
    
    break;
  case 1://GAME_OVER    (Only difference in game_over and menu is that a "game over" title is displayed before the game_over menu)
    /* HandleInputs, update game logic and render the next frame */
    while (!esc)
    {
      //"Game Over" - title displaying
      menu_obj.drawGame_over();
      btn_state=NON;
      for(;;){
        menu_obj.handleInput();
        if(btn_state!=NON) break;
      }
      switch (menu_btn)
      {
      case 0://PLAY
        /* what to display when on top of PLAY */
        //initial state.
        //renders the frame1 and then renders the highscore.
        menu_obj.drawMenu_play();
        displayText(2, 7, 51, "highscore"/*put highscore which is converted to string here*/);
        break;
      
      case 1://EXIT
        /* what to display when on top of EXIT */
        //renders the frame2 and then renders the highscore.
        menu_obj.drawMenu_exit();
        displayText(2, 7, 51, "highscore"/*put highscore which is converted to string here*/);
        break;
      }

      for(;;){
        menu_obj.handleInput();
        if(btn_state==DOWN){menu_btn=EXIT; break;}
        else if(btn_state==UP){menu_btn=PLAY; break;}
        else if(btn_state==SEL){
          if(menu_btn==EXIT){
            //sets gameState to GAMES_MENU
            game_state=GAMES_MENU;
            esc=true;
            break;
          }else if(menu_btn==PLAY){
            //sets gameState to PLAYING
            game_state=PLAYING;
            esc=true;
            break;
          }
        }
      }
    }
    break;
  case 2://PLAYING
    /* according to the game selected, run the specific game */
    switch (games)
    {
    case 1://GAME1
      while (!esc)
      {
        /* game logic */
      }
      break;
    
    case 2://GAME2
      while (!esc)
      {
        /* game logic */
      }
      break;
    }
    break;
  case 3://GAMES_MENU
    while (!esc)
    {
      //draw the frame for game selection
      //when selected, set game state to playing and also set which game is selected.
      //init initial frame.
      games_menu_obj.drawGame1();
      for(;;){/* This menu selection logic [checking btn state for frame] will only work for 2 Games */
        menu_obj.handleInput();
        if(btn_state==DOWN){games=GAME2;
          games_menu_obj.drawGame2();
          continue;}
        else if(btn_state==UP){games=GAME1;
          games_menu_obj.drawGame1();
          continue;}
        else if(btn_state==SEL){
          //sets gameState to PLAYING
          game_state=PLAYING;
          esc=true;
          break;}
        }
      }
    
    break;
  }
}
