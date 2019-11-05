#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <Adafruit_ADS1015.h>

#define F(I,S,N) for(I=S;I<N;I++)
#define W(A) while(A)
  
// shift register inputs
#define data 35
#define clk 36
#define latch 37
#define clr 40
#define en 41

// LED inputs
#define row0 4
#define row1 5
#define row2 2
#define row3 3
#define row4 6
#define row5 7
#define row6 8
#define row7 9

// switch inputs
#define scol0 42
#define scol1 43
#define scol2 44
#define scol3 45
#define scol4 46
#define scol5 47
#define scol6 48
#define scol7 49

#define srow0 22
#define srow1 23
#define srow2 24
#define srow3 25
#define srow4 26
#define srow5 27
#define srow6 28
#define srow7 29

// 0 for player vs computer
// 1 for player vs player
#define mode_selector 38
int mode = 0;

// 1 is chess
// 0 is checkers
#define game_selector 13
int game = 0;
                    
// each three in a 1d array refer to 1 LED
byte board[8][24] = {  {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 0
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1},    // row 1
                       {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 2
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1},    // row 3
                       {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 4
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1},    // row 5
                       {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 6
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1} };  // row 7

byte copy[8][24] = {   {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 0
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1},    // row 1
                       {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 2
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1},    // row 3
                       {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 4
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1},    // row 5
                       {B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0},    // row 6
                       {B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1, B0, B0, B0, B0, B1, B1} };  // row 7

// stores board pieces
char pieces[8][8];

// switch values
int switches[8][8] = {0};

// lightshow
byte light[8][24];

// store position of king
int kr, kc;

// adc variables
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
unsigned long average = 0, storedaverage = 0;

             
int V=112,M=136,S=128,I=8e3,C=799,Q,N,i, c[9];       /* V=0x70=rank mask, M=0x88 */

char O,K,L,
w[]={0,1,1,3,-1,3,5,9},                        /* relative piece values    */
o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
     7,-1,11,6,8,3,6,                          /* 1st dir. in o[] per piece*/
     6,3,5,7,4,5,3,6},                         /* initial piece setup      */
b[129],                                        /* board: half of 16x8+dummy*/
T[1035],                                       /* hash translation table   */

n[]="_?inkbrq?P?NKBRQ";                        /* piece symbols on printout*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 

  // set pvp or pvc
  pinMode(mode_selector, INPUT);
  mode = digitalRead(mode_selector);
  
  // set checkers or chess
  pinMode(game_selector, INPUT);
  game = digitalRead(game_selector);
  
  
  // Set pins for LEDs
  pinMode(row0, OUTPUT);
  pinMode(row1, OUTPUT);
  pinMode(row2, OUTPUT);
  pinMode(row3, OUTPUT);
  pinMode(row4, OUTPUT);
  pinMode(row5, OUTPUT);
  pinMode(row6, OUTPUT);
  pinMode(row7, OUTPUT);

  // set pins for shift registers
  pinMode(data, OUTPUT);
  pinMode(clk, OUTPUT);  
  pinMode(latch, OUTPUT);
  pinMode(clr, OUTPUT);
  pinMode(en, OUTPUT);

  // set up pins
  // columns are switches
  pinMode(scol0, INPUT_PULLUP);
  pinMode(scol1, INPUT_PULLUP);
  pinMode(scol2, INPUT_PULLUP);
  pinMode(scol3, INPUT_PULLUP);
  pinMode(scol4, INPUT_PULLUP);
  pinMode(scol5, INPUT_PULLUP);
  pinMode(scol6, INPUT_PULLUP);
  pinMode(scol7, INPUT_PULLUP);

  // rows are diodes
  pinMode(srow0, OUTPUT);
  pinMode(srow1, OUTPUT);
  pinMode(srow2, OUTPUT);
  pinMode(srow3, OUTPUT);
  pinMode(srow4, OUTPUT);
  pinMode(srow5, OUTPUT);
  pinMode(srow6, OUTPUT);
  pinMode(srow7, OUTPUT);

  digitalWrite(srow0, HIGH);
  digitalWrite(srow1, HIGH);
  digitalWrite(srow2, HIGH);
  digitalWrite(srow3, HIGH);
  digitalWrite(srow4, HIGH);
  digitalWrite(srow5, HIGH);
  digitalWrite(srow6, HIGH);
  digitalWrite(srow7, HIGH);

  // set up shift register
  digitalWrite(clr, HIGH);
  digitalWrite(latch, LOW);
  digitalWrite(data, LOW);
  digitalWrite(clk, LOW);
  digitalWrite(en, LOW);

  // adc
  ads.setGain(GAIN_ONE);
  ads.begin();

  lightshow();

  if (game)
    int val = runnin();
  
}

void loop() {
  // put your main code here, to run repeatedly:  
}

void read_board(byte chess[8][24]) {
  int i, j;
  
 // loop through columns
  for (i = 0; i < 8; i++) {
    
    switchrows(i);

    // loop through rows
    for (j = 0; j < 24; j++) {
      digitalWrite(data, chess[i][j]);
      digitalWrite(clk, HIGH);
      digitalWrite(clk, LOW);
    }

    digitalWrite(data, LOW);          // stop reading data
    digitalWrite(latch, HIGH);        // read data into latch
    digitalWrite(latch, LOW);

    digitalWrite(en, LOW);            // display row of LEDs
    delayMicroseconds(1000);
    digitalWrite(clr, LOW);           // clear shift registers
    digitalWrite(clr, HIGH);
    digitalWrite(en, HIGH);           // turn of display
  }
}

void lightshow() {
  int i, j, light_val = 0;
  int val1, val2, val3;
  
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 24; j+=3) {
      val1 = random(0, 2);
      val2 = random(0, 2);
      val3 = random(0, 2);

      light[i][j] = val1;
      light[i][j+1] = val2;
      light[i][j+2] = val3;

      if (val1 == val2 == val3 == 0) {
        int ran = random(0, 3);
        light[i][j+ran] = 1; 
      }

      while (light_val < 3) {
        read_board(light);
        light_val++;
      }
      light_val = 0;
    }
  }

  read_board(light);
}

char mygetchar(void) {
  while(Serial.available() <= 0);
  return (char)Serial.read();
}

void printboard() {
  int i, j;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      Serial.print(pieces[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
}

void printmove() {
  for (int i = 0; i < 4; i++)
    Serial.print((char)c[i]);
  Serial.println();
}

int check_help_1(char board[][8], int i, int j, int turn) {
  // columns
  
  if (turn == 8) {
    if (board[i][j] == 'q' || board[i][j] == 'r')
      return 1;
    else if (board[i][j] == 'k' || board[i][j] == 'b' || board[i][j] == 'n' || board[i][j] == 'i')
      return 0;
    else if (board[i][j]=='Q'||board[i][j]=='R'||board[i][j]=='B'||board[i][j]=='N'||board[i][j]=='P')
      return 0; 
  }
  
  if (turn == 16 && mode == 1) {
     if (board[i][j] == 'Q' || board[i][j] == 'R')
      return 1;
    else if (board[i][j] == 'K' || board[i][j] == 'B' || board[i][j] == 'N' || board[i][j] == 'P')
      return 0;
    else if (board[i][j]=='q'||board[i][j]=='r'||board[i][j]=='b'||board[i][j]=='n'||board[i][j]=='i')
      return 0; 
  }
  
  return -2;
}

// for diagonals
int check_help_2(char board[][8], int i, int j, int r, int c, int turn) {
  if (turn == 8) {
    if (board[i][j] == 'q' || board[i][j] == 'b')
      return 1;
    else if (board[i][j] == 'i' && (i==r-1) && (j==c-1 || j==c+1))
      return 1;
    else if (board[i][j] == 'k' || board[i][j] == 'r' || board[i][j] == 'n' || board[i][j] == 'i')
      return 0;
    else if (board[i][j]=='Q'||board[i][j]=='R'||board[i][j]=='B'||board[i][j]=='N'||board[i][j]=='P')
      return 0;
  }
  else if (turn == 16 && mode == 1) {
    if (board[i][j] == 'Q' || board[i][j] == 'B')
      return 1;
    else if (board[i][j] == 'P' && (i==r+1) && (j==c-1 || j==c+1))
      return 1;
    else if (board[i][j] == 'K' || board[i][j] == 'R' || board[i][j] == 'N' || board[i][j] == 'P')
      return 0;
    else if (board[i][j]=='q'||board[i][j]=='r'||board[i][j]=='b'||board[i][j]=='n'||board[i][j]=='i')
      return 0;
  }
  return -1;
}

int check_help_3(char board[][8], int r, int c, int turn) {

  // bottom right
  if (r < 6 && c <7) {
    if (board[r+2][c+1] == 'n' && turn == 8)
      return 1;
    else if (board[r+2][c+1] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // bottom left
  if (r < 6 && c > 0) {
    if (board[r+2][c-1] == 'n' && turn == 8)
      return 1;
    else if (board[r+2][c-1] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // top right
  if (r > 1 && c < 7) {
    if (board[r-2][c+1] == 'n' && turn == 8)
      return 1;
    else if (board[r-2][c+1] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // top left
  if (r > 1 && c > 0) {
    if (board[r-2][c-1] == 'n' && turn == 8)
      return 1;
    else if (board[r-2][c-1] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // middle bottom right
  if (r < 7 && c < 6) {
    if (board[r+1][c+2] == 'n' && turn == 8)
      return 1;
    else if (board[r+1][c+2] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // middle bottom left
  if (r < 7 && c > 1) {
    if (board[r+1][c-2] == 'n' && turn == 8)
      return 1;
    else if (board[r+1][c-2] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // middle top right
  if (r > 0 && c < 6) {
    if (board[r-1][c+2] == 'n' && turn == 8)
      return 1;
    else if (board[r-1][c+2] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  // middle top left
  if(r > 0 && c > 1) {
    if (board[r-1][c-2] == 'n' && turn == 8)
      return 1;
    else if (board[r-1][c-2] == 'N' && turn == 16 && mode == 1)
      return 1;
  }
  return 0;
}

void updateboard() {
  int i = 0, j = 0, q;

  // loop through board
  for(q = 0; q<121; q++) {
  // new row and reset column
    if (q&8 && (q+=7)) {
      i++; j=0;
    }
    else {
      pieces[i][j] = n[b[q]&15];    
     // increment column
      j++;
    }
  }
}

// function checks if Player king is in check
int check(int funct, int check_row, int check_col, int turn) {
  //char board[8][8] = {};
  int i = 0, j = 0, k, val = 0;

  // loop through board
  for (i = 0; i < 8; i++)
    for (j = 0; j < 8; j++)      
      // store king position
      if (pieces[i][j] == 'K' && turn == 8) {
        kr = i;
        kc = j;
      }
      else if (turn == 16 && mode == 1)
        if (pieces[i][j] == 'k') {
          kr = i;
          kc = j;
        }

  if (funct) {
    kr = check_row;
    kc = check_col;
  }

  // checking for check
  // searches down column of king
  for (i = kr, j = kc; i < 8; i++) {
    val = check_help_1(pieces, i, j, turn);
    if (val == 1)
      return val;
    else if (val == 0)
      break;
  }
               
  // searches up column of king
  for (i = kr, j = kc; i >= 0; i--) {
    val = check_help_1(pieces, i, j, turn);
    if (val == 1)
      return val+1;
    else if (val == 0)
      break;
  }

  // searches right column of king
  for (i = kr, j = kc; j < 8; j++) {
    val = check_help_1(pieces, i, j, turn);
    if (val == 1)
      return val+2;
    else if (val == 0)
      break;
  }
    
  // searches left column of king
  for (i = kr, j = kc; j >= 0; j--) {
    val = check_help_1(pieces, i, j, turn);
    if (val == 1)
      return val+3;
    else if (val == 0)
      break;
  }

  // search right up diagonal
  for (i = kr, j = kc; i >= 0, j < 8; i--, j++) {
    val = check_help_2(pieces, i ,j, kr, kc, turn);
    if (val == 1)
      return val+4;
    else if (val == 0)
      break;
  }

  // search left up diagonal
  for (i = kr, j = kc; i >= 0, j >= 0; i--, j--) {
    val = check_help_2(pieces, i, j, kr, kc, turn);
    if (val == 1)
      return val+5;
    else if (val == 0)
      break;
  }

  // check right down digonal
  for (i = kr, j = kc; i < 8, j <8; i++, j++) {
    val = check_help_2(pieces, i, j, kr, kc, turn);
    if (val == 1)
      return val+6;
    else if (val == 0)
      break;
  }

  // check left down diagonal
  for (i = kr, j = kc; i < 8, j >= 0; i++, j--) {
    val = check_help_2(pieces, i, j, kr, kc, turn);
    if (val == 1)
      return val+7;
    else if (val == 0)
      break;
  }

  // check for knights
  val = check_help_3(pieces, kr, kc, turn);
  if (val)
    return val+8; 
  else 
    return val = 0;
}

void switchrows(int row){
  int i, val;

  if (row == 0)
    val = row0;
  else if (row == 1)
    val = row1;
  else if (row == 2)
    val = row2;
  else if (row == 3)
    val = row3;
  else if (row == 4)
    val = row4;
  else if (row == 5)
    val = row5;
  else if (row == 6)
    val = row6;
  else if (row == 7)
    val = row7;

  for (i = 2; i < 10; i++) {
    if (i == val)
      digitalWrite(i, HIGH);   // turn row high
    else
      digitalWrite(i, LOW);    // turn other rows low
  } 
}

void reset_board(byte board[8][24], byte copy[8][24]) {
  int i, j;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 24; j++) {
      board[i][j] = copy[i][j];
    }
  }
}

void knight(int row, int col, int turn){
  int i = row, j = col;

  // if LED is off, all spots for knight to move
  // have their LED on
  if (!is_LED(i, j+2*(j+1))) {
    //board[i][j+2*(j+1) -1] = B1;          // turn on knight spot
    
    if (i < 6 && j < 7) {
      if (turn == 8 && pieces[i+2][j+1] > 90)
        board[i+2][j+2*(j+1)+3] = B0;       // top right
      else if (turn == 16 && pieces[i+2][j+1] < 97)
        board[i+2][j+2*(j+1)+3] = B0;
    }
    if (i < 6 && j > 0) {
      if (turn == 8 && pieces[i+2][j-1] > 90)
        board[i+2][j+2*(j+1)-3] = B0;       // top left
      else if (turn == 16 && pieces[i+2][j-1] < 97)
        board[i+2][j+2*(j+1)-3] = B0;
    }
    if (i > 1 && j < 7) {
      if (turn == 8 && pieces[i-2][j+1] > 90)
        board[i-2][j+2*(j+1)+3] = B0;       // bottom right
      else if (turn == 16 && pieces[i-2][j+1] < 97)
        board[i-2][j+2*(j+1)+3] = B0;
    }
    if (i > 1 && j > 0) {
      if (turn == 8 && pieces[i-2][j-1] > 90)
        board[i-2][j+2*(j+1)-3] = B0;       // bottom left
      else if (turn == 16 && pieces[i-2][j-1] < 97)
        board[i-2][j+2*(j+1)-3] = B0;
    }
    if (i < 7 && j < 6) {
      if (turn == 8 && pieces[i+1][j+2] > 90) 
        board[i+1][j+2*(j+1)+6] = B0;       // middle top right
      else if (turn == 16 && pieces[i+1][j+2] < 97)
        board[i+1][j+2*(j+1)+6] = B0;
    }
    if (i < 7 && j > 1) {
      if (turn == 8 && pieces[i+1][j-2] > 90)
        board[i+1][j+2*(j+1)-6] = B0;       // middle top left
      else if (turn == 16 && pieces[i+1][j-2] < 97)
        board[i+1][j+2*(j+1)-6] = B0;
    }
    if (i > 0 && j < 6) {
      if (turn == 8 && pieces[i-1][j+2] > 90)
        board[i-1][j+2*(j+1)+6] = B0;       // middle bottom right
      else if (turn == 16 && pieces[i-1][j+2] < 97)
        board[i-1][j+2*(j+1)+6] = B0;
    }
    if (i > 0 && j > 1) {
      if (turn == 8 && pieces[i-1][j-2] > 90)
        board[i-1][j+2*(j+1)-6] = B0;       // middle bottom left
      else if (turn == 16 && pieces[i-1][j-2] < 97)
        board[i-1][j+2*(j+1)-6] = B0;
    }
  }

  // if LED is on, all spots for knight are off
  else if (is_LED(i, j+2*(j+1))) {
    //board[i][j+2*(j+1)] = B0;
    
    if (i < 6 && j < 7) {
      if (turn == 8 && pieces[i+2][j+1] > 90)
        board[i+2][j+2*(j+1)+2] = B1;       // top right
      else if (turn == 16 && pieces[i+2][j+1] < 97)
        board[i+2][j+2*(j+1)+2] = B1;
    }
    if (i < 6 && j > 0) {
      if (turn == 8 && pieces[i+2][j-1] > 90)
        board[i+2][j+2*(j+1)-4] = B1;       // top left
      else if (turn == 16 && pieces[i+2][j-1] < 97)
        board[i+2][j+2*(j+1)-4] = B1;
    }
    if (i > 1 && j < 7) {
      if (turn == 8 && pieces[i-2][j+1] > 90)
        board[i-2][j+2*(j+1)+2] = B1;       // bottom right
      else if (turn == 16 && pieces[i-2][j+1] < 97)
        board[i-2][j+2*(j+1)+2] = B1;
    }
    if (i > 1 && j > 0) {
      if (turn == 8 && pieces[i-2][j-1] > 90)
        board[i-2][j+2*(j+1)-4] = B1;       // bottom left
      else if (turn == 16 && pieces[i-2][j-1] < 97)
        board[i-2][j+2*(j+1)-4] = B1;
    }
    if (i < 7 && j < 6) {
      if (turn == 8 && pieces[i+1][j+2] > 90)
        board[i+1][j+2*(j+1)+5] = B1;       // middle top right
      else if (turn == 16 && pieces[i+1][j+2] < 97)
        board[i+1][j+2*(j+1)+5] = B1;
    }
    if (i < 7 && j > 1) {
      if (turn == 8 && pieces[i+1][j-2] > 90)
        board[i+1][j+2*(j+1)-7] = B1;       // middle top left
      else if (turn == 16 && pieces[i+1][j-2] < 97)
        board[i+1][j+2*(j+1)-7] = B1;
    }
    if (i > 0 && j < 6) {
      if (turn== 8 && pieces[i-1][j+2] > 90)
        board[i-1][j+2*(j+1)+5] = B1;       // middle bottom right
      else if (turn == 16 && pieces[i-1][j+2] < 97)
        board[i-1][j+2*(j+1)+5] = B1;
    }
    if (i > 0 && j > 1) {
      if (turn == 8 && pieces[i-1][j-2] > 90)
        board[i-1][j+2*(j+1)-7] = B1;       // middle bottom left
      else if (turn == 16 && pieces[i-1][j-2] < 97)
        board[i-1][j+2*(j+1)-7] = B1;
    }
  }
}

int is_LED(int i , int j) {

  // returns 1 if LED is on
  if (board[i][j] == B1)
    return 1;

  // returns 0 if LED is off
  else if (board[i][j] == B0)
    return 0;
}

void pawn(int row, int col, int turn) {
  int i = row, j = col;
  
  if (i == 6 && turn == 8) {
    if (is_LED(i, j+2*(j+1))) {
      //board[i][j+2*(j+1)] = B0;       // lights up pawn position
      board[i-1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
      board[i-2][j+2*(j+1)] = B0;     // lights up pawn + 2; led on
    }
    else if (!is_LED(i, j+2*(j+1))) {
      //board[i][j+2*(j+1)-1] = B1;       // lights up pawn position
      board[i-1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on
      board[i-2][j+2*(j+1)-1] = B1;     // lights up pawn + 2; led off
    }
  }
  else if (i == 1 && turn == 16) {
    if (is_LED(i, j+2*(j+1))) {
      //board[i][j+2*(j+1)] = B0;       // lights up pawn position
      board[i+1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
      board[i+2][j+2*(j+1)] = B0;     // lights up pawn + 2; led on
    }
    else if (!is_LED(i, j+2*(j+1))) {
      //board[i][j+2*(j+1)-1] = B1;       // lights up pawn position
      board[i+1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on
      board[i+2][j+2*(j+1)-1] = B1;     // lights up pawn + 2; led off
    }
  }

  // move pawn one space
  if (turn == 8) {
    if (is_LED(i, j+2*(j+1)) && pieces[i-1][j] == 95) {
      //board[i][j+2*(j+1)] = B0;       // lights up pawn position
      board[i-1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
      
      // pawn capture
      if (pieces[i-1][j+1] > 95)
        board[i-1][j+2*(j+1)+3] = B0;
      else if (pieces[i-1][j-1] > 95)
        board[i-1][j+2*(j+1)-3] = B0;
    }
    else if (!is_LED(i, j+2*(j+1)) && pieces[i-1][j] == 95) {
      //board[i][j+2*(j+1)-1] = B1;       // lights up pawn position
      board[i-1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on
      
      // pawn capture
      if (pieces[i-1][j+1] > 95)
        board[i-1][j+2*(j+1)+2] = B1;
      else if (pieces[i-1][j-1] > 95)
        board[i-1][j+2*(j+1)-4] = B1;
    }
  }

  else if (turn == 16) {
    if (is_LED(i, j+2*(j+1)) && pieces[i+1][j] == 95) {
      //board[i][j+2*(j+1)] = B0;       // lights up pawn position
      board[i+1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
      
      // pawn capture
      if (pieces[i+1][j+1] < 95)
        board[i+1][j+2*(j+1)+3] = B0;
      else if (pieces[i+1][j-1] < 95)
        board[i+1][j+2*(j+1)-3] = B0;
    }
    else if (!is_LED(i, j+2*(j+1)) && pieces[i+1][j] == 95) {
      //board[i][j+2*(j+1)-1] = B1;       // lights up pawn position
      board[i+1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on
      
      // pawn capture
      if (pieces[i+1][j+1] < 95)
        board[i+1][j+2*(j+1)+2] = B1;
      else if (pieces[i+1][j-1] < 95)
        board[i+1][j+2*(j+1)-4] = B1;
    }
  }
}

void king(int row, int col, int turn){
  int i = row, j = col;
/*
  // light up center LED
  if (is_LED(i, j+2*(j+1)))
    board[i][j+2*(j+1)] = B0;
  else
    board[i][j+2*(j+1)-1] = B1;
*/
  // light up middle left
  if (col > 0)
    if (turn == 8 && pieces[i][j-1] > 90 || turn == 16 && pieces[i][j-1] < 97)
      if (!check(1, i, j-1, turn)) {
        // led is on
        if (is_LED(i, j+2*(j+1) - 3))
          board[i][j+2*(j+1) - 3] = B0;
        // led is off
        else if (!is_LED(i, j+2*(j+1) - 3))
          board[i][j+2*(j+1) - 4] = B1;
      }

  // light up middle right
  if (col < 7)
    if (turn == 8 && pieces[i][j+1] > 90 || turn == 16 && pieces[i][j+1] < 97)
      if (!check(1, i, j+1, turn)) {
        if (is_LED(i, j+(2*(j+1))+3))
          board[i][j+(2*(j+1))+3] = B0;
        else if(!is_LED(i, j+(2*(j+1))+3))
          board[i][j+(2*(j+1))+2] = B1;
      }
  
  // light up top row
  if (row < 7) {
    // light up top left
    if (col > 0)
      if (turn == 8 && pieces[i+1][j-1] > 90 || turn == 16 && pieces[i+1][j-1] < 97)
        if (!check(1, i+1, j-1, turn)) {
          if (is_LED(i+1, j+2*(j+1) - 3))
            board[i+1][j+2*(j+1) - 3] = B0;
          else if (!is_LED(i+1, j+2*(j+1) - 3))
            board[i+1][j+2*(j+1) - 4] = B1;
        }

    // light up top right
    if (col < 7)
      if (turn == 8 && pieces[i+1][j+1] > 90 || turn == 16 && pieces[i+1][j+1] < 97)
        if (!check(1, i+1, j+1, turn)) {
          if (is_LED(i+1, j+(2*(j+1))+3))
            board[i+1][j+(2*(j+1))+3] = B0;
          else if (!is_LED(i+1, j+(2*(j+1))+3))
            board[i+1][j+(2*(j+1))+2] = B1;
      }       
    
    // light up top middle
    if (turn == 8 && pieces[i+1][j] > 90 || turn == 16 && pieces[i+1][j] < 97)
      if (!check(1, i+1, j, turn)) {
        if (is_LED(i+1, j+2*(j+1)))
          board[i+1][j+2*(j+1)] = B0;
        else if (!is_LED(i+1, j+2*(j+1)))
          board[i+1][j+2*(j+1)-1] = B1;
      }  
  }

  // light up bottom row
  if (row > 0) {
    // light up bottom left
    if (col > 0)
      if (turn == 8 && pieces[i-1][j-1] > 90 || turn == 16 && pieces[i-1][j-1] < 97)
      if (!check(1, i-1, j-1, turn)) {
        if (is_LED(i-1, j+2*(j+1) - 3))
          board[i-1][j+2*(j+1) - 3] = B0;
        else if (!is_LED(i-1, j+2*(j+1) - 3))
          board[i-1][j+2*(j+1) - 4] = B1;
      }

    // light up bottom right
    if (col < 7)
      if (turn == 8 && pieces[i-1][j+1] > 90 || turn == 16 && pieces[i-1][j+1] < 97)
        if (!check(1, i-1, j+1, turn)) {
          if (is_LED(i-1, j+(2*(j+1))+3))
            board[i-1][j+(2*(j+1))+3] = B0;
          else if (!is_LED(i-1, j+(2*(j+1))+3))
            board[i-1][j+(2*(j+1))+2] = B1;
        }

    // light up bottom middle
    if (turn == 8 && pieces[i-1][j] > 90 || turn == 16 && pieces[i-1][j] < 97)
      if (!check(1, i-1, j, turn)) {
        if (is_LED(i-1, j+2*(j+1)))
          board[i-1][j+2*(j+1)] = B0;
        else if (!is_LED(i-1, j+2*(j+1)))
          board[i-1][j+2*(j+1)-1] = B1;
      }
  }
}

void bishop_help(int i, int j) {
  // if blue LED is on
  if (board[i][j+(2*(j+1))] == B1)
    // turns blue LED off
    board[i][j+(2*(j+1))] = B0;

  // works for when LED is off
  // compares if blue and green led are equal
  // an off led will have equal blue and green leds
  else if (board[i][j+(2*(j+1)) - 1] == board[i][j+(2*(j+1))]){
    // turn green LED on
    board[i][j+(2*(j+1)) - 1] = B1;
  }
  
}

void bishop(int row, int col, int turn){
  int i, j;
  // diagonal right and up
  for (i = row+1, j = col+1; ; i++, j++) {
    if (j > 7 || i > 7 || (pieces[i][j] < 90 && turn == 8) || (pieces[i][j] > 95 && turn == 16))
      break;
    bishop_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (pieces[i][j] < 95 && turn == 16))
      break;
  }

  // diagonal left and up
  for (i = row+1, j = col-1; ; i++, j--) {
    if (i > 7 || j < 0 || (pieces[i][j] < 90 && turn == 8) || (pieces[i][j] > 95 && turn == 16))
      break;
    bishop_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (pieces[i][j] < 95 && turn == 16))
      break;
  }

  // diagonal right and down
  for (i = row-1, j = col+1; ; i--, j++) {
    if (i < 0 || j > 7 || (pieces[i][j] < 90 && turn == 8) || (pieces[i][j] > 95 && turn == 16))
      break;
    bishop_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (pieces[i][j] < 95 && turn == 16))
      break;
  }

  // diagonal left and down
  for (i = row-1, j = col-1; ; i--, j--) {
    if (i < 0 || j < 0 || (pieces[i][j] < 90 && turn == 8) || (pieces[i][j] > 95 && turn == 16))
      break;
    bishop_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (pieces[i][j] < 95 && turn == 16))
      break;
  }
    
}

void rook_help(int i, int j) {
  // if row is even and column is even
  if (i % 2 == 0 && j % 2 == 0)
    board[i][j+(2*(j+1))] = B0;

  // if row is even and column is odd
  else if (i % 2 == 0 && j % 2 == 1)
    board[i][j+(2*(j+1)) - 1] = B1;
    
  // if row is odd and column is even
  else if (i % 2 == 1 && j % 2 == 0)
    board[i][j+(2*(j+1)) - 1] = B1;

  // if row is odd and column is odd
  else if (i % 2 == 1 && j % 2 == 1)
    board[i][j+(2*(j+1))] = B0;
}

void rook(int row, int col, int turn){
  int i, j;
  // right row
  for (i = row, j = col+1; j < 8; j++) {
    if ((turn == 8 && pieces[i][j] < 90) || (turn == 16 && pieces[i][j] > 95))
      break;
    rook_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (turn == 16 && pieces[i][j] < 95))
      break;
  }

  // up column;
  for (i = row+1, j = col; i < 8; i++) {
    if ((turn == 8 && pieces[i][j] < 90) || (turn == 16 && pieces[i][j] > 95))
      break;
    rook_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (turn == 16 && pieces[i][j] < 95))
      break;
  }

  // down column
  for (i = row-1, j = col; i >= 0; i--) {
    if ((turn == 8 && pieces[i][j] < 90) || (turn == 16 && pieces[i][j] > 95))
      break;
    rook_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (turn == 16 && pieces[i][j] < 95))
      break;
  }

  // left row
  for (i = row, j = col-1; j >= 0; j--) {
    if ((turn == 8 && pieces[i][j] < 90) || (turn == 16 && pieces[i][j] > 95))
      break;
    rook_help(i, j);
    if ((pieces[i][j] > 95 && turn == 8) || (turn == 16 && pieces[i][j] < 95))
      break;
  }
}

void queen(int row, int col, int turn){
  bishop(row, col, turn);
  rook(row, col, turn);
}

// computer move will be blue
void computer_led(int row_1, int col_1, int row_2, int col_2) {
  // check starting position
  if (is_LED(row_1, col_1+2*(col_1+1))) {
    board[row_1][col_1+2*(col_1+1)] = B0;       // turn blue off
    board[row_1][col_1+2*(col_1+1)-2] = B1;     // turn red on
  }
 else {
  // turn start led on
   board[row_1][col_1+2*(col_1+1)-1] = B1;        // turn green on
   board[row_1][col_1+2*(col_1+1)-2] = B1;        // turn red on
 }
   
    
  // check spot to move piece
  if (is_LED(row_2, col_2+2*(col_2+1))) {
    board[row_2][col_2+2*(col_2+1)] = B0;       // turn off blue
    board[row_2][col_2+2*(col_2+1)-2] = B1;     // turn on red
  }
  // led is off only need to turn green and red
  else {
    board[row_2][col_2+2*(col_2+1)-1] = B1;     // turn on green
    board[row_2][col_2+2*(col_2+1)-2] = B1;     // turn on yellow
  }
}

void switch_state() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      Serial.print(switches[i][j]);
      Serial.print(' ');
    }
    Serial.println();
  }
  Serial.println();
}

void player_pawn(int row, int col, int piece) {
  int i = row, j = col;

  // piece = 1 means white piece
  // move two places
  if (i == 6 && piece == 1) {
    // led is on
    if (is_LED(i, j+2*(j+1))) {
      
      if (switches[row-1][col] == 1)
        board[i-1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
      
      if (switches[row-2][col] == 1)
        board[i-2][j+2*(j+1)] = B0;     // lights up pawn + 2; led on
    }
    
    else if (!is_LED(i, j+2*(j+1))) {
      if (switches[row-1][col] == 1)
        board[i-1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on

      if (switches[row-2][col] == 1)
        board[i-2][j+2*(j+1)-1] = B1;     // lights up pawn + 2; led off
    }
  }
  // piece = 0 is black
  else if (i == 1 && piece == 0) {
    if (is_LED(i, j+2*(j+1))) {
      if (switches[row+1][col] == 1)
        board[i+1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off

      if (switches[row+2][col] == 1)
        board[i+2][j+2*(j+1)] = B0;     // lights up pawn + 2; led on
    }
    else if (!is_LED(i, j+2*(j+1))) {
      if (switches[row+1][col] == 1)
        board[i+1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on

      if (switches[row+1][col] == 1)
        board[i+2][j+2*(j+1)-1] = B1;     // lights up pawn + 2; led off
    }
  }

  // move pawn one space
  if (piece == 1) {
    if (is_LED(i, j+2*(j+1))) {
      if (switches[row-1][col] == 1)
        board[i-1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
    }
    else if (!is_LED(i, j+2*(j+1))) {
      if (switches[row-1][col] == 1)
        board[i-1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on
    }

    // pawn capture
    if(is_LED(i, j+2*(j+1))) { 
      if (switches[i-1][j+1] == 0) {
        board[i-1][j+2*(j+1)+3] = B0; // turn off blue
        board[i-1][j+2*(j+1)+1] = B1; // turn on red
      }
      if (switches[i-1][j-1] == 0) {
        board[i-1][j+2*(j+1)-3] = B0; // turn off blue
        board[i-1][j+2*(j+1)-5] = B1; // turn on red
      }
    }

    else if (!is_LED(i, j+2*(j+1))) {
      if (switches[i-1][j+1] == 0) {
        board[i-1][j+2*(j+1)+2] = B1; // turn on green
        board[i-1][j+2*(j+1)+1] = B1; // turn on red
      }
      if (switches[i-1][j-1] == 0) {
        board[i-1][j+2*(j+1)-4] = B1; // turn on green
        board[i-1][j+2*(j+1)-5] = B1; // turn on red
      }
    }
  }

  else if (piece == 0) {
    if (is_LED(i, j+2*(j+1))) {
      if (switches[row+1][col] == 1)
        board[i+1][j+2*(j+1)-1] = B1;   // lights up pawn + 1; led off
    }
    else if (!is_LED(i, j+2*(j+1))) {
      if (switches[row+1][col] == 1)
        board[i+1][j+2*(j+1)] = B0;   // lights up pawn + 1; led on
    }

    // capture pawn
    // led is on
    if (is_LED(i, j+2*(j+1))) {
      if (switches[i+1][j+1] == 0) {
        board[i+1][j+2*(j+1)+3] = B0;
        board[i+1][j+2*(j+1)+1] = B1;        
      }
      if (switches[i+1][j-1] == 0) {
        board[i+1][j+2*(j+1)-3] = B0;
        board[i+1][j+2*(j+1)-5] = B1;
      }
    }
    else if (!is_LED(i, j+2*(j+1))) {
      if (switches[i+1][j+1] == 0) {
        board[i+1][j+2*(j+1)+2] = B1;
        board[i+1][j+2*(j+1)+1] = B1;
      }
      if (switches[i+1][j-1] == 0) {
        board[i+1][j+2*(j+1)-4] = B1;
        board[i+1][j+2*(j+1)-5] = B1; // turn on red
      }
    }
        
  }
}

void player_rook(int row, int col) {
  int i, j;
  // right row
  for (i = row, j = col+1; j < 8; j++) {
    if (switches[i][j] == 0)
      break;
    rook_help(i, j);
  }

  // up column;
  for (i = row+1, j = col; i < 8; i++) {
    if (switches[i][j] == 0)
      break;
    rook_help(i, j);
  }

  // down column
  for (i = row-1, j = col; i >= 0; i--) {
    if (switches[i][j] == 0)
      break;
    rook_help(i, j);
  }

  // left row
  for (i = row, j = col-1; j >= 0; j--) {
    if (switches[i][j] == 0)
      break;
    rook_help(i, j);
  }
}

void player_bishop(int row, int col) {
  int i, j;
  // diagonal right and down
  for (i = row+1, j = col+1; i < 8, j < 8; i++, j++) {
    if (switches[i][j] == 0)
      break;
    bishop_help(i, j);
  }

  // diagonal left and down
  for (i = row+1, j = col-1; i < 8, j >= 0; i++, j--) {
    if (switches[i][j] == 0)
      break;
    bishop_help(i, j);
  }

  // diagonal right and up
  for (i = row-1, j = col+1; i >= 0, j < 8; i--, j++) {
    if (switches[i][j] == 0)
      break;
    bishop_help(i, j);
  }

  // diagonal left and up
  for (i = row-1, j = col-1; i >= 0, j >= 0; i--, j--) {
    if (switches[i][j] == 0)
      break;
    bishop_help(i, j);
  }
}

void player_knight(int row, int col) {
  int i = row, j = col;

  // if LED is off, all spots for knight to move
  // have their LED on
  if (!is_LED(i, j+2*(j+1))) {
    if (i < 6 && j < 7) {
      if (switches[i+2][j+1] == 1)
        board[i+2][j+2*(j+1)+3] = B0;       // top right
    }
    if (i < 6 && j > 0) {
      if (switches[i+2][j-1] == 1)
        board[i+2][j+2*(j+1)-3] = B0;       // top left
    }
    if (i > 1 && j < 7) {
      if (switches[i-2][j+1] == 1)
        board[i-2][j+2*(j+1)+3] = B0;       // bottom right
    }
    if (i > 1 && j > 0) {
      if (switches[i-2][j-1] == 1)
        board[i-2][j+2*(j+1)-3] = B0;       // bottom left
    }
    if (i < 7 && j < 6) {
      if (switches[i+1][j+2] == 1) 
        board[i+1][j+2*(j+1)+6] = B0;       // middle top right
    }
    if (i < 7 && j > 1) {
      if (switches[i+1][j-2] == 1)
        board[i+1][j+2*(j+1)-6] = B0;       // middle top left
    }
    if (i > 0 && j < 6) {
      if (switches[i-1][j+2] == 1)
        board[i-1][j+2*(j+1)+6] = B0;       // middle bottom right
    }
    if (i > 0 && j > 1) {
      if (switches[i-1][j-2] == 1)
        board[i-1][j+2*(j+1)-6] = B0;       // middle bottom left
    }
  }

  // if LED is on, all spots for knight are off
  else if (is_LED(i, j+2*(j+1))) {
    
    if (i < 6 && j < 7) {
      if (switches[i+2][j+1] == 1)
        board[i+2][j+2*(j+1)+2] = B1;       // top right
    }
    if (i < 6 && j > 0) {
      if (switches[i+2][j-1] == 1)
        board[i+2][j+2*(j+1)-4] = B1;       // top left
    }
    if (i > 1 && j < 7) {
      if (switches[i-2][j+1] == 1)
        board[i-2][j+2*(j+1)+2] = B1;       // bottom right
    }
    if (i > 1 && j > 0) {
      if (switches[i-2][j-1] == 1)
        board[i-2][j+2*(j+1)-4] = B1;       // bottom left
    }
    if (i < 7 && j < 6) {
      if (switches[i+1][j+2] == 1)
        board[i+1][j+2*(j+1)+5] = B1;       // middle top right
    }
    if (i < 7 && j > 1) {
      if (switches[i+1][j-2] == 1)
        board[i+1][j+2*(j+1)-7] = B1;       // middle top left
    }
    if (i > 0 && j < 6) {
      if (switches[i-1][j+2] == 1)
        board[i-1][j+2*(j+1)+5] = B1;       // middle bottom right
    }
    if (i > 0 && j > 1) {
      if (switches[i-1][j-2] == 1)
        board[i-1][j+2*(j+1)-7] = B1;       // middle bottom left
    }
  }
}

void player_king(int row, int col) {
  int i = row, j = col;

  // light up middle left
  if (col > 0) {
    // led is on
    if (is_LED(i, j+2*(j+1) - 3) && switches[i][j-1] == 1)
      board[i][j+2*(j+1) - 3] = B0;
    // led is off
      else if (!is_LED(i, j+2*(j+1) - 3) && switches[i][j-1] == 1)
        board[i][j+2*(j+1) - 4] = B1;
  }

  // light up middle right
  if (col < 7) {
    if (is_LED(i, j+(2*(j+1))+3) && switches[i][j+1] == 1)
      board[i][j+(2*(j+1))+3] = B0;
    else if(!is_LED(i, j+(2*(j+1))+3) && switches[i][j+1] == 1)
      board[i][j+(2*(j+1))+2] = B1;
  }
  
  // light up top row
  if (row < 7) {
    // light up top left
    if (col > 0) {
      if (is_LED(i+1, j+2*(j+1) - 3) && switches[i+1][j-1] == 1)
        board[i+1][j+2*(j+1) - 3] = B0;
      else if (!is_LED(i+1, j+2*(j+1) - 3) && switches[i+1][j-1] == 1)
        board[i+1][j+2*(j+1) - 4] = B1;
    }

    // light up top right
    if (col < 7) {
      if (is_LED(i+1, j+(2*(j+1))+3) && switches[i+1][j+1] == 1)
        board[i+1][j+(2*(j+1))+3] = B0;
      else if (!is_LED(i+1, j+(2*(j+1))+3) && switches[i+1][j+1] == 1)
        board[i+1][j+(2*(j+1))+2] = B1;
    }       
    
    // light up top middle
    if (is_LED(i+1, j+2*(j+1)) && switches[i+1][j] == 1)
      board[i+1][j+2*(j+1)] = B0;
    else if (!is_LED(i+1, j+2*(j+1)) && switches[i+1][j] == 1)
      board[i+1][j+2*(j+1)-1] = B1;
     
  }

  // light up bottom row
  if (row > 0) {
    // light up bottom left
    if (col > 0) {
      if (is_LED(i-1, j+2*(j+1) - 3) && switches[i-1][j-1] == 1)
        board[i-1][j+2*(j+1) - 3] = B0;
      else if (!is_LED(i-1, j+2*(j+1) - 3)  && switches[i-1][j-1] == 1)
        board[i-1][j+2*(j+1) - 4] = B1;
    }

    // light up bottom right
    if (col < 7) {
      if (is_LED(i-1, j+(2*(j+1))+3) && switches[i-1][j+1] == 1)
        board[i-1][j+(2*(j+1))+3] = B0;
       else if (!is_LED(i-1, j+(2*(j+1))+3)  && switches[i-1][j+1] == 1)
        board[i-1][j+(2*(j+1))+2] = B1;
    }

    // light up bottom middle
    if (is_LED(i-1, j+2*(j+1)) && switches[i-1][j] == 1)
      board[i-1][j+2*(j+1)] = B0;
    else if (!is_LED(i-1, j+2*(j+1))  && switches[i-1][j] == 1)
      board[i-1][j+2*(j+1)-1] = B1;
  }
}

void player_queen(int l, int m) {
  player_rook(l, m);
  player_bishop(l, m);
}

void adc_calculate(int l, int m, int turn) {
  // finds change in adc
  long val1 = (storedaverage-32768) * -1;
  long val2 = (average - 32768) * -1;
  unsigned long val3 = val1 * val2;
  unsigned long val4 = storedaverage - average;
  unsigned long val5 = val4 * 32768;
  unsigned long deltadc = 20 * (val3 / val5);
   
  //unsigned long deltadc = (20*(storedaverage-32768)*(average-32768))/(32768*(storedaverage-average));
  Serial.println(deltadc);

  if (deltadc >= 650 && deltadc <= 1250) {
    Serial.println("It's a white pawn");
    player_pawn(l, m, 1);
  }

  else if (deltadc >= 160 && deltadc <= 180) {
    Serial.println("It's a white rook");
    player_rook(l, m);
  }

  else if (deltadc >= 350 && deltadc <= 480) {
    Serial.println("It's a white knight");
    player_knight(l, m);
  }

  else if (deltadc >= 30 && deltadc <= 50) {
    Serial.println("It's a white king");
    player_king(l, m);
  }

  else if (deltadc >= 240 && deltadc <= 290) {
    Serial.println("It's white bishop");
    player_bishop(l, m);
  }

  else if (deltadc >= 90 && deltadc <= 120) {
    Serial.println("It's a white queen");
    player_queen(l, m);
  }

  else if (deltadc >= 1251 && deltadc <= 4000) {
    Serial.println("It's a black pawn");
    player_pawn(l, m, 0);
  }

  else if (deltadc >= 200 && deltadc <= 230) {
    Serial.println("It's a black rook");
    player_rook(l, m);
  }

  else if (deltadc >= 500 && deltadc <= 600) {
    Serial.println("It's a black knight");
    player_knight(l, m);
  }

  else if (deltadc >= 51 && deltadc <= 70) {
    Serial.println("It's a black king");
    player_king(l, m);
  }

  else if (deltadc >= 300 && deltadc <= 340) {
    Serial.println("It's black bishop");
    player_bishop(l, m);
  }

  else if (deltadc >= 130 && deltadc <= 159) {
    Serial.println("It's a black queen");
    player_queen(l, m);
  }
} 

void adc(int todo) {
  unsigned long adc0, total = 0;
  int readings = 5;

  adc0 = ads.readADC_SingleEnded(0);

  if (todo) {
    for (int i = 0; i < readings; i++) {
      Serial.println(adc0);
      total += adc0;
    }
    
    average = total / readings;
  }
  else {
    for (int i = 0; i < readings; i++) {
      Serial.println(adc0);
      total += adc0;
    }
    
    storedaverage = total / readings;
  }

  Serial.println();
}

int reed_switch(int start, int scan, int turn) {
  int i, j, k, l, m, n;

  // scan rows
  // row pins increment up
  for (i = srow0, l = 0; i <= srow7; i++, l++) {
    // set row to LOW - make ground
    digitalWrite(i, LOW);

    read_board(board);
    
    // scan columns
    for (j = scol0, m = 0; j <= scol7; j++, m++) {
      digitalWrite(j, HIGH);
      read_board(board);
      
      // read switch value
      k = digitalRead(j);

      // if k is zero, switch has piece on it
      // start allows function to compare switches to board      
      if (start) {
        if (switches[l][m] == 0 && k == 1) {
          c[0] = m + 97;
          c[1] = (8 - l) + 48;

          if (mode == 1) {
            adc(1);
            adc_calculate(l, m, turn);
          }
          
          switches[l][m] = k;
          if (turn == 8 || (turn == 16 && mode == 1))
            highlight_move(l, m, turn);
          return 1;
        }
      }

      else if (scan) {
        if (switches[l][m] == 0 && k == 1) {
          switches[l][m] = k;
        }
        
        else if (switches[l][m] == 1 && k == 0) {
          switches[l][m] = k;
          c[2] = m + 97;
          c[3] = (8 - l) + 48;

          // check if piece was not moved
          if (c[2] == c[0] && c[1] == c[3]) {
            digitalWrite(j, LOW);
            digitalWrite(i, HIGH);
            return 1;
          }

          // check for valid moves
          if (board[l][m+(2*(m+1))] == B1) {
            board[l][m+(2*(m+1))] = B0;
            board[l][m+(2*(m+1)) -1] = B0;
            board[l][m+(2*(m+1)) -2] = B1;
            digitalWrite(j, LOW);
            digitalWrite(i, HIGH);
            return 0;
          }

          else if (board[l][m+(2*(m+1))] == B0 && board[l][m+(2*(m+1))-1] == B0) {
            board[l][m+(2*(m+1)) -2] = B1;
            digitalWrite(j, LOW);
            digitalWrite(i, HIGH);
            return 0;
          }
         
          digitalWrite(j, LOW);
          digitalWrite(i, HIGH);
          return 1;
        }
      }
      
      else {
        // store switch value in array
        switches[l][m] = k;
      }
      digitalWrite(j, LOW);
    }
     // reset row to input
     digitalWrite(i, HIGH);
  }
  return 0;
}

void highlight_move(int i, int j, int k) {
  read_board(board);
  
  // if k is 16 the player1 made a valid move
  // if k is 8, player2 made a valid move; only highlight mode if player v player
  if (k == 8 || (k == 16 && mode == 1)) {
    //Serial.println(k);
    if (pieces[i][j] == 'P' || pieces[i][j] == 'i')
      pawn(i, j, k);
    else if (pieces[i][j] == 'N' || pieces[i][j] == 'n')
      knight(i, j, k);
    else if (pieces[i][j] == 'B' || pieces[i][j] == 'b')
      bishop(i, j, k);
    else if (pieces[i][j] == 'R' || pieces[i][j] == 'r')
      rook(i, j, k);
    else if (pieces[i][j] == 'Q' || pieces[i][j] == 'q')
      queen(i, j, k);
    else if (pieces[i][j] == 'K' || pieces[i][j] == 'k')
      king(i, j, k);
  }
}

/*                              micro-Max,                                */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller */

// function is the chess engine
int D(int k,int q,int l,int e,int E,int z,int n)    
/* recursive minimax search, k=moving side, n=depth*/
/* (q,l)=window, e=current eval. score, E=e.p. sqr.*/
/* e=score, z=prev.dest; J,Z=hashkeys; return score*/
{                       
 int j,r,m,v,d,h,i=0,F,G;
 char t,p,u,x,y,X,Y,H,B;
 read_board(board);

 d=X=Y=0;                                /* start iter., no best yet */
 N++;                                          /* node count (for timing)  */
 W(d++<n|z==8&N<1e2&d<98)                      /* iterative deepening loop */
 {x=B=X;                                       /* start scan at prev. best */
  Y|=8&Y>>4;                                   /* request try noncastl. 1st*/
  m=d>1?-I:e;                                  /* unconsidered:static eval */
  do{u=b[x];                                   /* scan board looking for   */
   if(u&k)                                     /*  own piece (inefficient!)*/
   {r=p=u&7;                                   /* p = piece type (set r>0) */
    j=o[p+16];                                 /* first step vector f.piece*/
    W(r=p>2&r<0?-r:-o[++j])                    /* loop over directions o[] */
    {A:                                        /* resume normal after best */
     y=x;F=G=S;                                /* (x,y)=move, (F,G)=castl.R*/
     do{H=y+=r;                                /* y traverses ray          */
      if(Y&8)H=y=Y&~M;                         /* sneak in prev. best move */
      if(y&M)break;                            /* board edge hit           */
      if(p<3&y==E)H=y^16;                      /* shift capt.sqr. H if e.p.*/
      t=b[H];if(t&k|p<3&!(r&7)!=!t)break;      /* capt. own, bad pawn mode */
      i=99*w[t&7];                             /* value of capt. piece t   */
      if(i<0||E-S&&b[E]&&y-E<2&E-y<2)m=I;      /* K capt. or bad castling  */
      if(m>=l)goto C;                          /* abort on fail high       */
      if(h=d-(y!=z))                           /* remaining depth(-recapt.)*/
      {v=p<6?b[x+8]-b[y+8]:0;                  /* center positional pts.   */
       b[G]=b[H]=b[x]=0;b[y]=u&31;             /* do move, strip virgin-bit*/
       if(!(G&M)){b[F]=k+6;v+=30;}             /* castling: put R & score  */
       if(p<3)                                 /* pawns:                   */
       {v-=9*(((x-2)&M||b[x-2]!=u)+            /* structure, undefended    */
              ((x+2)&M||b[x+2]!=u)-1);         /*        squares plus bias */
        if(y+r+1&S){b[y]|=7;i+=C;}             /* promote p to Q, add score*/
       }
       v=-D(24-k,-l-(l>e),m>q?-m:-q,-e-v-i,    /* recursive eval. of reply */
            F,y,h);          /* J,Z: hash keys           */
       v-=v>e;                                 /* delayed-gain penalty     */
       if(z==9)                                /* called as move-legality  */
       {if(v!=-I&x==K&y==L)                    /*   checker: if move found */
        {Q=-e-i;O=F;return l;}                 /*   & not in check, signal */
        v=m;                                   /* (prevent fail-lows on    */
       }                                       /*   K-capt. replies)       */
       b[G]=k+38;b[F]=b[y]=0;b[x]=u;b[H]=t;    /* undo move,G can be dummy */
       if(Y&8){m=v;Y&=~8;goto A;}              /* best=1st done,redo normal*/
       if(v>m){m=v;X=x;Y=y|S&G;}               /* update max, mark with S  */
      }                                        /*          if non castling */
      t+=p<5;                                  /* fake capt. for nonsliding*/
      if(p<3&6*k+(y&V)==S                      /* pawn on 3rd/6th, or      */
          ||(u&~24)==36&j==7&&                 /* virgin K moving sideways,*/
          G&M&&b[G=(x|7)-(r>>1&7)]&32          /* 1st, virgin R in corner G*/
          &&!(b[G^1]|b[G^2])                   /* 2 empty sqrs. next to R  */
      ){F=y;t--;}                              /* unfake capt., enable e.p.*/
     }W(!t);                                   /* if not capt. continue ray*/
  }}}W((x=x+9&~M)-B);                          /* next sqr. of board, wrap */
C:if(m>I/4|m<-I/4)d=99;                        /* mate is indep. of depth  */
  m=m+I?m:-D(24-k,-I,I,0,S,z,1)/2;         /* best loses K: (stale)mate*/
  if(z&&d>2)
   {*c='a'+(X&7);c[1]='8'-(X>>4);c[2]='a'+(Y&7);c[3]='8'-(Y>>4&7);c[4]=0;}                                            /*    encoded in X S,8 bits */
/*if(z==8)printf("%2d ply, %9d searched, %6d by (%2x,%2x)\n",d-1,N,m,X,Y&0x77);*/
 }
 if(z&8){K=X;L=Y&~M;}
 read_board(board);
 return m;                                     
}

int runnin(void) {
  int j,k=8,*p;
  int mate = 0;
  int timer = 0;
  int r1, d1, r2, d2;

  F(i,0,8)
  {b[i]=(b[i+V]=o[i+24]+40)+8;b[i+16]=18;b[i+96]=9;   /* initial board setup*/
  F(j,0,8)b[16*j+i+8]=(i-4)*(i-4)+(j-3.5)*(j-3.5);   /* center-pts table   */
  }                                               /*(in unused half b[])*/
  F(i,M,1035)T[i]=rand()>>9;

  if (mode == 0) {
    updateboard();
    printboard();
  }

  else if (mode == 1) {
    adc(0);
    adc(0);
  }
  
  reed_switch(0, 0, 0);
  //switch_state();
  
  // play game
  W(1) {
    
    if (timer > 50 && timer < 200) {
      read_board(copy);
    }
    /*
    else if (timer >= 200 && timer < 300) {
      read_board(copy);
      //adc();
    }
*/
    // if k == 16 -> compute turn
    // if k == 8 -> player turn
    else if (timer == 200) {
      //Serial.println("Chess Engine working");
      reset_board(board, copy);
      
      // check if player king in in check
      if (check(0, 0, 0, k)) {
        Serial.println("King is in check");
        Serial.print(kr);
        Serial.println(kc);
        if (is_LED(kr, kc+2*(kc+1))) {
          board[kr][kc+2*(kc+1)] = B0;       // turn off blue led
          board[kr][kc+2*(kc+1)-1] = B0;      // turn off green LED
          board[kr][kc+2*(kc+1)-2] = B1;      // turn on red LED
        }
        else if (!is_LED(kr, kc+2*(kc+1))) {
          board[kr][kc+2*(kc+1)-2] = B1;      // turn on red LED
        }
      }
      
      
      // get input
      /*
      if (k == 8) {p=c;W((*p++=mygetchar())>10);}
      else if (k == 16 && mode == 1) {p=c;W((*p++=mygetchar())>10);}
      else
        *c = 10;
*/
      if (k == 8) {
        if (mode == 1)
          adc(0);

        // wait for piece to be picked up
        while(!reed_switch(1, 0, k)) {
          read_board(copy);
        }

        // read new state of switches
        reed_switch(0, 0, k);
        reed_switch(0, 0, k);
        read_board(board);
        //switch_state();

        // wait for piece to be put down
        while(!reed_switch(0, 1, k)) {
          read_board(board);
        }

        if (c[2] == c[0] && c[1] == c[3])
          continue;

        reed_switch(0, 0, k);
        reed_switch(0, 0, k);
        read_board(board);
        //switch_state();
      }

      else if (k == 16 && mode == 1) {
        adc(0);
        
        // wait for piece to be picked up
        while(!reed_switch(1, 0, k)) {
          read_board(copy);
        }

        // read new state of switches
        reed_switch(0, 0, k);
        reed_switch(0, 0, k);
        read_board(board);
        //switch_state();

        // wait for piece to be put down
        while(!reed_switch(0, 1, k)) {
          read_board(board);
        }

        reed_switch(0, 0, k);
        reed_switch(0, 0, k);
        read_board(board);
        //switch_state();
      }
   
      else if (k == 16 && mode == 0) {
        *c = 10;
      }

      adc(0);
      adc(0);

      // make move

      if (mode == 0) {
        N=0;
        if(*c-10)
          {K=c[0]-16*c[1]+C;L=c[2]-16*c[3]+C;}
        // computer make move
        else {
          read_board(board);
          mate = D(k,-I,I,Q,O,8,0);
          read_board(board);
        }
      }

      //printmove();

      
      r1 = 8 - ((int)c[1] - 48);
      d1 = (int)c[0] -89 - 8;
      r2 = 8 - ((int)c[3] - 48);
      d2 = (int)c[2] -89 - 8;

      
      // highlight computer move
      // can highlight computer move before legality check because computer
      // never makes illegal move
      if (k == 16 && mode == 0) {
        computer_led(r1, d1, r2, d2);
        
        // wait for piece to be picked up
        while(!reed_switch(1, 0, k)) {
          read_board(board);
        }

        // read new state of switches
        reed_switch(0, 0, k);
        reed_switch(0, 0, k);
        read_board(board);
        switch_state();

        // wait for piece to be put down
        while(!reed_switch(0, 1, k)) {
          read_board(board);
        }

        if (c[2] == c[0] && c[1] == c[3])
          continue;

        reed_switch(0, 0, k);
        reed_switch(0, 0, k);
        read_board(copy);
        switch_state();
        
      }

      // engine returns -I/2 to indicate checkmate
      if (mate == -4000)
        break;

      // check legality; if legal make move
      if (mode == 0)
        if(D(k,-I,I,Q,O,9,2)==I)k^=24; 
      else if (mode == 1)
        k^=24;           

      if (mode == 0) {
        updateboard();
        printboard();
      }
      
      timer = 0;

      reset_board(board, copy);
    }
 
    timer++;
  }
  
  //Serial.println("Game Over");
  return 0;
}