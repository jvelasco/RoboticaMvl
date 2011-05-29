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
#include "laberinto.h"
#define SECURITY_THRESHOLD 0.65

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

	celda celda_mat[25];
	celda *celda_actual=celda_mat;
	int celdas_visitadas=0;
	int vengo,voy;
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



	// Create and subscribe to a graphics device
	/* gfx_robot: para dibujar sobre el robot con el sistema de coordenadas del robot.
	 * gfx_mapa: para dibujar sobre el mapa con el sistema de coordenadas global.
	 */


	// Enable motors 
	playerc_position2d_enable(position2d, 1);

	playerc_client_read(client);

	inspeccionar_celda(sonar,celda_actual);
	celda_actual->celda_origen=NULL;

	//Rutina para asegurarse de que ni nos dejamos un camino atr�s,
	//ni estamos ya en la salida:"Ponemos el culo contra la pared"
	
	if(celda_final(celda_actual))//Si no veo ninguna pared
	{
	  flag_celda_final=1;
	  girar_dch(client,position2d);
	  inspeccionar_celda(sonar,celda_actual);
	  if(celda_actual->pared[2])//Si despues de girar a la dcha no
				    //encuentro pared a mi dcha;
	  {
	    girar_izq(client,position2d);//volvemos a la posici�n original.
	    inspeccionar_celda(sonar,celda_actual);
	    flag_celda_final=0;
	  }
	}
	else
	{
	  flag_celda_final=0;
	  if(celda_actual->pared[2])//si tengo pared a la dcha
	  {
	    girar_izq(client,position2d);
	    inspeccionar_celda(sonar,celda_actual);
	  }
	  
	  else 
	  {
	    if(celda_actual->pared[0])//si tengo pared a la izda
	    {
	      girar_dch(client,position2d);
	      inspeccionar_celda(sonar,celda_actual);
	    }
	    else if(celda_actual->pared[1]); //si la tengo en fente
	    {
	      girar_dch(client,position2d);
	      girar_dch(client,position2d);
	      inspeccionar_celda(sonar,celda_actual);
	    }
	  }
	}

	while(!flag_celda_final&&!no_solucion)
	{
	  printf("celda: %d\n",celdas_visitadas);
	  if(forward)
	  {
	    for(i=0;i<3;i++)
	    {
	    
	      if(!celda_actual->pared[i])
	      {
		celdas_visitadas++;
		switch(i)
		{
		case 0:
		  ir_celda_izq(client,position2d);
		  //celda_actual->izquierda=&celda_mat[celdas_visitadas];
		  break;
		case 1:
		  ir_celda_delante(client,position2d);
		  //celda_actual->delante=&celda_mat[celdas_visitadas];
		  break;
		case 2:
		  ir_celda_dch(client,position2d);
		  //celda_actual->derecha=&celda_mat[celdas_visitadas];
		  break;

		}
		celda_actual->celda_destino=&celda_mat[celdas_visitadas];
		celda_actual->pared[i]=2;//En ruta
		celda_mat[celdas_visitadas].celda_origen=celda_actual;
		celda_actual=&celda_mat[celdas_visitadas];
		playerc_client_read(client);
		inspeccionar_celda(sonar,celda_actual);
		break;
	      }
	      
	    }
	    if(i==3) //Si son todo paredes
	    {
	      forward=0;
	      ir_celda_detras(client,position2d);
	      celda_actual=celda_actual->celda_origen;
	      celdas_visitadas--;
	    }
	  }
	  else
	  {
	    for(i=0;i<3;i++)
	    {
	      voy=3; //Peor caso: he probado todas las alernativas;
	      if(celda_actual->pared[i]==2)
	      {
		celda_actual->pared[i]=3;//NO SOLUCION
		vengo=i;
	      }
	      else if(!celda_actual->pared[i])
	      {
		voy=i;
		forward=1;
		break;
	      }
	      
	    }
	    printf("Vengo de %d, voy a %d\n",vengo,voy);
	   
	    if((celda_actual->celda_origen!=NULL)||voy!=3)
	    {
	      no_solucion=0;
	      switch(voy-vengo)
	      {
	      case 1:
		ir_celda_izq(client,position2d);
		break;
	      case 2:
		ir_celda_delante(client,position2d);
		break;
	      case 3:
		ir_celda_dch(client,position2d);
		break;
		
	      }
	    
	      switch(voy)
	      {
	      case 1:case 2:
		celdas_visitadas++;
		celda_actual->celda_destino=&celda_mat[celdas_visitadas];
		celda_actual->pared[voy]=2;//En ruta
		celda_mat[celdas_visitadas].celda_origen=celda_actual;
		celda_actual=&celda_mat[celdas_visitadas];
		playerc_client_read(client);
		inspeccionar_celda(sonar,celda_actual);
		break;
		
	      case 3:
		celda_actual=celda_actual->celda_origen;
		celdas_visitadas--;
	      }
	    }
	    else 
	    {
	      no_solucion=1;
	      printf("no hay solucion\n");
	    }
	   
	    
	  }
	  
	  flag_celda_final=celda_final(celda_actual);
	}

	for (i = 0; i < 50; i++)
	{
	  playerc_client_read(client);

	  // Print current robot pose
	  printf("position2d : %f %f %f\n",
		 position2d->px, position2d->py, position2d->pa);
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
