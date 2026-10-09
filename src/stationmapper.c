#include "stdio.h"
#include "string.h"
#include "math.h"
#include "unistd.h"

#include "../include/stationmapper.h"

#define LOADBMP_IMPLEMENTATION
#include "../include/loadbmp.h"

#define LIB_VERSION_MAJOR 1
#define LIB_VERSION_MINOR 0
#define LIB_VERSION_PATCH 2

const version_t get_library_version(void) {
    const version_t version = {.major = LIB_VERSION_MAJOR, .minor = LIB_VERSION_MINOR, .patch = LIB_VERSION_PATCH};
    return version;
}

peace_of_map_t load_map(const char* image_filename, const char* config_filename) {
    peace_of_map_t map;

    if(access(image_filename, F_OK) != 0) {
        printf("Failed to load map image file %s\n", image_filename);
        return map;
    }

    if(access(config_filename, F_OK) != 0) {
        printf("Failed to load config file %s\n", config_filename);
        return map;
    }

    unsigned int err = loadbmp_decode_file(image_filename, &map.image, &map.width, &map.height, LOADBMP_RGBA);
    if (err)
		printf("LoadBMP Load Error: %u\n", err);
    
    FILE *fp;
    fp = fopen(config_filename, "r");
    fscanf(fp, "%*[^\n]\n");
    fscanf(fp, "%f, %f, %f, %f\n", &map.top_left_lat, &map.top_left_lon, &map.bottom_right_lat, &map.bottom_right_lon);
    fclose(fp);

    return map;
}


int save_map(const peace_of_map_t *map, const char *output_filename) {
    return loadbmp_encode_file(output_filename, map->image, map->width, map->height, LOADBMP_RGBA);
}


void draw_pixel(unsigned char * image, int width, int x, int y, int r, int g, int b, int a)
{
	image[(width * y + x) * 4] = r;
	image[(width * y + x) * 4 + 1] = g;
	image[(width * y + x) * 4 + 2] = b;
	image[(width * y + x) * 4 + 3] = a;
}


void add_pixel(unsigned char * image, int width, int x, int y, int r, int g, int b, int a)
{
	image[(width * y + x) * 4] += r * a / 255;
	image[(width * y + x) * 4 + 1] += g * a / 255;
	image[(width * y + x) * 4 + 2] += b * a / 255;
}


void draw_point_by_lat_lon(peace_of_map_t * map, float lat, float lon, int r, int g, int b) {
    int x = map->width * ((lon - map->bottom_right_lon) / (map->top_left_lon - map->bottom_right_lon));
    int y = map->height - map->height * ((lat - map->bottom_right_lat) / (map->top_left_lat - map->bottom_right_lat)) - 1;
    if ((x < 0) || (x >= map->width) || (y < 0) || (y > map->height)) {
        printf("Incorrect lat lon: %f %f\n", lat, lon);
        return;
    }
    

    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            if ((x + i < 0) || (x + i >= map->width) || (y + j < 0) || (y + j > map->height)) {
                continue;
            }
            add_pixel(map->image, map->width, x + i, y + j, r, g, b, 32);
        }
    }
}


stations_list_t load_stations(const char * stations_list_filename) {
    stations_list_t stations_list;
    stations_list.num_stations = 0;

    // Count entries
    FILE *fp; 
    fp = fopen(stations_list_filename, "r");
    fscanf(fp, "%*[^\n]\n");
    while(!feof(fp))
    {
        fscanf(fp, "%*[^\n]\n");
        stations_list.num_stations++;
    }
    fclose(fp);
    stations_list.num_stations = stations_list.num_stations;

    // Read stations
    stations_list.stations = malloc(stations_list.num_stations * sizeof(station_t));
    fp = fopen(stations_list_filename, "r");
    fscanf(fp, "%*[^\n]\n");    
    for (int i = 0; i < stations_list.num_stations; i++)
    {
        char line[256];
        fgets(line, 256, fp);
        sscanf(line, "%d,%s,%f,%f", &stations_list.stations[i].id,
                                    stations_list.stations[i].name,
                                    &stations_list.stations[i].lat,
                                    &stations_list.stations[i].lon);
    }
    fclose(fp);

    return stations_list;
}


float get_distance_in_km(float lat_1, float lon_1, float lat_2, float lon_2) {
    return sqrt((lat_1 - lat_2) * (lat_1 - lat_2) + (lon_1 - lon_2) * (lon_1 - lon_2));
}


float deg_to_rad(float deg) {
  return deg * (3.1415 / 180);
}


station_t get_nearest_station(stations_list_t *stations, float lat, float lon) {
    int idx_min = 3000;
    float min_dist = 1.e5;
    for (int i = 0; i < stations->num_stations; i++) {
        float dist = get_distance_in_km(stations->stations[i].lat, stations->stations[i].lon, lat, lon);
        if (dist < min_dist) {
            min_dist = dist;
            idx_min = i;
        }
    }
    return stations->stations[idx_min];
}