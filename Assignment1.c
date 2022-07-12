#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include<errno.h>  
#include <sys/types.h>
#include <sys/stat.h> 
#include<sys/wait.h> 
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define BUFFER_SIZE 1024
char buff[BUFFER_SIZE];

void createFile(char *myFileName,char *typeOfFile);
void readFile(char *myFileName,char *typeOfFile);
void writeFile(char *myFileName,char *typeOfFile);
int moveFilePos(int fd, int position, int offset);
void inputForWrite(int fd);
void getInfo(char *myFileName);
void readWrite();



int main(int argc, char *argv[] )  { 
    int fd=-1;
    int mode;
    if(argc!=4){
        if(argc==2){
            if(strcmp(argv[1],"unnamed")==0){
                readWrite();
                return 0;
            }else{
                printf("Invalid type.\n");
                printf("Enter unnamed as your 2nd argument.");
                exit(0);
            }
        }else{
            printf("Enter the command in following format:\n");
            printf("FOR REGULAR FILE AND NAMED PIPES::\n");
            printf("argv[0]:for executing file.\n");
            printf("argv[1]:File name.\n");
            printf("argv[2]:Mode (0:creat, 1:read, 2:write, 3:info).\n");
            printf("argv[3]:Type of file (regular,named).\n\n");
            printf("FOR UNNAMED PIPE\n");
            printf("argv[0]:for executing file.\n");
            printf("argv[1]:unnamed");
            exit(0);
        }
    }else{
        printf("File name is: %s\n", argv[1]); 
        mode=atoi(argv[2]); 
        switch(mode){    
        case 0:     
        createFile(argv[1],argv[3]);
        break; 

        case 1:
        readFile(argv[1],argv[3]);  
        break;  

        case 2:
        writeFile(argv[1],argv[3]);
        break;

        case 3:
        getInfo(argv[1]);
        break;
            
        default:    
        printf("Specify a correct mode");    
        }     
    }
    
    return 0;
}  


void createFile(char *myFileName,char *typeOfFile){
    struct stat info;
    stat(myFileName,&info);
    if(strcmp(typeOfFile,"named")==0){
        if(mkfifo(myFileName,0777)<0){
            perror("Error:");
        }else
            printf("FIFO file created successfully!!");
    }else if(strcmp(typeOfFile,"regular")==0){
        if(S_ISFIFO(info.st_mode)!=0){
            printf("The file %s already exists as a FIFO file",myFileName);
        }else{
            int fd=open(myFileName,O_RDONLY);
            if (fd !=-1) 
            { 
                printf("File already exists \n");
                printf("Do you want to:\n1.Truncate the existing file.\n2.Abort.\n");
                int choice=0;
                scanf("%d", &choice);
                switch(choice){
                case 1:
                    creat(myFileName,S_IRUSR|S_IWUSR);
                    break;
                case 2:
                    exit(0);
                default:
                    printf("Invalid choice. Try again!!\n");
                    break;
                }                     
            } 
            else{
                if(creat(myFileName,S_IRUSR|S_IWUSR)==-1){
                    perror("Error");
                }else
                    printf("File %s created successfully .",myFileName);
            }
        }
    }else{
        printf("Invalid type.");
        exit(0);
    }
}

void readFile(char *myFileName,char *typeOfFile){
    struct stat info;
    stat(myFileName,&info);
    int fd;
    if(strcmp(typeOfFile,"regular")==0){
        if(S_ISREG(info.st_mode)==0){
            printf("There is no regular (%s) file.",myFileName);
            exit(0);
        }else{
            int position;
            int offset;
            int count;
            fd=open(myFileName,O_RDONLY);
            if(fd==-1){
                perror("Error:");
            }else{
                printf("Enter position: ");
                scanf("%d", &position);
                printf("Enter offset: ");
                scanf("%d", &offset);
                if (moveFilePos(fd, position, offset) == -1) {
                    printf("ERROR: failed to do lseek of position (%d), offset (%d)\n",position, offset);   
                }else{
                    printf("Number of bytes to read:");
                    scanf("%d",&count);
                    read(fd,buff,count);
                    printf("Read buffer:%s",buff);
                }
            }
        }
    }else if(strcmp(typeOfFile,"named")==0){
        if(S_ISFIFO(info.st_mode)==0){
            printf("There is no FIFO (%s) file..",myFileName);
            exit(0);
        }else{
            fd=open(myFileName,O_RDONLY);
            if(fd==-1){
                perror("Error:");
            }else{
                while(true){
                    int cnt=read(fd,buff,sizeof(buff));
                    if(cnt==0){
                        break;
                    }
                    printf("%s",buff);   
                }
            }
        }
    }else
        printf("Invalid type.");
        exit(0);
}
   

int moveFilePos(int fd, int position, int offset) {
  if (lseek(fd, position, SEEK_SET) == -1) {
    return -1;
  }
  if (lseek(fd, offset, SEEK_CUR) == -1) {
    return -1;
  }
  return 0;
}

void writeFile(char *myFileName,char *typeOfFile){
    struct stat info;
    stat(myFileName,&info);
    int fd;
    if(strcmp(typeOfFile,"regular")==0){
        if(S_ISREG(info.st_mode)==0){
            printf("There is no regular (%s) file.",myFileName);
            exit(0);
        }else{
            fd=open(myFileName,O_WRONLY);
            if(fd==-1){
                perror("Some error");
            }else{
                printf("Do you want to:\n1.Append to the existing file.\n2.Overwrite the existing file.\nCHOOSE ANY IF NAMED PIPE.\n");
                int choice=0;
                scanf("%d", &choice);
                switch(choice){
                case 1:
                    fd=open(myFileName,O_WRONLY|O_APPEND);
                    break;
                case 2:
                    fd=open(myFileName,O_WRONLY);
                    break;
                    
                default:
                    printf("Invalid choice. Try again!!\n");
                    exit(0);
                }
                getchar();
                printf("Enter content:\n");
                inputForWrite(fd);   
            }
        }
    }else if(strcmp(typeOfFile,"named")==0){
        if(S_ISFIFO(info.st_mode)==0){
            printf("There is no FIFO (%s) file..",myFileName);
            exit(0);
        }else{
            fd=open(myFileName,O_WRONLY);
            printf("Enter content:\n");
            inputForWrite(fd); 
        }
    }else
        printf("Invalid type.");
        exit(0);
}

void inputForWrite(int fd){
    while (!feof(stdin)) {
            char *input;
            if ((input = fgets(buff, sizeof(buff), stdin)) && input) {
                if (input[0] == '\n') {
                    break;
                 }
                 int x=write(fd, input, strlen(input)+1);
                 if(x==-1){
                     perror("Error:");
                 }
             }
        }   
}

void getInfo(char *myFileName){
  struct stat info;
  if (stat(myFileName, &info) == 0)
    {
        printf("inode:    %d\n",   (int) info.st_ino);
        printf("dev id:   %d\n",   (int) info.st_dev);
        printf("mode:     %d\n",   info.st_mode);
        printf("links:    %d\n",   info.st_nlink);
        printf("uid:      %d\n",   (int) info.st_uid);
        printf("gid:      %d\n",   (int) info.st_gid);
        printf("File Permissions: ");
        printf( (S_ISDIR(info.st_mode)) ? "d" : "_");
        printf( (info.st_mode & S_IRUSR) ? "r" : "-");
        printf( (info.st_mode & S_IWUSR) ? "w" : "-");
        printf( (info.st_mode & S_IXUSR) ? "x" : "-");
        printf( (info.st_mode & S_IRGRP) ? "r" : "-");
        printf( (info.st_mode & S_IWGRP) ? "w" : "-");
        printf( (info.st_mode & S_IXGRP) ? "x" : "-");
        printf( (info.st_mode & S_IROTH) ? "r" : "-");
        printf( (info.st_mode & S_IWOTH) ? "w" : "-");
        printf( (info.st_mode & S_IXOTH) ? "x" : "-");
        printf("\n\n");

    }
    else
    {
        printf("Unable to get file properties.\n");
        printf("Please check whether '%s' file exists.\n", myFileName);
    }
}

void readWrite(){
    int fd[2];
    int pid;
    if(pipe(fd)==-1){
        perror("Error");
    }
    int id=fork();
    if(id==0){
        close(fd[0]);
        printf("Writing from child process(pid:%d):",getpid());
        scanf("%s",buff);
        write(fd[1],buff,strlen(buff));
        close(fd[1]);
    }else{
        close(fd[1]);
        read(fd[0],buff,sizeof(buff));
        printf("Reading from parent process(pid:%d):%s",getpid(),buff);
        close(fd[0]);
    }
}


