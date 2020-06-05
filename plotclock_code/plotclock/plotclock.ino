#include <EEPROM.h>
#include <Servo.h>
#include "RTClib.h"
#include "utils.h"

// note:
// 1) measurement of widths, distances, offsets, area boundaries etc.
//    set in coordinate plane units (mm in this case);
// 2) angles set in degrees.

// drawing area boundaries
// (where the time is drawing in)
#define DA_MIN_X 19
#define DA_MIN_Y 61.2
#define DA_MAX_X 71.2
#define DA_MAX_Y 77.2

// eraser position
#define ERS_X 87
#define ERS_Y 66

// punctuation mark width relative to char width
#define PM_REL_WIDTH 0.65

// offsets
#define LK 0 // any value
#define OL 0 // >= 0

// design settings
#define AB 25.6
#define AF 35
#define BG 35
#define FC 45.1
#define CG 45.1
#define CD 13.2

// from 0 to 180 deg
#define DCF 135

// bounding angles
#define MAX_FCG 150
#define MAX_CFA 170
#define MAX_CGB 170

// servos min and max control values
#define SERVO_MIN 500
#define SERVO_MAX 2400

// servos calibrated positions
#define SERVO_LEFT_ANGLE_180 2220
#define SERVO_LEFT_ANGLE_90 1250
#define SERVO_RIGHT_ANGLE_0 750

// bottom servo values for predefined positions
#define SERVO_BOTTOM_PV_WR_BOTTOM 1479
#define SERVO_BOTTOM_PV_WR_UP 1380
#define SERVO_BOTTOM_PV_ERS_BOTTOM 1395
#define SERVO_BOTTOM_PV_FULLUP 1120

// servos move settings

// controlled (smooth) move settings

// control values step for left and right servos while iterating from A to B position,
// in servo control values (microseconds)
#define SERVO_LR_SPEED_CON_STEP 1
// delay after each iteration,
// in microseconds
#define SERVO_LR_SPEED_CON_DELAY 600

// uncontrolled move settings

// delay after writing control values to servos to give them time to move shafts,
// in milliseconds
#define SERVO_LR_SPEED_UNCON_DELAY 300

#define SERVO_BOTTOM_SPEED_CON_STEP 1
#define SERVO_BOTTOM_SPEED_CON_DELAY 1620
#define SERVO_BOTTOM_SPEED_UNCON_DELAY 500

// axis values iteration steps,
// in coordinate plane units
#define CALC_DRAWING_STEP 0.336
#define CALC_CLEARING_STEP 1.8

// rtc & date saving settings
#define EEPROM_ADDR_PREV_COMP_DT 0
#define NO_ALARM_INT 0

// pins
#define PIN_ALARM_INT 2
#define PIN_SERVO_LEFT 9
#define PIN_SERVO_RIGHT 10
#define PIN_SERVO_BOTTOM 11

typedef enum lift_stage {
  LIFT_WR_BOTTOM,
  LIFT_WR_UP,
  LIFT_ERS_BOTTOM,
  LIFT_FULLUP
};

typedef struct servo_angles {
  float left;
  float right;
};

// char width,
// 4 means there is 4 chars
const float CHAR_WIDTH = ((float)DA_MAX_X - DA_MIN_X) / (4 + PM_REL_WIDTH);
// punctuation mark width
const float PM_WIDTH = CHAR_WIDTH * PM_REL_WIDTH;

// calculated constants

const float DA_HEIGHT = (float)DA_MAX_Y - DA_MIN_Y;
const float DA_MIDDLE_Y = DA_MIN_Y + DA_HEIGHT / 2;

const float servoMicrosecondsPerAngle =
  (float)(SERVO_LEFT_ANGLE_180 - SERVO_LEFT_ANGLE_90) / 90;

const float FD = getSideBy2SidesAndAngle(CD, FC, DCF);
const float LA = (float)LK + AF;
const float DFC = getAngleBy3Sides(FD, FC, CD);

// program variables

float charAreaStartX = (float)DA_MIN_X;

Servo servoLeft;
Servo servoRight;
Servo servoBottom;

short servoLeftLastMicroseconds;
short servoRightLastMicroseconds;
short servoBottomLastMicroseconds;

RTC_DS3231 rtc;
bool isAlarm = false;
bool isInitialDrawing = true;

// calculate angles of left and right servos for specified marker position
servo_angles* getServoAngles(const float x, const float y) {
  const float DN = y - OL;
  
  if (DN < 0) {
    return nullptr;
  }

  const float NA = abs(LA - x);
  const float AD = sqrt(sq(DN) + sq(NA));
  const float NAD = degrees(atan(DN / NA));

  if (!doesTriangleExist(FD, AF, AD)) {
    return nullptr;
  }
  
  const float DFA = getAngleBy3Sides(FD, AF, AD);
  const float CFA = DFA - DFC;

  if (!doesTriangleAngleExist(CFA) || CFA > MAX_CFA) {
    return nullptr;
  }

  const float CA = getSideBy2SidesAndAngle(FC, AF, CFA);
  const float FAC = getAngleBy3Sides(AF, CA, FC);
  const float FCA = 180 - CFA - FAC;

  const float DAC = getAngleBy3Sides(AD, CA, CD);
  const float FAD = (DCF + FCA <= 180)
    ? FAC - DAC
    : FAC + DAC;

  const float FAB = (x < LA)
    ? FAD + (180 - NAD)
    : FAD + NAD;

  if (FAB > 180) {
    return nullptr;
  }

  const float CAB = FAB - FAC;
  const float CB = getSideBy2SidesAndAngle(CA, AB, CAB);
  
  const float ABC = getAngleBy3Sides(AB, CB, CA);

  if (!doesTriangleExist(CB, BG, CG)) {
    return nullptr;
  }

  const float CBG = getAngleBy3Sides(CB, BG, CG);
  const float CGB = getAngleBy3Sides(CG, BG, CB);
  if (CGB > MAX_CGB) {
    return nullptr;
  }

  const float ABG = ABC + CBG;
  if (ABG > 180) {
    return nullptr;
  }

  const float ACB = 180 - CAB - ABC;
  const float BCG = 180 - CBG - CGB;
  const float FCG = FCA + ACB + BCG;

  if (FCG > MAX_FCG) {
    return nullptr;
  }

  // servo left angle = FAB
  // servo right angle = 180 - ABG

  return new servo_angles {
    FAB,
    180 - ABG
  };
}

void moveTo(const float x, const float y, const bool controlSpeed = true, const bool waitMoving = true) {
  const servo_angles* angles = getServoAngles(x, y);
  if (angles == nullptr) {
    return;
  }

  // calculate and move from predefined
  // calibrated positions
  const float servoLeftAngleFrom180 = 180 - angles->left;
  const float servoRightAngleFrom0 = angles->right;

  // + 0.5 - round instead of floor
  const short servoLeftMicrosecondsFrom180 =
    (int)(servoLeftAngleFrom180 * servoMicrosecondsPerAngle + 0.5);
    
  const short servoRightMicrosecondsFrom0 =
    (int)(servoRightAngleFrom0 * servoMicrosecondsPerAngle + 0.5);

  const short servoLeftMicroseconds = SERVO_LEFT_ANGLE_180 - servoLeftMicrosecondsFrom180;
  const short servoRightMicroseconds = SERVO_RIGHT_ANGLE_0 + servoRightMicrosecondsFrom0;

  if (controlSpeed) {
    const int8_t servoLeftDirCoef = servoLeftMicroseconds > servoLeftLastMicroseconds ? 1 : -1;
    const int8_t servoRightDirCoef = servoRightMicroseconds > servoRightLastMicroseconds ? 1 : -1;
    
    const short servoLeftDiff = abs(servoLeftMicroseconds - servoLeftLastMicroseconds);
    const short servoRightDiff = abs(servoRightMicroseconds - servoRightLastMicroseconds);
    const short biggerDiff = servoLeftDiff > servoRightDiff ? servoLeftDiff : servoRightDiff;

    const float servoLeftStepCoef = (float)servoLeftDiff / biggerDiff;
    const float servoRightStepCoef = (float)servoRightDiff / biggerDiff;

    float i = 0;
    while (i < biggerDiff) {
      servoLeft.writeMicroseconds(servoLeftLastMicroseconds + (short)(i * servoLeftStepCoef * servoLeftDirCoef));
      servoRight.writeMicroseconds(servoRightLastMicroseconds + (short)(i * servoRightStepCoef * servoRightDirCoef));
      delayMicroseconds(SERVO_LR_SPEED_CON_DELAY);
      
      i += SERVO_LR_SPEED_CON_STEP;
    }
  
    servoLeft.writeMicroseconds(servoLeftMicroseconds);
    servoRight.writeMicroseconds(servoRightMicroseconds);
    delayMicroseconds(SERVO_LR_SPEED_CON_DELAY);
  } else {
    servoLeft.writeMicroseconds(servoLeftMicroseconds);
    servoRight.writeMicroseconds(servoRightMicroseconds);

    if (waitMoving) {
      delay(SERVO_LR_SPEED_UNCON_DELAY);
    }
  }

  delete angles;

  servoLeftLastMicroseconds = servoLeftMicroseconds;
  servoRightLastMicroseconds = servoRightMicroseconds;
}

void lift(const lift_stage stage, const bool controlSpeed = true, const bool waitLifting = true) {
  short microseconds;
  
  switch(stage) {
    case LIFT_WR_BOTTOM:  microseconds = SERVO_BOTTOM_PV_WR_BOTTOM;  break;
    case LIFT_WR_UP:      microseconds = SERVO_BOTTOM_PV_WR_UP;      break;
    case LIFT_ERS_BOTTOM: microseconds = SERVO_BOTTOM_PV_ERS_BOTTOM; break;
    case LIFT_FULLUP:     microseconds = SERVO_BOTTOM_PV_FULLUP;     break;

    default:
      return;
  }

  if (controlSpeed) {
    const int8_t dirCoef = microseconds > servoBottomLastMicroseconds ? 1 : -1;
    const short diff = abs(microseconds - servoBottomLastMicroseconds);
  
    float i = 0;
    while (i < diff) {
      servoBottom.writeMicroseconds(servoBottomLastMicroseconds + (short)i * dirCoef);
      delayMicroseconds(SERVO_BOTTOM_SPEED_CON_DELAY);
      
      i += SERVO_BOTTOM_SPEED_CON_STEP;
    }
  
    servoBottom.writeMicroseconds(microseconds);
    delayMicroseconds(SERVO_BOTTOM_SPEED_CON_DELAY);
  } else {
    servoBottom.writeMicroseconds(microseconds);
    
    if (waitLifting) {
      delay(SERVO_BOTTOM_SPEED_UNCON_DELAY);
    }
  }
  
  servoBottomLastMicroseconds = microseconds;
}

void draw_0() {
  const float a = CHAR_WIDTH * 5 / 11;
  const float b = DA_HEIGHT / 2;

  float elemStartX = charAreaStartX + CHAR_WIDTH / 2 - a;
  float elemStartY = DA_MIDDLE_Y;

  moveTo(elemStartX, elemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  // x & y - values from ellipse origin
  // bottom half-ellipse
  for (x = -a + CALC_DRAWING_STEP; x <= a; x += CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY - y);
  }

  // top half-ellipse
  // continue from last position
  for (x -= CALC_DRAWING_STEP; x >= -a; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY + y);
  }

  lift(LIFT_WR_UP);
}

void draw_1() {  
  float currentElemStartX = charAreaStartX + CHAR_WIDTH / 3;
  float currentElemStartY = DA_MIN_Y + DA_HEIGHT / 2;
  float currentElemEndX = charAreaStartX + (CHAR_WIDTH - CHAR_WIDTH / 5);
  float currentElemEndY = DA_MAX_Y;
  
  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  for (x = currentElemStartX + CALC_DRAWING_STEP; x <= currentElemEndX; x += CALC_DRAWING_STEP) {
    y = getLineY(x, currentElemStartX, currentElemStartY, currentElemEndX, currentElemEndY);
    moveTo(x, y);
  }

  currentElemStartX = currentElemEndX;
  currentElemStartY = currentElemEndY;
  currentElemEndY = DA_MIN_Y;

  for (y = currentElemStartY; y >= currentElemEndY; y -= CALC_DRAWING_STEP) {
    moveTo(currentElemStartX, y);
  }

  lift(LIFT_WR_UP);
}

void draw_2() {
  const float r = CHAR_WIDTH / 2;

  float currentElemStartX = charAreaStartX;
  float currentElemStartY = DA_MAX_Y - r;
  float currentElemEndX;
  float currentElemEndY;

  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  // x & y - values from circle origin
  // top half-circle
  for (x = -r + CALC_DRAWING_STEP; x <= r; x += CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX + (r + x), currentElemStartY + y);
  }

  currentElemStartX += 2 * r;
  // currentElemStartY stays the same
  currentElemEndX = charAreaStartX;
  currentElemEndY = DA_MIN_Y;

  for (x = currentElemStartX; x >= currentElemEndX; x -= CALC_DRAWING_STEP) {
    y = getLineY(x, currentElemStartX, currentElemStartY, currentElemEndX, currentElemEndY);
    moveTo(x, y);
  }

  currentElemStartX = currentElemEndX;
  // currentElemStartY = currentElemEndY; but it's
  // not important anymore
  currentElemEndX = charAreaStartX + CHAR_WIDTH;
  // currentElemEndY stays the same

  for (x = currentElemStartX; x <= currentElemEndX; x += CALC_DRAWING_STEP) {
    moveTo(x, currentElemEndY);
  }

  lift(LIFT_WR_UP);
}

void draw_3() {
  const float a = CHAR_WIDTH * 5 / 11;
  const float b = DA_HEIGHT / 4;

  float elemStartX = charAreaStartX + CHAR_WIDTH / 2 - a;
  float elemStartY = DA_MAX_Y - b;

  moveTo(elemStartX, elemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  // x & y - values from ellipse origin
  // top half-ellipse
  for (x = -a + CALC_DRAWING_STEP; x <= a; x += CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY + y);
  }

  // right bottom quarter-ellipse
  for (x -= CALC_DRAWING_STEP; x >= 0; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY - y);
  }

  elemStartY -= 2 * b;

  // right top quarter-ellipse
  for (x += CALC_DRAWING_STEP; x <= a; x += CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY + y);
  }

  // bottom half-ellipse
  for (x -= CALC_DRAWING_STEP; x >= -a; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY - y);
  }

  lift(LIFT_WR_UP);
}

void draw_4() {
  float currentElemStartX = charAreaStartX + CHAR_WIDTH * 4 / 5;
  float currentElemStartY = DA_MIN_Y;
  float currentElemEndY = DA_MAX_Y;
  float currentElemEndX;

  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  for (y = currentElemStartY + CALC_DRAWING_STEP; y <= currentElemEndY; y += CALC_DRAWING_STEP) {
    moveTo(currentElemStartX, y);
  }

  // currentElemStartX stays the same
  currentElemStartY = currentElemEndY;
  currentElemEndX = charAreaStartX;
  currentElemEndY = DA_MIN_Y + CHAR_WIDTH * 2 / 5;

  for (x = currentElemStartX; x >= currentElemEndX; x -= CALC_DRAWING_STEP) {
    y = getLineY(x, currentElemStartX, currentElemStartY, currentElemEndX, currentElemEndY);
    moveTo(x, y);
  }

  currentElemStartX = currentElemEndX;
  currentElemEndX = charAreaStartX + CHAR_WIDTH;
  currentElemStartY = currentElemEndY;

  for (x = currentElemStartX; x <= currentElemEndX; x += CALC_DRAWING_STEP) {
    moveTo(x, currentElemStartY);
  }

  lift(LIFT_WR_UP);
}

void draw_5() {
  const float r = CHAR_WIDTH * 2 / 5;
  
  float currentElemStartX = charAreaStartX + CHAR_WIDTH;
  float currentElemStartY = DA_MAX_Y;
  float currentElemEndX = charAreaStartX + CHAR_WIDTH / 3;
  float currentElemEndY;

  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  for (x = currentElemStartX - CALC_DRAWING_STEP; x >= currentElemEndX; x -= CALC_DRAWING_STEP) {
    moveTo(x, currentElemStartY);
  }

  currentElemStartX = currentElemEndX;
  currentElemEndY = DA_MIN_Y + 2 * r;

  for (y = currentElemStartY; y >= currentElemEndY; y -= CALC_DRAWING_STEP) {
    moveTo(currentElemStartX, y);
  }

  currentElemEndX = charAreaStartX + CHAR_WIDTH - r;
  currentElemStartY = currentElemEndY;

  for (x = currentElemStartX; x <= currentElemEndX; x += CALC_DRAWING_STEP) {
    moveTo(x, currentElemStartY);
  }

  currentElemStartX = currentElemEndX;
  currentElemStartY = DA_MIN_Y + r;
  // currentElemEndX & currentElemEndY are not important anymore

  // x & y - values from circle origin
  // right top quarter-circle
  for (x = 0; x <= r; x += CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX + x, currentElemStartY + y);
  }

  // bottom half-circle
  for (x -= CALC_DRAWING_STEP; x >= -r; x -= CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX + x, currentElemStartY - y);
  }

  lift(LIFT_WR_UP);
}

void draw_6() {
  const float r = CHAR_WIDTH * 2 / 5;
  
  float currentElemStartX = charAreaStartX + CHAR_WIDTH / 2;
  float currentElemStartY = DA_MAX_Y;
  float currentElemEndX = currentElemStartX - r;
  float currentElemEndY = DA_MIN_Y + r;

  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  for (x = currentElemStartX; x >= currentElemEndX; x -= CALC_DRAWING_STEP) {
    y = getLineY(x, currentElemStartX, currentElemStartY, currentElemEndX, currentElemEndY);
    moveTo(x, y);
  }

  currentElemStartX = currentElemEndX;
  currentElemStartY = currentElemEndY;
  // currentElemEndX & currentElemEndY are not important anymore

  // x & y - values from circle origin
  // bottom half-circle
  for (x = -r; x <= r; x += CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX + (r + x), currentElemStartY - y);
  }

  // top half-circle
  for (x -= CALC_DRAWING_STEP; x >= -r; x -= CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX + (r + x), currentElemStartY + y);
  }

  lift(LIFT_WR_UP);
}

void draw_7() {
  float currentElemStartX = charAreaStartX;
  float currentElemStartY = DA_MAX_Y;
  float currentElemEndX = charAreaStartX + CHAR_WIDTH;
  float currentElemEndY;

  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  for (x = currentElemStartX + CALC_DRAWING_STEP; x <= currentElemEndX; x += CALC_DRAWING_STEP) {
    moveTo(x, currentElemStartY);
  }

  currentElemStartX = currentElemEndX;
  // currentElemStartY stays the same
  currentElemEndX = charAreaStartX + CHAR_WIDTH / 2;
  currentElemEndY = DA_MIN_Y;

  for (x = currentElemStartX; x >= currentElemEndX; x -= CALC_DRAWING_STEP) {
    y = getLineY(x, currentElemStartX, currentElemStartY, currentElemEndX, currentElemEndY);
    moveTo(x, y);
  }

  lift(LIFT_WR_UP);
}

void draw_8() {
  const float a = CHAR_WIDTH * 5 / 11;
  const float b = DA_HEIGHT / 4;

  float elemStartX = charAreaStartX + CHAR_WIDTH / 2 - a;
  float elemStartY = DA_MAX_Y - b;

  moveTo(elemStartX, elemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  // x & y - values from ellipse origin
  // top half-ellipse
  for (x = -a + CALC_DRAWING_STEP; x <= a; x += CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY + y);
  }

  // right bottom quarter-ellipse
  for (x -= CALC_DRAWING_STEP; x >= 0; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY - y);
  }

  elemStartY -= 2 * b;

  // left top quater-ellipse
  for (; x >= -a; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY + y);
  }

  // bottom half-ellipse
  for (x += CALC_DRAWING_STEP; x <= a; x += CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY - y);
  }

  // right top quater-ellipse
  for (x -= CALC_DRAWING_STEP; x >= 0; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY + y);
  }

  elemStartY += 2 * b;

  // left bottom quater-ellipse
  for (; x >= -a; x -= CALC_DRAWING_STEP) {
    y = getHalfEllipseAbsY(x, a, b);
    moveTo(elemStartX + (a + x), elemStartY - y);
  }

  lift(LIFT_WR_UP);
}

void draw_9() {
  const float r = CHAR_WIDTH * 2 / 5;
  
  float currentElemStartX = charAreaStartX + CHAR_WIDTH / 2;
  float currentElemStartY = DA_MIN_Y;
  float currentElemEndX = currentElemStartX + r;
  float currentElemEndY = DA_MAX_Y - r;

  moveTo(currentElemStartX, currentElemStartY);
  lift(LIFT_WR_BOTTOM);

  float x;
  float y;

  for (x = currentElemStartX; x >= currentElemEndX; x -= CALC_DRAWING_STEP) {
    y = getLineY(x, currentElemStartX, currentElemStartY, currentElemEndX, currentElemEndY);
    moveTo(x, y);
  }

  currentElemStartX = currentElemEndX;
  currentElemStartY = currentElemEndY;
  // currentElemEndX & currentElemEndY are not important anymore

  // x & y - values from circle origin
  // top half-circle
  for (x = r; x >= -r; x -= CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX - (r - x), currentElemStartY + y);
  }

  // bottom half-circle
  for (x += CALC_DRAWING_STEP; x <= r; x += CALC_DRAWING_STEP) {
    y = getHalfCircleAbsY(x, r);
    moveTo(currentElemStartX - (r - x), currentElemStartY - y);
  }

  lift(LIFT_WR_UP);
}

void draw_colon() {
  const float dotsDistance = DA_HEIGHT / 2;
  const float dotsVerticalIndent = (DA_HEIGHT - dotsDistance) / 2;
  
  float currentDotX = charAreaStartX + PM_WIDTH / 2;
  float currentDotY = DA_MAX_Y - dotsVerticalIndent;

  moveTo(currentDotX, currentDotY);
  lift(LIFT_WR_BOTTOM);
  lift(LIFT_WR_UP);

  currentDotY = DA_MIN_Y + dotsVerticalIndent;

  moveTo(currentDotX, currentDotY);
  lift(LIFT_WR_BOTTOM);
  lift(LIFT_WR_UP);
}

void draw(const char* pString) {  
  while (*pString != '\0') {
    const float charWidth = pString[0] == ':'
      ? PM_WIDTH
      : CHAR_WIDTH;

    // here the calculation error should be taken into account.
    // note: when changing the values of physical parameters,
    // the last char can be lost
    if ((charAreaStartX + charWidth) > DA_MAX_X) {
      return;
    }

    switch (pString[0]) {
      case '0': draw_0();     break;
      case '1': draw_1();     break;
      case '2': draw_2();     break;
      case '3': draw_3();     break;
      case '4': draw_4();     break;
      case '5': draw_5();     break;
      case '6': draw_6();     break;
      case '7': draw_7();     break;
      case '8': draw_8();     break;
      case '9': draw_9();     break;
      case ':': draw_colon(); break;
      
      default:
        break;
    }

    charAreaStartX += charWidth;
    pString++;
  }
}

void clearArea() {  
  moveTo(DA_MAX_X, ERS_Y);
  moveTo(DA_MIN_X, DA_MAX_Y);

  float x;
  float y;

  for (byte k = 0; k < 2; k += 1) {
    // (x)  (y)     (x)   (y)
    // left top --> right top
    for (x = DA_MIN_X + CALC_CLEARING_STEP; x <= DA_MAX_X; x += CALC_CLEARING_STEP) {
      moveTo(x, DA_MAX_Y);
    }
  
    // (x)  (y)     (x)   (y)
    // right top --> left top
    for (x = DA_MAX_X - CALC_CLEARING_STEP; x >= DA_MIN_X; x -= CALC_CLEARING_STEP) {
      moveTo(x, DA_MAX_Y);
    }
  }

  // (x)  (y)     (x)   (y)
  // left top --> right top
  for (x = DA_MIN_X + CALC_CLEARING_STEP; x <= DA_MAX_X; x += CALC_CLEARING_STEP) {
    moveTo(x, DA_MAX_Y);
  }

  // (x)   (y)     (x)  (y)
  // right top --> left middle
  for (x = DA_MAX_X - CALC_CLEARING_STEP; x >= DA_MIN_X; x -= CALC_CLEARING_STEP) {
    y = getLineY(x, DA_MAX_X, DA_MAX_Y, DA_MIN_X, DA_MIDDLE_Y);
    moveTo(x, y);
  }

  // (x)  (y)        (x)   (y)
  // left middle --> right middle
  for (x = DA_MIN_X + CALC_CLEARING_STEP; x <= DA_MAX_X; x += CALC_CLEARING_STEP) {
    moveTo(x, DA_MIDDLE_Y);
  }

  // (x)  (y)        (x)   (y)
  // right middle --> left middle
  for (x = DA_MAX_X - CALC_CLEARING_STEP; x >= DA_MIN_X; x -= CALC_CLEARING_STEP) {
    moveTo(x, DA_MIDDLE_Y);
  }

  // (x)  (y)        (x)   (y)
  // left middle --> right middle
  for (x = DA_MIN_X + CALC_CLEARING_STEP; x <= DA_MAX_X; x += CALC_CLEARING_STEP) {
    moveTo(x, DA_MIDDLE_Y);
  }

  // (x)   (y)        (x)  (y)
  // right middle --> left bottom
  for (x = DA_MAX_X - CALC_CLEARING_STEP; x >= DA_MIN_X; x -= CALC_CLEARING_STEP) {
    y = getLineY(x, DA_MAX_X, DA_MIDDLE_Y, DA_MIN_X, DA_MIN_Y);
    moveTo(x, y);
  }

  // (x)  (y)        (x)   (y)
  // left bottom --> right bottom
  for (x = DA_MIN_X + CALC_CLEARING_STEP; x <= DA_MAX_X; x += CALC_CLEARING_STEP) {
    moveTo(x, DA_MIN_Y);
  }

  // (x)   (y)        (x)  (y)
  // right bottom --> left bottom
  for (x = DA_MAX_X - CALC_CLEARING_STEP; x >= DA_MIN_X; x -= CALC_CLEARING_STEP) {
    moveTo(x, DA_MIN_Y);
  }

  // (x)  (y)        (x)   (y)
  // left bottom --> right ERS_Y
  for (x = DA_MIN_X + CALC_CLEARING_STEP; x <= DA_MAX_X; x += CALC_CLEARING_STEP) {
    y = getLineY(x, DA_MIN_X, DA_MIN_Y, DA_MAX_X, ERS_Y);
    moveTo(x, y);
  }

  moveTo(ERS_X, ERS_Y);
  
  charAreaStartX = (float)DA_MIN_X;
}

void alarmRoutine() {
  isAlarm = true;
}

void setup() {
  Serial.begin(9600);

  // rtc setup
  
  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC"));
    while (1);
  }

  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  DateTime compileDT = DateTime(F(__DATE__), F(__TIME__));
  uint32_t unixCompileDT = compileDT.unixtime();
  uint32_t unixPrevCompileDT;
  
  EEPROM.get(EEPROM_ADDR_PREV_COMP_DT, unixPrevCompileDT);

  if (unixPrevCompileDT != unixCompileDT) {    
    EEPROM.put(EEPROM_ADDR_PREV_COMP_DT, unixCompileDT);
    rtc.adjust(compileDT);
  }

  // set INTCN bit to 1
  // (disable square wave and enable interrupt mode).
  // it's required to set explicitly because of
  // setAlarm1/2 methods actually will not set alarms if it is 0
  rtc.writeSqwPinMode(DS3231_OFF);

  DateTime alarmDT = DateTime(0, 0, 0, 0, 0, 0);
  rtc.setAlarm2(alarmDT, DS3231_A2_PerMinute);

  rtc.clearAlarm(2);

  pinMode(PIN_ALARM_INT, INPUT_PULLUP);
  attachInterrupt(NO_ALARM_INT, alarmRoutine, FALLING);

  // servos setup

  servoLeft.attach(PIN_SERVO_LEFT, SERVO_MIN, SERVO_MAX);
  servoRight.attach(PIN_SERVO_RIGHT, SERVO_MIN, SERVO_MAX);
  servoBottom.attach(PIN_SERVO_BOTTOM, SERVO_MIN, SERVO_MAX);

  servoLeftLastMicroseconds = servoLeft.readMicroseconds();
  servoRightLastMicroseconds = servoRight.readMicroseconds();
  servoBottomLastMicroseconds = servoBottom.readMicroseconds();

  // move to start point

  lift(LIFT_FULLUP);
  moveTo(ERS_X, ERS_Y);
  lift(LIFT_ERS_BOTTOM);
}

void loop() {
  if (isInitialDrawing || isAlarm) {    
    clearArea();
    
    DateTime dt = rtc.now();

    if (isInitialDrawing && isAlarm) {
      rtc.clearAlarm(2);
      isAlarm = false;
    }
    
    char dtString[5];
    sprintf(dtString, "%02d:%02d", dt.hour(), dt.minute());
  
    lift(LIFT_FULLUP);
    moveTo(charAreaStartX, DA_MIN_Y);
    lift(LIFT_WR_UP);
  
    draw(dtString);
  
    lift(LIFT_FULLUP);
    moveTo(ERS_X, ERS_Y);
    lift(LIFT_ERS_BOTTOM);

    if (isInitialDrawing) {
      isInitialDrawing = false;
    } else if (isAlarm) {
      rtc.clearAlarm(2);
      isAlarm = false;
    }
  }
}
