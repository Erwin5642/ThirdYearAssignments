/////////////////////////////////          
// box.cpp
//
// This program draws a wire box.
//
// Sumanta Guha.
/////////////////////////////////

#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/freeglut.h>
#endif

using namespace std;

#define min -0.02
#define max 0.02
static unsigned int object; // List index.

double x = -1, y = 0, z = 1, a = 0;
double vX = 0, vY = 0, vZ = 0;
static int animationPeriod = 1;
void writeBitmapString(void *font, char *string)
{  
   char *c;

   for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

static long font = (long)GLUT_BITMAP_8_BY_13; // Font selection.

void drawScene(void)
{
   glClear (GL_COLOR_BUFFER_BIT);
   glColor3f(1.0,0.0,0.0);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -15.0);
   glRotatef(a, x, y, z);
   glPushMatrix();
   glRotatef(-45, 0, 0, 1);
   glCallList(object);
   glPopMatrix();
   glPushMatrix();
   glRotatef(90, 0, 0, 1);
   glCallList(object);
   glPopMatrix();
   glPushMatrix();
   glRotatef(45, 0, 0, 1);
   glCallList(object);
   glPopMatrix();
   glCallList(object);
   glFlush();
}

void animate(int value)
{
   a += 1;
   double tempX = x, tempY = y, tempZ = z;
   double accel = min + ((float)rand() / RAND_MAX) * (max - min);
   vX += accel;
   accel = min + ((float)rand() / RAND_MAX) * (max - min);
   vY += accel;
   accel = min + ((float)rand() / RAND_MAX) * (max - min);
   vZ += accel;

   x += vX;
   y += vY;
   z += vZ;

   if(x > 100){
      vX *= -1;
      x = 100;
   }
   else if(x < -100){
      vX *= -1;
      x = -100;
   }
   if(y > 100){
      vY *= -1;
      y = 100;
   }
   else if(y < -100){
      vY *= -1;
      y = -100;
   }
   if(z > 100){
      vZ *= -1;
      z = 100;
   }
   else if(z < -100){
      vZ *= -1;
      z = -100;
   }
	if (a > 360.0) a -= 360.0;
   glutTimerFunc(animationPeriod, animate, 1);
   glutPostRedisplay();
}

// Initialization routine.
void setup(void) 
{
   glClearColor(1.0, 1.0, 1.0, 0.0); 
   
   object = glGenLists(1); // Return a list index.

   // Begin create a display list.
   glNewList(object, GL_COMPILE);
   glColor3f(1.0,0.0,0.0);
   glScalef(16.6, 3.3, 1.0);
   glutSolidRhombicDodecahedron();
   glEnd();
   
   glEndList();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
   glViewport(0, 0, (GLsizei)w, (GLsizei)h); 
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0);

   glMatrixMode(GL_MODELVIEW);
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
   glutCreateWindow("box.cpp");
   setup(); 
   glutDisplayFunc(drawScene); 
   glutReshapeFunc(resize);  
   glutKeyboardFunc(keyInput);
   glutTimerFunc(5, animate, 1);
   glutMainLoop(); 

   return 0;  
}
