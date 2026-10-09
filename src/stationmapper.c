#include "stdio.h"
#include "string.h"
#include "math.h"
#include "unistd.h"
#include "stdlib.h"

#include "../include/stationmapper.h"

#define LOADBMP_IMPLEMENTATION
#include "../include/loadbmp.h"

#define LIB_VERSION_MAJOR 1
#define LIB_VERSION_MINOR 1
#define LIB_VERSION_PATCH 0

#define EARTH_RADIUS_KM 6371.0
#define PI 3.14159265358979323846

const version_t get_library_version(void) {
    const version_t version = {.major = LIB_VERSION_MAJOR, .minor = LIB_VERSION_MINOR, .patch = LIB_VERSION_PATCH};
    return version;
}

peace_of_map_t load_map(const char* image_filename, const char* config_filename) {
    peace_of_map_t map = {0};

    if(access(image_filename, F_OK) != 0) {
        printf("Failed to load map image file %s\n", image_filename);
        return map;
    }

    if(access(config_filename, F_OK) != 0) {
        printf("Failed to load config file %s\n", config_filename);
        return map;
    }

    unsigned int err = loadbmp_decode_file(image_filename, &map.image, &map.width, &map.height, LOADBMP_RGBA);
    if (err) {
		printf("LoadBMP Load Error: %u\n", err);
		map.image = NULL;
		return map;
    }
    
    FILE *fp;
    fp = fopen(config_filename, "r");
    if (fp == NULL) {
        printf("Failed to open config file %s\n", config_filename);
        free(map.image);
        map.image = NULL;
        return map;
    }
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
    int x = map->width * ((lon - map->top_left_lon) / (map->bottom_right_lon - map->top_left_lon));
    int y = map->height * ((map->top_left_lat - lat) / (map->top_left_lat - map->bottom_right_lat));
    if ((x < 0) || (x >= map->width) || (y < 0) || (y >= map->height)) {
        printf("Incorrect lat lon: %f %f\n", lat, lon);
        return;
    }
    

    for (int i = -5; i < 5; i++) {
        for (int j = -5; j < 5; j++) {
            if ((x + i < 0) || (x + i >= map->width) || (y + j < 0) || (y + j >= map->height)) {
                continue;
            }
            draw_pixel(map->image, map->width, x + i, y + j, r, g, b, 255);
        }
    }
}


stations_list_t load_stations(const char * stations_list_filename) {
    stations_list_t stations_list;
    stations_list.num_stations = 0;
    stations_list.stations = NULL;

    // Count entries
    FILE *fp; 
    fp = fopen(stations_list_filename, "r");
    if (fp == NULL) {
        printf("Failed to open stations file %s\n", stations_list_filename);
        return stations_list;
    }
    fscanf(fp, "%*[^\n]\n");
    while(!feof(fp))
    {
        fscanf(fp, "%*[^\n]\n");
        stations_list.num_stations++;
    }
    fclose(fp);

    // Read stations
    stations_list.stations = malloc(stations_list.num_stations * sizeof(station_t));
    fp = fopen(stations_list_filename, "r");
    if (fp == NULL) {
        printf("Failed to open stations file %s\n", stations_list_filename);
        free(stations_list.stations);
        stations_list.stations = NULL;
        stations_list.num_stations = 0;
        return stations_list;
    }
    fscanf(fp, "%*[^\n]\n");    
    for (int i = 0; i < stations_list.num_stations; i++)
    {
        char line[256];
        fgets(line, 256, fp);
        sscanf(line, "%d,%255[^,],%f,%f", &stations_list.stations[i].id,
                                    stations_list.stations[i].name,
                                    &stations_list.stations[i].lat,
                                    &stations_list.stations[i].lon);
    }
    fclose(fp);

    return stations_list;
}

float deg_to_rad(float deg) {
  return deg * (PI / 180);
}

float get_distance_in_km(float lat_1, float lon_1, float lat_2, float lon_2) {
    double d_lat = deg_to_rad(lat_2 - lat_1);
    double d_lon = deg_to_rad(lon_2 - lon_1);

    double a = sin(d_lat / 2) * sin(d_lat / 2) +
        cos(deg_to_rad(lat_1)) * cos(deg_to_rad(lat_2)) *
        sin(d_lon / 2) * sin(d_lon / 2);

    return 2 * EARTH_RADIUS_KM * asin(sqrt(a));
}

station_t get_nearest_station(stations_list_t *stations, float lat, float lon) {
    int idx_min = 0;
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

void free_map(peace_of_map_t *map) {
    if (map == NULL) return;
    free(map->image);
    map->image = NULL;
    map->width = 0;
    map->height = 0;
}

void free_stations(stations_list_t *stations) {
    if (stations == NULL) return;
    free(stations->stations);
    stations->stations = NULL;
    stations->num_stations = 0;
}