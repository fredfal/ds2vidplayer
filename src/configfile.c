/* configfile.c
 *
 * Authors: dking <dking024@gmail.com>
 *          nebuleon
 *          fredfal
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <ds2_malloc.h>
#include <ds2io.h>
#include "fs_api.h"
#include "configfile.h"

/*--------------------------------------------------------
        Initialise application configuration
--------------------------------------------------------*/
void
init_application_config (APPLICATION_CONFIG * application_config)
{
  memset (application_config, 0, sizeof (APPLICATION_CONFIG));
  application_config->language = 0;	// Default language: English
  // Default trade-off between compression and speed: Speed
  // The processor in the DSTwo is just not the same as a 3.0 GHz
  // quad core, and the gains gotten from using "computer gzip"'s
  // default of 6 over the full speed of 1 are minimal.
  // The user can set his or her preference in Options anyway. [Neb]
  application_config->CompressionLevel = 1;
}

/*--------------------------------------------------------
        Load application configuration file
--------------------------------------------------------*/
int
load_application_config_file (APPLICATION_CONFIG * application_config,
			      char *main_path)
{
  char tmp_path[MAX_PATH];
  FILE *fp;
  char *pt;
  err_msg (DOWN_SCREEN, APPLICATION_CONFIG_FILENAME);

  sprintf (tmp_path, "%s/%s", main_path, APPLICATION_CONFIG_FILENAME);

  fp = fopen (tmp_path, "r");
  if (NULL != fp)
    {
      // check the file header
      pt = tmp_path;
      fread (pt, 1, APPLICATION_CONFIG_HEADER_SIZE, fp);
      pt[APPLICATION_CONFIG_HEADER_SIZE] = 0;
      if (!strcmp (pt, APPLICATION_CONFIG_HEADER))
	{
	  memset (application_config, 0, sizeof (APPLICATION_CONFIG));
	  fread (application_config, 1, sizeof (APPLICATION_CONFIG), fp);
	  fclose (fp);
	  return 0;
	}
      else
	{
	  fclose (fp);
	}
    }

  // No configuration or in the wrong format. Set defaults.
  init_application_config (application_config);
  return -1;
}

/*--------------------------------------------------------
        Save application configuration file
--------------------------------------------------------*/
int
save_application_config_file (APPLICATION_CONFIG * application_config,
			      char *main_path)
{
  char tmp_path[MAX_PATH];
  FILE *fp;

  sprintf (tmp_path, "%s/%s", main_path, APPLICATION_CONFIG_FILENAME);
  fp = fopen (tmp_path, "w");
  if (NULL != fp)
    {
      fwrite (APPLICATION_CONFIG_HEADER, 1, APPLICATION_CONFIG_HEADER_SIZE,
	      fp);
      fwrite (application_config, 1, sizeof (APPLICATION_CONFIG), fp);
      fclose (fp);
      return 0;
    }

  return -1;
}
