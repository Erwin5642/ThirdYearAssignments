////////////////////////////////////////////////////////////////////////////////////
// canvas.cpp
//
// This program allows the user to draw simple shapes on a canvas.
//
// Interaction:
// Left click on a box on the left to select a primitive.
// Then left click on the drawing area: once for point, twice for line or rectangle.
// Right click for menu options.
//
//  Sumanta Guha.
////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <vector>
#include <iostream>
#include <cmath>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

#define DASHED_STYLE 0x00ff
#define TOLERANCE 7

#define INACTIVE 0
#define POINT 1
#define LINESEGMENT 2
#define RECTANGLE 3
#define VECTOR 4
#define FIXEDLINE 5
#define POLYLINE 6
#define LINE 7
#define PERPENDICULAR 8
#define PARALLEL 9
#define MEDIATRIZ 10
#define BISECTOR 11
#define TEXT 12
#define NUMBERPRIMITIVES 12

// Use the STL extension of C++.
using namespace std;

// Globals.
static GLsizei width, height;    // OpenGL window size.
static float pointSize = 3.0;    // Size of point
static int primitive = INACTIVE; // Current drawing primitive.
static int pointCount = 0;       // Number of  specified points.
static int tempX, tempY;         // Co-ordinates of clicked point.
static int tempX2, tempY2;
static int currentX, currentY;
static int isGrid = 1; // Is there grid?
static bool isDashed = true;
static bool isFilled = false;
static float curColor[3] = {0, 0, 0};
static int grid_divisions = 10;
static bool point_interaction = true;
static float fixed_size = 10.0;

// Point class.
class Point
{
public:
   Point(int xVal, int yVal)
   {
      x = xVal;
      y = yVal;
      for (int i = 0; i < 3; i++)
         color[i] = curColor[i];
   }
   void drawPoint(void); // Function to draw a point.
private:
   int x, y;          // x and y co-ordinates of point.
   static float size; // Size of point.
   float color[3];
};

float Point::size = pointSize; // Set point size.

// Function to draw a point.
void Point::drawPoint()
{
   glColor3fv(color);

   glPointSize(size);
   glBegin(GL_POINTS);
   glVertex3f(x, y, 0.0);
   glEnd();
}

// Vector of points.
vector<Point> points;

// Iterator to traverse a Point array.
vector<Point>::iterator pointsIterator;

// Function to draw all points in the points array.
void drawPoints(void)
{
   // Loop through the points array drawing each point.
   pointsIterator = points.begin();
   while (pointsIterator != points.end())
   {
      pointsIterator->drawPoint();
      pointsIterator++;
   }
}

// LineSegment class.
class LineSegment
{
public:
   LineSegment(int x1Val, int y1Val, int x2Val, int y2Val)
   {
      x1 = x1Val;
      y1 = y1Val;
      x2 = x2Val;
      y2 = y2Val;
      slope = x1 == x2 ? MAXFLOAT : (float)(y2 - y1) / (x2 - x1);
      intersect = y2 - slope * x1;
      for (int i = 0; i < 3; i++)
         color[i] = curColor[i];
      dashed = isDashed;
   }
   void drawLine();
   float getSlope();
   float getX1();
   float getX2();
   float getY1();
   float getY2();
   float getIntersect();

private:
   int x1, y1, x2, y2; // x and y co-ordinates of endpoints.
   float color[3];
   bool dashed;
   float slope;
   float intersect;
};

// Function to draw a line.
void LineSegment::drawLine()
{
   glColor3fv(color);

   if (dashed)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, DASHED_STYLE);
   }

   glBegin(GL_LINES);
   glVertex3f(x1, y1, 0.0);
   glVertex3f(x2, y2, 0.0);
   glEnd();
   if (dashed)
   {
      glDisable(GL_LINE_STIPPLE);
   }
}

float LineSegment::getX1() { return x1; }
float LineSegment::getX2() { return x2; }
float LineSegment::getY1() { return y1; }
float LineSegment::getY2() { return y2; }
float LineSegment::getSlope() { return slope; }
float LineSegment::getIntersect() { return intersect; }

// Vector of lines.
vector<LineSegment> lineSegments;

// Iterator to traverse a LineSegment array.
vector<LineSegment>::iterator lineSegmentsIterator;

static LineSegment *selectedLine1 = nullptr;
static LineSegment *selectedLine2 = nullptr;

// Function to draw all lines in the lines array.
void drawLineSegments(void)
{
   // Loop through the lines array drawing each line.
   lineSegmentsIterator = lineSegments.begin();
   while (lineSegmentsIterator != lineSegments.end())
   {
      lineSegmentsIterator->drawLine();
      lineSegmentsIterator++;
   }
}

// Rectangle class.
class Rectangle
{
public:
   Rectangle(int x1Val, int y1Val, int x2Val, int y2Val)
   {
      x1 = x1Val;
      y1 = y1Val;
      x2 = x2Val;
      y2 = y2Val;
      for (int i = 0; i < 3; i++)
         color[i] = curColor[i];
      dashed = isDashed;
      filled = isFilled;
   }
   void drawRectangle();

private:
   int x1, y1, x2, y2; // x and y co-ordinates of diagonally opposite vertices.
   float color[3];
   bool dashed;
   bool filled;
};

// Function to draw a rectangle.
void Rectangle::drawRectangle()
{
   glColor3fv(color);

   if (dashed)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, DASHED_STYLE);
   }

   glPolygonMode(GL_FRONT_AND_BACK, filled ? GL_FILL : GL_LINE);
   glRectf(x1, y1, x2, y2);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   if (dashed)
   {
      glDisable(GL_LINE_STIPPLE);
   }
}

// Vector of rectangles.
vector<Rectangle> rectangles;

// Iterator to traverse a Rectangle array.
vector<Rectangle>::iterator rectanglesIterator;

// Function to draw all rectangles in the rectangles array.
void drawRectangles(void)
{
   // Loop through the rectangles array drawing each rectangle.
   rectanglesIterator = rectangles.begin();
   while (rectanglesIterator != rectangles.end())
   {
      rectanglesIterator->drawRectangle();
      rectanglesIterator++;
   }
}

class Vector
{
public:
   Vector(int x1Val, int y1Val, int x2Val, int y2Val, bool temporary = false)
   {
      x1 = x1Val;
      y1 = y1Val;
      x2 = x2Val;
      y2 = y2Val;
      line = LineSegment(x1Val, y1Val, x2Val, y2Val);
      if (!temporary)
         lineSegments.push_back(line);
      for (int i = 0; i < 3; i++)
         color[i] = curColor[i];
      dashed = isDashed;
      filled = isFilled;
   }
   void drawVector();

private:
   LineSegment line = LineSegment(x1, y1, x2, y2);
   int x1, y1, x2, y2;
   float color[3];
   bool dashed;
   bool filled;
};

void Vector::drawVector()
{
   glColor3fv(color);

   // Desenha a linha do vetor
   if (dashed)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, DASHED_STYLE);
   }

   line.drawLine();

   // Calcula o ângulo da seta
   float angle = atan2(y2 - y1, x2 - x1);

   // Desenha a ponta do vetor (triângulo)
   glPolygonMode(GL_FRONT_AND_BACK, filled ? GL_FILL : GL_LINE);

   glBegin(GL_TRIANGLES);
   glVertex3f(x2, y2, 0.0);
   glVertex3f(x2 - 15 * cos(angle - M_PI / 6), y2 - 15 * sin(angle - M_PI / 6), 0.0);
   glVertex3f(x2 - 15 * cos(angle + M_PI / 6), y2 - 15 * sin(angle + M_PI / 6), 0.0);
   glEnd();

   // Restaura o modo de polígono para o padrão
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   if (dashed)
      glDisable(GL_LINE_STIPPLE);
}

vector<Vector> vectors;
vector<Vector>::iterator vectorsIterator;

void drawVectors(void)
{
   vectorsIterator = vectors.begin();
   while (vectorsIterator != vectors.end())
   {
      vectorsIterator->drawVector();
      vectorsIterator++;
   }
}

class Line
{
public:
   Line(int x1Val, int y1Val, int x2Val, int y2Val, bool temporary = false)
   {
      x1 = x1Val;
      y1 = y1Val;
      x2 = x2Val;
      y2 = y2Val;
      float startX, startY, endX, endY;
      if (x2 != x1)
      {
         slope = (float)(y2 - y1) / (x2 - x1);
         startX = 0.1f * width;
         startY = y1 + slope * (startX - x1);
         endX = width;
         endY = y1 + slope * (width - x1);
      }
      else
      {
         if (x1 >= 0.1f * width)
         {
            startX = x1;
            endX = x1;
            startY = 0;
            endY = height;
         }
      }
      x1 = startX;
      x2 = endX;
      y1 = startY;
      y2 = endY;
      line = LineSegment(x1, y1, x2, y2);
      if (!temporary)
         lineSegments.push_back(line);
      for (int i = 0; i < 3; i++)
         color[i] = curColor[i];
      dashed = isDashed;
   }
   void drawLine();

private:
   int x1, x2, y1, y2; // Dois pontos que definem a direção da Line
   LineSegment line = LineSegment(x1, y1, x2, y2);
   float slope;
   float color[3];
   bool dashed;
};

void Line::drawLine()
{
   glColor3fv(color);
   if (isDashed)
   {
      glEnable(GL_LINE_STIPPLE);
      glLineStipple(1, DASHED_STYLE);
   }
   glBegin(GL_LINES);
   glVertex3f(x1, y1, 0.0);
   glVertex3f(x2, y2, 0.0);
   glEnd();

   if (isDashed)
      glDisable(GL_LINE_STIPPLE);
}

vector<Line> lines;                   // vetor para lines
vector<Line>::iterator linesIterator; // Iterador para percorrer um array Line.

// Função para desenhar todos os lines no array lines.
void drawLines(void)
{
   // Percorra o array lines desenhando cada Line.
   linesIterator = lines.begin();
   while (linesIterator != lines.end())
   {
      linesIterator->drawLine();
      linesIterator++;
   }
}

void createPerpendicularLine(LineSegment line_segment, int x, int y, bool temporary = false)
{
   float slope;
   float intersect;
   float otherSlope;
   float otherIntersect;
   otherSlope = line_segment.getSlope();
   otherIntersect = line_segment.getIntersect();
   slope = -(1 / otherSlope);
   intersect = y - slope * x;
   LineSegment line = LineSegment(width * 0.1, slope * (width * 0.1) + intersect, width, slope * width + intersect);
   if (!temporary)
   {
      lineSegments.push_back(line);
   }
   else
   {
      line.drawLine();
   }
}

void createParallelLine(LineSegment line_segment, int x, int y, bool temporary = false)
{
   float slope;
   float intersect;
   float otherSlope;
   float otherIntersect;
   otherSlope = line_segment.getSlope();
   otherIntersect = line_segment.getIntersect();
   slope = otherSlope;
   intersect = y - slope * x;
   LineSegment line = LineSegment(width * 0.1, slope * (width * 0.1) + intersect, width, slope * width + intersect);
   if (!temporary)
      lineSegments.push_back(line);
   else
   {
      line.drawLine();
   }
}

void createBisectorLine(float x1, float y1, float x2, float y2, float x3, float y3, bool temporary = false)
{
   if (point_interaction)
   { // Faz o desenho inteiro de uma bissetriz, isso serve para caso eu tenha 2 retas ele não completar
      LineSegment l1 = LineSegment(x1, y1, x2, y2);
      LineSegment l2 = LineSegment(x2, y2, x3, y3);
      if (temporary)
      {
         l1.drawLine();
         l2.drawLine();
      }
      else
      {
         lineSegments.push_back(l1);
         lineSegments.push_back(l2);
      }
   }

   // encontrando bissetriz

   float v1x = x1 - x2;
   float v1y = y1 - y2;

   float v2x = x3 - x2;
   float v2y = y3 - y2;

   // Normaliza os vetores
   float len1 = std::sqrt(v1x * v1x + v1y * v1y);
   float len2 = std::sqrt(v2x * v2x + v2y * v2y);

   float u1x = v1x / len1;
   float u1y = v1y / len1;

   float u2x = v2x / len2;
   float u2y = v2y / len2;

   // Soma dos vetores unitários -> direção da bissetriz
   float bx = u1x + u2x;
   float by = u1y + u2y;

   // Normaliza a direção da bissetriz
   float bLen = std::sqrt(bx * bx + by * by);
   float ubx = bx / bLen;
   float uby = by / bLen;

   // Define o comprimento da bissetriz
   float bisLength = 1000;

   // Calcula os pontos de Bissetriz apartir de P2
   float bxFinal = x2 + ubx * bisLength;
   float byFinal = y2 + uby * bisLength;

   float bxFinalOp = x2 - ubx * bisLength;
   float byFinalOp = y2 - uby * bisLength;

   if (!point_interaction)
      createPerpendicularLine(LineSegment(bxFinalOp, byFinalOp, bxFinal, byFinal), x2, y2, temporary);
   Line line = Line(bxFinalOp, byFinalOp, bxFinal, byFinal, temporary);
   if (temporary)
   {
      line.drawLine();
   }
}

void createMediatrizLine(float x1, float y1, float x2, float y2, bool temporary = false){
   //ponto médio
   float mx = (x1 + x2) / 2.0f;
   float my = (y1 + y2) / 2.0f;
   createPerpendicularLine(LineSegment(x1, y1, x2, y2), mx, my, temporary);
}

class Text
{
   int x, y;
   string text;
   float color[3];

public:
   Text(int x, int y, string str) : x(x), y(y), text(str)
   {
      for (int i = 0; i < 3; i++)
         color[i] = curColor[i];
   }

   void drawText();
};

void Text::drawText()
{
   glColor3fv(color);
   glRasterPos2i(x, y);
   for (char c : text)
   {
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
   }
}

vector<Text> texts;
vector<Text>::iterator texts_iterator;

void drawTexts()
{
   texts_iterator = texts.begin();
   while (texts_iterator != texts.end())
   {
      texts_iterator->drawText();
      texts_iterator++;
   }
}

// Function to draw point selection box in left selection area.
void drawPointSelectionBox(void)
{
   if (primitive == POINT)
      glColor3f(1.0, 1.0, 1.0); // Highlight.
   else
      glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - POINT)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - POINT + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - POINT)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - POINT + 1)) / NUMBERPRIMITIVES * height);

   // Draw point icon.
   glPointSize(pointSize);
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_POINTS);
   glVertex3f(0.05 * width, (0.5 + NUMBERPRIMITIVES - POINT) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
}

// Function to draw line selection box in left selection area.
void drawLineSegmentSelectionBox(void)
{
   if (primitive == LINESEGMENT)
      glColor3f(1.0, 1.0, 1.0); // Highlight.
   else
      glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - LINESEGMENT)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - LINESEGMENT + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - LINESEGMENT)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - LINESEGMENT + 1)) / NUMBERPRIMITIVES * height);

   // Draw line icon.
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_LINES);
   glVertex3f(0.025 * width, (0.75 + NUMBERPRIMITIVES - LINESEGMENT) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.25 + NUMBERPRIMITIVES - LINESEGMENT) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
}

// Function to draw rectangle selection box in left selection area.
void drawRectangleSelectionBox(void)
{
   if (primitive == RECTANGLE)
      glColor3f(1.0, 1.0, 1.0); // Highlight.
   else
      glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - RECTANGLE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - RECTANGLE + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - RECTANGLE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - RECTANGLE + 1)) / NUMBERPRIMITIVES * height);

   // Draw rectangle icon.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.025 * width, (0.35 + NUMBERPRIMITIVES - RECTANGLE) / NUMBERPRIMITIVES * height, 0.075 * width, (0.65 + NUMBERPRIMITIVES - RECTANGLE) / NUMBERPRIMITIVES * height);
   glEnd();
}

void drawVectorSelectionBox(void)
{
   if (primitive == VECTOR)
      glColor3f(1.0, 1.0, 1.0);
   else
      glColor3f(0.8, 0.8, 0.8);

   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - VECTOR)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - VECTOR + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - VECTOR)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - VECTOR + 1)) / NUMBERPRIMITIVES * height);

   float x1 = 0.025 * width, y1 = (0.25 + NUMBERPRIMITIVES - VECTOR) / NUMBERPRIMITIVES * height;
   float x2 = 0.075 * width, y2 = (0.75 + NUMBERPRIMITIVES - VECTOR) / NUMBERPRIMITIVES * height;

   // Draw vector icon (diagonal line)
   glBegin(GL_LINES);
   glVertex3f(x1, y1, 0.0); // Start of vector line
   glVertex3f(x2, y2, 0.0); // End of vector line
   glEnd();

   // Calculate the direction of the vector
   float angle = atan2(y2 - y1, x2 - x1); // Calculate angle between start and end of vector

   // Adjust the size of the arrowhead
   float arrowSize = 0.02 * width; // Adjust arrowhead size relative to width

   // Draw arrowhead (adjusted based on the angle)
   glBegin(GL_TRIANGLES);
   // Tip of the vector
   glVertex3f(x2, y2, 0.0);
   // Right part of the arrowhead
   glVertex3f(x2 - arrowSize * cos(angle - M_PI / 6), y2 - arrowSize * sin(angle - M_PI / 6), 0.0);
   // Left part of the arrowhead
   glVertex3f(x2 - arrowSize * cos(angle + M_PI / 6), y2 - arrowSize * sin(angle + M_PI / 6), 0.0);
   glEnd();
}

void drawFixedSegmentSelectionBox(void)
{
   if (primitive == FIXEDLINE)
      glColor3f(1.0, 1.0, 1.0); // Highlight.
   else
      glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - FIXEDLINE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - FIXEDLINE + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - FIXEDLINE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - FIXEDLINE + 1)) / NUMBERPRIMITIVES * height);

   glBegin(GL_LINES);
   glVertex3f(0.025 * width, (0.25 + NUMBERPRIMITIVES - FIXEDLINE) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.75 + NUMBERPRIMITIVES - FIXEDLINE) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
   // Draw fixed segment icon (a line with a circle at one end).
   glPointSize(5.0); // Circle at one end
   glBegin(GL_POINTS);
   glVertex3f(0.025 * width, (0.25 + NUMBERPRIMITIVES - FIXEDLINE) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
   glPointSize(pointSize);
}

// Função para desenhar caixa de seleção Line na área de seleção esquerda.
void drawLineSelectionBox(void)
{
   if (primitive == LINE)
      glColor3f(1.0, 1.0, 1.0);
   else
      glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - LINE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - LINE + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - LINE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - LINE + 1)) / NUMBERPRIMITIVES * height);

   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_LINES);                                                                           // Desenho da Line
   glVertex3f(0.025 * width, (0.5 + NUMBERPRIMITIVES - LINE) / NUMBERPRIMITIVES * height, 0.0); // Ponto inicial da Line
   glVertex3f(0.075 * width, (0.5 + NUMBERPRIMITIVES - LINE) / NUMBERPRIMITIVES * height, 0.0); // Ponto final da Line
   glEnd();

   // Desenhar um pequeno triângulo (como ícone) para a Line
   glBegin(GL_TRIANGLES);
   glVertex3f(0.075 * width, (0.5 + NUMBERPRIMITIVES - LINE) / NUMBERPRIMITIVES * height, 0.0);  // Ponto do triângulo
   glVertex3f(0.065 * width, (0.45 + NUMBERPRIMITIVES - LINE) / NUMBERPRIMITIVES * height, 0.0); // Ponto do triângulo
   glVertex3f(0.065 * width, (0.55 + NUMBERPRIMITIVES - LINE) / NUMBERPRIMITIVES * height, 0.0); // Ponto do triângulo
   glEnd();
}

void drawPolyLineSelectionBox(void)
{
   if (primitive == POLYLINE)
      glColor3f(1.0, 1.0, 1.0); // destaque.
   else
      glColor3f(0.8, 0.8, 0.8); // sem destaque.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - POLYLINE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - POLYLINE + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - POLYLINE)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - POLYLINE + 1)) / NUMBERPRIMITIVES * height);

   // desenha o ícone da polilinha
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_LINE_STRIP);
   glVertex3f(0.025 * width, (0.25 + NUMBERPRIMITIVES - POLYLINE) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.05 * width, (0.5 + NUMBERPRIMITIVES - POLYLINE) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.45 + NUMBERPRIMITIVES - POLYLINE) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
}

void drawPerpendicularLineSelectionBox()
{

   if (primitive == PERPENDICULAR)
      glColor3f(1.0, 1.0, 1.0);
   else
      glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - PERPENDICULAR)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - PERPENDICULAR + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - PERPENDICULAR)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - PERPENDICULAR + 1)) / NUMBERPRIMITIVES * height);

   glBegin(GL_LINES);
   glVertex3f(0.025 * width, (0.5 + NUMBERPRIMITIVES - PERPENDICULAR) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.5 + NUMBERPRIMITIVES - PERPENDICULAR) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();

   glBegin(GL_LINES);
   glVertex3f(0.050 * width, (0.25 + NUMBERPRIMITIVES - PERPENDICULAR) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.050 * width, (0.75 + NUMBERPRIMITIVES - PERPENDICULAR) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
}

// Desenha a caixa de seleção da primitiva linha paralela.
void drawParallelLineSelectionBox(void)
{
   if (primitive == PARALLEL)
      glColor3f(1.0, 1.0, 1.0);
   else
      glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - PARALLEL)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - PARALLEL + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - PARALLEL)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - PARALLEL + 1)) / NUMBERPRIMITIVES * height);

   // Ícone de retas paralelas.
   glBegin(GL_LINES);
   glVertex3f(0.025 * width, (0.25 + NUMBERPRIMITIVES - PARALLEL) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.025 * width, (0.75 + NUMBERPRIMITIVES - PARALLEL) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();

   glBegin(GL_LINES);
   glVertex3f(0.075 * width, (0.25 + NUMBERPRIMITIVES - PARALLEL) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.75 + NUMBERPRIMITIVES - PARALLEL) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
}

void drawMediatrizLineSelectionBox()
{

   if (primitive == MEDIATRIZ)
      glColor3f(1.0, 1.0, 1.0);
   else
      glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - MEDIATRIZ)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - MEDIATRIZ + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - MEDIATRIZ)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - MEDIATRIZ + 1)) / NUMBERPRIMITIVES * height);

   glBegin(GL_LINES);
   glVertex3f(0.025 * width, (0.25 + NUMBERPRIMITIVES - MEDIATRIZ) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.75 + NUMBERPRIMITIVES - MEDIATRIZ) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();

   glBegin(GL_LINES);
   glVertex3f(0.050 * width, (0.50 + NUMBERPRIMITIVES - MEDIATRIZ) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.25 + NUMBERPRIMITIVES - MEDIATRIZ) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
}

void drawBisectorSelectionBox(void)
{
   if (primitive == BISECTOR)
      glColor3f(1.0, 1.0, 1.0); // Highlight.
   else
      glColor3f(0.8, 0.8, 0.8); // No highlight.
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - BISECTOR)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - BISECTOR + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - BISECTOR)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - BISECTOR + 1)) / NUMBERPRIMITIVES * height);

   // Draw BISSETRIZ icon. - Trocar dps
   glColor3f(0.0, 0.0, 0.0);
   glBegin(GL_LINES);
   glVertex3f(0.025 * width, (0.25 + NUMBERPRIMITIVES - BISECTOR) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.025 * width, (0.65 + NUMBERPRIMITIVES - BISECTOR) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.025 * width, (0.65 + NUMBERPRIMITIVES - BISECTOR) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(0.075 * width, (0.65 + NUMBERPRIMITIVES - BISECTOR) / NUMBERPRIMITIVES * height, 0.0);

   // Linha bissetriz
   glColor3f(1.0, 0.0, 0.0);
   glVertex3f(0.025 * width, (0.65 + NUMBERPRIMITIVES - BISECTOR) / NUMBERPRIMITIVES * height, 0.0);
   glVertex3f(width * 0.0532, (0.367 + NUMBERPRIMITIVES - BISECTOR) / NUMBERPRIMITIVES * height, 0.0);
   glEnd();
   glColor3f(0.0, 0.0, 0.0);
}

void drawTextSelectionBox()
{
   if (primitive == TEXT)
      glColor3f(1.0, 1.0, 1.0);
   else
      glColor3f(0.8, 0.8, 0.8);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - TEXT)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - TEXT + 1)) / NUMBERPRIMITIVES * height);

   // Draw black boundary.
   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, ((float)(NUMBERPRIMITIVES - TEXT)) / NUMBERPRIMITIVES * height, 0.1 * width, ((float)(NUMBERPRIMITIVES - TEXT + 1)) / NUMBERPRIMITIVES * height);

   glRasterPos2f(0.02 * width, (0.4 + NUMBERPRIMITIVES - TEXT) / NUMBERPRIMITIVES * height);
   string str = "TXT";
   for (char c : str)
   {
      glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
   }
}

// Function to draw unused part of left selection area.
void drawInactiveArea(void)
{
   glColor3f(0.6, 0.6, 0.6);
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(0.0, 0.0, 0.1 * width, (1 - NUMBERPRIMITIVES * 0.1) * height);

   glColor3f(0.0, 0.0, 0.0);
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   glRectf(0.0, 0.0, 0.1 * width, (1 - NUMBERPRIMITIVES * 0.1) * height);
}

void passiveMouseFunc(int x, int y)
{
   currentX = x;
   currentY = height - y;
}
// Function to draw temporary point.
void drawTempPoint(void)
{
   glColor3f(1.0, 0.0, 0.0);
   glPointSize(pointSize);
   glBegin(GL_POINTS);
   glVertex3f(tempX, tempY, 0.0);
   glEnd();
}

int checkIfColliding(int x, int y)
{
   size_t size = lineSegments.size();
   for (int i = 0; i < size; i++)
   {
      int x1, y1, x2, y2;
      LineSegment temp = lineSegments[i];
      x1 = temp.getX1();
      y1 = temp.getY1();
      x2 = temp.getX2();
      y2 = temp.getY2();
      int aux;
      if(x1 > x2){
         aux = x1;
         x1 = x2;
         x2 = aux;
      }
      if ((x > x1 && x < x2) && abs((x2 - x1) * (y1 - y) - (y2 - y1) * (x1 - x)) / sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2)) < TOLERANCE)
      {
         return i;
      }
   }
   return -1;
}

bool isSamePoint(float ax, float ay, float bx, float by, float tol = 1.0f)
{
   return hypot(ax - bx, ay - by) < tol;
}

void drawTempPoint2(void)
{
   glColor3f(1.0, 0.0, 0.0);
   glPointSize(pointSize);
   glBegin(GL_POINTS);
   glVertex3f(tempX, tempY, 0.0);
   glVertex3f(tempX2, tempY2, 0.0);
   glEnd();
}

void drawTempPrimitive(void)
{
   switch (primitive)
   {
   case LINE:
   {
      Line tempLine = Line(tempX, tempY, currentX, currentY, true);
      tempLine.drawLine();
   }
   break;
   case VECTOR:
   {
      Vector tempVector = Vector(tempX, tempY, currentX, currentY, true);
      tempVector.drawVector();
   }
   break;
   case RECTANGLE:
   {
      Rectangle tempRectangle = Rectangle(tempX, tempY, currentX, currentY);
      tempRectangle.drawRectangle();
   }
   break;
   case POLYLINE:
   case LINESEGMENT:
   {
      LineSegment tempLineSegment = LineSegment(tempX, tempY, currentX, currentY);
      tempLineSegment.drawLine();
   }
   break;
   case FIXEDLINE:
   {
      float dx = currentX - tempX;
      float dy = currentY - tempY;
      float magnitude = sqrt(dx * dx + dy * dy);

      // Normalize direction vector.
      dx /= magnitude;
      dy /= magnitude;

      // Calculate endpoint with specified length.
      int endX = tempX + dx * fixed_size;
      int endY = tempY + dy * fixed_size;

      // Store the fixed segment in the vector.
      LineSegment tempFixedLine = LineSegment(tempX, tempY, endX, endY);
      tempFixedLine.drawLine();
   }
   break;
   case PERPENDICULAR:
   {
      int i;
      if ((i = checkIfColliding(currentX, currentY)) != -1)
      {
         createPerpendicularLine(lineSegments[i], tempX, tempY, true);
      }
   }
   break;
   case PARALLEL:
   {
      int i;
      if ((i = checkIfColliding(currentX, currentY)) != -1)
      {
         createParallelLine(lineSegments[i], tempX, tempY, true);
      }
   }
   break;
   case MEDIATRIZ:
   {
      int i;
      if(point_interaction && pointCount == 1){
         createMediatrizLine(tempX, tempY, currentX, currentY, true);
      }
      else if((i = checkIfColliding(currentX, currentY)) != -1){
         createMediatrizLine(lineSegments[i].getX1(),lineSegments[i].getY1(),lineSegments[i].getX2(),lineSegments[i].getY2(), true);
      }
   }
   break;
   case BISECTOR:
   {
      int i;
      if (point_interaction && pointCount == 2)
      {
         createBisectorLine(tempX, tempY, tempX2, tempY2, currentX, currentY, true);
      }
      else if (!point_interaction && (i = checkIfColliding(currentX, currentY)) != -1)
      {

         selectedLine2 = &lineSegments[i];
         tempX2 = lineSegments[i].getX1();
         tempY2 = lineSegments[i].getY1();

         float x1 = selectedLine1->getX1(), y1 = selectedLine1->getY1();
         float x2 = selectedLine1->getX2(), y2 = selectedLine1->getY2();
         float x3 = selectedLine2->getX1(), y3 = selectedLine2->getY1();
         float x4 = selectedLine2->getX2(), y4 = selectedLine2->getY2();

         float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

         if (denom != 0)
         {
            float Px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
            float Py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

            // P1 = ponto da primeira linha que não é P2
            float p1x = isSamePoint(x1, y1, Px, Py) ? x2 : x1;
            float p1y = isSamePoint(x1, y1, Px, Py) ? y2 : y1;

            // P3 = ponto da segunda linha que não é P2
            float p3x = isSamePoint(x3, y3, Px, Py) ? x4 : x3;
            float p3y = isSamePoint(x3, y3, Px, Py) ? y4 : y3;

            createBisectorLine(p1x, p1y, Px, Py, p3x, p3y, true);
         }
      }
      break;
   }
   }
   glutPostRedisplay();
}

// Function to draw a grid.
void drawGrid(void)
{
   int i;

   float x_axis_divisions = (0.9 * width) / grid_divisions;
   float y_axis_divisions = (float)height / grid_divisions;
   float boundary = 0.1 * width;

   glEnable(GL_LINE_STIPPLE);
   glLineStipple(1, 0x5555);
   glColor3f(0.75, 0.75, 0.75);

   glBegin(GL_LINES);
   for (i = 1; i <= grid_divisions; i++)
   {
      glVertex3f(boundary + i * x_axis_divisions, 0.0, 0.0);
      glVertex3f(boundary + i * x_axis_divisions, height, 0.0);
   }
   for (i = 1; i <= grid_divisions; i++)
   {
      glVertex3f(boundary, i * y_axis_divisions, 0.0);
      glVertex3f(width, i * y_axis_divisions, 0.0);
   }
   glEnd();
   glDisable(GL_LINE_STIPPLE);
}

// Drawing routine.
void drawScene(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(0.0, 0.0, 0.0);

   drawPoints();
   drawLineSegments();
   drawRectangles();
   drawVectors();
   drawLines();
   drawTexts();

   drawPointSelectionBox();
   drawLineSegmentSelectionBox();
   drawRectangleSelectionBox();
   drawVectorSelectionBox();
   drawFixedSegmentSelectionBox();
   drawLineSelectionBox();
   drawPolyLineSelectionBox();
   drawPerpendicularLineSelectionBox();
   drawMediatrizLineSelectionBox();
   drawParallelLineSelectionBox();
   drawBisectorSelectionBox();
   drawTextSelectionBox();
   drawInactiveArea();
   if((primitive == MEDIATRIZ) && (pointCount == 0)){
      drawTempPrimitive();
   }
   else if ((((primitive == LINESEGMENT) || (primitive == RECTANGLE) || (primitive == VECTOR) || (primitive == FIXEDLINE) || (primitive == LINE) || (primitive == PERPENDICULAR) || (primitive == PARALLEL) || (primitive == BISECTOR) || (primitive == MEDIATRIZ)) && (pointCount == 1)) || ((primitive == POLYLINE) && (pointCount >= 1)))
   {
      drawTempPoint();
      drawTempPrimitive();
   }
   else if ((primitive == BISECTOR) && (pointCount == 2))
   {
      drawTempPoint2();
      drawTempPrimitive();
   }
   if (isGrid)
      drawGrid();

   glutSwapBuffers();
}

// Function to pick primitive if click is in left selection area.
void pickPrimitive(int y)
{
   if (y < (1 - NUMBERPRIMITIVES * 0.1) * height)
      primitive = INACTIVE;
   else if (y < (((float)(NUMBERPRIMITIVES - TEXT + 1)) / NUMBERPRIMITIVES) * height)
      primitive = TEXT;
   else if (y < (((float)(NUMBERPRIMITIVES - BISECTOR + 1)) / NUMBERPRIMITIVES) * height)
      primitive = BISECTOR;
   else if (y < (((float)(NUMBERPRIMITIVES - MEDIATRIZ + 1)) / NUMBERPRIMITIVES) * height)
      primitive = MEDIATRIZ;
   else if (y < (((float)(NUMBERPRIMITIVES - PARALLEL + 1)) / NUMBERPRIMITIVES) * height)
      primitive = PARALLEL;
   else if (y < (((float)(NUMBERPRIMITIVES - PERPENDICULAR + 1)) / NUMBERPRIMITIVES) * height)
      primitive = PERPENDICULAR;
   else if (y < (((float)(NUMBERPRIMITIVES - LINE + 1)) / NUMBERPRIMITIVES) * height)
      primitive = LINE;
   else if (y < (((float)(NUMBERPRIMITIVES - POLYLINE + 1)) / NUMBERPRIMITIVES) * height)
      primitive = POLYLINE;
   else if (y < (((float)(NUMBERPRIMITIVES - FIXEDLINE + 1)) / NUMBERPRIMITIVES) * height)
      primitive = FIXEDLINE;
   else if (y < (((float)(NUMBERPRIMITIVES - VECTOR + 1)) / NUMBERPRIMITIVES) * height)
      primitive = VECTOR;
   else if (y < (((float)(NUMBERPRIMITIVES - RECTANGLE + 1)) / NUMBERPRIMITIVES) * height)
      primitive = RECTANGLE;
   else if (y < (((float)(NUMBERPRIMITIVES - LINESEGMENT + 1)) / NUMBERPRIMITIVES) * height)
      primitive = LINESEGMENT;
   else
      primitive = POINT;
}

// The mouse callback routine.
void mouseControl(int button, int state, int x, int y)
{
   y = height - y; // Correct from mouse to OpenGL co-ordinates.
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
   {
      // Click outside canvas - do nothing.
      if (x < 0 || x > width || y < 0 || y > height)
         ;

      // Click in left selection area.
      else if (x < 0.1 * width)
      {
         pickPrimitive(y);
         pointCount = 0;
      }

      // Click in canvas.
      else
      {
         switch (primitive)
         {
         case POINT:
            points.push_back(Point(x, y));
            break;
         case LINESEGMENT:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               lineSegments.push_back(LineSegment(tempX, tempY, x, y));
               pointCount = 0;
            }
            break;
         case RECTANGLE:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               rectangles.push_back(Rectangle(tempX, tempY, x, y));
               pointCount = 0;
            }
            break;
         case VECTOR:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               vectors.push_back(Vector(tempX, tempY, x, y));
               pointCount = 0;
            }
            break;
         case FIXEDLINE:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
               cout << "Digite o comprimento do segmento: ";
               cin >> fixed_size;
            }
            else if (pointCount == 1)
            {
               // Calculate direction vector.
               float dx = x - tempX;
               float dy = y - tempY;
               float magnitude = sqrt(dx * dx + dy * dy);

               // Normalize direction vector.
               dx /= magnitude;
               dy /= magnitude;

               // Calculate endpoint with specified length.
               int endX = tempX + dx * fixed_size;
               int endY = tempY + dy * fixed_size;

               // Store the fixed segment in the vector.
               lineSegments.push_back(LineSegment(tempX, tempY, endX, endY));
               pointCount = 0;
            }
            break;
         case LINE:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               lines.push_back(Line(tempX, tempY, x, y));
               pointCount = 0;
            }
            break;
         case POLYLINE:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               lineSegments.push_back(LineSegment(tempX, tempY, x, y));
               tempX = x;
               tempY = y;
               pointCount++;
            }
            break;
         case PERPENDICULAR:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               int i;
               if ((i = checkIfColliding(x, y)) != -1)
               {
                  createPerpendicularLine(lineSegments[i], tempX, tempY);
                  pointCount = 0;
               }
            }
            break;
         case PARALLEL:
            if (pointCount == 0)
            {
               tempX = x;
               tempY = y;
               pointCount++;
            }
            else
            {
               int i;
               if ((i = checkIfColliding(x, y)) != -1)
               {
                  createParallelLine(lineSegments[i], tempX, tempY);
                  pointCount = 0;
               }
            }
            break;
         case MEDIATRIZ:
         {
            if(pointCount == 0){
               int i;
               if((i = checkIfColliding(x, y)) != -1){
                  createMediatrizLine(lineSegments[i].getX1(),lineSegments[i].getY1(),lineSegments[i].getX2(),lineSegments[i].getY2());
                  point_interaction = true;
                  pointCount = 0;
               }
               else{
                  tempX = x;
                  tempY = y;
                  pointCount++;
               }
            }
            else{
               createMediatrizLine(tempX, tempY, x, y);
               pointCount = 0;
            }
            break;
         }
         case BISECTOR:
            if (pointCount == 0)
            {
               int i;
               if ((i = checkIfColliding(x, y)) != -1)
               {
                  point_interaction = false;
                  selectedLine1 = &lineSegments[i];
                  tempX = lineSegments[i].getX1();
                  tempY = lineSegments[i].getY1();
               }
               else
               {
                  tempX = x;
                  tempY = y;
               }
               pointCount++;
            }
            else if (pointCount == 1)
            {
               if (point_interaction)
               {
                  tempX2 = x;
                  tempY2 = y;
                  pointCount++;
               }
               else
               {
                  int i;
                  if ((i = checkIfColliding(x, y)) != -1)
                  {
                     selectedLine2 = &lineSegments[i];
                     tempX2 = lineSegments[i].getX1();
                     tempY2 = lineSegments[i].getY1();

                     float x1 = selectedLine1->getX1(), y1 = selectedLine1->getY1();
                     float x2 = selectedLine1->getX2(), y2 = selectedLine1->getY2();
                     float x3 = selectedLine2->getX1(), y3 = selectedLine2->getY1();
                     float x4 = selectedLine2->getX2(), y4 = selectedLine2->getY2();

                     float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

                     if (denom == 0)
                     {
                        cout << "As linhas são paralelas ou coincidentes. Não há interseção." << endl;
                     }
                     else
                     {
                        float Px = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / denom;
                        float Py = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / denom;

                        // P1 = ponto da primeira linha que não é P2
                        float p1x = isSamePoint(x1, y1, Px, Py) ? x2 : x1;
                        float p1y = isSamePoint(x1, y1, Px, Py) ? y2 : y1;

                        // P3 = ponto da segunda linha que não é P2
                        float p3x = isSamePoint(x3, y3, Px, Py) ? x4 : x3;
                        float p3y = isSamePoint(x3, y3, Px, Py) ? y4 : y3;

                        createBisectorLine(p1x, p1y, Px, Py, p3x, p3y);
                        pointCount = 0;
                        point_interaction = true;
                     }
                  }
               }
            }
            else
            {
               createBisectorLine(tempX, tempY, tempX2, tempY2, x, y);
               pointCount = 0;
               point_interaction = true;
            }
            break;
         case TEXT:
            string text;
            cout << "Digite o texto para ser exibido na tela: ";
            getline(cin, text);
            texts.push_back(Text(x, y, text));
            break;
         }
      }
   }
   else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
   {
      if (primitive == POLYLINE && pointCount > 0)
      {
         lineSegments.push_back(LineSegment(tempX, tempY, x, y));
         pointCount = 0;
      }
   }
   glutPostRedisplay();
}

// Initialization routine.
void setup(void)
{
   glClearColor(1.0, 1.0, 1.0, 0.0);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

   // Set viewing box dimensions equal to window dimensions.
   glOrtho(0.0, (float)w, 0.0, (float)h, -1.0, 1.0);

   // Pass the size of the OpenGL window to globals.
   width = w;
   height = h;

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch (key)
   {
   case 27:
      exit(0);
      break;
   default:
      break;
   }
}

// Clear the canvas and reset for fresh drawing.
void clearAll(void)
{
   points.clear();
   lineSegments.clear();
   lines.clear();
   rectangles.clear();
   vectors.clear();
   texts.clear();
   primitive = INACTIVE;
   pointCount = 0;
}

// The right button menu callback function.
void rightMenu(int id)
{
   if (id == 1)
   {
      clearAll();
      glutPostRedisplay();
   }
   if (id == 2)
      exit(0);
}

// The sub-menu callback function.
void grid_menu(int id)
{
   if (id == 1)
      grid_divisions = 20;
   if (id == 2)
      grid_divisions = 10;
   if (id == 3)
      grid_divisions = 5;
   if (id == 4)
      isGrid = 1;
   if (id == 5)
      isGrid = 0;
   glutPostRedisplay();
}

void dash_menu(int id)
{
   if (id == 3)
      isDashed = true;
   if (id == 4)
      isDashed = false;
   glutPostRedisplay();
}

void fill_menu(int id)
{
   if (id == 3)
      isFilled = true;
   if (id == 4)
      isFilled = false;
   glutPostRedisplay();
}

void color_menu(int id)
{
   if (id == 3)
   {
      curColor[0] = 1;
      curColor[1] = 0;
      curColor[2] = 0;
   }
   if (id == 4)
   {
      curColor[0] = 0;
      curColor[1] = 1;
      curColor[2] = 0;
   }
   if (id == 5)
   {
      curColor[0] = 0;
      curColor[1] = 0;
      curColor[2] = 1;
   }
   if (id == 6)
   {
      curColor[0] = 0;
      curColor[1] = 0;
      curColor[2] = 0;
   }
   glutPostRedisplay();
}

// Function to create menu.
void makeMenu(void)
{
   int sub_menu_grid;
   sub_menu_grid = glutCreateMenu(grid_menu);
   glutAddMenuEntry("Grade Pequena", 1);
   glutAddMenuEntry("Grade Media", 2);
   glutAddMenuEntry("Grade Grande", 3);
   glutAddMenuEntry("On", 4);
   glutAddMenuEntry("Off", 5);

   int sub_menu_dash;
   sub_menu_dash = glutCreateMenu(dash_menu);
   glutAddMenuEntry("On", 3);
   glutAddMenuEntry("Off", 4);

   int sub_menu_fill;
   sub_menu_fill = glutCreateMenu(fill_menu);
   glutAddMenuEntry("On", 3);
   glutAddMenuEntry("Off", 4);

   int sub_menu_color;
   sub_menu_color = glutCreateMenu(color_menu);
   glutAddMenuEntry("Vermelho", 3);
   glutAddMenuEntry("Verde", 4);
   glutAddMenuEntry("Azul", 5);
   glutAddMenuEntry("Preto", 6);

   glutCreateMenu(rightMenu);
   glutAddSubMenu("Grid", sub_menu_grid);
   glutAddSubMenu("Dash", sub_menu_dash);
   glutAddSubMenu("Fill", sub_menu_fill);
   glutAddSubMenu("Color", sub_menu_color);
   glutAddMenuEntry("Clear", 1);
   glutAddMenuEntry("Quit", 2);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   cout << "Interaction:" << endl;
   cout << "Left click on a box on the left to select a primitive." << endl
        << "Then left click on the drawing area: once for point, twice for line or rectangle." << endl
        << "Right click for menu options." << endl;
}

// Main routine.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("canvas.cpp");
   setup();
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);
   glutMouseFunc(mouseControl);
   glutPassiveMotionFunc(passiveMouseFunc);
   makeMenu(); // Create menu.

   glutMainLoop();

   return 0;
}