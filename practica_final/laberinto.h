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

typedef struct str_celda
{/*
  struct str_celda *delante;
  struct str_celda *derecha;
  struct str_celda *izquierda;*/
  struct str_celda *celda_destino;
  struct str_celda *celda_origen;

  int pared[3]; //1: si hay pared, izq, delante, dcha (detras nunca
		 //va a haber porque es de donde vengo)

  int pos[2];
}celda;

int ir_celda_delante(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_izq(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_dch(playerc_client_t *client,playerc_position2d_t *position2d);
int ir_celda_detras(playerc_client_t *client,playerc_position2d_t *position2d);
void inspeccionar_celda(playerc_sonar_t *sonar,celda *celda_actual);
int celda_final(celda *celda_actual);




#endif
