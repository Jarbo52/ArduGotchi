#include <SPI.h>
#include <EEPROM.h>
#include <Arduboy.h>
#include "PlatformPrecomp.h"
#include "game_bitmaps.h"

Arduboy arduboy;

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3
#define A 4
#define B 5

#define WIDTH 128
#define HEIGHT 64
#define FLIP_TIME 120

#define COOLDOWN 120

//Variables

//Life Stage
//0 = Baby, 1 = Kid, 2 = Teen, 3 = Adult
int stage;
int convert[4];
int chance[4];
boolean buttonPressed[6];
int THRESHOLD;
int actionCounter;

int petX;
int petY;

bool everIncentive;
bool evil;

bool paused;

//Metrics
int food;
int play;
int clean;
int incentive;
int affection;
int decayFactor;
bool isGood;

int happyHold = 0;
int sadHold = 0;
int poofHold = 0;

int buttonCooldown = COOLDOWN;

int idle = FLIP_TIME;
int phase = 0;
bool gameStart = false;
bool firstStart = true;

byte tick;

#include "pins_arduino.h"

void intro();
void draw();
void updatePressed();
void updateStats();
void boundMetrics();
int calculateIncentive();
void updateAffection();
void age();
void changeThreshold();
void babyDecide();
void changeGood();
void drawPauseScreen();
bool anyPressed();
void drawAll(bool);
void drawPetIdle();
void drawPetHappy();
void drawPetSad();
void movePet();
void gameTick();
void keepHappy();
void keepSad();
void keepPoof();
void drawBounding();
void drawIcons();
double genRandom();
int anyZero();
bool allZero();
void die();

void main_setup()
{
  arduboy.begin();
  arduboy.setFrameRate(60);
  arduboy.display();
  intro();
  gameStart = true;
}

void die()
{
  arduboy.drawBitmap(25,16,death,77,33,WHITE);

  arduboy.display();

  delay(5000);  
}

bool allZero()
{
  boundMetrics();
  return ((food == 0) && (clean == 0) && (play == 0));
}

int anyZero()
{
  int amt = 0;
  if(food == 0)
    amt+=1;
  if(clean == 0)
    amt+=1;
  if(play == 0)
    amt+=1;

  return amt;
}

double genRandom()
{
	return ((double)rand() / ((double)RAND_MAX + 1));
}

void gameTick()
{
  idle -= 1;
  
  if(idle < 0)
    idle = 0;

  if(idle == 0)
  {
    phase = 1-phase;  
    idle = FLIP_TIME;
  }

  buttonCooldown -= 1;

  if(buttonCooldown < 0)
    buttonCooldown = 0;

  happyHold -= 1;
  if(happyHold < 0)
    happyHold = 0;

  sadHold -= 1;
  if(sadHold < 0)
    sadHold = 0;

  poofHold -= 1;
  if(poofHold < 0)
    poofHold = 0;

  //movePet();
}

void movePet()
{
  int randX = (int) (genRandom() * 3) - 3;
  int randY = (int) (genRandom() * 4) - 4;

  petX += randX;
  petY += randY;

  if(petX > 88)
    petX = 88;
  if(petX < 30)
    petX = 30;
  if(petY > 40)
    petY = 40;
  if(petY < 24)
    petY < 24;
}

void keepHappy()
{
  drawPetHappy();
}

void keepSad()
{
  drawPetSad();
}

void keepPoof()
{
  arduboy.drawCircle(30,30,5,WHITE);
  arduboy.display();
}

void drawBounding()
{
  arduboy.drawRect(15,15,98,34,WHITE);

  arduboy.display();
}

void drawIcons()
{
  arduboy.drawBitmap(2,27,foodSprite,10,8,WHITE);
  arduboy.drawBitmap(59,2,playSprite,10,10,WHITE);
  arduboy.drawBitmap(119,27,cleanSprite,7,10,WHITE);
  arduboy.drawBitmap(59,51,incentiveSprite,11,11,WHITE);

  arduboy.display();
}

void drawAll(bool didAction)
{
  if(happyHold > 0)
  {
     drawPetHappy();
     return;
   }
  if(sadHold > 0)
  {
    drawPetSad();
    return;
  }

  if(didAction)
  {
    if(buttonPressed[DOWN] && buttonPressed[A])
    {
      if(incentive > THRESHOLD)
      {
        drawPetHappy();
        happyHold = 40;
      }
      else
      {
        drawPetSad();
        sadHold = 40;
      }
    }
    else
    {
      if(buttonPressed[A] && (buttonPressed[LEFT] || buttonPressed[RIGHT] || buttonPressed[UP]))
       {
          happyHold = 40; 
          drawPetHappy();
       }
      else
        drawPetIdle();
    }
  }
  else
  {
    drawPetIdle();
  }
}

void drawPetHappy()
{
  //Baby
  if(stage == 0)
  {
    arduboy.drawBitmap(petX,petY,babyHappy,10,8,WHITE); 
  }
  
  //Kid
  if(stage == 1)
  {
    if(isGood)
    {
      arduboy.drawBitmap(petX,petY,kidGoodHappy,14,12,WHITE); 
    }
    else
    {
      arduboy.drawBitmap(petX,petY,kidBadHappy,7,11,WHITE);
    }
  }

  //Teen
  if(stage == 2)
  {
    if(isGood)
    {
      arduboy.drawBitmap(petX,petY,teenGoodHappy,15,21,WHITE); 
    }
    else
    {
      arduboy.drawBitmap(petX,petY,teenBadHappy,20,21,WHITE);
    }
  }

  //Adult
  if(stage == 3)
  {
    if(isGood)
    {
      arduboy.drawBitmap(petX,petY,adultGoodHappy,10,20,WHITE);
    }
    else
    {
      arduboy.drawBitmap(petX,petY,adultBadHappy,22,25,WHITE);
    }
  }

  arduboy.display();
}

void drawPetSad()
{

  //Baby
  if(stage == 0)
  {
    arduboy.drawBitmap(petX,petY,babySad,10,8,WHITE); 
  }
  
  //Kid
  if(stage == 1)
  {
    if(isGood)
    {
      arduboy.drawBitmap(petX,petY,kidGoodSad,14,12,WHITE); 
    }
    else
    {
      arduboy.drawBitmap(petX,petY,kidBadSad,7,11,WHITE);
    }
  }

  //Teen
  if(stage == 2)
  {
    if(isGood)
    {
      arduboy.drawBitmap(petX,petY,teenGoodSad,14,21,WHITE); 
    }
    else
    {
      arduboy.drawBitmap(petX,petY,teenBadSad,20,21,WHITE);
    }
  }

  //Adult
  if(stage == 3)
  {
    if(isGood)
    {
      arduboy.drawBitmap(petX,petY,adultGoodSad,10,21,WHITE);
    }
    else
    {
      arduboy.drawBitmap(petX,petY,adultBadSad,26,28,WHITE);
    }
  }

  arduboy.display();
}

void drawPetIdle()
{

  //Baby
  if(stage == 0)
  {
    if(phase == 0) 
      arduboy.drawBitmap(petX,petY,babyIdleA,10,8,WHITE);
    else
      arduboy.drawBitmap(petX,petY,babyIdleB,10,8,WHITE); 
  }
  
  //Kid
  if(stage == 1)
  {
    if(isGood)
    {
      if(phase == 0) 
        arduboy.drawBitmap(petX,petY,kidGoodIdleA,14,12,WHITE);
      else
        arduboy.drawBitmap(petX,petY,kidGoodIdleB,14,12,WHITE); 
    }
    else
    {
      if(phase == 0) 
        arduboy.drawBitmap(petX,petY,kidBadIdleA,7,11,WHITE);
      else
        arduboy.drawBitmap(petX,petY,kidBadIdleB,10,10,WHITE);
    }
  }

  //Teen
  if(stage == 2)
  {
    if(isGood)
    {
      if(phase == 0) 
        arduboy.drawBitmap(petX,petY,teenGoodIdleA,14,21,WHITE);
      else
        arduboy.drawBitmap(petX,petY,teenGoodIdleB,14,20,WHITE); 
    }
    else
    {
      if(phase == 0) 
        arduboy.drawBitmap(petX,petY,teenBadIdleA,21,21,WHITE);
      else
        arduboy.drawBitmap(petX,petY,teenBadIdleB,21,21,WHITE);
    }
  }

  //Adult
  if(stage == 3)
  {
    if(isGood)
    {
      if(phase == 0) 
        arduboy.drawBitmap(petX,petY,adultGoodIdleA,10,21,WHITE);
      else
        arduboy.drawBitmap(petX,petY,adultGoodIdleB,10,21,WHITE);
    }
    else
    {
      if(phase == 0) 
        arduboy.drawBitmap(petX,petY,adultBadIdleA,22,25,WHITE);
      else
        arduboy.drawBitmap(petX,petY,adultBadIdleB,22,25,WHITE);
    }
  }

  //Evil
  if(stage == -1)
  {
    if(phase == 0) 
      arduboy.drawBitmap(petX,petY,evilIdleA,13,9,WHITE);
    else
      arduboy.drawBitmap(petX,petY,evilIdleB,13,10,WHITE); 
  }  

  arduboy.display();
}

bool anyPressed()
{
  for(int i = 0; i < 6; i++)
    if(buttonPressed[i])
      return true;

  return false;
}

void babyDecide()
{

  isGood = (0.6 * food + 0.2 * clean + 0.2 * play) > 6500;
}

void changeGood()
{

  isGood = affection > 5000;
}

void updatePressed()
{
  if(!evil)
  {
    //Left Right Up Down A B
    buttonPressed[LEFT] = arduboy.pressed(LEFT_BUTTON);
    buttonPressed[RIGHT] = arduboy.pressed(RIGHT_BUTTON);
    buttonPressed[UP] = arduboy.pressed(UP_BUTTON);
    buttonPressed[DOWN] = arduboy.pressed(DOWN_BUTTON);
    buttonPressed[A] = arduboy.pressed(A_BUTTON);
    buttonPressed[B] = arduboy.pressed(B_BUTTON);
  }
}

void ruinInput()
{
  for(int i = 0; i < 6; i++)
    buttonPressed[i] = false;
}

void intro()
{
  stage = 0;
  decayFactor = 1;
  convert[0] = 5;
  convert[1] = 10;
  convert[2] = 15;
  convert[3] = 20;
  chance[0] = 2;
  chance[1] = 3;
  chance[2] = 4;
  chance[3] = 5;
  actionCounter = 0;
  evil = false;
  food = 4500;
  play = 5000;
  clean = 6500;
  incentive = 50;
  THRESHOLD = 10000;
  affection = 5000;
  everIncentive = false;  
  isGood = false;
  paused = false;
  petX = WIDTH/2-5;
  petY = HEIGHT/2 - 5;
}

void updateStats(int deltaFood, int deltaPlay, int deltaClean)
{
  food += deltaFood;
  play += deltaPlay;
  clean += deltaClean;
}

void boundMetrics()
{
  if(food < 0)
    food = 0;
  if(food > 9999)
    food = 9999;
  if(play < 0)
    play = 0;
  if(play > 9999)
    play = 9999;
  if(clean < 0)
    clean = 0;
  if(clean > 9999)
    clean = 9999;
}

int calculateIncentive()
{
  return (int) (0.45 * food + 0.3 * play + 0.25 * clean);
}

void updateAffection(int deltaAffection)
{
  
  affection += deltaAffection;
}

void age()
{
  float random = genRandom() * chance[stage];
  
  if(random < 1)
  {
    stage += 1;
    
    //Win
    if(stage == 4)
    {
      arduboy.clear();

      arduboy.setCursor(40,20);
      arduboy.print("You win!!");

      arduboy.display();
      delay(5000);
      main_setup();
    }

    poofHold = 30;
    keepPoof();

    switch(stage)
    {
      case(1):
        petX = WIDTH/2 - 5;
        petY = HEIGHT/2 - 6;
        break;
      case(2):
        petX = WIDTH/2 - 8;
        petY = HEIGHT/2 - 10;
        break;
      case(3):
        petX = WIDTH/2 - 7;
        petY = HEIGHT/2 - 12;
        break;
      default:
        petX = WIDTH/2 - 5;
        petY = HEIGHT/2 - 6;
        break;
    }

    if(stage == 1)
      babyDecide();
    else
      changeGood();
    
    changeThreshold();
	
    actionCounter = 0;
    
    //Uh oh.
    if(stage == 2 && !everIncentive)
    {
      evil = true;
      stage = -1;
      ruinInput();
      decayFactor = 100;
    }
      
  }
}

void changeThreshold()
{
  if(stage == 1)
  {
    if(isGood)
      THRESHOLD = 3000;
    else
      THRESHOLD = 6000;
  }

  if(stage == 2)
  {
    if(isGood)
      THRESHOLD = 4000;
    else
      THRESHOLD = 8000;
  }

  if(stage == 3)
  {
    if(isGood)
      THRESHOLD = 3500;
    else
      THRESHOLD = 9000;
  }
}

void drawPauseScreen()
{
	arduboy.clear();
  
  //Bounding Box
  arduboy.drawRect(4,4,120,56,WHITE);
	arduboy.fillRect(5,5,118,54,BLACK);

  //Draw Words
  if(true)
  {
    arduboy.setCursor(45,6);
    arduboy.setTextSize(0.01);
    arduboy.print("STATUS");

    arduboy.setCursor(10,18);
    arduboy.setTextSize(0.01);
    arduboy.print("FOOD");

    arduboy.setCursor(10,33);
    arduboy.setTextSize(0.01);
    arduboy.print("PLAY");

    arduboy.setCursor(10,48);
    arduboy.setTextSize(0.01);
    arduboy.print("POOP"); 
  }
   
  //Draw large bars
  if(true)
  {
    //Draw Food Bar
    arduboy.drawRect(37,17,80,10,WHITE);

    //Draw Play Bar
    arduboy.drawRect(37,32,80,10,WHITE);

    //Draw Poo Bar
    arduboy.drawRect(37,47,80,10,WHITE);
  } 

  //Draw Smaller Bars
  if(true)
  {
    //To find length of bar, 
    //take value, divide by 100, then multiply by 0.8

    //Draw Food Bar
    arduboy.fillRect(37,18,(int)(float)food/100*4/5,9,WHITE);
    
    //Draw Play Bar
    arduboy.fillRect(37,33,(int)(float)play/100*4/5,9,WHITE);

    //Draw Poo Bar
    arduboy.fillRect(37,48,(int)(float)clean/100*4/5,9,WHITE);
  }

	arduboy.display();
}

void main_loop()
{
  if(!(arduboy.nextFrame()))
    return;

  if(!gameStart)
  {
    //Draw logo 
    //check for A press
    //Start game
    gameStart = true;
  }
  else
  {

    if(firstStart)
    {
      firstStart = false;
      arduboy.clear();
    }

    arduboy.clear();

    if(allZero())
    {
      die();
      main_setup();
    }

    gameTick();
  
    if(clean < 2000)
      arduboy.drawBitmap(100,30,poo,10,14,WHITE);

    updatePressed();

    boundMetrics();

    decayFactor = (anyZero() + 1);

    if(poofHold > 0)
      keepPoof();

    if(!paused)
    {
      drawBounding();
      drawIcons();
    }

    incentive = calculateIncentive();

    if(anyPressed() && buttonCooldown == 0 && !evil)
    {
      //Directional + A
      //Action Check
      if(buttonPressed[A])
      {

        buttonCooldown = COOLDOWN;

        //Left = Food
        if(buttonPressed[LEFT] && buttonPressed[A])
        {
          updateStats(1500,0,-500);
          drawAll(true);
        }
        //Right = Clean
        if(buttonPressed[RIGHT] && buttonPressed[A])
        {
          updateStats(0, 0, 2000);
          drawAll(true);
        }
        //Up = Play
        if(buttonPressed[UP] && buttonPressed[A])
        {
          updateStats(-750,2000,-750);
          drawAll(true);
        }
        //Down = Incentive
        if(buttonPressed[DOWN] && buttonPressed[A])
        {
          everIncentive = true;
          
          //If it likes you, do "insane" thing
          if(incentive > THRESHOLD)
          {
             updateAffection(500);
			       updateStats(-1000, -1000, -1000);
          }
          //Else don't do anything you are a failure as a parent
          else
          {
            updateAffection(-750);
            updateStats(-500,-500,-500);
          }

          drawAll(true);
        }
              
        actionCounter += 1;
        if(actionCounter > convert[stage])
          age();  
      }
      
      //Status Check
      if(buttonPressed[B])
      {

        buttonCooldown = COOLDOWN;

        paused = !paused;
        //Draw Pause Screen
        if(paused)
          drawPauseScreen();
      }

      drawPetIdle();
    }
    else
    {
      if(paused)
      {
        drawPauseScreen();
      }
      else
      {
        drawAll(false);  
        updateStats(-1*decayFactor,-1*decayFactor,-1*decayFactor);
      }        
    }
  }
  
}