/* ROBÓÓTICA MÓVIL
 *
 * Práctica 3.
 * Alumnos:
 * 		Miguel Martínez Rey
 * 		Jose Francisco Velasco Cerpa
 */

#include <stdio.h>
#include <math.h>

#include <libplayerc/playerc.h>
#include "laberinto.h"

int mejor_ruta(celda* c, int* forward);

int main(int argc, const char **argv)
{
	int r, i;
	playerc_client_t *client;
	playerc_position2d_t *position2d;


	//sonar
	playerc_sonar_t *sonar;

	celda celdas[25];
	int actual=0;
	int forward=1;
	int no_solucion=0;
	int flag_celda_final=0;

	// Create a client and connect it to the server.
	client = playerc_client_create(NULL, "localhost", 6665);
	if (playerc_client_connect(client) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}

	// Create and subscribe to a position2d device.
	position2d = playerc_position2d_create(client, 0);
	if (playerc_position2d_subscribe(position2d, PLAYER_OPEN_MODE) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
	// Fixing initial position
	playerc_position2d_set_odom(position2d, 0.0, 0.0, 0.0);

	// Create and subscribe to a sonar device
	sonar = playerc_sonar_create(client, 0);
	if (playerc_sonar_subscribe(sonar, PLAYER_OPEN_MODE) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
/* Obtener la geometría de los sensores de ultrasonidos sobre el pioneer 2 */
	if (playerc_sonar_get_geom(sonar) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}

	// Enable motors 
	playerc_position2d_enable(position2d, 1);

	playerc_client_read(client);

	inspeccionar_celda(sonar,&celdas[actual]);

	//Rutina para asegurarse de que ni nos dejamos un camino atrás,
	//ni estamos ya en la salida:"Ponemos el culo contra la pared"
	
	if(celda_final(&celdas[actual])) {//Si no veo ninguna pared
		girar_dch(client,position2d); // giro para comprobar la espalda
		inspeccionar_celda(sonar,&celdas[actual]);
		if(celdas[actual].pared[D_DCH]) { // había pared detrás
	    		girar_izq(client,position2d); 
			inspeccionar_celda(sonar,&celdas[actual]);
		} else {
			printf("Ya estamos en la salida\n");
			flag_celda_final=1;
		}
	} else {
		// me pongo de espaldas a la pared
		if(celdas[actual].pared[D_DCH]) { //si tengo pared a la dcha
			girar_izq(client,position2d); 
		} else if(celdas[actual].pared[D_IZQ]) { //si tengo pared a la izda
			girar_dch(client,position2d);
		} else if(celdas[actual].pared[D_ARRIBA]) { //si la tengo en fente
			girar_dch(client,position2d);
			girar_dch(client,position2d);
		}
		inspeccionar_celda(sonar,&celdas[actual]);
	}

	while(!(flag_celda_final || no_solucion)) {
		printf("celda: %d [%d,%d]\n",actual, x, y);
		r = mejor_ruta(&celdas[actual], &forward);
		if (forward) {
			ir_direccion(client, position2d, r);
			actual++;
			// playerc_client_read(client); ?? no hace falta
			inspeccionar_celda(sonar,&celdas[actual]);
		} else if (actual > 0) {
			ir_direccion(client, position2d, r);
			actual--;
		} else {
			no_solucion=1;
			printf("no hay solucion\n");
		}
		
		if ((flag_celda_final = celda_final(&celdas[actual])) != 0) {
			printf("Fuera del laberinto!!\n\n");
			printf("Camino:\n");
			printf("-------\n");
			for (i=0; i <= actual; i++) {
				printf("(%d,%d), ", celdas[i].pos[0], celdas[i].pos[1]);
			}
			printf("final\n");
		}
	}


	// Unsuscribe and Destroy
	// position2d
	playerc_position2d_unsubscribe(position2d);
	playerc_position2d_destroy(position2d);
	// sonar
	playerc_sonar_unsubscribe(sonar);
	playerc_sonar_destroy(sonar);



	// client
	playerc_client_disconnect(client);
	playerc_client_destroy(client);

	// End
	return 0;
}

int mejor_ruta(celda* c, int* forward)
{
	int i, i_opt;
	int mejor_puntuacion=-1, puntuacion, mejor_opcion=-1;
	int nx, ny, ntheta, giro;
	for (i = 0; i < 3; i++) {
		puntuacion = 0;
		if (c->pared[i] != NO_MURO) {
			continue;
		}
		if (i == D_ARRIBA) {
			puntuacion+=50; //favorecer ir hacia alante
		}

		puntuacion += rand()%5; // para desempatar (por si acaso)

		// si forward=1, c->orientacion y theta deben ser iguales
		giro = DEG2DIR(DIR2DEG(c->orientacion) - DIR2DEG(theta) + DIR2DEG(i));
		ntheta = DEG2DIR(DIR2DEG(theta) + DIR2DEG(giro));

		switch (ntheta) {
		case D_IZQ:
			nx = x - 1;
			ny = y;
			break;
		case D_ARRIBA:
			nx = x;
			ny = y + 1;
			break;
		case D_DCH:
			nx = x + 1;
			ny = y;
			break;
		case D_ABAJO:
			nx = x;
			ny = y - 1;
			break;
		}
		if (nx >= xmax || nx <= xmin || ny >= ymax || ny <= ymin) {
			puntuacion +=100; // favorecer ir hacia los bordes
		}
 		// TODO: comprobar bucles aquí y dar puntuación -10.


		if (puntuacion > mejor_puntuacion) {
			mejor_puntuacion = puntuacion;
			mejor_opcion = giro;
			i_opt = i;
		}
		printf("Opcion %s (a [%d,%d]) %d puntos\n", dirs[giro], nx, ny, puntuacion);
	}
	if (mejor_opcion >= 0) {
		*forward = 1;
		c->pared[i_opt]=PROBADO; //en ruta
		printf("Gana opcion %s\n", dirs[i_opt]);
		return mejor_opcion;
	} else { //ninguna opción válida
		*forward = 0;
		return DEG2DIR(DIR2DEG(c->orientacion) - DIR2DEG(theta) + 180); // volver por donde se ha venido
	}

}
