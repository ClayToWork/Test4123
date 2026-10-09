/* ==================================================================
   Stationmapper library.
   This library performs basic operations with bus stations geodata.
   ================================================================== */

#ifndef stationmapper_h__
#define stationmapper_h__

/* ==================  Data types  ================================== */

// Map binary image with related metadata 
typedef struct {
    unsigned char *image;
    int width;
    int height;
    float top_left_lat;
    float top_left_lon;
    float bottom_right_lat;
    float bottom_right_lon;
} peace_of_map_t;


// Bus station entry. Container for the bus station properties.
typedef struct {
    int id;
    unsigned char name[256];
    float lon;
    float lat;
} station_t;


// List of the bus stations.
typedef struct {
    station_t *stations;
    int num_stations;
} stations_list_t;


// Structure for the library version information
typedef struct {
    int major;
    int minor;
    int patch;
} version_t;



/* ===================  Functions  ================================== */

// Return current library version.
const version_t get_library_version(void);


// Load map image (BMP) linked to the geographical coordinates.
// Linkage information is loaded from config file.
peace_of_map_t load_map(const char *image_filename, const char *config_filename);


// Save map as bmp image.
int save_map(const peace_of_map_t *map, const char *output_filename);


// Load stations list from csv file.
stations_list_t load_stations(const char *stations_list_filename);


// Calculate distance in km between to geographical points
// described by latitude and longitude.
float get_distance_in_km(float lat_1, float lon_1, float lat_2, float lon_2);


// Draw point (square) specified by latitude and longitude on the map.
// The color of the point can be specified.
void draw_point_by_lat_lon(peace_of_map_t *map, float lat, float lon, int r, int g, int b);


// Search nearest station from the given stations list to the specified geographical point.
station_t get_nearest_station(stations_list_t *stations, float lat, float lon);
 
#endif  // stationmapper_h__