///////////////////////////////////////////////////////////        
// helixList.cpp
//
// This program draws several helixes using a display list.
// 
// Sumanta Guha.
///////////////////////////////////////////////////////////

#include <cstdlib>
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265

using namespace std;

// Globals.
static unsigned int aHelix; // List index.

// Initialization routine.
void setup(void) 
{
   float t; // Angle parameter.

   aHelix = glGenLists(1); // Return a list index.

   // Begin create a display list.
   glNewList(aHelix, GL_COMPILE);

   // Draw a helix.
   glBegin(GL_LINE_STRIP);
   for(t = -10 * PI; t <= 10 * PI; t += PI/20.0) 
      glVertex3f(20 * cos(t), 20 * sin(t), -50);
   glEnd();
   
   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   glRectf(100, 100, 150, 200);
   glRectf(100, 150, 300, 350);

   glEndList();
   // End create a display list.

   glClearColor(1.0, 1.0, 1.0, 0.0);  
}

// Initialization routine.
void drawScene(void)
{  
   glClear(GL_COLOR_BUFFER_BIT);

   GLfloat u;
   for(u = 0; u <= 1; u += 0.1){
      glColor3f(u, 0.0, 1 - u);
      glPushMatrix();
      glScalef(u, u, 1.0);
      glCallList(aHelix);
      glPopMatrix();   
   }

   glFlush();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch(key) 
   {
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

// Main routine.
int main(int argc, char **argv) 
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(100, 100); 
   glutCreateWindow("helixList.cpp");
   setup(); 
   glutDisplayFunc(drawScene); 
   glutReshapeFunc(resize);  
   glutKeyboardFunc(keyInput);
   glutMainLoop(); 

   return 0;  
}