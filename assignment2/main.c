/****************************************************************************************************************************************************************************************************************************************uthor: Connor Bentzley
 *  * ONID: 933-285-315
 *   * Assignment 2: Files & Directories
 *    * CS 344 Operating Systems I
 *     * *************************************************************************************************************************************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>

struct movie {
   char *title;
   char *year;
   struct movie *next;
};

//Reused the same functions I coded for the last assignment. I excluded some node variables since we only needed to access title and year
struct movie *createMovie(char *current) {
   struct movie *currentMovie = malloc(sizeof(struct movie));
   char *saveptr;
   
   //Read and save title
   char * token = strtok_r(current, ",", &saveptr);
   currentMovie->title = calloc(strlen(token) + 1, sizeof(char));
   strcpy(currentMovie->title, token);

   //Read and save year
   token = strtok_r(NULL, ",", &saveptr);
   currentMovie->year = calloc(strlen(token) + 1, sizeof(char));
   strcpy(currentMovie->year, token);

   //Preventing seg fault
   currentMovie->next = NULL;

   return currentMovie;
}

//Copied from assignment 1 & the students linked list assignment
struct movie *readMovies(char *filePath) {
   FILE *movieFile = fopen(filePath, "r");
   char *current = NULL;
   struct movie *head = NULL;
   struct movie *tail = NULL;
   size_t len = 0;
   ssize_t nread;

   //Skip the header line
   nread = getline(&current, &len, movieFile);

   //Read file and create linked list
   while((nread = getline(&current, &len, movieFile)) != -1) {
      struct movie *newNode = createMovie(current);
      if (head == NULL) {
	 head = newNode;
	 tail = newNode;
      }else {
	 tail->next = newNode;
	 tail = newNode;
      }
   }

   fclose(movieFile);
   return head;
}


void processFile(char *fileName) {
   //Seeding for random number for directory name
   srand(time(NULL));
   int random = (rand() % 100000); //Getting random number between 0 and 99999

   char dirName[50];
   char newFilename[100]; 
   int fileDescriptor;

   //add the random number to bentzlec.movies. prefix and create a directory with that name
   sprintf(dirName, "bentzlec.movies.%d", random);
   mkdir(dirName, 0750);
   printf("Created directory with name %s\n", dirName);

   //Reused code from last assignment to make a linked list of the movies read from file
   struct movie *moviesList = readMovies(fileName);
   //Iterate through the linked list and concatenated the strings to get the proper file path
   while(moviesList != NULL) {
      strcpy(newFilename, dirName);
      strcat(newFilename, "/");
      strcat(newFilename, moviesList->year);
      strcat(newFilename, ".txt");
      
      //make a file with the concatenated file path
      fileDescriptor = open(newFilename, O_WRONLY | O_APPEND | O_CREAT, 0640);
      if (fileDescriptor == -1) {
	 printf("Failed to create or append file name: %s\n", newFilename);
      }else {
	 write(fileDescriptor, moviesList->title, strlen(moviesList->title));
	 write(fileDescriptor, "\n", strlen("\n"));
      }

      close(fileDescriptor);
      moviesList = moviesList->next;
   }
}

int findChosenfile() {
   //Grab current directory
   DIR* currDir = opendir(".");
   struct dirent *aDir;
   struct stat dirStat;
   char chosenFilename[300];
   int found = 0;

   //Get file name from user
   printf("Enter the complete file name: ");
   scanf("%s", &chosenFilename);
   printf("\n");

   //Check directory to see if file name is present
   while((aDir = readdir(currDir)) != NULL) {
      if(strcmp(chosenFilename, aDir->d_name) == 0) {
	    found = 1;
	    break;
      }
   }

   if(found == 1) {
      printf("Now processing chosen file %s\n", chosenFilename);
      processFile(chosenFilename);
   }

   return found;
}

void findSmallestfile() {
   DIR* currDir = opendir("."); 
   struct dirent *aDir;
   struct stat dirStat;
   char fileName[300]; 
   int fileSize;
   int smallestFilesize = 100000000; //set this to a huge size so it will get replaced
   const char dot = '.';
   char* extension; 

   //Iterate through current directory
   while((aDir = readdir(currDir)) != NULL) {
      //Check each file to see if it has the prefix
      if(strncmp("movies_", aDir->d_name, strlen("movies_")) == 0) {
	 //If the file has the prefix, check the file extension after the dot
	 extension = strchr(aDir->d_name, dot);
	 if(strcmp(extension, ".csv") == 0) {
	    //Get file size and check it against the current smallest
	    stat(aDir->d_name, &dirStat);
	    fileSize = dirStat.st_size;
	    if(fileSize < smallestFilesize) {
	       strcpy(fileName, aDir->d_name);
	       smallestFilesize = fileSize;
	    }
	 }
      }
   }

   //If the file size hasn't changed then there aren't any good files
   if(smallestFilesize == 100000000) {
      printf("No files with prefix movies_ and extension .csv were found\n");
   }else {
      printf("Now processing smallest file %s\n", fileName);
      processFile(fileName);
   }

   closedir(currDir);
}

//Works exactly the same as the last function, the filesize comparison is just backwards
void findLargestfile() {
   DIR* currDir = opendir(".");
   struct dirent *aDir;
   struct stat dirStat;
   char fileName[300];
   int fileSize;
   int largestFilesize = -1; //Set really small so these variable changes
   const char dot = '.';
   char *extension;


   while((aDir = readdir(currDir)) != NULL) {
      if(strncmp("movies_", aDir->d_name, strlen("movies_")) == 0) {
	 extension = strchr(aDir->d_name, dot);
	 if(strcmp(extension, ".csv") == 0) {
	       stat(aDir->d_name, &dirStat);
	       fileSize = dirStat.st_size;
	       if(fileSize > largestFilesize) {
		  strcpy(fileName, aDir->d_name);
		  largestFilesize = fileSize;
	       }
	 }
      }
   }

   if(largestFilesize == -1) {
      printf("No files with prefix movies_ and extension .csv were found\n");
   }else {
      printf("Now processing largest file %s\n", fileName);
      processFile(fileName);
   }

   closedir(currDir);
}

int main() {
   int choiceA, choiceB;
   int fileFound = 0;
   //Infinited while loop, I used a break when the user chooses to quit.
   while(1) {
      printf("1. Select file to process\n2. Exit the program\nEnter a choice 1 or 2: ");
      scanf("%i", &choiceA);
      printf("\n");
      if(choiceA == 1) {
	 while(fileFound == 0) {
	    printf("Which file do you want to process?\nEnter 1 to pick the largest file\nEnter 2 to pick the smallest file\nEnter 3 to specify the name of a file\nEnter a choice from 1 to 3: ");
	    scanf("%i", &choiceB);
	    printf("\n");
	    if(choiceB == 1) {
	       findLargestfile(); 
	       fileFound = 1;
	       choiceA = 2;
	    }else if(choiceB == 2) {
	       findSmallestfile();
	       fileFound = 1;
	       choiceA = 2;
	    }else if(choiceB == 3) {
	       fileFound = findChosenfile();
	       choiceA = 2;
	       //If the chosen file name is not found print this message, the function will not process it, it handles that itself
	       if(fileFound == 0) {
		  printf("File not found, please try again!\n");
	       }
	    }else {
	       printf("Error!\n");
	    }
	 }
	 fileFound = 0;
      }else if(choiceA == 2) {
	 break;
      }else if((choiceA != 1) && (choiceA != 2)){
	 printf("Error! Please input 1 or 2\n");
      }
   }

   return 0;
}
