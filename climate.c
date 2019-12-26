#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_STATES 50

struct climate_info {
	char code[3];					// state code
	unsigned long num_records;		// total number of records
	long double sum_humidity;		// sum of all of the humidity
	long double sum_temperature; 	// sum of all surface temperature
	double max_temp;				// max temperature
	time_t max_temp_time;			// the date of the maximum temperature
	double min_temp;				// min temperature
	time_t min_temp_time;			// the date of the minimum temperature
	int lightning;					// the total number of records with lightning
	int snow;						// the total number of records with snow
	long double sum_cloud; 			// the sum of all of the cloud coverage
};

void analyze_file(FILE *file, struct climate_info *states[], int num_states);
void print_report(struct climate_info *states[], int num_states);

int main(int argc, char *argv[]) {
	// if there are no files to be read in from the command line
	if (argc < 2) {
		printf("Usage: %s tdv_file1 tdv_file2 ... tdv_fileN \n", argv[0]);
		return EXIT_FAILURE;
	}

	// creates an array of struct pointers to store our state data (50 US states)
	struct climate_info *states[NUM_STATES] = { NULL }; 

	// for every file in the command line, read and add data to states array
	for (int i = 1; i < argc; ++i) {
		FILE* file = NULL; // initializing file pointer to null

		// opening file to read
		file = fopen(argv[i], "r");
		if(file == NULL){
			printf("Error opening file.\n");
			return EXIT_FAILURE;
		} else {
			printf("Opening file: %s\n", argv[i]);
		}

		// now need to analyze the file and add the information to the states array
		analyze_file(file, states, NUM_STATES);
		fclose(file); // closing file 
	}

	// prints the summary of the state data
	print_report(states, NUM_STATES);

	// must free all of the dynamically allocated memory before closing the program
	for (int i = 0; i < NUM_STATES; ++i) {
		if (states[i] != NULL) {
			free(states[i]);
			states[i] = NULL;
		}
	}
	return 0;
}

void analyze_file(FILE *file, struct climate_info **states, int num_states) {
	const int line_sz = 100;
	char line[line_sz];
	while (fgets(line, line_sz, file) != NULL) {
		char line_arr[9][20]; // array of the words in the line

		char * token = strtok(line, "\t");

		// while loop below copies all of the words into an array, to access each piece of information using index
		int i=0;
		while(token != NULL){
			strcpy(line_arr[i], token);
			i++;
			token = strtok(NULL , "\t");
		}

		int index = 0;
		while(states[index] != NULL && strcmp(states[index] -> code, line_arr[0]) != 0){ // while the code is not the state and the info is not null
			++index;
		}
		// at the end, the index is either at a null pointer meaning the state does not yet exist in our data, or is pointing to the correct struct

		struct climate_info *info = NULL; // pointer to easily manipulate the correct struct (initialized to null)
		if(states[index] == NULL){ // then we need to create struct for this state
			states[index] = (struct climate_info*) malloc(sizeof(struct climate_info)); // allocating space for our new struct in memory
			// if the allocation does not return any memory
			if(states[index] == NULL){
				printf("Error allocating memory\n");
				EXIT_FAILURE;
			}
			info = states[index]; // info points to the newly created struct
		

			// now must copy all of the information from the line into the struct
			strcpy(info -> code, line_arr[0]);
			info -> num_records = 1; // init to 1
			info -> sum_humidity = atof(line_arr[3]);
			info -> sum_temperature = (atof(line_arr[8]) * 1.8) - 459.67; // this and below is a conversion from kelvin
			info -> max_temp = (atof(line_arr[8]) * 1.8) - 459.67;
			info -> max_temp_time = atol(line_arr[1])/1000; 
			info -> min_temp = (atof(line_arr[8]) * 1.8) - 459.67;
			info -> min_temp_time = atol(line_arr[1])/1000;
			info -> lightning = atoi(line_arr[6]);
			info -> snow = atoi(line_arr[4]);
			info -> sum_cloud = atoi(line_arr[5]);

		} else {
			// now need to add onto the struct that already exists
			info = states[index]; // info points to the state's struct, which already has some data
			info -> num_records += 1; // increase by one
			info -> sum_humidity += atof(line_arr[3]); // this and below add values to the sums
			info -> sum_temperature += (atof(line_arr[8]) * 1.8) - 459.67; // converted from kelvin
			// must check for min / max
			if(info -> max_temp < (atof(line_arr[8]) * 1.8) - 459.67){
				// if the new temp is bigger than the previous max, then replace the value and date
				info -> max_temp = (atof(line_arr[8]) * 1.8) - 459.67;
				info -> max_temp_time = atol(line_arr[1])/1000;
			}
			if(info -> min_temp > (atof(line_arr[8]) * 1.8) - 459.67){
				// if the new temp is smaller than the previous min, then replace the value and date
				info -> min_temp = (atof(line_arr[8]) * 1.8) - 459.67;
				info -> min_temp_time = atol(line_arr[1])/1000;
			}
			info -> lightning += atoi(line_arr[6]);
			info -> snow += atoi(line_arr[4]);
			info -> sum_cloud += atoi(line_arr[5]);
		}

	}
}

void print_report(struct climate_info *states[], int num_states) {
	// prints the codes of all of the states found
	printf("States found: ");
	for (int i = 0; i < num_states; ++i) {
		if (states[i] != NULL) {
			struct climate_info *info = states[i];
			printf("%s ", info->code);
		}
	}
	printf("\n");

	// loop to print the individual information for each state struct
	int index = 0;
	while(states[index] != NULL){ // another way to loop through the states array to print the information
		struct climate_info *info = states[index];
		printf("-- State: %s --\n", info -> code); // prints the state code
		printf("Number of Records: %lu\n", info -> num_records); // total number of records for the state
		// the averages here, rounded, are calculated by dividinng the sum by the total number of records
		printf("Average Humidity: %.1Lf%%\n", (info -> sum_humidity)/(info -> num_records)); // average humidity
		printf("Average Temperature: %.1LfF\n", (info -> sum_temperature)/(info -> num_records)); // average temperature
		printf("Max Temperature: %.1fF\n", info -> max_temp); 
		printf("Max Temperature on: %s", ctime(&(info->max_temp_time))); // converts the time to a string date to be displayed
		printf("Min Temperature: %.1fF\n", info -> min_temp); 
		printf("Min Temperature on: %s", ctime(&(info->min_temp_time)));
		printf("Lightning Strikes: %d\n", info -> lightning); // total number of records with lightning
		printf("Records with Snow Cover: %d\n", info -> snow); // total number of records with snow
		printf("Average Cloud Cover: %.1Lf%%\n", (info -> sum_cloud)/(info -> num_records)); // average cloud coverage of all of the records
		index++;
	}

}
