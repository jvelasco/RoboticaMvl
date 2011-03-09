#include <stdio.h>
#include <math.h>

#include <libplayerc/playerc.h>

int
main(int argc, const char **argv)
{
  int i;  
  playerc_client_t *client;
  playerc_position2d_t *position2d;
  player_pose2d_t position2d_target;
  double arrayx[5]={ 5.0 ,  -5.0 , -5.0 ,  5.0 ,  5.0};
  double arrayy[5]={ 2.5 ,   2.5 , -2.5 , -2.5 ,  0.0};
  double arraya[5]={3.14 , -1.57 ,  0.0 , 1.57 , 1.57};

  //sonar
  playerc_sonar_t *sonar;

  //laser

  //drawing
  playerc_graphics2d_t *graficos;
  player_point_2d_t *puntos;
  player_color_t color;
  puntos=(player_point_2d_t *)malloc(sizeof(player_point_2d_t)*(1)); //(1) punto
  color.red=255; color.green=0; color.blue=0;
  

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, "localhost", 6665);
  if (playerc_client_connect(client) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }


  // Create and subscribe to a position2d device.
  position2d = playerc_position2d_create(client, 0);
  if (playerc_position2d_subscribe(position2d, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  // Fixing initial position
  playerc_position2d_set_odom(position2d,5.0,0.0,1.57);

/* Si no se incluye esta línea el robot averigua la pose que tiene en el
 * servidor player.

 * En el caso del simulador, es posible saber la posición inicial del robot,
 * pero en un caso real, si ni se conoce la posición inicial ni es posible
 * averiguarla la odometría no resulta útil porque no se puede localizar ningún
 * elemento del mapa.
 */

  // Create and subscribe to a sonar device
  sonar = playerc_sonar_create(client, 0);
  if (playerc_sonar_subscribe(sonar, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  // Fixing initial position
  playerc_position2d_set_odom(position2d,5.0,0.0,1.57);

  // Create and subscribe to a graphics device
  graficos = playerc_graphics2d_create(client, 0);
  if (playerc_graphics2d_subscribe(graficos, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  // Fix colour
  playerc_graphics2d_setcolor (graficos, color); 

  // Clear screen
  playerc_graphics2d_clear(graficos); 

  // Enable motors 
  playerc_position2d_enable(position2d,1);
  
  for ( i=0 ; i<5 ; i++ )
    {
      position2d_target.px = arrayx[i];
      position2d_target.py = arrayy[i];
      position2d_target.pa = arraya[i];
      
      // Move to pose  
      playerc_position2d_set_cmd_pose(position2d, position2d_target.px , position2d_target.py, position2d_target.pa , 1);

      // Stop when reach the target
      while  (sqrt(pow(position2d->px - position2d_target.px,2.0) + pow(position2d->py - position2d_target.py,2.0)) > 0.05 )
	{
	  // Wait for new data from server
	  playerc_client_read(client);
	  
	  // Print current robot pose
	  printf("position2d : x %f y %f th %f stall %d\n",position2d->px, position2d->py, position2d->pa, position2d->stall); 
	  // What does mean stall?
	  // x, y, th, world frame or robot frame?

/* stall indica si después de aplicar una consigna a los motores las ruedas se
 * mueven. Si stall vale 1 el robot no se está moviendo porque se ha encontrado
 * con un obstáculo. Sin embargo, si el suelo es resbaladizo, al alcanzar un
 * obstáculo puede que las ruedas patinen. Como en este caso los encoders sí
 * detectarían movimiento stall valdría 0 y no podríamos detectar el obstáculo
 * sólo con odometría.
 */

	  // Draw current robot pose
	  puntos[0].px=position2d->px;
	  puntos[0].py=position2d->py;
	  playerc_graphics2d_draw_points (graficos, puntos, 1);
	  
	  // Print sonar readings

	  // printf("sonar : x %f y %f th %f stall %d\n",sonar->px, sonar->py, sonar->pa, sonar->stall); 
	}
    }


  // Unsuscribe and Destroy
  // position2d
  playerc_position2d_unsubscribe(position2d); playerc_position2d_destroy(position2d);
  // sonar
  playerc_sonar_unsubscribe(sonar); playerc_sonar_destroy(sonar);

  // graphics2d
  playerc_graphics2d_unsubscribe(graficos); playerc_graphics2d_destroy(graficos);
  // client
  playerc_client_disconnect(client); playerc_client_destroy(client);

  // End
  return 0;
}
