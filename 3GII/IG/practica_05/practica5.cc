#include <ctype.h>
#include <math.h>
#include <vector>
#include <GL/glut.h>
#include "objetos.h"
 #include <stdio.h>
 #include <iostream>

using namespace std;

typedef enum {CAR, GRUA} _object_type;

_object_type Object_type;

_car Car;
_grua Grua;

int estadoRaton[3], xc, yc; // Estados y posición del ratón

int velocidad_estructura=0, velocidad_enganche=0, velocidad_cable=0; // Controles de velocidad

/** variables que determninan la posicion y tamaño de la ventana X */
int Window_x=50,Window_y=50,Window_width=500,Window_high=500;

const float AXIS_SIZE=500; // Longitud de los ejes

/** variables que definen la posicion de la camara en coordenadas polares */
GLfloat Observer_distance;
GLfloat Observer_angle_x;
GLfloat Observer_angle_y;

/** variables que controlan la ventana y la transformacion de perspectiva */
GLfloat Size_x,Size_y,Front_plane,Back_plane;

//**********************************************************************
// modificar la posicion angular de la camara
void setCamara(float longc, float latc)
{
	Observer_angle_y = longc;
	Observer_angle_x = latc ;
}

//**********************************************************************
// obtener la posicion angular de la camara
void getCamara(float *longc, float *latc)
{
	*longc = Observer_angle_y;
	*latc = Observer_angle_x;
}

//**********************************************************************
// realiza acción asociada al elemento seleccionado
void move(GLint hits, GLuint *selectBuf){
int i;

	for (i = 0; i < hits; i++){
		cout << "Seleccionado: " << selectBuf[i * 4 + 3] << endl;

		switch (selectBuf[i * 4 + 3]){
			case 1: Object_type=GRUA; velocidad_estructura-=1; break;	// Base			-> Cambio de velocidad estructura hacia la izquierda
			case 2: Object_type=GRUA; velocidad_estructura+=1; break;	// Estructura	-> Cambio de velocidad estructura hacia la derecha
			case 4: Object_type=GRUA; velocidad_cable-=1; break;		// Torre		-> Cambio de velocidad gancho hacia abajo
			case 6: Object_type=GRUA; velocidad_enganche-=1; break;		// Contrapesa	-> Cambio de velocidad enganche hacia la izquierda
			case 7: Object_type=GRUA; velocidad_enganche+=1; break;		// Brazo-gancho	-> Cambio de velocidad enganche hacia la derecha
			case 10: Object_type=GRUA; velocidad_cable+=1; break;		// Gancho		-> Cambio de velocidad gancho hacia arriba
			} 		
		}	
}

void draw(void);

/**
void pick(int x, int y);

    Función de selección
    
Entrada
  x,y Posición del cursor

**/ 
void pick(int x, int y){
GLuint selectBuf[256];
GLint hits, viewport[4];
 
	// 1. Declarar buffer de selección
	glSelectBuffer(256,selectBuf);
 
	// 2. Obtener los parámetros del viewport
	glGetIntegerv(GL_VIEWPORT, viewport);
 
	// 3. Pasar OpenGL a modo selección
	glRenderMode(GL_SELECT);

	// Inicializamos la pila de nombres
	glInitNames();
	glPushName(0);
 
	// 4. Fijar la transformación de proyección para la seleccion
	glMatrixMode(GL_PROJECTION);				
	glLoadIdentity();
	gluPickMatrix (x,(viewport[3] - y),1.0, 1.0, viewport);
	glFrustum (-0.030,0.030,-0.030,0.030,0.1,200);

	// 5. Dibujar la escena
	draw();	
	glLoadIdentity();
 
	// 6. Pasar OpenGL a modo render
	hits = glRenderMode(GL_RENDER);	
 
	// 7. Restablecer la transformación de proyección (sin gluPickMatrix)
	glMatrixMode(GL_PROJECTION);	
 	glLoadIdentity();
	glFrustum (-0.030,0.030,-0.030,0.030,0.1,200);

	// Ejecutar el movimiento correspondiente a la figura seleccionada
 	move(hits, selectBuf);
 }

/**
 
void clickRaton( int boton, int estado, int x, int y );

Procedimiento para gestionar los eventos de pulsacion de los botones del raton.

Argumentos:

boton: Codigo glut del boton que ha manipulado el usuario.

estado: Estado en el que esta el boton que se ha pulsado.

x,y: Posicion, en coordenadas de pantalla, en que se encuantra el cursor.

**/
void clickRaton( int boton, int estado, int x, int y )
{
	if(boton== GLUT_RIGHT_BUTTON) {
		if(estado == GLUT_DOWN) {
			estadoRaton[2] = 1;
			xc=x;
			yc=y;
		} else{
			estadoRaton[2] = 0;
		}
	}
	
	if 	((boton == GLUT_LEFT_BUTTON) && (estado == GLUT_DOWN)){
		pick(x,y);
	}
}

/**

void RatonMovido( int x, int y );

Procedimiento para gestionar los eventos de movimiento del raton.

Argumentos:

x,y: Posicion, en coordenadas de pantalla, en que se encuantra el cursor.

**/

void RatonMovido( int x, int y )
{
float yn,xn,y0,x0;	

	if(estadoRaton[2]==1){
		getCamara(&x0,&y0);
		yn=y0+(y-yc);
		xn=x0-(x-xc);
		setCamara(xn,yn);
		xc=x;
		yc=y;
		glutPostRedisplay();
	}
}

/** @brief Dibujo de ejes
  * 
  * Dibuja los semiejes positivos del sistema de coordenadas. 
  * X de color rojo
  * y de color verde
  * z de color azul
  * 
  * Deja la iluminación desactivada
  */
void draw_axis()
{
	
glDisable(GL_LIGHTING);

glBegin(GL_LINES);
// eje X, color rojo
glColor3f(1,0,0);
glVertex3f(0,0,0);
glVertex3f(AXIS_SIZE,0,0);
// eje Y, color verde
glColor3f(0,1,0);
glVertex3f(0,0,0);
glVertex3f(0,AXIS_SIZE,0);
// eje Z, color azul
glColor3f(0,0,1);
glVertex3f(0,0,0);
glVertex3f(0,0,AXIS_SIZE);
glEnd();
}

/** @brief Funcion para definir la transformación de proyeccion
 *
 * Fija la transformación de proyección. 
 * La versión inicial usa proyección paralela con glOrtho
 * 
 * Deja el contexto de OpenGL aceptando atrices de proyección
 * 
 * En la primera práctica se debe añadir además proyección perspectiva
*/

void change_projection()
{

glMatrixMode(GL_PROJECTION);
glLoadIdentity();

// formato(x_minimo,x_maximo, y_minimo, y_maximo,plano_delantero, plano_trasero)
//  plano_delantero>0  plano_trasero>PlanoDelantero)
//glFrustum(-Size_x,Size_x,-Size_y,Size_y,Front_plane,Back_plane);
glFrustum (-0.030,0.030,-0.030,0.030,0.1,200);
}


/** @brief Funcion para definir la transformación de vista (posicionar la camara)
 *
 * Fija la transformación de vista. 
 * La versión inicial la especifica como dos rotaciones y una traslación.
 * 
 * Deja el contexto de OpenGL aceptando atrices de modelado
 * 
 * Se podría dar igualmente con gluLookAt
*/
void change_observer()
{

// posicion del observador
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
glTranslatef(0,-5,-Observer_distance);
glRotatef(Observer_angle_x,1,0,0);
glRotatef(Observer_angle_y,0,1,0);
}

/** @brief Funcion para modificar el tamaño del viewport
 *
 * Fija el tamaño del viewport de dibujo
 * 
 * Esta función es llamada por glut cuando el usuario modifica el tamaño de la ventana X.
 * 
 * Los parámetros son el nuevo tamaño del viewport.
*/
void change_window_size(int Ancho1,int Alto1)
{
float Aspect_ratio;

Aspect_ratio=(float) Alto1/(float )Ancho1;
Size_y=Size_x*Aspect_ratio;
change_projection();
glViewport(0,0,Ancho1,Alto1);
glutPostRedisplay();
}

/** @brief Funcion para dibujar los objetos
 *
 * En esta función incluiremos el código para dibujar nuestros objetos
 * 
*/
void draw_objects()
{
glColor3f(0,1,0);
glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

// Codigo para dibujar objetos
switch (Object_type){
	case CAR:Car.draw();break;
	case GRUA:Grua.draw();break;
  }
}

/** @brief Funcion para borrar el frame buffer
 * 
*/
void clean_window()
{
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


/** @brief Funcion para dibujar la escena
 *
 * Borra el frame buffer, fija la transformación de vista, dibuja los ejes y los objetos.
 * 
*/
void draw(void)
{
clean_window();
change_observer();
draw_axis();
draw_objects();
glutSwapBuffers();
}


/** @brief Funcion llamada cuando se aprieta una tecla normal
 *
 * el evento manda a la funcion:
 * 	codigo de la tecla
 * 	posicion x del raton
 *	posicion y del raton
 * 
 * 	Inicialmente solo interpreta Q y escape, ambos termina el programa.
*/
void normal_key(unsigned char Tecla1,int x,int y)
{

switch (toupper(Tecla1)){
	case 'Q':exit(0); break;
	case 27: exit(0); break;

	case 'B': Car.ruedas_left(); break;
	case 'N': Car.ruedas_right(); break;
	
	case 'Z': Object_type=GRUA; velocidad_estructura=0; break;
	case 'X': Object_type=GRUA; velocidad_enganche=0; break;
	case 'C': Object_type=GRUA; velocidad_cable=0; break;
	
	case 'A': Object_type=GRUA; velocidad_estructura-=1; break;
	case 'S': Object_type=GRUA; velocidad_estructura+=1; break;
	case 'D': Object_type=GRUA; velocidad_enganche+=1; break;
	case 'F': Object_type=GRUA; velocidad_enganche-=1; break;
	case 'G': Object_type=GRUA; velocidad_cable+=1; break;
	case 'H': Object_type=GRUA; velocidad_cable-=1; break;
	
	default: glutPostRedisplay();
	}
glutPostRedisplay();
}





/** @brief Funcion llamada cuando se aprieta una tecla especial
 *
 * el evento manda a la funcion:
 * 	codigo de la tecla
 * 	posicion x del raton
 *	posicion y del raton
 * 
 * 	Inicialmente solo interpreta las teclas de movimiento del cursor y avance y retroceso de página. 
 * 	Con ellas se controla la cámara
 * 
 * 	Las teclas de funciones están definidas sin asignar funcionalidad. Se pueden usar para cargar los objetos.
*/
void special_key(int Tecla1,int x,int y)
{
switch (Tecla1){
	case GLUT_KEY_LEFT:Observer_angle_y--;break;
	case GLUT_KEY_RIGHT:Observer_angle_y++;break;
	case GLUT_KEY_UP:Observer_angle_x--;break;
	case GLUT_KEY_DOWN:Observer_angle_x++;break;
	case GLUT_KEY_PAGE_UP:Observer_distance*=1.2;break;
	case GLUT_KEY_PAGE_DOWN:Observer_distance/=1.2;break;
	// primitivas graficas
	case GLUT_KEY_F1:Object_type=CAR; Car.ruedas_stop(); break;
	case GLUT_KEY_F2:Object_type=GRUA; velocidad_estructura=0; velocidad_enganche=0; velocidad_cable=0; break;
	case GLUT_KEY_F3: break;
	case GLUT_KEY_F4: break;
	case GLUT_KEY_F5: break;
	case GLUT_KEY_F6: break;
	case GLUT_KEY_F7: break;
	}
glutPostRedisplay();
}

/** @brief Funcion de inicialización
 *
 * Asigna valores iniciales a los parámetros de vista 
 * 
*/
void initialize(void)
{
// se inicalizan la ventana y los planos de corte
Size_x=1;
Size_y=1;
Front_plane=2;
Back_plane=1000;

// se incia la posicion del observador, en el eje z
Observer_distance=30;
Observer_angle_x=40;
Observer_angle_y=30;

// se indica cual sera el color para limpiar la ventana	(r,v,a,al)
// blanco=(1,1,1,1) rojo=(1,0,0,1), ...
glClearColor(0,0,0,1);

// se habilita el z-bufer
glEnable(GL_DEPTH_TEST);
//
change_projection();
//
glViewport(0,0,Window_width,Window_high);
//
Object_type=GRUA;
}

//***************************************************************************
// Funcion IDLE
// Procedimiento de fondo. Es llamado por glut cuando no hay eventos pendientes.
//***************************************************************************

void idle()
{
  if (Object_type==CAR)
      Car.control_animacion();
  else if (Object_type==GRUA)
      Grua.control_animacion(velocidad_estructura, velocidad_enganche, velocidad_cable);

  glutPostRedisplay();

}


//***************************************************************************
// Programa principal
//
// Se encarga de iniciar la ventana, asignar las funciones e comenzar el
// bucle de eventos
//***************************************************************************

int main(int argc, char **argv)
{

// se llama a la inicialización de glut
glutInit(&argc, argv);

// se indica las caracteristicas que se desean para la visualización con OpenGL
// Las posibilidades son:
// GLUT_SIMPLE -> memoria de imagen simple
// GLUT_DOUBLE -> memoria de imagen doble
// GLUT_INDEX -> memoria de imagen con color indizado
// GLUT_RGB -> memoria de imagen con componentes rojo, verde y azul para cada pixel
// GLUT_RGBA -> memoria de imagen con componentes rojo, verde, azul y alfa para cada pixel
// GLUT_DEPTH -> memoria de profundidad o z-bufer
// GLUT_STENCIL -> memoria de estarcido
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

// posicion de la esquina inferior izquierdad de la ventana
glutInitWindowPosition(Window_x,Window_y);

// tamaño de la ventana (ancho y alto)
glutInitWindowSize(Window_width,Window_high);

// llamada para crear la ventana, indicando el titulo (no se visualiza hasta que se llama
// al bucle de eventos)
glutCreateWindow("IG: Practica 5");

// asignación de la funcion llamada "dibujar" al evento de dibujo
glutDisplayFunc(draw);
// asignación de la funcion llamada "change_window_size" al evento correspondiente
glutReshapeFunc(change_window_size);
// asignación de la funcion llamada "normal_key" al evento correspondiente
glutKeyboardFunc(normal_key);
// asignación de la funcion llamada "tecla_Especial" al evento correspondiente
glutSpecialFunc(special_key);

// asignación de la funcion "idle" para ejecución de fondo
glutIdleFunc( idle);
   
// funcion de inicialización
initialize();

// funciones que procesan los callback de ratón
glutMouseFunc(clickRaton);
glutMotionFunc(RatonMovido);

// inicio del bucle de eventos
glutMainLoop();
return 0;
}


