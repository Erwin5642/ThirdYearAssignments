//Bruna Coimbra Perboni 8102   data 05/09/05
//=============================================================================
// Bibliotecas
//=============================================================================
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
//=============================================================================
// Defini??o de tipos
//=============================================================================
struct Ponto{
	float x, y;
};
//=============================================================================
// Vari?veis Globais
//=============================================================================
Ponto pt[35];
Ponto aux, aux1;
float t,q,x,y,xmin,xmax,ymin,ymax;
int p, n = 0;
bool curva = false;
static int eixoz, eixoy, eixox;

void resize(int w, int h){
   glViewport(0, 0, (GLsizei)w, (GLsizei)h); 
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-w/2, w/2, -h/2, h/2, -200, 200);
   glMatrixMode(GL_MODELVIEW);
   xmin=-w/2;xmax=w/2; ymin=-h/2; ymax=h/2;
}
//=============================================================================
// Desenha Plano Carteziano
//=============================================================================
void plota_eixo( void ){
	//glLineWidth(5);
	glBegin(GL_LINES);
 		glColor3f(0.4,0.4,0.4);
		glVertex2f(xmin,0);
		glVertex2f(xmax,0);
		glVertex2f(0,ymin);
		glVertex2f(0,ymax);
	glEnd();
}
//=============================================================================
// Desenha Pontos de controle
//=============================================================================
void pontos_controle( void ){
	for(int i = 0; i < n - 1; i++ ){
      glBegin(GL_LINES);
         glColor3f(9.0,0.0,9.0);
         glVertex2f(pt[i].x,pt[i].y);
         glVertex2f(pt[i + 1].x,pt[i + 1].y);
      glEnd();
      glPointSize(7);
      glBegin(GL_POINTS);
         glColor3f(1.0,0.0,0.0);
         glVertex2f(pt[i].x,pt[i].y);
         glVertex2f(pt[i + 1].x,pt[i + 1].y);
      glEnd();
   }
   glBegin(GL_POINTS);
         glColor3f(9.0,9.0,0.0);
         glVertex2f(pt[0].x,pt[0].y);
   glEnd();
   glBegin(GL_POINTS);
         glColor3f(0.0,9.0,0.0);
         glVertex2f(pt[p].x,pt[p].y);
   glEnd();
}
//=============================================================================
// Altera Pontos
//=============================================================================
void lePontos( int x, int y ){
     int ax, ay;
     if(n < 35){
          ax = x - xmax;
          ay = ymax - y;
          pt[n].x = ax;
          pt[n++].y = ay;
     }else{
          printf("\n Ja era, acabou.");
      }
}
//=============================================================================
// Mouse
//=============================================================================
void mudaPonto (int x, int y){
	int ax, ay;

   ax = x - xmax;
   ay = ymax - y;
   for(int i = 0; i < n; i++){
   	if( (ax < (pt[i].x + 10)) && ( ax > (pt[i].x - 10)) ){
			if( (ay < (pt[i].y + 10)) && ( ay > (pt[i].y - 10)) )
         	p = i;
      }
   }
   pt[p].x = ax;
   pt[p].y = ay;
}
//=============================================================================
// Desenha um Segmento Calculado
//=============================================================================
void desenha_ponto(void){
   float i = -3.1415, xi, zi, xf, zf;
   if(curva){
      while(i < 3.1415){
         xi = x * cos(i);
         zi = x * sin(i);
         i += 0.3;
         xf = x * cos(i);
         zf = x * sin(i);
         glBegin(GL_LINES);
            //desenha segmento
            glColor3f(1.0,1.0,1.0);
            glVertex3f(xi, y, zi);
            glVertex3f(xf, y, zf);
         glEnd();
      }
   }
   else{
      	if(t > 0.05){
      	glBegin(GL_LINES);
            glColor3f(1.0,1.0,1.0);
            glVertex2f(x,y);
            glVertex2f(aux1.x,aux1.y);
         glEnd();
      }
      aux1.x = x;
      aux1.y = y;
   }
}
//=============================================================================
// Fun??o B1
//=============================================================================
float funcao( void ){
	float fat = 1, m = 1, div = 1, aux = 1;
	if(q != 0){
   	for(int i = 1; i < n; i ++)
      	fat = fat * i;
      aux = n - q;
      for(int i = 1; i < aux; i++)
      	m = m * i;
      for(int i = 1; i <= q; i++)
      	div = div * i;

      fat = fat / (m * div);
      aux = 1;
      for(int i = 1; i <= q; i ++)
      	aux = aux * t;
      fat = fat * aux;
      aux = 1;
      for(int i = 1; i < (n - q); i++)
      	aux = aux * (1 - t);
      fat = fat * aux;
   }
   else{
   	for(int i = 1; i < n; i++)
      	fat *= (1 - t);
   }
   return fat;
}
//=============================================================================
// Calcula pontos da curva
//=============================================================================
void Bezier( void ){
   int i;
	t = 0;
   i = aux.x = aux.y = 0;
   for( q = 0; q < n; q++){
      aux.x += (funcao() *  pt[i].x);
      aux.y += (funcao() *  pt[i].y);
      i++;
   }
   x = aux.x;
   y = aux.y;
   desenha_ponto();
   while( t < 1){
      i = x = y = 0;
      t += 0.05;
      for( q = 0; q < n; q++){
      	x += (funcao() *  pt[i].x);
         y += (funcao() *  pt[i].y);
         i++;
      }
      desenha_ponto();
      
   }
}
//=============================================================================
// Mensagem de Ajuda
//=============================================================================
void mensagem ( void ){
//	clrscr();
   printf("\n\n Posicao dos Pontos de Controle: \n");

   for(int i = 0; i < n; i++)
   	printf("\n Ponto[%d] = {%0.1f, %0.1f}", i, pt[i].x, pt[i].y );

   printf("\n\n Ponto Ativado = %d", p);
   printf("\n\nClique com o botao esquerdo do mouse para criar pontos de controle");
   printf("\nClique com o botao direito do mouse para muda-los de lugar.");
   printf("\n\nMas antes: tecla 'o'");
   printf("\n\nPara Alternar entre Curva e Solido;");
   printf("\n  Tecla 's'");
   printf("\n\nPara Tirar Pontos:");
   printf("\n  Tecla 'a'");
}
//=============================================================================
// Mouse
//=============================================================================
void le_botao_mouse(int b, int state, int x, int y){
	switch(b){
      case GLUT_RIGHT_BUTTON:
         switch(state){
            case GLUT_DOWN:
               mudaPonto(x, y);
                glutPostRedisplay();
               mensagem();
            break;
         }
      break;
      case GLUT_LEFT_BUTTON:
         switch(state){
            case GLUT_DOWN:
            	lePontos(x, y);
                glutPostRedisplay();
            break;
         }
      break;
   }
}
//=============================================================================
// Display
//=============================================================================
void display( void ){
	glPushMatrix();
  	glRotatef ((GLfloat) eixoy, 0.0, 1.0, 0.0);
  	glRotatef ((GLfloat) eixox, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    plota_eixo();
    pontos_controle();
	Bezier();
	glPopMatrix();
  	glutSwapBuffers();
}


//=============================================================================
// Teclado
//=============================================================================
void le_tecla(unsigned char key, int x, int y){
	switch(key){
   	case 27:
    		exit(0);
    	break;

      case 'x':
    		eixox = (eixox + 5) % 360;
    		glutPostRedisplay();
    	break;

      // tecla X: rotaciona o cubo em torno do eixo X (sentido hor?rio)
  		case 'c':
    		eixox = (eixox - 5) % 360;
    		glutPostRedisplay();
    	break;

      // tecla y: rotaciona o cubo em torno do eixo Y (sentido anti-hor?rio)
  		case 'y':
    		eixoy = (eixoy + 5) % 360;
    		glutPostRedisplay();
   	break;

      // tecla Y: rotaciona o cubo em torno do eixo y (sentido hor?rio)
  		case 'u':
    		eixoy = (eixoy - 5) % 360;
    		glutPostRedisplay();
    	break;
      case'o':
      	eixox = eixoy = eixoz = 0;
         glutPostRedisplay();
      break;
      case 'a':
      	p = 0;
      	n--;
         display();
      break;
      case 's':
      	if(curva)
      		curva = false;
         else
         	curva = true;
         display();
      break;
	}
}
//=============================================================================
// Programa principal
//=============================================================================
int main(int argc, char **argv){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(700,700);
	glutInitWindowPosition(1,1);
	glutCreateWindow("Curvas de B?zier");
    glutReshapeFunc(resize);  
   glClearColor(0.0,0.0,0.0,0.0);
	glutDisplayFunc(display);
   glutKeyboardFunc(le_tecla);
   glutMouseFunc(le_botao_mouse);

   printf("\n     Para Criar Pontos:");
   printf("\n        Botao Esquerdo do Mouse");

   printf("\n     Para Rotacionar:");
   printf("\n        Teclas: 'x', 'c', 'y', 'u'");

   printf("\n     Para Alterar Posicao dos Pontos:");
   printf("\n        Tecla 'o' e Botao Direito do Mouse");

   printf("\n     Para Alternar entre Curva e Solido;");
   printf("\n        Tecla 's'");

   printf("\n     Para Tirar Pontos:");
   printf("\n        Tecla 'a'");

   glutMainLoop();
}

//=============================================================================
// Fim Programa
//=============================================================================