/* functions.c --- Definicion funciones practica final 
**
** This file was part of Robotica Movil proyect.
**
** Copyright (c): 2011 Department of Electronics, University of Alcala
** Author: Miguel Martinez & Jose Velasco
** 
** Created: Wed May 25 2011
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libplayerc/playerc.h>
#include "laberinto.h"

const char *dirs[4] = { "derecha", "delante", "izquierda", "FIN" };
int x=0, y=0, xmax=0, xmin=0, ymax=0, ymin=0, theta=D_ARRIBA;

int girar_izq(playerc_client_t * client, playerc_position2d_t * position2d)
{
	double target_a;

	target_a = fmod((round(position2d->pa / M_PI_2) + 1), 4.0) * M_PI_2;

	playerc_position2d_set_cmd_pose(position2d, position2d->px,
					position2d->py, target_a, 1);
	while (fmod(fabs(position2d->pa - target_a), 2 * M_PI) > 0.01)
		playerc_client_read(client);

	theta = (theta + 1) % 4;
	return 0;
}

int girar_dch(playerc_client_t * client, playerc_position2d_t * position2d)
{				// !! fundirse con girar_izq
	double target_a;

	target_a = fmod((round(position2d->pa / M_PI_2) - 1), 4.0) * M_PI_2;

	playerc_position2d_set_cmd_pose(position2d, position2d->px,
					position2d->py, target_a, 1);
	while (fmod(fabs(position2d->pa - target_a), 2 * M_PI) > 0.01)
		playerc_client_read(client);

	theta = (theta + 3) % 4; // -1 mod 4
	return 0;
}

int girar_180(playerc_client_t * client, playerc_position2d_t * position2d)
{
	double target_a;

	target_a = fmod((round(position2d->pa / M_PI_2) + 2), 4.0) * M_PI_2;

	playerc_position2d_set_cmd_pose(position2d, position2d->px,
					position2d->py, target_a, 1);
	while ((fmod(fabs(position2d->pa - target_a), 2 * M_PI) > 0.01) && (fmod(fabs(position2d->pa - target_a), 2 * M_PI) < 2*M_PI - 0.01)) {
		playerc_client_read(client);
	}
	theta = (theta + 2) % 4;
	return 0;
}

int ir_celda_delante(playerc_client_t * client,
		     playerc_position2d_t * position2d)
{
	double target_x, target_y;

	target_x = round(position2d->px) + cos(position2d->pa);
	target_y = round(position2d->py) + sin(position2d->pa);

	printf(">>>>>>>>>>>>>\n");

	playerc_position2d_set_cmd_pose(position2d, target_x, target_y, position2d->pa, 1);
	while (sqrt(pow(position2d->px - target_x, 2.0) +
	            pow(position2d->py - target_y, 2.0)) > 0.05)
		playerc_client_read(client);

	switch (theta) {
	case D_IZQ:
		x--;
		if (x < xmin) { xmin = x; };
		break;
	case D_ARRIBA:
		y++;
		if (y > ymax) { ymax = y; };
		break;
	case D_DCH:
		x++;
		if (x > xmax) { xmax = x; };
		break;
	case D_ABAJO:
		y--;
		if (y < ymin) { ymin = y; };
		break;
	}
	return 0;
}

void inspeccionar_celda(playerc_sonar_t * sonar, celda * celda_actual)
{
	int is;

	for (is = 0; is < sonar->scan_count; is++) {
		printf("(%d)%g\t", is + 1, sonar->scan[is]);
	}
	printf("\n");

	if (sonar->scan[0] > 1)
		celda_actual->pared[D_IZQ] = NO_MURO;
	else
		celda_actual->pared[D_IZQ] = HAY_MURO;
	if (((sonar->scan[2] + sonar->scan[3]) / 2) > 1)
		celda_actual->pared[D_ARRIBA] = NO_MURO;
	else
		celda_actual->pared[D_ARRIBA] = HAY_MURO;
	if (sonar->scan[5] > 1)
		celda_actual->pared[D_DCH] = NO_MURO;
	else
		celda_actual->pared[D_DCH] = HAY_MURO;

	for (is = 0; is < 3; is++) {
		if (celda_actual->pared[is])
			printf("pared %s\n", dirs[is]);
	}

	celda_actual->orientacion = theta;
	celda_actual->pos[0] = x;
	celda_actual->pos[1] = y;
	return;

}

int celda_final(celda * celda_actual)
{
	return (! (celda_actual->pared[D_IZQ] ||
	           celda_actual->pared[D_DCH] ||
		   celda_actual->pared[D_ARRIBA]));
}


int ir_direccion(playerc_client_t * client, playerc_position2d_t * position2d, int dir)
{
	switch(dir) {
	case D_IZQ:
		girar_izq(client, position2d);
		return ir_celda_delante(client,position2d);
	case D_ARRIBA:
		return ir_celda_delante(client,position2d);
	case D_DCH:
		girar_dch(client, position2d);
		return ir_celda_delante(client,position2d);
	case D_ABAJO:
		girar_180(client, position2d);
		return ir_celda_delante(client,position2d);
	default:
		return -1;
	}
}
