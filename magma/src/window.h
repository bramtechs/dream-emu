#pragma once

#include "raylib.h"
#include "raymath.h"
#include "math_custom.h"

void InitMagmaWindow(int gameWidth, int gameHeight, int winWidth, int winHeight, const char* title); // create a raylib window with 'letterbox' support for
                                                                                                     // those for crusty PS1 looks
void BeginMagmaDrawing();

void EndMagmaDrawing();

void CloseMagmaWindow();

float GetMagmaScaleFactor();
float GetLeftMagmaWindowOffset(); // get the width of the horizontal black bars
float GetTopMagmaWindowOffset();  // get the height of the vertical black bars

Vector2 GetMagmaGameSize();
Vector2 GetScaledMousePosition();
Ray GetWindowMouseRay(Camera camera);
