#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <fcntl.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
//#include<arpa/inet.h>

int server_process(int con_socket);
int check_add_user(char *); 
int check_del_user(char *); 

char username[5][8], newuser[16];
int users, max_users=5;


int main(int argc, char *argv[])
{
    int error_check, error_user_check, pid;
    

    //creating server socket
    users=0;
    int server_socket;
    int server_port;

    server_socket=socket(AF_INET,SOCK_STREAM,0);
    if(server_socket==-1)
        perror("error in socket creation");

    //defining server's address
    struct sockaddr_in server_address;
    server_address.sin_family=AF_INET;          
    //server_address.sin_port=htons(7001);        //port no here
    server_address.sin_port=htons(atoi(argv[1])); 
    //server_address.sin_addr.s_addr=INADDR_ANY;  
    server_address.sin_addr.s_addr=INADDR_ANY; 

    //binding the socket to IP & port
    error_check=bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address));
    if (error_check==-1)
        perror("error in binding");

    //listening for client sockets, allow up to 5
    error_check=listen(server_socket,5);
    if(error_check==-1)	
        perror("error in listening\n");

    //fetching client's address to establish connection
    struct sockaddr_in client_address;
    socklen_t client_len =sizeof(client_address);  
    int connection_socket;  
    char connection_msg[256]; 
    
    while(1)
    {
        connection_socket=accept(server_socket,(struct sockaddr*)&client_address,&client_len);
        if(connection_socket==-1)
            perror("error in establishing connection with the client\n");

        bzero(username,40); 
        bzero(newuser,8);       
        if(recv(connection_socket,newuser,sizeof(newuser),0)>0)
            error_user_check=check_add_user(newuser); 
        if (error_user_check==-1)
            printf("username is %s \n", newuser);
        while (error_user_check ==-1)
        {
            strcpy(connection_msg, "username is in used, please try another:"); 
            //sscanf(con_msg, "%s %s", cmd, file_name);
            //printf("username is in used, please try another:");
            error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);
            if(recv(connection_socket,newuser,sizeof(newuser),0)>0)
               error_user_check=check_add_user(newuser);  
            //printf("username is %s in used %d 2\n", newuser, error_user_check);
        }
        //printf("username is %s in used 3end\n", newuser);
        strcpy(connection_msg, "connect successfully"); 
        error_check=send(connection_socket,&connection_msg,sizeof(connection_msg),0);

    // Create child process 
        pid = fork();	
        if (pid < 0)
        {
            perror("ERROR on fork");
            exit(1);
        }
        if (pid == 0) 
        {
    // Client process
        close(server_socket);
        //printf("username is %s end\n", newuser);
        server_process(connection_socket);
        check_del_user(newuser);
        printf("username %s logout\n", newuser);
        exit(0);
        }
        else 
       {
         close(connection_socket);
       }
    }
}
    
int server_process(int con_socket) 
{
    int n, err_check, count_file, i, file_fd;
    FILE *file_dir;
    struct dirent *dir, *entry, **entry_list;
    char cmd[256], file_name[256], putname[16];
    size_t ret;
    char buffer[256];
    bzero(buffer,256);
    char con_msg[256];
    char buf[256],buf1[256],buf2[256];
    char cwd[256];
    fd_set descp_set;
    while(1)
    {
        FD_ZERO(&descp_set);//to remove all file descriptors from the set
		FD_SET(0,&descp_set);//setting the bit of the standard input descriptor
		FD_SET(con_socket,&descp_set);//setting the bit for the connection socket from the client
        err_check=select(5,&descp_set,0,0,0);//monitoring multiple descriptors and waiting until anyone of the descriptors in the set is ready to be read for input in this case
        if(err_check==-1){
            printf("error in select operation");
            close(con_socket);
            return 0;
        }
        //now finding who is ready for input ; is it the stdin or the connection(client) socket

        //select() operation will clear fds of all descriptors other than the ones who are ready to be read

        //checking if standard input could be read
        else if(FD_ISSET(0,&descp_set)){
            fgets(con_msg,sizeof(con_msg),stdin);//get the message to give to the client from the terminal
            if(strncmp(con_msg,"bye",3)==0)
            {
                return 0;
            }

            err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
            /*similar syntax to recv first argument is the connection socket(client) ;second argument is the variable in which the message received from client is to be stored
            third argument is the size of the received message;0 is a flag*/
            if(err_check==-1){
                perror("error msg: error in sending message to client\n\n");
                return 0;
            }

        }
        //checking if connection socket could be read
        else if(FD_ISSET(con_socket,&descp_set))
         {
            err_check=recv(con_socket,&con_msg,sizeof(con_msg),0);
            if(err_check==-1)
            {
                perror("error msg:msg wasnt recieved from client\n");
                return 0;
            }
            if (strncmp(con_msg,"bye",3)==0)
			{
				close(con_socket);
				return 0;
			}
            //printf("cli-cmd %s\n",con_msg);

            if(strncmp(con_msg,"ls",2)==0)
            {
             // err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
              //printf("cli-%s\n",con_msg);
              //dir_server = opendir(".");
              sscanf(con_msg, "%s %s", cmd, file_name);  
              if( (file_dir = fopen ("dirtest.dat", "w")) < 0 )
	          {
		         perror("failed to open input file\n");
		         return 1;
              }
              getcwd(cwd, sizeof(cwd));
              count_file = scandir(".", &entry_list,0,alphasort);
              if (count_file <0) 
              {
                 perror("scandir");
                 return EXIT_FAILURE;
              }
              for(i = 0; i < count_file; i++)
              {
                entry = entry_list[i];
                if (entry->d_name[0] !='.')
                fprintf(file_dir,"%s\n", entry->d_name);
              }
                fclose(file_dir);

              if( (file_dir = fopen ("dirtest.dat", "r+")) < 0 )
	          {
		         perror("failed to open input file\n");
		         return 1;
              }
              for(i = 0; i < count_file-3; i++)
              {
              	ret=fscanf(file_dir, "%s\n", putname); // read from the input file
                //printf("buf %s 2 %s 3  %s 1\n",buf, buf, buf);
                sprintf(con_msg, "LS: %16s", putname);
                err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
              } ;
              fclose(file_dir);
            }

            if(strncmp(con_msg,"Get",3)==0)
            { 
              //printf("server-get %s\n",con_msg);
              sscanf(con_msg, "%s %s", cmd, file_name);  
              //printf("server-get %s %s\n",file_name, con_msg);
              //err_check = (file_dir = fopen (file_name, "r"));
              //printf("server-get %s %d\n",file_name, err_check);
              if( (file_dir = fopen (file_name, "r")) == NULL)
	          {
		         perror("failed to open input file\n");
                 strncpy(putname, file_name,16);
                 sprintf(con_msg, "Err:The %16s doesn’t exist", putname);
                 //printf("The %s doesn’t exist",file_name);
                 err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
              } 
			  else 
              {
                  err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
                do
			    {
				  ret = fread(buf ,sizeof(char), 256, file_dir); // read from the input file
				  //write(dev_fd, buf, ret);//write to the the device
                  //printf("buf %s %d\n",buf, (int) ret);
                  err_check=send(con_socket,&buf,ret,0);
			    }while(ret > 0);
                //printf("end buf %s %d\n",buf, (int) ret);
                fclose(file_dir);
                strncpy(putname, file_name,16);
                sprintf(con_msg, "get %s successfully", putname);
                printf("get %s successfully",file_name);
              }
            }

            if(strncmp(con_msg,"Put",3)==0)
            {
              //err_check=recv(con_socket,&con_msg,sizeof(con_msg),0);
              //printf("1 %s cmd put 1\n",con_msg);
              //printf("2 %s get 2\n",con_msg);
              err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
              sscanf(con_msg, "%s %s", cmd, file_name);  

              if( (file_dir = fopen (file_name, "w+")) < 0 )
	          {
		        perror("failed to open input file\n");
		        return 1;
              } 

			  do
			  {
                err_check=recv(con_socket,&buf,sizeof(buf),0);
				if (err_check>0)
                  ret = fwrite(buf ,sizeof(char), err_check, file_dir); 
                //printf("3 buf %s %d server\n",buf, err_check);

			  } while( err_check==256);
              //printf("end buf %s %d 5\n",buf, (int) ret);
              fclose(file_dir);
              strncpy(putname, file_name,16);
              sprintf(con_msg, "put %16s successfully", putname);
              //printf("end buf %s %d 6\n",putname,err_check);
              err_check=send(con_socket,&con_msg,sizeof(con_msg),0);
            }
         }
        
    }
    close(con_socket);
    return 0;
}

int check_del_user(char *newuser)
{
    FILE *file_dir;
    char existname[5][16], read_name[16], newname[16];
    int i=0, ret, err_check, exist_user=-1, total_users;

    sscanf(newuser, "%s", newname); 
    if( (file_dir = fopen ("users.dat", "r+")) < 0 )
	{
		perror("failed to open input file\n");
		return 1;
    } 
	while(err_check = fscanf(file_dir, "%s", read_name)>0)
	{
		//err_check = fscanf(file_dir, "%s", read_name); 
		//printf("users %s 1\n", read_name);
        if(strncmp(read_name, newname,12) !=0)
        {
        strcpy(existname[i], read_name);
        //printf("end in %s %s %s %d %d %d 2dd\n",newuser,existname[i], read_name, i, err_check, exist_user);
        i++;
        }
        //printf("end buf %d 2d\n",strncmp(newuser, read_name,sizeof(newuser)));
        //printf("end buf %s %s %s %d %d %d 2d\n",newuser,existname[i-1], read_name, i, err_check, exist_user);
    } ;
    total_users=i;
    fclose(file_dir);


    if( (file_dir = fopen ("users.dat", "w")) < 0 )
	{
		perror("failed to open input file\n");
		return 1;
    } 
	
    for (i=0; i<total_users; i++)
	{
        err_check = fprintf(file_dir,"%s\n",existname[i]);  
		//printf("users %s \n", read_name);
        //printf("end buf %d\n",strncmp(newuser, read_name,sizeof(newuser)));
        //printf("end buf %s %s %d %d %d 3ee\n",newuser, existname[i], i, err_check, total_users);

    }
    fclose(file_dir);
    return err_check;

}

int check_add_user(char *newuser)
{
    FILE *file_dir;
    char existname[5][16], read_name[16], newname[16];
    int i=0, ret, err_check, exist_user=-1;
    
    sscanf(newuser, "%s", newname); 
    if( (file_dir = fopen ("users.dat", "r+")) < 0 )
	{
		perror("failed to open input file\n");
		return 1;
    } 
	do
	{
		err_check = fscanf(file_dir, "%s", read_name); 
		//printf("users %s \n", read_name);

        if (strncmp(newname, read_name,12)==0)
        {
            exist_user=i+1;
        }
        //printf("e %d %d %d\n",strncmp(newuser, read_name,8), (int) sizeof(newuser),(int) sizeof(read_name));
        //printf("e %s %s %d %d %d\n",newuser, read_name, i, err_check, exist_user);
        //printf("e %d %d %d\n",strncmp(newuser, read_name,2), strncmp(newuser, read_name,3),strncmp(newuser, read_name,4));
        //printf("e %d %d %d\n",strncmp(newuser, read_name,5), strncmp(newuser, read_name,6),strncmp(newuser, read_name,7));
    } while (err_check >0);
        //printf("e %d %d %d\n",strncmp(newuser, read_name,8), (int) sizeof(newuser),(int) sizeof(read_name));
        //printf("e %s %s %d %d %d\n",newuser, read_name, i, err_check, exist_user);
        //printf("e %d %d %d\n",strncmp(newuser, read_name,2), strncmp(newuser, read_name,3),strncmp(newuser, read_name,4));
        //printf("e %d %d %d\n",strncmp(newname, read_name,5), strncmp(newname, read_name,6),strncmp(newname, read_name,8));
    if (exist_user==-1) 
       err_check = fprintf(file_dir,"%s",newuser); 
    fclose(file_dir);

    if (exist_user >0) ret=-1;
    else ret = 5;
    return ret;

}