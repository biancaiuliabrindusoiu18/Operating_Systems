 #include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <libgen.h>


DIR *openDirectory(char *nume)
{
	DIR *dir;
	dir=opendir(nume);
	if(dir==NULL)
	{
		exit(-4);
	}
	return dir;
}

void writeInSnapshot(int file, struct stat file_info, struct dirent *entry)
{
	time_t curr_time=time(NULL);
	char *ora=ctime(&curr_time);
	
	if(write(file, "Timestamp:", strlen("Timestamp:"))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ora, strlen(ora))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, "Last modified:", strlen("Last modified:"))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ctime(&file_info.st_mtime), strlen(ctime(&file_info.st_mtime)))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	
	
	char buffer[500]="nume fisier:";
	strcat(buffer, entry->d_name);		
	if(write(file,buffer,strlen(buffer))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	
	sprintf(buffer,"\ndimensiune: %ld\n", file_info.st_size);
	if(write(file,buffer,strlen(buffer))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
					
	sprintf(buffer,"inode: %ld\n", file_info.st_ino);
	if(write(file,buffer,strlen(buffer))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	
	if(write(file, "Permissions: ",strlen("Permissions: "))==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((S_ISDIR(file_info.st_mode)) ? "d" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IRUSR) ? "r" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IWUSR) ? "w" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IXUSR) ? "x" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IRGRP) ? "r" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IWGRP) ? "w" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IXGRP) ? "x" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IROTH) ? "r" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IWOTH) ? "w" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	if(write(file, ((file_info.st_mode & S_IXOTH) ? "x" : "-"),1)==-1)
	{
		perror("eroare la scriere in snapshot\n");
		exit(-11);
	}
	
	
}

int checkCorrupt(char pathFile[1000], struct stat file_info)
{
	int corrupt=1;
	//1-corupt
	//0-consider clean
	if(file_info.st_mode & S_IRUSR) 
		corrupt=0;
	if(file_info.st_mode & S_IWUSR)  
		corrupt=0;
	if(file_info.st_mode & S_IXUSR)  
		corrupt=0;
	if(file_info.st_mode & S_IRGRP)  
		corrupt=0;
	if(file_info.st_mode & S_IWGRP)  
		corrupt=0;
	if(file_info.st_mode & S_IXGRP)  
		corrupt=0;
	if(file_info.st_mode & S_IROTH)  
		corrupt=0;
	if(file_info.st_mode & S_IWOTH) 
		corrupt=0;
	if(file_info.st_mode & S_IXOTH) 
		corrupt=0;
	
	return corrupt;
}

void mutareCorupt(char path[1000], char pathCorupt[1000])
{
	char newPath[2000];
	sprintf(newPath,"%s/%s", pathCorupt, basename(path));
	
	if(rename(path, newPath)!=0)
	{
		perror("nu s-a mutat fisierul\n");
		exit(-30);
	}
	else
	{
		printf("s-a mutat fisierul corupt\n");
	}

}






void checkDir(DIR *dir, char path[1000],char pathOut[1000], int k,  int out, char izolated[1000], int *countPericulos, char* script)
{
	
	struct dirent* entry;
	while((entry=readdir(dir))!=NULL)
	{
		if(strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0 )
		{
			continue;
		}
		
		
		char pathaux[1000];
		strcpy(pathaux, path);
		strcat(pathaux, "/");
		strcat(pathaux, entry->d_name);
		
		/*
		for(int i=0;i<k;i++)
		{
			printf("\t");
		}
		printf("--->%s\n", entry->d_name);
		*/
		//strict afisarea din primele taskuri :))))))
		
		
		struct stat file_info;
		if(lstat(pathaux, &file_info)==-1)
		{
			perror("eroare la lstat\n");
			exit(-11);
		}
		if(S_ISDIR(file_info.st_mode))         //its a directory
		{
			DIR *d=opendir(pathaux);
			if(d==NULL)
			{
				exit(-7);
			}
			checkDir(d, pathaux,pathOut,k+1,out,izolated,countPericulos, script);
		}
		else
		{
			if(S_ISREG(file_info.st_mode))           //its a file
			{
				
				if(checkCorrupt(pathaux, file_info))
				{
					//("e corrrrupt???\n");
					//(*countPericulos)++;
					
					pid_t pidNepot;
					
					pid_t pipefd[2];
					
					if(pipe(pipefd)<0)
					{
						perror("eroare pipe file descriptor\n");
						exit(-7);
					}
					
					if((pidNepot=fork())<0)
					{
						perror("eroare pid nepot\n");
						exit(-7);
					}
					else
					{
						if(pidNepot==0)
						{
							if(close(pipefd[0])!=0)
							{
								perror("eroare pipe1\n");
								exit(-9);
							}
							
							//inchid capat citire
							
							if(dup2(pipefd[1], 1)==-1)
							{
								perror("eroree pipe2\n");
								exit(-9);
							}
							execl("/bin/bash", "bin", script, pathaux,izolated, NULL);
							perror("Eroare la exec aiureaaa\n");
							exit(-1);
							
						}
						else
						{
							if(close(pipefd[1])!=0)
							{
								perror("eroare pipe3\n");
								exit(-9);
							}
							//inchid capat scriere
							
							char resultVerify[1000];
							resultVerify[strlen(resultVerify)-1]='\0';
							
							if(read(pipefd[0], resultVerify, 1000)==-1)
							{
								perror("eroare pipe4\n");
								exit(-9);
							}
							
							if(close(pipefd[0])!=0)
							{
								perror("eroare pipe5\n");
								exit(-9);
							}
							//inchid si capat citire aka cel folosit
							
							int status;
							pid_t pidWaitNepot=waitpid(pidNepot, &status,0);
							if(WIFEXITED(status))
							{
								int exit_status=WEXITSTATUS(status);
								printf("nepotul cu pid-ul %d s-a terminat cu status %d\n", pidNepot, exit_status);
								if(strcmp(resultVerify, "SAFE\n")==0)
								{
									printf("nu e corupt\n");
								}
								else
								{
									printf("e corupt\n");
									(*countPericulos)++;
									mutareCorupt(pathaux, izolated);
								}
							}
							else
							{
								perror("eroare iesire proces\n");
							}
						}
					}
					
					
				}
				else
				{
					if(strstr(entry->d_name, "_snapshot.txt"))
					{
						if(out==1)
						{
							remove(pathaux);
						}
						continue;
					}
					
					char numefis[100];
					strcpy(numefis, entry->d_name);
					
					char *aux=strtok(numefis, ".txt");
					strcpy(numefis, aux);
					
					strcat(numefis, "_snapshot.txt");
					
					char new_path[1000];
					if(out==1)
					{
						strcpy(new_path, pathOut);
					}
					else
					{
						strcpy(new_path, path);
					}
					
					
					strcat(new_path,"/");
					strcat(new_path, numefis);
					
					
					int file=open(new_path, O_CREAT  |O_RDWR, S_IWUSR| S_IRUSR);
					if(file==-1)
					{
						printf("nu s-a deschis fisierul\n");
						exit(-13);
					}
					
					
					
					struct stat info2;
					if(lstat(new_path, &info2)==-1)
					{
						perror("eroare la lstat\n");
						exit(-11);
					}
				
					if(info2.st_size==0)
					{
						writeInSnapshot(file, file_info, entry);
						close(file);
						//if the snapshot didnt exist just create
					}
					else
					{
						//check for changes
						int file2=open("tmp.txt", O_CREAT|O_TRUNC  |O_RDWR, S_IWUSR| S_IRUSR);
						if(file2==-1)
						{
							printf("tmp dechis prost\n");
						}
						writeInSnapshot(file2, file_info, entry);
						
						
						char c1, c2;
						ssize_t nr_bytes, nr_bytes2;
						
						lseek(file2, 0, SEEK_SET);
						
						
						while((nr_bytes=read(file,&c1, sizeof(char)))>0 && (nr_bytes2=read(file2,&c2, sizeof(char)))>0)
						{
							if (c1=='\n' && c2=='\n')
							{
								break;
							}
							if(c1=='\n')
							{
								while((nr_bytes=read(file2,&c2, sizeof(char)))>0)
								{
									if(c2=='\n')
									{
										break;
									}
								}
								break;
							}
							else
							{
								if(c2=='\n')
								{
									while((nr_bytes=read(file,&c1, sizeof(char)))>0)
									{
										if(c1=='\n')
										{
											break;
										}
									}
									break;
									}
							}
						}
						
						while((nr_bytes=read(file,&c1, sizeof(char)))>0 && (nr_bytes2=read(file2,&c2, sizeof(char)))>0)
						{	
							if(c1!=c2)
							{
								lseek(file, 0, SEEK_SET);
								file=open(new_path, O_CREAT|O_TRUNC  |O_RDWR, S_IWUSR| S_IRUSR);
								writeInSnapshot(file,file_info,entry);
								
								break;
							}
						}
						close(file2);
						remove("tmp.txt");
					}
					//this was to check for changes
					
				}
		}
	}


}
}



int main(int argc, char ** argv)
{
	if(argc>15 || argc<4)
	{
		exit(-3);
	}
	
	pid_t p[10];
	int countProcese=0;
	int cod=0;
	
	int start=0;
	int fisierOut=0;
	char pathOut[1000]="";
	
	
	char numeDir[10][1000];
					
	
	if(strcmp(argv[1], "-o")==0)
	{
		struct stat info;
		if(lstat(argv[2], &info)==-1)
		{
			perror("eroare la lstat\n");
			exit(-11);
		}
		if(S_ISDIR(info.st_mode))
		{
			start=3;
			fisierOut=1;
			strcpy(pathOut, argv[2]);
		}
		else
		{
			printf("nu am director output corect\n");
			return 0;
		}
	}
	else
	{
		start=1;
		fisierOut=0;
		printf("nu am director output\n");
	}
	
	if(strcmp(argv[start], "-s")!=0)
	{
		printf("nu am director de izolat\n");
		exit(-3);
	}
	else
	{
		struct stat info;
		if(lstat(argv[start+1], &info)==-1)
		{
			perror("eroare la lstat\n");
			exit(-11);
		}
		if(!S_ISDIR(info.st_mode))
		{
			printf("nu am director de izolat corect\n");
			exit(-3);
		}
	}
	start=start+2;
	
	for(int i=start;i<argc;i++)
	{
		struct stat file_info;
		if(lstat(argv[i], &file_info)==-1)
		{
			perror("eroare la lstat\n");
			exit(-11);
		}
		if(S_ISDIR(file_info.st_mode))
		{
			strcpy(numeDir[countProcese], argv[i]);
			
			p[countProcese]=fork();
			if(p[countProcese]<0)
			{
				printf("pid aiurea\n");
				exit(-1000);
			}
			else
			{
				
				if(p[countProcese]==0)
				{
					DIR *dir=openDirectory(argv[i]);
					//printf("%s\n", argv[i]);
					//afisare de care nu mai am nevoie
					if(fisierOut==0)
					{
						strcpy(pathOut, argv[i]);
					}
					char path[1000]="";
					strcpy(path, argv[i]);
					
					int countPericol=0;
					checkDir(dir, path,pathOut,1,fisierOut,argv[start-1],&countPericol, "verify_malicious.sh");
					
					printf("\tin directorul %s s-au gasit %d fisiere posibile corupte\n",argv[i],countPericol); 
					
					
					exit(cod);
				}
				else
				{
						
				}
			}
			
			countProcese++;
			}
			
		}
		
	
	
	
	for(int i=0;i<countProcese;i++)
	{
		int status;
		pid_t pid=waitpid(0, &status, 0);
		
		if(WIFEXITED(status))
		{
			int exit_status=WEXITSTATUS(status);
			printf("\tstatus iesire proces %d cu pid-ul %d (nume director verificat:%s) este:%d\n",i,pid,numeDir[i], exit_status);
		}
		else
		{
			printf("\tprocesul %d cu pid-ul %d s-a incheiat prost :(\n", i, pid);
		}
		
	}

	
	return 0;
	
	
}
