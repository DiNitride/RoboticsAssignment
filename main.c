/* Simple skeleton test program for in-robot API */

#include "allcode_api.h"    // MUST include this to define robot functions

const int MOVE_ENCODER_VALUE = 440;

// Lab Maze
//const int IR_THRESH_FRONT = 480;
//const int IR_THRESH_RIGHT = 230; // Prev 376
//const int IR_THRESH_REAR = 850;
//const int IR_THRESH_LEFT = 500;

// Homemade cardboard replica
const int IR_THRESH_FRONT = 50;
const int IR_THRESH_RIGHT = 50; // Prev 376
const int IR_THRESH_REAR = 50;
const int IR_THRESH_LEFT = 50;

unsigned short rightAverage;

bool wallFront;
bool wallRight;
bool wallRear;
bool wallLeft;

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



CompassDirections direction = NORTH;
short currentCell = 14;

int main()
{
		FA_RobotInit();
		FA_LCDBacklight(70);
		FA_SetDriveSpeed(30);

		while(1)
		{
				FA_LCDClear();
				// == Logic ==
				// Scan the room
				scan();
				calculateAndMove();
				// Map the room internally
				// Check if it is a nest
				//   Mark cell as nest
				//   Increase nest counter
				// Move to a new unexplored cell
				
				FA_DelayMillis(1500);
		}

		return 0; // Actually, we should never get here...
}

// Scans using the IR sensors and maps 
void scan() {
		// Start at front, work clockwise
		unsigned short front = FA_ReadIR(IR_FRONT);
		unsigned short right = FA_ReadIR(IR_RIGHT);
		unsigned short rear = FA_ReadIR(IR_REAR);
		unsigned short left = FA_ReadIR(IR_LEFT);
			
		FA_LCDPrint("F:", 2, 0, 0, FONT_NORMAL, LCD_OPAQUE);
		FA_LCDPrint("R:", 2, 50, 0, FONT_NORMAL, LCD_OPAQUE);
		FA_LCDPrint("B:", 2, 0, 10, FONT_NORMAL, LCD_OPAQUE);
		FA_LCDPrint("L:", 2, 50, 10, FONT_NORMAL, LCD_OPAQUE);
		
		FA_LCDUnsigned((unsigned long) front, 15, 0, FONT_NORMAL, LCD_OPAQUE);
		FA_LCDUnsigned((unsigned long) right, 65, 0, FONT_NORMAL, LCD_OPAQUE);
		FA_LCDUnsigned((unsigned long) rear, 15, 10, FONT_NORMAL, LCD_OPAQUE);
		FA_LCDUnsigned((unsigned long) left, 65, 10, FONT_NORMAL, LCD_OPAQUE);
		
		wallFront = front > IR_THRESH_FRONT;
		wallRight = right > IR_THRESH_RIGHT;
		wallRear = rear > IR_THRESH_REAR;
		wallLeft = left > IR_THRESH_LEFT;
	
}



CompassDirections relativeToCompass(RelativeDirections side) {
		return (direction + side) % 4;
}

// Map the current location into memory
void map() {
		// Need to convert relative sides to compass points
		// I.e. NORTH could be the rear sensor,
		// need to work out how the robot is rotated
		
}

void move(RelativeDirections move_direction) {
		if (move_direction < 3) {
				FA_Right(92 * move_direction);
		} else {
				FA_Left(92);
		}
		
		direction = relativeToCompass(move_direction);
		switch (direction) {
				case NORTH: 
						currentCell -= 5;
						break;
				case EAST:
						currentCell += 1;
						break;
				case SOUTH:
						currentCell -= -5;
						break;
				case WEST:
						currentCell -= 1;
						break;
		}
		FA_ResetEncoders();
		unsigned short average = 0;
		short speed = 30;
		short leftMod = 0;
		short rightMod = 0;
		while (average < MOVE_ENCODER_VALUE) {
				unsigned short leftEnc = FA_ReadEncoder(0);
				unsigned short rightEnc = FA_ReadEncoder(1);
				average = ((leftEnc + rightEnc) / 2);
				short diff = leftEnc - rightEnc;
				if (diff > 0) {
						rightMod = 50;
						leftMod = 0;
				} else if (diff < 0) {
						leftMod = 50;
						rightMod = 0;
				} else {
						rightMod = 0;
						leftMod = 0;
				}
				FA_SetMotors(speed + leftMod, speed + rightMod);
				FA_DelayMillis(10);
		}
		
		FA_SetMotors(0, 0);
}

// Calculates what direction to move in
void calculateAndMove() {
		if (wallFront && wallRight && wallLeft) {
				// We have entered a dead end, possible nest!
				// Need to move behind us to exit
				FA_LCDPrint("REAR", 4, 0, 20, FONT_NORMAL, LCD_OPAQUE);
				move(REAR);
				return;
		}
		if (!(wallRight)) {
				FA_LCDPrint("RIGHT", 5, 0, 20, FONT_NORMAL, LCD_OPAQUE);
				move(RIGHT);
				return;
		}
		if (!(wallFront)) {
				FA_LCDPrint("FORWARD", 7, 0, 20, FONT_NORMAL, LCD_OPAQUE);
				move(FORWARD);
				return;
		}
		if (!(wallLeft)) {
				FA_LCDPrint("LEFT", 4, 0, 20, FONT_NORMAL, LCD_OPAQUE);
				move(LEFT);
		}
}