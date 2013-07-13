
#define APPLICATION_CONFIG_FILENAME "SYSTEM/ds2vidplayer.cfg"

#define APPLICATION_CONFIG_HEADER  "D2VIDPLAY0.1a"
#define APPLICATION_CONFIG_HEADER_SIZE 13


struct _APPLICATION_CONFIG
{
  u32 language;
  u32 CompressionLevel;
  u32 Reserved[126];
};

typedef struct _APPLICATION_CONFIG APPLICATION_CONFIG;

void init_application_config (APPLICATION_CONFIG * application_config);
int load_application_config_file (APPLICATION_CONFIG * application_config,
				  char *main_path);
int save_application_config_file (APPLICATION_CONFIG * application_config,
				  char *main_path);
