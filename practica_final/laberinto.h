/* laberinto.h --- Declaracion de estructuras y funciones de la practica final  
**
** This file was part of Robotica Movil proyect.
**
** Copyright (c): 2011 Department of Electronics, University of Alcala
** Author: Miguel Martinez y Jose Velasco
** 
** Created: Wed May 25 2011, 
*/

#ifndef _LABERINTO_H
#define _LABERINTO_H

#define D_DCH   	0
#define D_ARRIBA	1
#define D_IZQ   	2
#define D_ABAJO 	3

#define DIR2DEG(dir)	(((dir)-1) * 90)	// convertir a ángulo (arriba 0, izquierda 90, derecha -90, atrás 180)
#define DEG2DIR(ang)	(((ang)/90 + 5) % 4)	// convertir a valores de 0 a 3

#define NO_MURO 	0
#define HAY_MURO	1
#define RUTA      	2
#define DEAD_END      	3

const char *dirs[4];

typedef struct str_celda {
	//struct str_celda *celda_destino;
	//struct str_celda *celda_origen;

	int pared[3];

	int pos[2];
	int orientacion;
} celda;

extern int x, y, xmax, xmin, ymax, ymin, theta;
int girar_izq(playerc_client_t *client,playerc_position2d_t *position2d);
int girar_dch(playerc_client_t *client,playerc_position2d_t *position2d);
int girar_180(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_delante(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_izq(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_dch(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_detras(playerc_client_t *client,playerc_position2d_t *position2d);
void inspeccionar_celda(playerc_sonar_t *sonar,celda *celda_actual);
int celda_final(celda *celda_actual);

int ir_direccion(playerc_client_t * client, playerc_position2d_t * position2d, int dir);


#endif
