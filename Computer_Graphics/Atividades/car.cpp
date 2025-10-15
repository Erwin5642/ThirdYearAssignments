/////////////////////////////////
// box.cpp
//
// This program draws a wire box.
//
// Sumanta Guha.
/////////////////////////////////

#include <iostream>
#include <cmath>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

using namespace std;

static unsigned int rodas;
static unsigned int parafuso;

static int animationPeriod = 1;
static double angle = 0;
static double carX = 0, carZ = 3, carAngle = 0;
static double carDx = 1, carDz = 1;
static float Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate scene.

bool isJumping = false;
float jumpTime = 0.0f;
float carY = 0.0f;
float jumpRotation = 0.0f;

const double stepSize = 1.0;

float width = 50.0f;
float height = 10.0f;
float depth = 24.0f;

void drawParallelepiped()
{
    float x = width / 2.0f;
    float y = height / 2.0f;
    float z = depth / 2.0f;

    glBegin(GL_QUADS);
    glColor3f(0.0, 0.0, 1.0);

    glVertex3f(-x, -y, z);
    glVertex3f(x, -y, z);
    glVertex3f(x, y, z);
    glVertex3f(-x, y, z);

    glVertex3f(-x, -y, -z);
    glVertex3f(-x, y, -z);
    glVertex3f(x, y, -z);
    glVertex3f(x, -y, -z);

    glVertex3f(-x, -y, -z);
    glVertex3f(-x, -y, z);
    glVertex3f(-x, y, z);
    glVertex3f(-x, y, -z);

    glVertex3f(x, -y, -z);
    glVertex3f(x, y, -z);
    glVertex3f(x, y, z);
    glVertex3f(x, -y, z);

    glVertex3f(-x, y, -z);
    glVertex3f(-x, y, z);
    glVertex3f(x, y, z);
    glVertex3f(x, y, -z);

    glVertex3f(-x, -y, -z);
    glVertex3f(x, -y, -z);
    glVertex3f(x, -y, z);
    glVertex3f(-x, -y, z);

    glEnd();
}

void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glLoadIdentity();

    glTranslatef(0, 0, -100); // camera

    glRotatef(Zangle, 0.0, 0.0, 1.0);
    glRotatef(Yangle, 0.0, 1.0, 0.0);
    glRotatef(Xangle, 1.0, 0.0, 0.0);

    glPushMatrix();

    glTranslatef(carX, carY, carZ);
    glRotatef(carAngle, 0, 1, 0);
    glRotatef(jumpRotation, 0, 0, 1);

    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    glTranslatef(-15, 0, 15);
    glRotatef(angle, 0, 0, 1);
    glCallList(rodas);
    glPopMatrix();

    glPushMatrix();
    glRotatef(180, 1, 0, 0);
    glTranslatef(15, 0, 15);
    glRotatef(angle, 0, 0, 1);
    glCallList(rodas);
    glPopMatrix();

    //Corpo do carro
    glPushMatrix();
    glTranslatef(0, height / 2 - 3, 0);
    
    //Parte de cima
    glPushMatrix();
    glTranslatef(0, height, 0);
    glColor3f(1.0, 0.0, 0.0);
    glutSolidCube(18);
    glPopMatrix();
    
    //Farois traseiros
    glPushMatrix();
    glColor3f(0.5, 0, 0);
    glTranslatef(-width/2, 0, 2*depth/5);
    glutSolidCube(2);
    glPopMatrix();
    glPushMatrix();
    glColor3f(0.5, 0, 0);
    glTranslatef(-width/2, 0, -2*depth/5);
    glutSolidCube(2);
    glPopMatrix();
    
    //Farois frontais
    glPushMatrix();
    glColor3f(0.5, 0.5, 0);
    glTranslatef(width/2, 0, 2*depth/5);
    glutSolidSphere(2, 30, 30);
    glPopMatrix();
    glPushMatrix();
    glColor3f(0.5, 0.5, 0);
    glTranslatef(width/2, 0, -2*depth/5);
    glutSolidSphere(2, 30, 30);
    glPopMatrix();

    //Chasi
    drawParallelepiped();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(15, 0, 15);
    glRotatef(-angle, 0, 0, 1);
    glCallList(rodas);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-15, 0, 15);
    glRotatef(-angle, 0, 0, 1);
    glCallList(rodas);
    glPopMatrix();

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    glutSwapBuffers();
}

// Initialization routine.
void setup(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0);

    rodas = glGenLists(1);    // Return a list index.
    parafuso = glGenLists(1); // Return a list index.

    glNewList(parafuso, GL_COMPILE);
    glColor3f(0.5, 0.5, 0.5);
    glutSolidCylinder(0.25, 0.25, 10, 10);
    glTranslatef(0, 0.5, 0);
    glutSolidCylinder(0.5, 0.25, 10, 10);
    glEnd();
    glEndList();

    glNewList(rodas, GL_COMPILE);
    glColor3f(0.0, 0.0, 0.0);
    glutSolidTorus(3, 5, 50, 50);
    glColor3f(0.3, 0.3, 0.3);
    glutSolidCylinder(5, 3, 10, 10);
    glPushMatrix();
    glRotatef(288, 0, 0, 1);
    glTranslatef(0, 3, 3);
    glCallList(parafuso);
    glPopMatrix();
    glPushMatrix();
    glRotatef(216, 0, 0, 1);
    glTranslatef(0, 3, 3);
    glCallList(parafuso);
    glPopMatrix();
    glPushMatrix();
    glRotatef(144, 0, 0, 1);
    glTranslatef(0, 3, 3);
    glCallList(parafuso);
    glPopMatrix();
    glPushMatrix();
    glRotatef(72, 0, 0, 1);
    glTranslatef(0, 3, 3);
    glCallList(parafuso);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0, 3, 3);
    glCallList(parafuso);
    glPopMatrix();
    glEnd();
    glEndList();
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 500.0);

    glMatrixMode(GL_MODELVIEW);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case 'w':
        carX += stepSize * sin((carAngle + 90) * M_PI / 180.0);
        carZ += stepSize * cos((carAngle + 90) * M_PI / 180.0);
        angle += 15;
        break;
    case 's':
        carX -= stepSize * sin((carAngle + 90) * M_PI / 180.0);
        carZ -= stepSize * cos((carAngle + 90) * M_PI / 180.0);
        angle -= 15;
        break;
    case 'a':
        carAngle += 5;
        break;
    case 'd':
        carAngle -= 5;
        break;
    case ' ':
        if (!isJumping)
        {
            isJumping = true;
            jumpTime = 0.0f;
        }
        break;
    case 'x':
         Xangle += 5.0;
		 if (Xangle > 360.0) Xangle -= 360.0;
         break;
      case 'X':
         Xangle -= 5.0;
		 if (Xangle < 0.0) Xangle += 360.0;
         break;
      case 'y':
         Yangle += 5.0;
		 if (Yangle > 360.0) Yangle -= 360.0;
         break;
      case 'Y':
         Yangle -= 5.0;
		 if (Yangle < 0.0) Yangle += 360.0;
         break;
      case 'z':
         Zangle += 5.0;
		 if (Zangle > 360.0) Zangle -= 360.0;
         break;
      case 'Z':
         Zangle -= 5.0;
		 if (Zangle < 0.0) Zangle += 360.0;
         break;
        case 'f':
            Zangle = 0;
            Xangle = 0;
            Yangle = 0;
            break;
      default:
         break;
    }
    
    glutPostRedisplay();
}

void update(int value)
{
    if (isJumping)
    {
        jumpTime += 0.025;

        if (jumpTime <= 1.0)
        {
            carY = 200.0 * jumpTime * (1.0 - jumpTime); 
            jumpRotation = 360.0 * jumpTime;            
        }
        else
        {
            isJumping = false;
            jumpTime = 0.0;
            carY = 0.0;
            jumpRotation = 0.0;
        }
    }

    glutTimerFunc(16, update, 0);
    glutPostRedisplay();
}


// Main routine.
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("car.cpp");
    setup();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}
