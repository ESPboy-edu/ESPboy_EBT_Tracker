#define MAX_LINE_LEN	(MAX_PATH+4)

char ebt_file_line[MAX_LINE_LEN] = { 0 };



#ifdef TARGET_SDL

FILE* ebt_file_handler = NULL;

BOOL ebt_file_is_exist(const char* filename)
{
  char filepath[MAX_PATH];

  strncpy(filepath, dataDirectory, sizeof(filepath) - 1);
  strncat(filepath, filename, sizeof(filepath) - 1);

  FILE* file = fopen(filepath, "rb");

  if (file)
  {
    fclose(file);
    return TRUE;
  }

  return FALSE;
}



BOOL ebt_file_open(const char* filename, BOOL write)
{
  char filepath[MAX_PATH];

  strncpy(filepath, dataDirectory, sizeof(filepath) - 1);
  strncat(filepath, filename, sizeof(filepath) - 1);

  printf("opening %s for %s\n", filepath, write ? "write" : "read");

  ebt_file_handler = fopen(filepath, write ? "wt" : "rt");

  if (!ebt_file_handler)
  {
	  printf("failed!\n");
	  return FALSE;
  }

  return TRUE;
}



void ebt_file_close(void)
{
  printf("closing file\n");

  fclose(ebt_file_handler);
}



const char* ebt_file_get_line(void)
{
  if (fgets(ebt_file_line, sizeof(ebt_file_line), ebt_file_handler) == NULL) return NULL;

  return ebt_file_line;
}



void ebt_file_put_line(const char* line)
{
  fprintf(ebt_file_handler, "%s\n", line);
}



void ebt_file_rename(const char* old_name, const char* new_name)
{
	char filepath_old[MAX_PATH];
	char filepath_new[MAX_PATH];

	strncpy(filepath_old, dataDirectory, sizeof(filepath_old) - 1);
	strncat(filepath_old, old_name, sizeof(filepath_old) - 1);

	strncpy(filepath_new, dataDirectory, sizeof(filepath_new) - 1);
	strncat(filepath_new, new_name, sizeof(filepath_new) - 1);

	printf("rename file %s to %s\n", filepath_old, filepath_new);

	remove(filepath_new);
	rename(filepath_old, filepath_new);
}



void ebt_file_delete(const char* filename)
{
	char filepath[MAX_PATH];

	strncpy(filepath, dataDirectory, sizeof(filepath) - 1);
	strncat(filepath, filename, sizeof(filepath) - 1);

	printf("delete file %s\n", filepath);

	remove(filepath);
}

#endif


#ifdef TARGET_ESPBOY


fs::File ebt_file_entry;

BOOL ebt_file_is_exist(const char* filename)
{
  sound_output_shut();

  ebt_file_entry = LittleFS.open(filename, "r");

  if (!ebt_file_entry)
  {
    sound_output_init();
    return FALSE;
  }

  ebt_file_entry.close();
  sound_output_init();

  return TRUE;
}



BOOL ebt_file_open(const char* filename, uint8_t write)
{
  printf("opening %s for %s\n", filename, write ? "write" : "read");

  sound_output_shut();

  ebt_file_entry = LittleFS.open(filename, write ? "w" : "r");

  if (!ebt_file_entry)
  {
    sound_output_init();
    return FALSE;
  }

  return TRUE;
}



void ebt_file_close(void)
{
  printf("closing file\n");

  ebt_file_entry.close();

  sound_output_init();
}



const char* ebt_file_get_line(void)
{
  if (!ebt_file_entry.available()) return NULL;

  String line = ebt_file_entry.readStringUntil('\n');

  strncpy(ebt_file_line, line.c_str(), sizeof(ebt_file_line) - 1);

  line = String();

  return ebt_file_line;
}



void ebt_file_put_line(const char* line)
{
  snprintf(ebt_file_line, sizeof(ebt_file_line), "%s", line);

  ebt_file_entry.println(ebt_file_line);
}



void ebt_file_rename(const char* old_name, const char* new_name)
{
	printf("rename file %s to %s\n", old_name, new_name);

  sound_output_shut();
  
	LittleFS.remove(new_name);
	LittleFS.rename(old_name, new_name);
  
  sound_output_init();
}



void ebt_file_delete(const char* filename)
{
	printf("delete file %s\n", filename);

  sound_output_shut();
  
	LittleFS.remove(filename);

  sound_output_init();
}

#endif
