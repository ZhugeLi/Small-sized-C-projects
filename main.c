/*
 *
 * main.c
 *
 *  Created on: 20 mai 2019
 *      Author: epalkoma
 */


#include "dirent.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "unistd.h"
#include "limits.h"

const char *get_filename_extension(const char* filename){
	const char* dot = strrchr(filename, '.');
	if (!dot || dot == filename) return "";
	return dot + 1;
}

void perform_simple_pattern_search (FILE *infile, FILE* outfile, char* argument_to_search_for, int size_of_argument){

	char* line = malloc(2000*sizeof(char));
	char *ptr_to_found_substring = NULL;
	char* prev_line_length = line;
	char result[200];

	// STORE THE POINTER TO THE BEGGINING OF THE LINE POINTER
	// THE LINE POINTER WILL BE INCREMENTED
	// SO WE NEED TO KNOW WHERE WE STARTED FROM
	char* prev_pointer = line;

	int line_index = 0;

	while (fgets(line, 2000, infile) != NULL)
	{
		line_index++;
		//if the searched substring has been found in the line
		while((ptr_to_found_substring = strstr(line,argument_to_search_for)) != NULL)
		{
			//subtract from ptr_to_found_string the prev_line_length to find the index in the line where the first character of the pattern is found
			snprintf(result, sizeof(result), "Found at line %d and column %d.\n",line_index,(ptr_to_found_substring-prev_line_length)+1);
			fputs(result, outfile);
			//INCREMENTS THE LINE POINTER
			//WILL HAVE TO RESET THE LINE POINTER TO THE ORIGINAL VALUE
			//IF NOT, THE PROGRAL WILL CRASH RANDOMLY DEPENDING ON WHICH MEMORY WE ACCESS
			line += (ptr_to_found_substring-line)+size_of_argument;
		}
		line = prev_pointer;
	}

	free(ptr_to_found_substring);
	free(line);
	free(prev_pointer);
	free(prev_line_length);
}

void write_columns_to_file(FILE* infile, FILE* outfile, int nr_of_columns, char** input_array_of_strings){
	// treat in a special way the first line
	// find the indeces of the columns to be searched
	int line_index = 0;
	char *line = malloc(2000*sizeof(char));

	int column_index = 1;
	int current_index_of_index_array = 0;
	int *col_index_array = malloc(nr_of_columns * sizeof(int));

	char* token = NULL;
	char *output_string = NULL;

	//checks regarding the state of the input file have to done before the call of this function
	while (fgets(line, 2000, infile) != NULL)
	{
		line_index++;
		//process the first line, i.e. the line with the column names
		if(line_index == 1){
			//tokenize the line: separator is ";"
			char header[200] = "";
			token = strtok(line, ";");
			while (token) {
				//check if the the token matches the column names to find
				for(int i = 0;i<nr_of_columns;i++)
				{
					//have to check if token is NULL
					//otherwise it will crash
					if(token != NULL && strcmp(input_array_of_strings[i],token) == 0){
						//store the column index
						strcat(header,"  ");
						strcat(header,token);
						col_index_array[current_index_of_index_array] = column_index;
						current_index_of_index_array++;
					}
				}
				column_index++;
				//get the next token
				token = strtok(NULL, ";");
				//have to check if token is NULL
				//otherwise it will crash
				if(token != NULL)
					//if the end if line is reached, and the searched string is the last
					//the strcmp will fail because the token will have a trailing "\n"...
					//this next line replaces the EOL char
					token[strcspn(token, "\n")] = '\0';
			}
			strcat(header,"\n");
			fputs(header, outfile);
		}
		else{
			column_index = 1;
			current_index_of_index_array = 0;

			output_string = malloc(200*sizeof(char));
			strcpy(output_string, "");

			token = strtok(line, ";");
			while (token)
			{

				if(col_index_array[current_index_of_index_array] == column_index)
				{
					current_index_of_index_array++;
					strcat(output_string,"   ");
					strcat(output_string, token);
				}
				column_index++;
				//get the next token
				token = strtok(NULL, ";");
			}
			strcat(output_string,"\n");
			fputs(output_string, outfile);
			free(output_string);
		}
	}

	free(line);

}

int main(int argc, char *argv[]){

	//this is to find the current directory of the source file
	char cwd[PATH_MAX];

	//doesn't seem to be possible for this to fail, EVER
	//it gets the current working direcotory, so in essence, it seems dumb to check for failure
	getcwd(cwd, sizeof(cwd));

	DIR *dir = NULL;
	FILE *infile = NULL;
	FILE *outfile = NULL;

	struct dirent *ent = NULL;

	//1st operation
	char *argument_to_search_for = NULL;
	int size_of_argument = 0;

	//2nd operation
	int nr_of_columns = 0;
	char **column_names = NULL;

	//there are 3 operation types
	// 1. simple value search in all csv files in a specific folder
	// 2. search the columns of a csv file and write them to an output file

	//test if there are any arguments provided
	if(argc > 1){

		int operation_type = atoi(argv[1]);

		switch(operation_type)
		{
		case 1:
			size_of_argument = atoi(argv[2]);
			argument_to_search_for = malloc(size_of_argument*sizeof(char));
			argument_to_search_for = argv[3];

			break;
		case 2:
			nr_of_columns = atoi(argv[2]);
			//first allocate memory for the number of strings
			column_names = malloc(nr_of_columns*sizeof(char*));

			for(int i = 1+2;i<=nr_of_columns+2;i++)
			{
				//allocate memory for each string
				column_names[i-3] = malloc(200*sizeof(char));
				strcpy(column_names[i-3],argv[i]);
			}
			break;

		default:
			break;
		}

		//open directory
		//don't continue if the directory doesn't exist
		if ((dir = opendir(cwd)) != NULL)
		{
			while ((ent = readdir(dir)) != NULL)
			{
				//if the extension of the file is csv
				if(strcmp(get_filename_extension(ent->d_name),"csv") == 0)
				{
					//print the filename
					printf ("\nNow processing %s ...\n\n", ent->d_name);

					//building the full path together with the file_name
					//this will be used in the "fopen" function
					getcwd(cwd, sizeof(cwd));
					strcat(cwd,"\\");
					strcat(cwd,ent->d_name);
					//printf("%s\n",cwd);


					if ((infile = fopen(cwd, "r")) == NULL)
						printf("Input file cannot be opened! \n");
					else{
						//loop over the lines of the file until the EOF
						switch(operation_type){
						case 1:
							if((outfile = fopen("file_with_searched_value.txt","w")) == NULL)
								printf("Output file cannot be opened! \n");
							else
							{
								perform_simple_pattern_search (infile, outfile, argument_to_search_for, size_of_argument);
								fclose(outfile);
							}
							fclose(infile);
							break;

						case 2:
							if((outfile = fopen("file_with_found_columns.txt","w")) == NULL)
								printf("Output file cannot be opened! \n");
							else
							{
								write_columns_to_file(infile, outfile, nr_of_columns, column_names);
								fclose(outfile);
							}
							break;

						default:
							break;
						}
					}
				}
			}
			closedir(dir);
		}
		else
		{
			printf("Directory path is incorrect!");
		}
	}
	else
		printf("No arguments provided!\n");

	free(argument_to_search_for);
	printf("\nReturned 0...\n");
	return 0;
}
