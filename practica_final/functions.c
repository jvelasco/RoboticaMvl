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

const double PI_OVER_2 = 1.57079632679489661923;
const double  PI = 3.14159265358979323846;

int ir_celda_delante(playerc_client_t *client,playerc_position2d_t *position2d)
{
  double target_x,target_y;

  target_x=round(position2d->px)+cos(position2d->pa);
  target_y=round(position2d->py)+sin(position2d->pa);

  printf(">>>>>>>>>>>>>\n");
  
  playerc_position2d_set_cmd_pose(position2d, target_x , 
				  target_y, position2d->pa , 1);
  while (sqrt(pow(position2d->px - target_x,2.0) + pow(position2d->py - target_y,2.0)) > 0.05 )
    playerc_client_read(client);
  return 0;
}

int ir_celda_izq(playerc_client_t *client,playerc_position2d_t *position2d)
{
  double target_a;
  
  target_a=fmod((round(position2d->pa/PI_OVER_2)+1),4.0)*PI_OVER_2;
 
  playerc_position2d_set_cmd_pose(position2d, position2d->px , 
				  position2d->py, target_a , 1);
  while (fmod(fabs(position2d->pa - target_a),2*PI)>0.01)
    playerc_client_read(client);

  ir_celda_delante(client,position2d);
  return 0;
}
int ir_celda_dch(playerc_client_t *client,playerc_position2d_t *position2d)
{
  double target_a;
  
  target_a=fmod((round(position2d->pa/PI_OVER_2)-1),4.0)*PI_OVER_2;
 
  playerc_position2d_set_cmd_pose(position2d, position2d->px , 
				  position2d->py, target_a , 1);
  while (fmod(fabs(position2d->pa - target_a),2*PI)>0.01)
    playerc_client_read(client);

  ir_celda_delante(client,position2d);
  return 0;
}

int ir_celda_detras(playerc_client_t *client,playerc_position2d_t *position2d)
{
  double target_a;
  
  target_a=fmod((round(position2d->pa/PI_OVER_2)+2),4.0)*PI_OVER_2;
 
  playerc_position2d_set_cmd_pose(position2d, position2d->px , 
				  position2d->py, target_a , 1);
  while (fmod(fabs(position2d->pa - target_a),2*PI)>0.1)
  {
    playerc_client_read(client);
    printf("angulo: %g (%g,%g) \n",fabs(fmod(position2d->pa,2*PI) - target_a),position2d->pa,target_a);
  }
  ir_celda_delante(client,position2d);
  return 0;
}

void inspeccionar_celda(playerc_sonar_t *sonar,celda *celda_actual)
{
  int is;
  const char* cadena[3]={"izquierda","delante", "derecha" };

  
  for (is = 0; is < sonar->scan_count; is++) {
		printf("(%d)%g\t", is+1, sonar->scan[is]);
  }
  printf("\n");

  if(sonar->scan[0]>1) celda_actual->pared[0]=0;
  else celda_actual->pared[0]=1;
  if(((sonar->scan[2]+sonar->scan[3])/2)>1) celda_actual->pared[1]=0;
  else celda_actual->pared[1]=1;
  if(sonar->scan[5]>1) celda_actual->pared[2]=0;
  else celda_actual->pared[2]=1;
  
  for(is=0;is<3;is++)
  {
    if(celda_actual->pared[is])
      printf("pared %s\n",cadena[is]);
  }
  return;

}

int celda_final(celda *celda_actual)
{
  return(!(celda_actual->pared[0]||celda_actual->pared[1]||celda_actual->pared[2]));
}
