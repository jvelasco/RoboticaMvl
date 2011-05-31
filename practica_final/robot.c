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

int mejor_ruta(celda* celdas, int actual, int* forward);

int main(int argc, const char **argv)
{
	int r, i, j;
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
			//establecer orientación de referencia aquí
			theta = D_ARRIBA;
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
			girar_180(client,position2d);
		}
		//establecer orientación de referencia aquí
		theta = D_ARRIBA;
		inspeccionar_celda(sonar,&celdas[actual]);
	}


	while(!(flag_celda_final || no_solucion)) {
		printf("celda: %d [%d,%d]\n",actual, x, y);
		r = mejor_ruta(celdas, actual, &forward);
		if (forward) {
			ir_direccion(client, position2d, r);
			actual++;
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
				for (j = 0; (j < 3) && (celdas[i].pared[j] != RUTA); j++) ;
				printf("(%d,%d) - %s\n", celdas[i].pos[0], celdas[i].pos[1], dirs[j]);
			}
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

int mejor_ruta(celda* celdas, int actual, int* forward)
{
	int i, i_opt, j;
	int mejor_puntuacion = 0, puntuacion, mejor_opcion = -1;
	int nx, ny, ntheta, giro;
	celda * c = celdas + actual;

	printf("xmax %d, xmin %d, ymax %d, ymin %d\n", xmax, xmin, ymax, ymin);
	for (i = 0; i < 3; i++) {
		puntuacion = 0;
		if (c->pared[i] == RUTA) { // venimos de aqui
			c->pared[i] = DEAD_END; // por lo tanto no hay salida
			continue;
		} else if (c->pared[i] != NO_MURO) { // muro o sin salida
			continue;
		}

		// si forward=1, c->orientacion y theta deben ser iguales
		// pero si no, hay que tener en cuenta el cambio de perspectiva
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
		
		// favorecer ir hacia los bordes
		if ((nx > xmax) || (nx < xmin) || (ny > ymax) || (ny < ymin)) {
			puntuacion +=200;
		} else if ((nx == xmax) || (nx == xmin) || (ny == ymax) || (ny == ymin)) {
			puntuacion +=100;
		}

		if (giro == D_ARRIBA) {
			puntuacion+=50; //favorecer ir hacia delante
		}

		puntuacion += (rand() % 4) + 1; // para hacer >0 desempatar (por si acaso)

 		// Comprobar posibles bucles.
		for (j = 0; j < actual - 1; j++) {
			if ((celdas[j].pos[0] == nx) && (celdas[j].pos[1] == ny)) {
				puntuacion = -10; // ya visitado. no tomar esta ruta.
				break;
			}
		}


		if (puntuacion > mejor_puntuacion) {
			mejor_puntuacion = puntuacion;
			mejor_opcion = giro;
			i_opt = i;
		}
		printf("Opcion %s (a [%d,%d]) %d puntos\n", dirs[giro], nx, ny, puntuacion);
	}
	if (mejor_opcion >= 0) {
		*forward = 1;
		c->pared[i_opt]=RUTA; //en ruta
		printf("Gana opcion %s\n", dirs[mejor_opcion]);
		return mejor_opcion;
	} else { //ninguna opción válida
		*forward = 0;
		return DEG2DIR(DIR2DEG(c->orientacion) - DIR2DEG(theta) + 180); // volver por donde se ha venido
	}

}
