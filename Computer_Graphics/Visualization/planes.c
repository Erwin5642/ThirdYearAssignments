/*
   *  planes.c
 *  This program demonstrates the use of glDepthFunc.
 *  It is adpted from plane.c by Ting.
 */
#include <GL/glut.h>

void myinit(void)
{
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
}

void drawPlane(void)
{
    glBegin (GL_QUADS);
    glNormal3f (0.0, 0.0, 1.0);
    glVertex3f (-1.0, -1.0, 0.0);
    glVertex3f (0.0, -1.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);
    glVertex3f (-1.0, 0.0, 0.0);

    glNormal3f (0.0, 0.0, 1.0);
    glVertex3f (0.0, -1.0, 0.0);
    glVertex3f (1.0, -1.0, 0.0);
    glVertex3f (1.0, 0.0, 0.0);
    glVertex3f (0.0, 0.0, 0.0);

    glNormal3f (0.0, 0.0, 1.0);
    glVertex3f (0.0, 0.0, 0.0);
    glVertex3f (1.0, 0.0, 0.0);
    glVertex3f (1.0, 1.0, 0.0);
    glVertex3f (0.0, 1.0, 0.0);

    glNormal3f (0.0, 0.0, 1.0);
    glVertex3f (0.0, 0.0, 0.0);
    glVertex3f (0.0, 1.0, 0.0);
    glVertex3f (-1.0, 1.0, 0.0);
    glVertex3f (-1.0, 0.0, 0.0);
    glEnd();
}

void display (void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix ();
    glTranslatef (0.75, 1.0, -9.9);
    glColor3f (0.0, 0.0, 1.0);
    drawPlane ();
    glPopMatrix ();

    glPushMatrix ();
    glTranslatef (0.0, 0.0, 0.5);
    glColor3f (0.0, 1.0, 0.0);
    drawPlane ();
    glPopMatrix ();

    glPushMatrix ();
    glTranslatef (1.5, 0.0, 0.0);
    glColor3f (1.0, 0.0, 0.0);
    drawPlane ();
    glPopMatrix ();

    glFlush ();
}

void myReshape(int w, int h)
{
    glViewport (0, 0, w, h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    if (w <= h)
	glOrtho (-1.5, 3.0, -1.5*(GLdouble)h/(GLdouble)w,
	    3.0*(GLdouble)h/(GLdouble)w, -10.0, 10.0);
    else
	glOrtho (-1.5*(GLdouble)w/(GLdouble)h,
	    1.5*(GLdouble)w/(GLdouble)h, -1.5, 1.5, -10.0, 10.0);
    glMatrixMode (GL_MODELVIEW);
}

/*  Main Loop
 *  Open window with initial window size, title bar,
 *  RGBA display mode, and handle input events.
 */
int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(argv[0]);
  glutReshapeFunc(myReshape);
  glutDisplayFunc(display);
  myinit();
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}