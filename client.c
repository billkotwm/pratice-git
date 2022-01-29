#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <fcntl.h>
#include<unistd.h>
#include<string.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
  int error_check, count_file, i, file_fd, get_cmd;
  struct dirent *dir, *entry, **entry_list;
  FILE *file_dir;
  char cmd[256],file_name[256],full_pathname[256];
  size_t ret;
  char  *ip_addr=NULL, *str_portno=NULL;
  char * input=":";
  char *client_path="./client_dir/";

  if (0 != access(client_path, F_OK)) 
  { 
    if (ENOENT == errno) 
    {
     // does not exist
      mkdir(client_path,0755);
    }
  }

    ip_addr = strtok(argv[1], input);
    str_portno = strtok(NULL, ":");
    //printf("%s %s \n", ip_addr, str_portno);

    //socket for client
    int connection_socket;
    connection_socket=socket(AF_INET,SOCK_STREAM,0);

    //Server address for the socket to connect
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    //server_address.sin_port = htons(8074);
    server_address.sin_port = htons(atoi(str_portno));
    //server_address.sin_addr.s_addr =INADDR_ANY;
    server_address.sin_addr.s_addr = inet_addr(ip_addr);

    int connection_status=connect(connection_socket,(struct sockaddr*)&server_address,sizeof(server_address));
    if (connection_status == -1)
		  perror("error, in connecting with the server\n");

    char connection_msg[256], username[16];
    char buf[256];
    fd_set descp_set;

    printf("input your username:\n");
    //scanf("input your username:\n %s", username);
    fgets(username,sizeof(username),stdin);
    //printf("name %s please \n",username);
    error_check=send(connection_socket,&username,sizeof(username),0);
    error_check=recv(connection_socket,&connection_msg,sizeof(connection_msg),0);
    //printf("name %s please end \n",connection_msg);
    while (strncmp(connection_msg,"username",8)==0)
    {
      printf("username is in used, please try another:\n");
      //scanf("input your username: \n %s", username);
      fgets(username,sizeof(username),stdin);
      error_check=send(connection_socket,&username,sizeof(username),0);
      error_check=recv(connection_socket,&connection_msg,sizeof(connection_msg),0);
    }
    printf("%s\n",connection_msg);
    while(1)
    {
      FD_ZERO(&descp_set);//to remove all file descriptors from the set
		  FD_SET(0,&descp_set);//setting the bit of the standard input descriptor
		  FD_SET(connection_socket,&descp_set);//setting the bit for the connection socket from the client
      error_check=select(5,&descp_set,0,0,0);//monitoring multiple descriptors and waiting until anyone of the descriptors in the set is ready to be read for input in this case
      if(error_check==-1)
        {
          perror("error in select operation");
          close(connection_socket);
          return 0;
        }
        else if(FD_ISSET(0,&descp_set))//checking if standard input could be read
          {
            fgets(connection_msg,sizeof(connection_msg),stdin);//get the message to give to the client from the terminal
            system("clear");
            get_cmd=0;
            if(strncmp(connection_msg,"bye",3)==0)
            {
             //   strcat (connection_msg, "");
              //  strcat (connection_msg, newuser);
                error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);
                close(connection_socket);
                return 0;
            }

            if(strncmp(connection_msg,"ls",2)==0)
            {
              printf("%s\n", connection_msg);
              error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);
              //printf("cli-ls stdin %s\n", connection_msg);
              get_cmd=2;
            }

            if(strncmp(connection_msg,"get",3)==0)
            { 
              strcpy(file_name,"");
              sscanf(connection_msg, "%s %s", cmd, file_name);  
              //printf(" %s Command format error 1\n", file_name);
              if (file_name[0]==0)
              {
                printf("Command format error \n");
              }
              else 
              {
                connection_msg[0]='G';
                error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);
              //printf("cli-get %s\n", connection_msg);
              }
              get_cmd=3;
            }

            if(strncmp(connection_msg,"put",3)==0)
            { 
              strcpy(file_name,"");
              sscanf(connection_msg, "%s %s", cmd, file_name);  
              strcpy(full_pathname,client_path);
              strcat(full_pathname, file_name);

              if (file_name[0]==0)
              {
                printf("Command format error \n");
              }
              else 
              {
                if( (file_dir = fopen (full_pathname, "r")) ==NULL )
	              {
                printf("The %s doesn’t exist\n",file_name );
                } 
                else
                {
                fclose(file_dir);
                connection_msg[0]='P';
                error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);
                }
              }
              get_cmd=4;
            }
            //printf("get cmd %d\n", get_cmd);
            if(get_cmd <1) printf("Command not found\n");
            //printf("get cmd %d %d end\n", get_cmd, get_cmd <1);
          }
          else if(FD_ISSET(connection_socket,&descp_set))
          {
              error_check=recv(connection_socket,&connection_msg,sizeof(connection_msg),0);
              //printf("%s\n",connection_msg);
              if(strncmp(connection_msg,"ls",2)==0)
              {
                printf("%s\n",connection_msg);
                //error_check=recv(connection_socket,&buf,sizeof(buf),0);
                if(error_check==-1)
                {
                  perror("error msg:msg wasnt recieved from server\n");
                  return 0;
                }
                printf("%s\n",connection_msg);
              }

              if(strncmp(connection_msg,"LS:",3)==0)
              {
                sscanf(connection_msg, "%s %s", cmd, file_name);  
                printf("%s\n",file_name);
              }

              if(strncmp(connection_msg,"Get",3)==0)
              {
                printf("%s\n",connection_msg);
                //error_check=recv(connection_socket,&connection_msg,sizeof(connection_msg),0);
                //printf("1 %s cmd get 1\n",connection_msg);
                //error_check=recv(connection_socket,&buf,sizeof(buf),0);
                //printf("2 %s get 2\n",connection_msg);
                sscanf(connection_msg, "%s %s", cmd, file_name);
                strcpy(full_pathname,client_path);
                strcat(full_pathname, file_name);
                if( (file_dir = fopen (full_pathname, "w+")) < 0 )
	              {
		              perror("failed to open input file\n");
		              return 1;
                } 

			          do
			          {
                  error_check=recv(connection_socket,&buf,sizeof(buf),0);
				          if (error_check >0) 
                    ret = fwrite(buf ,sizeof(char), error_check, file_dir); 
				          //write(dev_fd, buf, ret);//write to the the device
                  //printf("3 buf %s %d %d 3\n",buf, error_check, (int)sizeof(buf));

			          }while( error_check==256);
                //printf("end buf %s %d\n",buf, (int) ret);
                printf("get %s successfully\n",file_name);
              fclose(file_dir);

              }

            if(strncmp(connection_msg,"Put",3)==0)
            { 
              printf("%s\n",connection_msg);
              sscanf(connection_msg, "%s %s", cmd, file_name);
              strcpy(full_pathname, client_path);
              strcat(full_pathname, file_name);

              if( (file_dir = fopen (full_pathname, "r")) ==NULL )
	            {
		            //perror("failed to open input file\n");
                strcpy(file_name,"");
		            sscanf(connection_msg, "%s %s", cmd, file_name);
                printf("The %s doesn’t exist\n",file_name );
              } 
              else
              {
			          do
			          {
				          ret = fread(buf ,sizeof(char), 256, file_dir); // read from the input file
                  //printf("cli-put %s %d\n",buf, (int) ret);
                  error_check=send(connection_socket,&buf,ret,0);
			          }while(ret ==256);
              //printf("end cli-buf %s %d put\n",buf, (int) ret);
                fclose(file_dir);
              //error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);
                //printf("end put %s\n",connection_msg);
             }
            }

            if(strncmp(connection_msg,"put",3)==0)
            {
                //error_check=recv(connection_socket,&connection_msg,sizeof(connection_msg),0);
                //sscanf(connection_msg, "%s %s", cmd, file_name);
                printf("%s\n",connection_msg);
            }

            if(strncmp(connection_msg,"Err",3)==0)
            {
                //error_check=recv(connection_socket,&connection_msg,sizeof(connection_msg),0);
                sscanf(connection_msg, "%s %s", cmd, file_name);
                printf("The %s doesn’t exist\n",file_name );
            }

            if(strncmp(connection_msg,"bye",3)==0)
            {
                //printf("ser-bye: %s\n", connection_msg);
                close(connection_socket);
                return 0;
            }
          }
    }
    close(connection_socket);
    return 0;
}