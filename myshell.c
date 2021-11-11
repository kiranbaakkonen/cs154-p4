#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

void printError(){
	char error_message[30] = "An error has occurred\n";
	write(STDOUT_FILENO, error_message, strlen(error_message));
}


void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void getArgs(char *cmd, char *argv[], int num_args){
	
	char *cpy = strdup(cmd);
	char *save3 = NULL;
	char *temp;
	temp = strtok_r(cpy, " \t>+", &save3);
	for(int i = 0; i < num_args; i++){
		argv[i] = temp;
		temp = strtok_r(NULL, " \t>+", &save3);
	}
}


int main(int argc, char *argv[]) 
{
	char cmd_buff[514];
	char *pinput;
	FILE *fp;
		
	if(argc == 2){
		char *filename = argv[1];
		fp = fopen(filename, "r");

		if(fp == NULL){
			printError();
			exit(0);
		}
	}
	else if(argc > 2){
		printError();
		exit(0);
	}

	while (1) {

		if(argc == 1){
			myPrint("myshell> ");
			pinput = fgets(cmd_buff, 514, stdin);
			if (!pinput) {
				exit(0);
	        	}
		}
		else if(argc == 2){
			
			if(!(fgets(cmd_buff, 514,fp))){
				exit(0);		
			}
			pinput = cmd_buff;
		}


		//pinput will be one line of input by this point
		char *line = strdup(pinput);
		char *save1 = NULL;
		char *temp = strtok_r(line, ";\n", &save1);
		
		//check to see if string is empty
		char* save4 = NULL;
		char *line1 = strdup(pinput);
		char *t = strtok_r(line1, " \n\t", &save4);
		if(t != NULL && argc == 2){
			write(STDOUT_FILENO, pinput, strlen(pinput));
		}
		
		while(temp != NULL){	

			int arrows = 0;
			int advanced = 0;
			// count > and check if advanced
			for(int l = 0; l < strlen(temp); l++){
				if(temp[l] == '>'){
					arrows++;
				}
				if(l < strlen(temp) - 1){
					if(temp[l +1] == '+' && arrows == 1){
						advanced = 1;
					}
				}
			}

			char *count = strdup(temp);
			char *save2 = NULL;
			char *tmp = strtok_r(count, " \t>+", &save2);
			int num_arg = 0;
			//counts the number of arguments
			while(tmp != NULL){
				num_arg++;
				tmp = strtok_r(NULL, " \t>+", &save2);
			}
			
			//print number of arguments
	//		char d[100];
	//		sprintf(d, "num_args = %d\n", num_arg);
	//		myPrint(d);
	
			if(num_arg > 0){

				char *args[num_arg + 1];
				getArgs(temp, args, num_arg);
				args[num_arg] = NULL;
				int leftarg = 0;	
			
				if(arrows > 0){
					char *dup = strdup(temp);
					char *right = NULL;
					char *left = strtok_r(dup, ">+", &right);
					
					char *countl = strdup(left);
					char *save6 = NULL;
					char *t = strtok_r(countl, " \t", &save6);
					while(t != NULL){
						leftarg++;
						t = strtok_r(NULL, " \t", &save6);
					}
				}

				//print out all arguments	
//				for(int i = 0; i < num_arg; i++){
//					if(args[i] == NULL){
//						myPrint(NULL);
//					}
//					char c[100];
//					sprintf(c, ":%s", args[i]);
//					myPrint(c);
//				}
//				myPrint("\n");
//				char v[100];
//				sprintf(v, "left = %d, arrows = %d, advanced = %d\n", leftarg, arrows, advanced);
//				myPrint(v);
//				myPrint("\n");
	
	
				if(strcmp(args[0], "exit") == 0){
					if(num_arg == 1 && arrows == 0){
						exit(0);
					}
					else{
						printError();
					}
				}
				else if(strcmp(args[0], "pwd") == 0){
					if(num_arg == 1 && arrows == 0){
						char cwd[100];	
						getcwd(cwd, 100);
						sprintf(cwd, "%s\n", cwd);
						myPrint(cwd);
					}
					else{
						printError();
					}
				}
				else if(strcmp(args[0], "cd") == 0){
						if(num_arg == 2 && arrows == 0){
							if(chdir(args[1]) < 0){
								printError();
								}
							}
						else if(num_arg == 1 && arrows == 0){
							chdir(getenv("HOME"));
						}
						else{
							printError();
						}
				}
				else{

					int status;
					int forkret = fork();
					if(forkret == 0){
					
						if(arrows > 1 || (arrows > 0 && num_arg - leftarg != 1)){
							printError();
							//myPrint("format error\n");
							exit(0);
						}
						else if(leftarg == 0 && arrows > 1){
							printError();
							exit(0);
						}	
						else if(arrows == 1 || advanced){
							//myPrint("r e d i r e c t i o n\n");
//							myPrint("to: ");
//							myPrint(args[leftarg]);
//							myPrint("\n");


							char *left[leftarg + 1];
							left[leftarg] = NULL;
							for(int k = 0; k < leftarg; k++){
								left[k] = args[k];
							}
							

							if(advanced){
								int out = open(args[leftarg], O_CREAT | O_RDWR, S_IRWXU);	
								if(out == -1){
									printError();
									close(out);
									exit(0);
								}								

								int cache = open("temp", O_CREAT | O_RDWR, S_IRWXU);
								if(cache == -1){
									printError();
									close(out);
									exit(0);
								}
								char buf[1];
								while(read(out, buf, 1) != 0){
									write(cache, buf, 1);
								}


								lseek(out, 0, SEEK_SET);
	

								dup2(out, STDOUT_FILENO);
								if(execvp(left[0], left) == -1){
									printError();
								}
								else{
									close(out);
									out = open(args[leftarg], O_APPEND | O_CREAT | O_RDWR, S_IRWXU);
									while(read(cache, buf, 1) != 0){
										write(STDOUT_FILENO, buf, 1);
									}
								}

								close(out);
								close(cache);								
								exit(0);
							}
							else{
								int out = open(args[leftarg], O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
								if(out < 0){
									printError();
									close(out);
									exit(0);
								}
								dup2(out, STDOUT_FILENO);
	
								if(execvp(left[0], left) == -1){
									printError();
									//myPrint("redirection error\n");
								}
								close(out);
							}
						}
						else{
							if(execvp(args[0], args) == -1){
								printError();
							}
						}
					
					exit(0);
					}
					else{
						wait(&status);
					}
				}
			}
			else if(arrows > 0){
				printError();
			}
			temp = strtok_r(NULL, ";\n", &save1);
		}
	}
        	myPrint(cmd_buff);
}
