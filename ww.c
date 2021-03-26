#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>

typedef struct{
  size_t length;
  size_t used;
  char *data;
} arraylist_t;

int wrap(unsigned int width, int input_fd, int output_fd);

int al_init(arraylist_t *L, size_t length)
{
  L->data = malloc(sizeof(char) * length);
  if (!L->data) return 1;
  
  L->length = length;
  L->used   = 0;
  
  return 0;
}


void al_destroy(arraylist_t *L)
{
  free(L->data);
}


int al_append(arraylist_t *L, char item)
{
  if (L->used == L->length) {
      size_t size = L->length * 2;
      char *p = realloc(L->data, sizeof(char) * size);
      if (!p) return 1;
      
      L->data = p;
      L->length = size;
  }

  L->data[L->used] = item;
  ++L->used;
  
  return 0;
}


//for part 1 return 0 if success -1 if error
int openFile (char* name){
  int file = open(name, O_RDONLY);
  if (file == -1){
    perror("Error: ");
    return -1;
  }
  return file;
}


//returns 0 for file 1 for dir and -1 for error and other types
int isfile(const char* name){
  struct stat data;

  int err = stat(name, &data);
  //printf("err: %d\n", err);
  //for error
  if(err){
    perror(name);
    return -1;
  }
  //for dir
  if(S_ISREG(data.st_mode)){
    return 1;
  }
  return 0;
}

int wrapDirs(char *dirName, unsigned int width){
    struct dirent *dir;
    //printf("%s\n", dirName);
    DIR *d = opendir(dirName);
    //printf("idk: %s\n",getcwd("test1.txt"));
    chdir(dirName);
    int fileNameLength;
    
    if (d){

      while((dir = readdir(d)) != NULL){
        //printf("LOOP: ");
        //printf("%s\n", dir->d_name);
        fileNameLength = strlen(dir->d_name);
        char *fileName = malloc(sizeof(char)*fileNameLength+1);
        strcpy(fileName, dir->d_name);
        if(isfile(fileName)){
          //printf(fileName);

          
          if(!(strncmp(".",fileName, 1) == 0 || strncmp("wrap.",fileName,5) == 0)){
            //printf("GOOD NAME^^\n");
            
            int fd = open(fileName, O_RDONLY);
            char *wrapName = malloc(sizeof(char)*6+fileNameLength);
            strcpy(wrapName, "wrap.");
            
            
            strcat(wrapName, fileName);
            //printf("WRAPNAME: %s\n", wrapName);

            int newfd = open(wrapName, O_WRONLY|O_TRUNC|O_CREAT, 0666);
            wrap(width, fd, newfd);
            free(wrapName);
            close(fd);
            close(newfd);
          }
	  
        }
        free(fileName);
        
      }

      closedir(d);
    }
    return 0;
}




//returns 0 for file 1 for dir and -1 for error and other types
int isdir(const char* name){
  struct stat data;

  int err = stat(name, &data);
  //for error
  if(err){
    perror(name);
    return -1;
  }
  //for dir
  if(S_ISDIR(data.st_mode)){
    return 1;
  }
  return 0;
}

//this function wraps
int wrap(unsigned int width, int input_fd, int output_fd){
  int bytes_read = 1;
  char buf[10];
  unsigned int total_used = 0;
  unsigned int success = 0;
  unsigned int newline;
  unsigned int space;

  unsigned int first = 1;

  arraylist_t arr;
  al_init(&arr, 10);
  
  while((bytes_read = read(input_fd, &buf, 10)) > 0){
    for(int i = 0; i < sizeof(char) * bytes_read; i++){
      
      if (isspace(buf[i])){
	if(buf[i] == '\n'){
	  newline++;
	}
	//first space introduced after letter
	if(space == 0){
	  space = 1;
	  
	  //case of previous words existing in line
	  if(total_used > 0){
	    total_used += arr.used+1;
	  }

	  //total_used goes out of width or just new word goes out of width
	  //write word to new line and set total_used to word's length
	  if(total_used > width || arr.used > width){
	    if (total_used > 0){
	      write(output_fd, "\n", sizeof(char));
	    }
	    write(output_fd, arr.data, sizeof(char)*arr.used);
	    total_used = arr.used;
	    
	    
	    if(arr.used > width){
	      success = -1;
	    }
	    arr.used = 0;
	    first = 0;
	  }
	  
	  //fits inside width
	  else{
	    if(total_used > 0){ 
	      write(output_fd, " ", sizeof(char));
	    }
	    //if there is no previous words
	    else{
	      total_used = arr.used;
	    }
	    write(output_fd, arr.data, sizeof(char) * arr.used);
	    arr.used = 0;
	    first = 0;
	  }
	}
      }

      //nonspace
      else{
	space = 0;
	if (newline >= 2 && first == 0){
	  write(output_fd, "\n\n", sizeof(char) * 2);
	  total_used = 0;
	}
	newline = 0;
	al_append(&arr, buf[i]);
      }
    }
  }

  write(output_fd, "\n", sizeof(char));
  al_destroy(&arr);

  return success;
}





int main (int argc, char* argv[]){

  assert(!(argc==1));


  //file descriptor
  unsigned int width = atoi(argv[1]);
  int success;
  
  assert(width > 0);
  
  //stdin
  if(argc == 2){
    success = wrap(width, 0, 1);
  }

  //dir
  else if(isdir(argv[2])){
    // printf("dir\n");
    char *dirName = argv[2];
    
    wrapDirs(dirName, width);

  }
  

  //file
  else{
    int fd = openFile(argv[2]);
    if (fd == -1){
      return EXIT_FAILURE;
    }
    
    success = wrap(width, fd, 1);
    close(fd);
  }

  if (success == -1){
    //printf("\nfailure\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
  
  
