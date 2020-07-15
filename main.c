/* Simple skeleton test program for in-robot API */

#include "allcode_api.h"    // MUST include this to define robot functions


// Structs!!!

struct Cell {
  bool north, east, south, west, nest, explored;
};

// ================
// ENUM DEFENITIONS
// ===============

typedef enum {
  NORTH = 0,
  EAST = 1,
  SOUTH = 2,
  WEST = 3
} CompassDirections;

typedef enum {
  FORWARD = 0,
  RIGHT = 1,
  REAR = 2,
  LEFT = 3,
} RelativeDirections;

// ================
// THRESHOLD VALUES
// ================
const int MOVE_ENCODER_VALUE = 440;

// Wall Detecting Thresholds
const int IR_THRESH_FRONT = 150;
const int IR_THRESH_RIGHT = 150;
const int IR_THRESH_REAR = 150;
const int IR_THRESH_LEFT = 150;

const int LIGHT_THRESH = 25;

const int LINE_THRESH = 25;

const unsigned long HALF_CELL_TIME = 300;

// ==============
// RUNTIME VALUES
// ==============

bool wallFront;
bool wallRight;
bool wallRear;
bool wallLeft;

bool inNest = false;

// IR Sensor values
unsigned short front;
unsigned short front_right;
unsigned short right;
unsigned short rear_right;
unsigned short rear;
unsigned short rear_left;
unsigned short left;
unsigned short front_left;

// Line IR Sensor values
unsigned short leftLine;
unsigned short rightLine;
unsigned short lineAverage;

CompassDirections direction = NORTH;
short currentCell = 14; // The current cell relative to the mapping array
short previousCell = 0; // The previous cell relative to the mapping array
struct Cell cellMap[25];

// Converts a relative direction to a compass direction, based on where
// the robot is currently facing
// i.e. if it is facing north, and the relative direction is right, it 
// is now facing east
CompassDirections relativeToCompass (RelativeDirections side) {
  return (direction + side) % 4;
}

// Scans and updates all the IR sensor values
void scanAllIR() {
  // Get values from all IR sensors
  front       = FA_ReadIR (IR_FRONT);
  front_right = FA_ReadIR (IR_FRONT_RIGHT); // 379 
  right       = FA_ReadIR (IR_RIGHT); // 50
  rear_right  = FA_ReadIR (IR_REAR_RIGHT);
  rear        = FA_ReadIR (IR_REAR);
  rear_left   = FA_ReadIR (IR_REAR_LEFT);
  left        = FA_ReadIR (IR_LEFT);
  front_left  = FA_ReadIR (IR_FRONT_LEFT);
}

void detectWalls () {
  // Detects walls by comparing IR values to thresholds
  wallFront = front > IR_THRESH_FRONT;
  wallRight = right > IR_THRESH_RIGHT;
  wallRear = rear > IR_THRESH_REAR;
  wallLeft = left > IR_THRESH_LEFT;
}

// Map the current location into memory
void map () {
  bool walls[4] = {false, false, false, false};
  // Need to convert relative sides to compass points
  // I.e. NORTH could be the rear sensor,
  // need to work out how the robot is rotated
  if (!cellMap[currentCell].explored) {
    scanAllIR(); // Scan all around us
    detectWalls(); // Detect the walls
    previousCell = currentCell;
    if (inNest) {
      // Party bois
      FA_PlayNote(4000, 60);
      FA_PlayNote(6000, 30);
      FA_PlayNote(4000, 60);
      FA_PlayNote(2000, 30);
      cellMap[currentCell].nest = true;
    }
    
     
    walls[0] = wallFront;
    walls[1] = wallRight;
    walls[2] = wallRear;
    walls[3] = wallLeft;
    bool temp;
    
    // Calculate rotations
    int rotations = (0 % direction) % 4;
    int x = 0;
    while (x < rotations) {
      // Rotate enough times so the relative positions match
      // to the order N E S W in the array
      // i.e. if we are facing EAST, wallFront = the wall East but is inserted
      // in array pos 0. Needs shifting once right;
      temp = walls[3];
      walls[3] = walls[2];
      walls[2] = walls[1];
      walls[1] = walls[0];
      walls[0] = temp;
      x++;
    }
    
    cellMap[currentCell].north = walls[0];
    cellMap[currentCell].east = walls[1];
    cellMap[currentCell].south = walls[2];
    cellMap[currentCell].west = walls[3];
    
    cellMap[currentCell].explored = true;
    // Mapping done!!!!
  }
}


// Calculates what direction to move in
RelativeDirections calculateNextDirection () {
  RelativeDirections dir;
  if (wallFront && wallRight && wallLeft) {
    // We have entered a dead end
    // Need to move behind us to exit
    //FA_LCDPrint ("REAR", 4, 90, 20, FONT_NORMAL, LCD_OPAQUE);
    dir = REAR;
  } else if (!(wallRight)) {
    //FA_LCDPrint ("RIGHT", 5, 90, 20, FONT_NORMAL, LCD_OPAQUE);
    dir = RIGHT;
  } else if (!(wallFront)) {
    //FA_LCDPrint ("FORWARD", 7, 90, 20, FONT_NORMAL, LCD_OPAQUE);
    dir = FORWARD;
  } else if (!(wallLeft)) {
    //FA_LCDPrint ("LEFT", 4, 90, 20, FONT_NORMAL, LCD_OPAQUE);
    dir = LEFT;
  } else {
    // SCREAM
    // THERE ARE WALLS EVERYWHERE
    // This shouldn't happen
    dir = REAR;
    // Turn round and hope life resolves itself :)
  }
  return dir;
}

// Checks the light level for a nest!
void checkForNest () {
  if (wallRight && wallLeft && wallFront) {
    // Check light level;
    unsigned short lightLevel = FA_ReadLight();
    if (lightLevel < LIGHT_THRESH) {
      // We're in a nest!!!!
      // Party time boys
      if (!inNest) {
        inNest = true;
      }
    }
  }
}

// Turns to a given direction
void turn(RelativeDirections direction) {
  direction = relativeToCompass(direction);
  if (direction == 1) {
    FA_Right(90); 
  } else if (direction == 2) {
    if (direction )
    FA_Right(180);
  } else if (direction == 3) {
    FA_Left(90);
  }
}

// Reads in the line sensor values and calculates the average
void readLineIR() {
  leftLine = FA_ReadLine(0);
  rightLine = FA_ReadLine(1); 
  lineAverage = (leftLine + rightLine) / 2;
}

// Starts the robots movement
void move() {
  
  short speed = 10;
  short leftMod = 0;
  short rightMod = 0;

  // Maintains roughly equal distance from each wall
  if ((right > 350) || (left < 280 && left > 50)) {
    // Needs to turn left
    leftMod = 0;
    rightMod = 10;
  } else if ((right < 280 && right > 50) || left > 350) {
    // Needs to turn right
    rightMod = 0;
    leftMod = 10;
  } 
   
  // If the front left or front right sensor gets too close
  // Reverse a little and rotate to fix angle
  if (front_right > 350) {
    FA_SetMotors(0,0);
    FA_DelayMillis(100);
    FA_Backwards(15);
    FA_Left(5);
  } else if (front_left > 350) {
    FA_SetMotors(0,0);
    FA_DelayMillis(100);
    FA_Backwards(15);
    FA_Right(5);
  } else {
    FA_SetMotors(speed + leftMod, speed + rightMod);
  }
}


// Stops the robots movement
void stop() {
  FA_SetMotors (0, 0);
  FA_DelayMillis (1000);
}

// Debug info
void display() {
  FA_LCDClear();
  
  FA_LCDPrint("F", 1, 0, 20, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDPrint("R", 1, 15, 20, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDPrint("R", 1, 30, 20, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDPrint("L", 1, 45, 20, FONT_NORMAL, LCD_OPAQUE);
  
  if (wallFront) { FA_LCDUnsigned((unsigned long) 1, 5, 20, FONT_NORMAL, LCD_OPAQUE); }
  else { FA_LCDUnsigned((unsigned long) 0, 5, 20, FONT_NORMAL, LCD_OPAQUE); }
  
  if (wallRight) { FA_LCDUnsigned((unsigned long) 1, 20, 20, FONT_NORMAL, LCD_OPAQUE); }
  else { FA_LCDUnsigned((unsigned long) 0, 20, 20, FONT_NORMAL, LCD_OPAQUE); }
  
  if (wallRear) { FA_LCDUnsigned((unsigned long) 1, 35, 20, FONT_NORMAL, LCD_OPAQUE); }
  else { FA_LCDUnsigned((unsigned long) 0, 35, 20, FONT_NORMAL, LCD_OPAQUE); }
  
  if (wallLeft) { FA_LCDUnsigned((unsigned long) 1, 50, 20, FONT_NORMAL, LCD_OPAQUE); }
  else { FA_LCDUnsigned((unsigned long) 0, 50, 20, FONT_NORMAL, LCD_OPAQUE); }

  FA_LCDUnsigned ((unsigned long) front,      0,    0,  FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) front_right,30,   0,  FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) right,      60,   0,  FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) rear_right, 90,   0,  FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) rear,       0,    10, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) rear_left,  30,   10, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) left,       60,   10, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) front_left, 90,   10, FONT_NORMAL, LCD_OPAQUE);
  
  FA_LCDUnsigned ((unsigned long) leftLine,   70,    20, FONT_NORMAL, LCD_OPAQUE);
  FA_LCDUnsigned ((unsigned long) rightLine,  100,   20, FONT_NORMAL, LCD_OPAQUE);
  
}

// Updates the currentl cell index when we move into a new cell, based on the
// current direction we're facing
void updateCurrentCell() {
  if (direction == 0) {
    currentCell += 5;
  } else if (direction == 1) {
    currentCell += 1;
  } else if (direction == 2) {
    currentCell -= 5;
  } else if (direction == 3) {
    currentCell -= 1;
  }
}

// Checks if all cells in the cell map have been explored
bool allExplored() {
  int i = 0;
  int cap = 25;
  while (i < cap) {
    if (cellMap[i].explored == false) {
      return false;
    }
    i++;
  }
  return true;
}

// Main logic loop!
int main () {
  FA_RobotInit ();
  FA_LCDBacklight (70);
  FA_DelayMillis(500);
  
  RelativeDirections nextMove;
  
  bool dirChange = false;
  bool onLine = false;
  bool exiting = true;
  
  while (1) {
    
    scanAllIR(); // Scan all around us
    detectWalls(); // Detect the walls
    readLineIR();
    // display();
    checkForNest(); // Check if it is a nest
    
    if (wallFront) {
      // Adjusting in case we are facing too far left or right to the front wall
      if (front_right > front) {
        FA_Backwards(30);
        FA_Left(10);
        FA_Forwards(30);
      } else if (front_left > front) {
        FA_Backwards(30);
        FA_Right(10);
        FA_Forwards(30);
      }
      
      map();
      dirChange = true;
      exiting = true;
    }
    
    if ((lineAverage < LINE_THRESH) && !onLine) {
      // JUST WENT ONTO A LINE
      onLine = true;
      if (!exiting) {
        stop();
        FA_Backwards(50);
        map();
        if (!wallRight) {
          dirChange = true;
        }
        exiting = true;
      } else {
        exiting = false; // We have exited
      }
    } else if ((lineAverage > LINE_THRESH) && onLine) {
      // JUST WENT OFF A LINE
      onLine = false;
      if (!exiting) {
        // Just entered new cell
        inNest = false;
        updateCurrentCell();
      }
    }
    
    // A change in direction was detected earlier, do that now!
    if (dirChange) {
      stop();
      nextMove = calculateNextDirection (); // Calculate next move
      turn(nextMove);
      dirChange = false;
      FA_DelayMillis(50);
    }
    
    if (allExplored()) {
      return 0;
    }
    
    move(); // Continue forward
    FA_DelayMillis (10);
  }

  return 0; // Actually, we should never get here...
}