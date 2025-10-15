#include <GL/glut.h>
#include <stdio.h>

struct Point{
    float x,y;
} w[4],oVer[12]; 

int Nout=0;    
int Nin;
void drawLine(Point p[],int n){
    glBegin(GL_LINE_LOOP);            
    for(int i=0;i<n;i++)
        glVertex2f(p[i].x,p[i].y);    
    glEnd();
}
void drawPoly(Point p[],int n){
    glBegin(GL_POLYGON);            
    for(int i=0;i<n;i++)
        glVertex2f(p[i].x,p[i].y);    
    glEnd();
}
bool insideVer(Point p, int i){    
        if(i==0 && p.x>=w[i].x)
              return true;
        else if (i==1 && p.y<=w[i].y)
              return true;
        else if (i==2 && p.x<=w[i].x)
                return true;
        else if (i==3 && p.y>=w[i].y)
                return true;                
        return false;        
}

void addVer(Point p){        
    oVer[Nout++]=p;       
}

Point getInterSect(Point s,Point p,int edge){ 
    Point in;            
    float m;
    if (p.x==s.x) 
       m=90000.0; /* cuidando a divisao por zero quando o segmento for vertical*/
    else
       m=(p.y-s.y)/(p.x-s.x);  
    if(w[edge].x==w[(edge+1)%4].x){ //Vertical Line        
        in.x=w[edge].x;                
        in.y=(in.x-s.x)*m+s.y;
    }
    else{//Horizontal Line            
        in.y=w[edge].y;
        in.x=((in.y-s.y)/m)+s.x;        
    }    
    return in;
}

void clipAndDraw(Point inVer[]){
    Point s,p,interSec; 
    for(int i=0;i<4;i++)
    {
        printf ("i=%d\n",i);
        Nout=0;
        s=inVer[Nin-1];
        printf ("Vertices\n");
        printf ("====\n");
        for (int k=0;k<Nin;k++)
           printf ("%f %f\n",inVer[k].x,inVer[k].y);           
        printf ("====\n");
        
        for(int j=0;j<Nin;++j){            
            p=inVer[j];
            if(insideVer(p,i)==true){               
                 if(insideVer(s,i)==true){
                    addVer(p);
                }else{
                    interSec=getInterSect(s,p,i);
                    addVer(interSec);                    
                    addVer(p);                    
                }
            }else{        
                if(insideVer(s,i)==true){
                    interSec=getInterSect(s,p,i);                
                    addVer(interSec);                                                    
                }
            }
            s=p;
            
        }       
 
      //inVer=oVer;        
        Nin=Nout;    
       for (int j=0;j<Nin;j++){
            inVer[j].x=oVer[j].x;
            inVer[j].y=oVer[j].y;
       }
    } 
    printf ("Nout=%d\n",Nout); 
    drawPoly(oVer,Nout); 
}

void init(){
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glMatrixMode(GL_PROJECTION);        
    glLoadIdentity();    
    glOrtho(0.0,120.0,0.0,120.0,0.0,100.0);
    glClear(GL_COLOR_BUFFER_BIT);    
    w[0].x =20,w[0].y=10;
    w[1].x =20,w[1].y=80;
    w[2].x =80,w[2].y=80;
    w[3].x =80,w[3].y=10;
}
void display(void){    
    Point inVer[12];
    Nin=5;
    init();
    // As Rect
    glColor3f(0.0f,1.0f,0.0f);
    inVer[0].x =30,inVer[0].y=20;
    inVer[1].x =10,inVer[1].y=60;
    inVer[2].x =60,inVer[2].y=100;
    inVer[3].x =120,inVer[3].y=50;
    inVer[4].x =70,inVer[4].y=82;    
    inVer[5].x =70,inVer[5].y=0;    
    inVer[6].x =50,inVer[6].y=85;
    
//    inVer[3].x =60,inVer[3].y=40;
    drawLine(inVer,Nin); 
    // As Rect
   
        // As Window for Clipping
    glColor3f(1.0f,0.0f,0.0f);        
    drawPoly(w,4); 
 glColor3f(0.0f,0.0f,1.0f);
    clipAndDraw(inVer);        
    // Print
    glFlush();
}

int main(int argc,char *argv[]){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
    glutInitWindowSize(600,600);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Polygon Clipping!");
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}