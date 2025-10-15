// BolaRolandoEmPlano.cpp

#include <iostream>
#include <cmath>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace std;

// Globals.
static float t = 0.0;                                               // Animation parameter.
static float angle = 0.0, Xangle = 0.0, Yangle = 0.0, Zangle = 0.0; // Angles to rotate scene.
static int isAnimate = 0;                                           // Animated?
static int animationPeriod = 100;                                   // Time interval between frames.
static int wallUp = -10, wallDown = 10, wallLeft = -10, wallRight = 10;
static double ballX = 0, ballZ = 0, ballDx = 1, ballDz = -1, ballVx = 0.3, ballVz = 0.5, ballRadius = 1.0;
static float ballRotation = 0.0; // Angle of the ball's rotation based on its movement

// Drawing routine.
void drawScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glColor3f(0.0, 0.0, 0.0);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -15.0);

    // Rotate scene.
    glRotatef(Zangle, 0.0, 0.0, 1.0);
    glRotatef(Yangle, 0.0, 1.0, 0.0);
    glRotatef(Xangle, 1.0, 0.0, 0.0);

    // Draw walls (same as before)
    glPushMatrix();
    glTranslatef(0, 0, wallDown);
    glScalef(1.0, 0.25, 0.01);
    glutWireCube(20.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, wallUp);
    glScalef(1.0, 0.25, 0.01);
    glutWireCube(20.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(wallRight, 0, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(1.0, 0.25, 0.01);
    glutWireCube(20.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(wallLeft, 0, 0);
    glRotatef(90, 0, 1, 0);
    glScalef(1.0, 0.25, 0.01);
    glutWireCube(20.0);
    glPopMatrix();

    // Translate and rotate the ball based on its movement
    glTranslatef(ballX, 0, ballZ);

    // Update the rotation of the ball
    glRotatef(ballRotation, -ballDx, 0.0, ballDz);
    
    glutWireCube(ballRadius);

    glDisable(GL_DEPTH_TEST);
    glutSwapBuffers();
}

// Initialization routine.
void setup(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

void animate(int value)
{
    if (isAnimate)
    {
        if(ballVx < 0 || ballVz < 0){
            isAnimate = 0;
        }
        
        // Update ball position
        ballX += ballDx * ballVx;
        ballZ += ballDz * ballVz;

        // Update ball's rotation angle based on the distance moved
        ballRotation += sqrt(ballDx * ballDx + ballDz * ballDz) * 10; // Proportional to the ball's movement

        // Check for collisions with the walls
        if (ballX + ballRadius > wallRight)
        {
            ballDx *= -1;
        }
        if (ballX - ballRadius < wallLeft)
        {
            ballDx *= -1;
        }
        if (ballZ - ballRadius < wallUp)
        {
            ballDz *= -1;
        }
        if (ballZ + ballRadius > wallDown)
        {
            ballDz *= -1;
        }
    }
    glutTimerFunc(animationPeriod, animate, 1);
    glutPostRedisplay();
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
    switch (key)
    {
    case 27:
        exit(0);
        break;
    case ' ':
        if (isAnimate)
            isAnimate = 0;
        else
            isAnimate = 1;
        glutPostRedisplay();
        break;
    case 127:
        if (isAnimate)
            isAnimate = 0;
        t = 0.0;
        glutPostRedisplay();
        break;
    case 'x':
        Xangle += 5.0;
        if (Xangle > 360.0)
            Xangle -= 360.0;
        glutPostRedisplay();
        break;
    case 'X':
        Xangle -= 5.0;
        if (Xangle < 0.0)
            Xangle += 360.0;
        glutPostRedisplay();
        break;
    case 'y':
        Yangle += 5.0;
        if (Yangle > 360.0)
            Yangle -= 360.0;
        glutPostRedisplay();
        break;
    case 'Y':
        Yangle -= 5.0;
        if (Yangle < 0.0)
            Yangle += 360.0;
        glutPostRedisplay();
        break;
    case 'z':
        Zangle += 5.0;
        if (Zangle > 360.0)
            Zangle -= 360.0;
        glutPostRedisplay();
        break;
    case 'Z':
        Zangle -= 5.0;
        if (Zangle < 0.0)
            Zangle += 360.0;
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
    cout << "Interaction:" << endl;
    cout << "Press space to toggle between animation on and off." << endl
         << "Press delete to reset." << endl
         << "Press the x, X, y, Y, z, Z keys to rotate the scene." << endl;
}

// Main routine.
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("bola.cpp");
    setup();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);
    glutTimerFunc(5, animate, 1);
    glutMainLoop();

    return 0;
}
