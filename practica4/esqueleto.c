#include <stdio.h>
#include <math.h>

#include <libplayerc/playerc.h>

int
main(int argc, const char **argv)
{
  int i;  
  playerc_client_t *client;
  playerc_position2d_t *position2d_amcl,*position2d_vfh;
  player_pose2d_t position2d_target;
  
  double arrayx[5]={ 5.0 ,  -5.0 , -5.0 ,  5.0 ,  5.0};
  double arrayy[5]={ 2.5 ,   2.5 , -2.5 , -2.5 ,  0.0};
  double arraya[5]={3.14 , -1.57 ,  0.0 , 1.57 , 1.57};

  //sonar

  //laser

  //drawing
  playerc_graphics2d_t *graficos;
  player_point_2d_t *puntos;
  player_color_t color;
  puntos=(player_point_2d_t *)malloc(sizeof(player_point_2d_t)*(1)); //(1) punto
  color.red=255; color.green=0; color.blue=0;
  //localize:
  playerc_localize_t *localize;

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, "localhost", 6665);
  if (playerc_client_connect(client) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }


  // Create and subscribe to a position2d device.
  position2d_vfh = playerc_position2d_create(client, 1);
  if (playerc_position2d_subscribe(position2d_vfh, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  position2d_amcl = playerc_position2d_create(client, 2);
  if (playerc_position2d_subscribe(position2d_amcl, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  // Fixing initial position
  //playerc_position2d_set_odom(position2d,5.0,0.0,1.57);

  // Create and subscribe to a sonar device
  
  // Create and subscribe to a graphics device
  graficos = playerc_graphics2d_create(client, 0);
  if (playerc_graphics2d_subscribe(graficos, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  //crear y suscribir localize
  localize = playerc_localize_create(client,0);
  if (playerc_localize_subscribe(localize, PLAYER_OPEN_MODE) != 0)
    {
      fprintf(stderr, "error: %s\n", playerc_error_str());
      return -1;
    }

  //crear set de particulas
  playerc_localize_get_particles(localize);

  // Fix colour
  playerc_graphics2d_setcolor (graficos, color); 

  // Clear screen
  playerc_graphics2d_clear(graficos); 

  // Enable motors 
  playerc_position2d_enable(position2d_vfh,1);
  
  for ( i=0 ; i<5 ; i++ )
    {
      position2d_target.px = arrayx[i];
      position2d_target.py = arrayy[i];
      position2d_target.pa = arraya[i];
      
      // Move to pose  
      playerc_position2d_set_cmd_pose(position2d_vfh, position2d_target.px , position2d_target.py, position2d_target.pa , 1);

      // Stop when reach the target
      while  (sqrt(pow(position2d_vfh->px - position2d_target.px,2.0) + pow(position2d_vfh->py - position2d_target.py,2.0)) > 0.05 )
	{
	  // Wait for new data from server
	  playerc_client_read(client);
	  
	  // Print current robot pose
	  printf("position2d : x %f y %f th %f stall %d\n",position2d_amcl->px, position2d_amcl->py, position2d_amcl->pa, position2d_amcl->stall); 
	  // What does mean stall?
	  // x, y, th, world frame or robot frame?

	  // Draw current robot pose
	  puntos[0].px=position2d_amcl->px;
	  puntos[0].py=position2d_amcl->py;
	  playerc_graphics2d_draw_points (graficos, puntos, 1);
	  
	  // Print sonar readings
	
	}
    }


  // Unsuscribe and Destroy
  // position2d
  playerc_position2d_unsubscribe(position2d_vfh); playerc_position2d_destroy(position2d_vfh);
  playerc_position2d_unsubscribe(position2d_amcl); playerc_position2d_destroy(position2d_amcl);

  // sonar

  // graphics2d
  playerc_graphics2d_unsubscribe(graficos); playerc_graphics2d_destroy(graficos);
  // client
  playerc_client_disconnect(client); playerc_client_destroy(client);

  // End
  return 0;
}
