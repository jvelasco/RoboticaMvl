/* ROBÓTICA MÓVIL
 *
 * Práctica 3.
 * Alumnos:
 * 		Miguel Martínez Rey
 * 		Jose Francisco Velasco Cerpa
 */

#include <stdio.h>
#include <math.h>

#include <libplayerc/playerc.h>
#define SECURITY_THRESHOLD 0.7

#define DISPLAY_WIDTH	50
#define DISPLAY_HEIGHT	50
#define DISPLAY_STEP	4

int main(int argc, const char **argv)
{
	int i, is;
	playerc_client_t *client;
	playerc_position2d_t *position2d;
	player_pose2d_t position2d_target;
	double arrayx[5] = { 5.0, -5.0, -5.0, 5.0, 5.0 };
	double arrayy[5] = { 2.5, 2.5, -2.5, -2.5, 0.0 };
	double arraya[5] = { 3.14, -1.57, 0.0, 1.57, 1.57 };
	int stop_go, stop_go_prev;	//0- sin obtaculo, 1-parado

	/* Mapa */
	int x_map, y_map, ix, iy;
	playerc_map_t *mapa;

	//sonar
	playerc_sonar_t *sonar;

	//laser

	//drawing
	playerc_graphics2d_t *gfx_robot;
	playerc_graphics2d_t *gfx_mapa;
	player_point_2d_t *puntos;
	player_point_2d_t *perimetro;
	player_color_t color;
	puntos = (player_point_2d_t *) malloc(sizeof(player_point_2d_t) * (1));	//(1) punto
	color.red = 255;
	color.green = 0;
	color.blue = 0;

	// Create a client and connect it to the server.
	client = playerc_client_create(NULL, "localhost", 6665);
	if (playerc_client_connect(client) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
	/* Crear proxy map */
	mapa = playerc_map_create(client, 0);
	if (playerc_map_subscribe(mapa, PLAYER_OPEN_MODE) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
	// Create and subscribe to a position2d device.
	position2d = playerc_position2d_create(client, 1);
	if (playerc_position2d_subscribe(position2d, PLAYER_OPEN_MODE) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
	// Fixing initial position
	playerc_position2d_set_odom(position2d, 5.0, 0.0, 1.57);

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

	perimetro = (player_point_2d_t *) malloc(sizeof(player_point_2d_t) * (sonar->pose_count));	// [pose_count] puntos (16)

	// Fixing initial position
	playerc_position2d_set_odom(position2d, 5.0, 0.0, 1.57);

	// Create and subscribe to a graphics device
	/* gfx_robot: para dibujar sobre el robot con el sistema de coordenadas del robot.
	 * gfx_mapa: para dibujar sobre el mapa con el sistema de coordenadas global.
	 */
	gfx_robot = playerc_graphics2d_create(client, 1);
	if (playerc_graphics2d_subscribe(gfx_robot, PLAYER_OPEN_MODE) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
	gfx_mapa = playerc_graphics2d_create(client, 0);
	if (playerc_graphics2d_subscribe(gfx_mapa, PLAYER_OPEN_MODE) != 0) {
		fprintf(stderr, "error: %s\n", playerc_error_str());
		return -1;
	}
	// Fix colour
	playerc_graphics2d_setcolor(gfx_mapa, color);
	playerc_graphics2d_setcolor(gfx_robot, color);

	// Clear screen
	playerc_graphics2d_clear(gfx_mapa);
	playerc_graphics2d_clear(gfx_robot);

	// Enable motors 
	playerc_position2d_enable(position2d, 1);

	/* Obtener el mapa y mostrar los datos del mapa por la consola */
	playerc_map_get_map(mapa);
	printf
	    ("Mapa %dx%d celdas, resolucion %.4g m/celda, origen en [%.4g,%.4g]\n",
	     mapa->width, mapa->height, mapa->resolution, mapa->origin[0],
	     mapa->origin[1]);

	for (i = 0; i < 5; i++) {
		position2d_target.px = arrayx[i];
		position2d_target.py = arrayy[i];
		position2d_target.pa = arraya[i];

		// Move to pose  
/* Usamos el driver VFH+ para mover el robot. Los comandos set_cmd_pose se
 * envían al dispositivo position2d:1 que es el que provee el driver VFH.  La
 * evitación de obstáculos está a cargo de este driver, el cual se encargará de
 * actuar sobre el dispotivo positon2d:0. Por ello, en el código no es
 * necesario hacer nada.
 */
		playerc_position2d_set_cmd_pose(position2d,
						position2d_target.px,
						position2d_target.py,
						position2d_target.pa, 1);

		// Stop when reach the target
		while (sqrt (pow(position2d->px - position2d_target.px, 2.0) + pow(position2d->py - position2d_target.py, 2.0)) > 0.05) {
			// Wait for new data from server
			playerc_client_read(client);

			// Draw current robot pose
			puntos[0].px = position2d->px;
			puntos[0].py = position2d->py;
			playerc_graphics2d_draw_points(gfx_mapa, puntos, 1);


			// Print sonar readings
			if (playerc_sonar_get_geom(sonar) != 0) {
				fprintf(stderr, "error: %s\n",
					playerc_error_str());
				return -1;
			}

			for (is = 0; is < sonar->scan_count; is++) {

				/* puntos detectados por el sonar para dibujar un polígono alrededor del robot */
				perimetro[is].px =
				    sonar->poses[is].px +
				    sonar->scan[is] *
				    cos(sonar->poses[is].pyaw);
				perimetro[is].py =
				    sonar->poses[is].py +
				    sonar->scan[is] *
				    sin(sonar->poses[is].pyaw);

				/* puntos detectados por el sonar dibujados sobre el mapa */
				puntos[0].px =
				    position2d->px +
				    perimetro[is].px * cos(position2d->pa) -
				    perimetro[is].py * sin(position2d->pa);
				puntos[0].py =
				    position2d->py +
				    perimetro[is].py * cos(position2d->pa) +
				    perimetro[is].px * sin(position2d->pa);
				playerc_graphics2d_draw_points(gfx_mapa, puntos, 1);

			}

			/* dibjuar los puntos calculados */
			playerc_graphics2d_clear(gfx_robot);
			playerc_graphics2d_draw_polyline(gfx_robot, perimetro, sonar->scan_count);


		}
	}

	// Unsuscribe and Destroy
	// position2d
	playerc_position2d_unsubscribe(position2d);
	playerc_position2d_destroy(position2d);
	// sonar
	playerc_sonar_unsubscribe(sonar);
	playerc_sonar_destroy(sonar);

	// graphics2d
	playerc_graphics2d_unsubscribe(gfx_mapa);
	playerc_graphics2d_destroy(gfx_mapa);
	playerc_graphics2d_unsubscribe(gfx_robot);
	playerc_graphics2d_destroy(gfx_robot);

	/* map */
	playerc_map_unsubscribe(mapa);
	playerc_map_destroy(mapa);

	// client
	playerc_client_disconnect(client);
	playerc_client_destroy(client);

	// End
	return 0;
}
