/*
Copyright (c) 2002, Andrew Howard
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of the Player Project nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <math.h>

#include <libplayerc/playerc.h>

int
main(int argc, const char **argv)
{
  int i;
  playerc_client_t *client;
  player_pose2d_t position2d_vel;
  player_pose2d_t position2d_target;
  playerc_position2d_t *position2d;

  // Create a client and connect it to the server.
  client = playerc_client_create(NULL, "localhost", 6665);
  if (0 != playerc_client_connect(client))
    return -1;

  // Create and subscribe to a position2d device.
  position2d = playerc_position2d_create(client, 0);
  if (playerc_position2d_subscribe(position2d, PLAYER_OPEN_MODE))
    return -1;

// Make the robot spin!
 if (0 != playerc_position2d_set_cmd_vel(position2d, 0.25, 0, DTOR(40.0), 1))
    return -1;

  for (i = 0; i < 50; i++)
  {
		playerc_client_read(client);

		// Print current robot pose
		printf("position2d : %f %f %f\n",
		position2d->px, position2d->py, position2d->pa);
  }
/* con el comando playerc_position2d_set_cmd_vel() simplemente se establece una
 * consigna de velocidad la cual es independiente de la posición en la que se
 * encuentra el robot */




      position2d_target.px = 2;
      position2d_target.py = -3;
      position2d_target.pa = 0;
      
      // Move to pose  
      playerc_position2d_set_cmd_pose(position2d, position2d_target.px , position2d_target.py, position2d_target.pa , 1);

	printf("position2d : %f %f %f\n",
	position2d->px, position2d->py, position2d->pa);
      // Stop when reach the target
      while  (sqrt(pow(position2d->px - position2d_target.px,2.0) + pow(position2d->py - position2d_target.py,2.0)) > 0.05 )
	{
		playerc_client_read(client);

		// Print current robot pose
		printf("position2d : %f %f %f\n",
		position2d->px, position2d->py, position2d->pa);
	}
/* Con el comando position2d_set_cmd_pose() se establece una pose de destino.
 * Cuando se utiliza este método la información obtenida de la odometría sí que
 * afecta a la trayectoria seguida y al punto final */


      position2d_target.px = 0;
      position2d_target.py = -3;
      position2d_target.pa = 0;
      position2d_vel.px = 0.6;
      position2d_vel.py = 0;
      position2d_vel.pa = 0;

      // Move to pose  
      playerc_position2d_set_cmd_pose_with_vel(position2d, position2d_target, position2d_vel, 1);

      // Stop when reach the target
      while  (sqrt(pow(position2d->px - position2d_target.px,2.0) + pow(position2d->py - position2d_target.py,2.0)) > 0.05 )
	{
		playerc_client_read(client);

		// Print current robot pose
		printf("position2d : %f %f %f\n",
		position2d->px, position2d->py, position2d->pa);
	}
/* Con el comando playerc_position2d_set_cmd_pose_with_vel se hace lo mismo que
 * con playerc_position2d_set_cmd_pose() pero además se puede indicar una
 * consigna de velocidad */

  // Shutdown
  playerc_position2d_unsubscribe(position2d);
  playerc_position2d_destroy(position2d);
  playerc_client_disconnect(client);
  playerc_client_destroy(client);

  return 0;
}
