#include <stdio.h>
#include <math.h>

#define RADIO_AZUL 0.5
#define PF_MAX_SAMPLES 250000
#include <libplayerc/playerc.h>

//Declaración

void DibujaCirculo(double, double, double, player_color_t);

playerc_graphics2d_t *graficos;

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
  player_point_2d_t *puntos;
  player_color_t rojo, verde, azul;
  puntos=(player_point_2d_t *)malloc(sizeof(player_point_2d_t)*(PF_MAX_SAMPLES)); //(1) punto
  rojo.red=255; rojo.green=0; rojo.blue=0;
  verde.red=0; verde.green=128; verde.blue=0;
  azul.red=0; azul.green=0; azul.blue=255;
  //localize:
  playerc_localize_t *localize;
  int p;

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
	  playerc_localize_get_particles(localize);

	  
	  // Print current robot pose
	   printf("position2d : x %f y %f th %f stall %d\n",position2d_amcl->px, position2d_amcl->py, position2d_amcl->pa, position2d_amcl->stall); 
	  // What does mean stall?
	  // x, y, th, world frame or robot frame?

	  // Clear screen
	  playerc_graphics2d_clear(graficos); 


	  // Draw current robot pose
	   printf("num particulas: %d num hipotesis %d\n",localize->num_particles,localize->hypoth_count);
	  for(p=0;p<localize->num_particles;p++){
	  
	    puntos[p].px=(float)(localize->particles[p].pose[0]);
	    puntos[p].py=(float)(localize->particles[p].pose[1]);
	  }
	  // Fix colour
	  playerc_graphics2d_setcolor (graficos, rojo); 
	  playerc_graphics2d_draw_points (graficos, puntos, localize->num_particles);
	  DibujaCirculo(localize->mean[0], localize->mean[1], localize->variance, azul);
	  DibujaCirculo(position2d_amcl->px, position2d_amcl->py, RADIO_AZUL, azul);

	  printf("(%g, %g) var %g\n", localize->mean[0], localize->mean[1], localize->variance);
	  
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

//Definición. El centro es (xc, yc). También se indican el radio y el color
//Debe existir la variable global "graficos" que es el proxie al dispositivo graphics2d

void DibujaCirculo(double xc, double yc, double radio, player_color_t color)
{
	double x[10], senos[10], cosenos[10], inc;
	int i;
	player_point_2d_t puntos[10];

        inc=2*M_PI/10;
	x[0]=0.0;
	for(i=1;i<10;i++)
        {
		
            x[i]=x[i-1]+inc;
        }
	for(i=0;i<10;i++)
	{
	    senos[i]=sin(x[i]);
	    cosenos[i]=cos(x[i]);
	}
        for(i=0;i<10;i++)
	{
	    puntos[i].px=xc+radio*cosenos[i];
	    puntos[i].py=yc+radio*senos[i];
	}

	playerc_graphics2d_setcolor (graficos, color);
	playerc_graphics2d_draw_polyline (graficos, puntos, 10);

}
