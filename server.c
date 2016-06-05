/**
 * Video Sharing Over NFS
 * @version v1.0.0
 * @author Sumit Bansal
 * @link https://github.com/sumitasr/Video-Sharing-Over-NFS
 * @license MIT License, http://www.opensource.org/licenses/MIT
 */
 
// program:    server.c -- a stream socket server 

// purpose:    allocate a socket and then repeatedly execute the following:
//             1. wait for the next connection from a client
//             2. receive a packet from client connection
//             3. echo the packet back to the client
//             4. go back to step (2)
//             5. close the connection
           
//  server will store a video on the comman storage which client wants to share.
//  client will sends the data ,server recv the data in an array and store this array 
//  in a file which will be open in write mode on client side.                  
//  if any  client  wants to download a video then server checks in the common storage 
//  and if video is present then server will send the video to the client side  


/* Define Header Files */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>   
#include <signal.h>
#include <fcntl.h>     
#include <unistd.h>
#include <limits.h>
#include <time.h> 
#include <sys/stat.h>

#define PORT "3490"  // the port users will be connecting to
#define size 1024
#define BACKLOG 10     // how many pending connections queue will hold




// there are two structure
// 1. user structure
// store the userneme , password or video name which user wants to store
// 2. video structure 
// store the name of all videos

typedef struct {
	
        char recv_user_name[30];
        char recv_passwd[30];
        char recv_video_name1[30];

}USER;

USER user;

typedef struct {

         char dvideo_name[30];

}VIDEO;

VIDEO video;


// buffer[size] use for storing the video in packets(some specific bytes).
// int sockfd , new_fd are socket file discriptor ,new file discriptor,used to create the connection and accept the new connection.
char buffer[size],buf[32];
int sockfd,new_fd;

void video_uploading(int,FILE *,FILE *,FILE *);
void video_downloading(int,FILE *,FILE *,FILE *);
void video_delete(int,FILE *,FILE *,FILE *);
void termination(int,char []);


// struct stat is a system struct that is defined to store information about files
// Information such as the file's size, access permissions, and the time it was last modified are all available using the stst()
// but we are using this function to calulate the size of file.

struct stat status;


/*
  waitpid() system call suspends execution of the calling process until a child specified by pid(process id) argument has changed state.  By default, waitpid() 
  waits only for terminated children 
*/

// s defines the 
// WHOHANG return immediately if no child has exited. 
// If pid is greater than 0, it specifies the process ID of a single child process for which status is requested.
// -1 represents the pid(If pid is equal to (pid)-1, status is requested for any child process. In this respect, waitpid() is then equivalent to wait().
/* on success, returns the process ID of the child whose state has changed;
   if WNOHANG was specified and one or more child(ren) specified by pid exist,
  but have not yet changed state, then 0 is returned. On error, -1 is returned. 
*/


void sigchld_handler(int s) {

	    while(waitpid(-1, NULL, WNOHANG) > 0);

}

// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa)
{
	    if (sa->sa_family == AF_INET)
	    {
	    	    return &(((struct sockaddr_in*)sa)->sin_addr);
	    }
    	    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// struct addrinfo hints, *servinfo, *p These are the basic structures for all syscalls and functions that deal with internet addresses. 
// struct sockaddr_storage their_addr  connector's address information
// struct sigaction sa;  sigaction - examine and change a signal action. system  call  is used to change the action taken by a process on a specific signal. 
// char * leftover If no conversion is performed, zero is returned and the value of buffer is stored in the location referenced by leftover.
// choice is used for the entering choice
// char s[INET6_ADDRSTRLEN] used to store ip address
// memset is used to fill memory with constant byte-function fills the hints bytes of the memory area pointed to by hints with constant byte 0.
// hints.ai_family desired address family for return address
// AF_UNSPEC return socket address for any address family (ipv4,ipv6)
// hints.ai_socktype defile the socket type (tcp,udp)
// SOCK_STREAM used for tcp socket
// ints.ai_flags used our ip address
// AI_PASSIVE flag indicates the caller intends to use the returned socket address structure in a call to the bind function



int main(void)
{           
	    char * leftover; 
	    int choice;  // listen on sock_fd, new connection on new_fd
	    struct addrinfo hints, *servinfo, *p;
	    struct sockaddr_storage their_addr; // connector's address information
	    socklen_t sin_size;
	    struct sigaction sa;
	    int yes=1,len;
	    char s[INET6_ADDRSTRLEN],list_txt1[32],video_txt[32],list_txt[32];//store the ip address
	    int rv;
	    FILE *fp1,*fo1,*f1,*pf;  
	    memset(&hints, 0, sizeof hints); // clear sokadder structure
	    hints.ai_family = AF_UNSPEC;    //set family ,
	    hints.ai_socktype = SOCK_STREAM; // for tcp socket
	    hints.ai_flags = AI_PASSIVE; // use my IP

 
       // returns a list of address structure
       // Given node(NULL) and service(Port), which identify an Internet host and a service,
       // getaddrinfo() returns one or more addrinfo structures, each of which
       // contains an Internet address that can be specified in a call to bind(2) or connect(2).
    	   
  	    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {

		        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }


        //socket is to create an endpoint for communcation.
        // servinfo->ai_family argument select the protocol family which will be used for communcation
        // servinfo->ai_socktype, type of socket
        // servinfo->ai_protocol specify the particular to be used with socket
        // setsocket is used to set option on socket
        // sockfd socket server file discriptor
        // SOL_SOCKET which option resides and name of the options must be specified
        // SO_REUSEADDR Allows other sockets to bind() to this port, unless there is an active listening socket bound to the port already.
        // sizeof(int) defines the size 
        // Returns zero on success, or -1 on error


            /* creat a server socket */

            if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol)) == -1){
		          
              perror("server: socket");
            }

            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {

            	   perror("setsockopt");
            	   exit(1);
            }


          // bind a local address to the socket identified by sockfd
          //servinfo->ai_addr, pointed to source address structure contain address to bounded to socket
          //servinfo->ai_addrlen specify the address length of address
          //servinfo == NULL used if server failed to bind
          //freeaddrinfo(servinfo) Function frees address information that the getaddrinfo function dynamically allocates in addrinfo structures.
          //this function does not return any value.
        

             if (bind(sockfd,servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
                  
                  close(sockfd);
                  perror("server: bind");                             
             }

             if (servinfo == NULL) {

       		 	     fprintf(stderr, "server: failed to bind\n");
        	 	     return 2;
             }

          	
             freeaddrinfo(servinfo); // all done with this structure


 
      // specify size of request queue & listen the connection of client.
      // BACKLOG specify the size of queue.
      // sa.sa_handler = sigchld_handler;
      // sa_handler specifies the action to be associated with signum.
      // this function receives the signal number as its only argument
      // sigemptyset(&sa.sa_mask)sa_mask specifies a mask of signals which should be blocked
      //sa.sa_flags specifies a set of flags which modify the behavior of the signal.


/************************************************************************************************************************************************************/


    	 if (listen(sockfd, BACKLOG) == -1) 
    	 {
        	perror("listen");
        	exit(1);
    	 }

    		sa.sa_handler = sigchld_handler; // reap all dead processes
    		sigemptyset(&sa.sa_mask);
   	        sa.sa_flags = SA_RESTART;

 

  //  The sigaction() system call is used to change the action taken by a process on receipt of a specific signal
  //  signum specifies the signal and can be any valid signal except SIGKILL and SIGSTOP.
  //  SIGCHLD specifies the signal and can be any valid signal
  //  &sa If act is non-null, the new action for signal signum is installed from act.
  //  If oldact is non-null, the previous action is saved in oldact.
  //  NULL define oldact is non-null, the previous action is saved in oldact.



        if (sigaction(SIGCHLD, &sa, NULL) == -1){

                perror("sigaction");
                exit(1);
        }

        
        printf("server: waiting for connections...\n");

        while(1) 
        {  
                
               sin_size = sizeof their_addr;
  
              // accept function it  extracts  the  first connection request  on  the queue
              // of pending connections for the listening socket,sockfd, creates a new connected socket,
              // and returns a new file descriptor referring  to that socket.  
              // The newly created socket is not in the listening state.  
              // The original socket  sockfd  is  unaffected by this call.


        	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        	if (new_fd == -1) {

            		perror("accept");
            		continue;
        	}
 

       // inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
       // their_addr.ss_family This argument shall specify the family of the address.
       // This can be AF_INET [IP6] [Option Start]  or AF_INET6. [Option End]
       // get_in_addr((struct sockaddr *)&their_addr argument points to a buffer holding an 
       // IPv4 address if the af argument is AF_INET, [IP6] [Option Start]  or an IPv6 address 
       // if the af argument is AF_INET6; [Option End] the address must be in network byte order
       // s,argument points to a buffer where the function stores the resulting text string;
       // it shall not be NULL.
       // sizeof(s) argument specifies the size of this buffer, 
       // which shall be large enough to hold the text string 
       // (INET_ADDRSTRLEN characters for IPv4, [IP6] [Option Start]  INET6_ADDRSTRLEN characters for IPv6). [Option End]
  

      	inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
      	printf("server: got connection from %s\n", s);
           


        /*
        fork creates  a new process by duplicating the calling process.
        the new process, referred to as the child,
        is an  exact duplicate  the   calling  process,
        referred  to as the parent
        */


        	if (!fork()) {

          
                close(sockfd);   
            		
                while(1) {     
                             
                            
                    // 1. open the file f1  in append mode which includes username ,passwd,videoname
                    // 2. open the file pf which include only video names
                    // 3. recv the choice from the client side 
                    // if the user send the value 1 then server recv the 1 in a string 
                    // then convert the string to int by using atoi function 
                    // then according to the choice the function will be called

				
					          f1=fopen("/home/sumit/temp/list.txt","a+");

                    pf=fopen("/home/sumit/temp/video.txt","a+");

                    printf("\n Server side programming: \n"); 
                  
                    recv(new_fd,buf,sizeof(buf),0);
                    
                    int choice = atoi(buf);
                    
                    printf("choice =%d\n",choice);
                                                     
                 	  switch(choice){

           	 		   
                        case 1:   
                                video_uploading(new_fd,fo1,f1,pf);
					   			              break;
               			
               					case 2:
                                video_downloading(new_fd,fp1,f1,pf);
                                break;  
              					
                        case 3:
      					 			          video_delete(new_fd,fo1,f1,pf);
               		              break;
                        
                        case 4:
                                termination(sockfd,s);
                                close(new_fd);
                                break;

                        default:
                                printf("Wrong choice");
                                break;
                    }
                                        
                    fclose(f1);
                    fclose(pf);
                }
               	                     
          }
      }
  
        close(sockfd);  // parent doesn't need thi
    	return 0;
}

//struct flock fl used for file locking.
//offset 
//recv_data_byte
// store the info user_name,password,v_name which is send by client.
//first of all server checks that the v_name is already exist or not
//if video is already exist then server send the info to the client for entering the video name again
//and if video is not exist then user can upload that video
//then store all the information of client in the file f1
//also store the video name in a file pf
//then recv the size of video in string from the client side
//convert the string into double by using strtod function
//now open the file *fo1 in write mode
//now the client accquire a write lock
//in our program  are using record lock
//record lock will be assigned on a particular block(Ex-0-512)
//server recv the data from the client side and that time no other downloder read the that particular block
//if server recv the whole data of the  block then it will free the lock 
//then reader can read tha particular block
//this process will be continued utill the full video will not be recieved
//after receving the complete video nserver free the lock



void video_uploading(int new_fd,FILE *fo1,FILE *f1,FILE *pf)
{ 
		struct flock fl;
		int offset=0;	        
		int i,len;
    
    char upload_path[30];
	  char recv_data_byte[size], v_name[32],user_name[32],passward[32],buffer[size];
		double num,m1;
		char *leftover;
	  char buffer1[size];
	
    
    int flag1=1,flag2=1;
	        

    recv(new_fd,user_name,32,0);                  //recv the user name 
	  recv(new_fd,passward,32,0);                    //recv the passwd
    

    do {
       	      

          recv(new_fd,v_name,sizeof(v_name),0);                 //recv the video name...
          
          flag2=1;
    		  
          fseek(f1,0L,0);
                                 

          while(flag2==1) {
                                            
    				if(fread(&user,sizeof(user),1,f1)==1) {

                //compare the video name and recv video name is same or not 
    				    if(strcmp(user.recv_video_name1,v_name)==0)
    				    { 
          					send(new_fd,"Video Exists",32,0);
    						    flag2=0;
           		      flag1=1; 
           	                                                       
                }
            }
            else {
                                             	 
              send(new_fd,"You can upload the video ",32,0);   
    					flag1=0;
              flag2=0;
    			  
            }
          
          }
                       
    }
    while(flag1==1);   

		pf=fopen("/home/sumit/temp/video.txt","a+");

    strcpy(user.recv_user_name,user_name);
    strcpy(user.recv_passwd,passward);
    strcpy(user.recv_video_name1,v_name);//copy the video name to the item.recv_video_name(for the structure)
              
	  
    fwrite(&user, sizeof(user), 1,f1);
	  strcpy(video.dvideo_name,v_name);//copy the video name to the item.recv_video_name(for the structure) 
    fwrite(&video, sizeof(video),1,pf);
	  fclose(pf);
	  fseek(f1,0,0);

	  while(fread(&user,sizeof(user),1,f1)){

			  printf("username: %s,\tpasswd: %s,\trevieve_video_name %s\n",user.recv_user_name,user.recv_passwd,user.recv_video_name1);
    }


		recv(new_fd,buffer,sizeof(buffer),0);//recieve file size in string

		num = strtod(buffer, &leftover); //convert string into double
	  
    strcpy(upload_path,"/home/sumit/temp/");

    strcat(upload_path,v_name);
                                  
    fo1=fopen(upload_path,"w");     
		        	  
    if(fo1==NULL) {
			           		
        printf("error in open the file");
			  exit(0);
		}
    else {

            int prevOffset = 0;
					  fl.l_type=F_WRLCK;
					  fl.l_whence=SEEK_SET;
					  fl.l_start=0;
					  fl.l_len=size;
					 
            if(fcntl(fileno(fo1),F_SETLK,&fl)==-1){

						 perror("fcntl error");
					 
            }
					  else {

					    printf("lock find\n");
					  
            }
					 
            int bytewritten = 0;
					 

            do 
            {
                
                if(len=recv(new_fd,buffer1,sizeof(buffer1),0)) {
	                                               

                      if(len>0) {

							          usleep(1000);
								        fwrite(buffer1,len,1,fo1);//write data in the file
								        m1=(double)(len);	
		                    num-=m1;//subtract recieving data from file size
					            
                      }

						      bytewritten += len;
						       
                      if(bytewritten >= size){

							           bytewritten -=size;
							           offset+=size;
							           fl.l_start=offset;
							           fl.l_type=F_WRLCK;
							         

                          if(fcntl(fileno(fo1),F_SETLK,&fl)==-1) {

								            perror("fcntl error");
							            
                          }
                                                              
                          fl.l_type=F_UNLCK;
							            fl.l_start=prevOffset;
							      

                          if(fcntl(fileno(fo1),F_SETLK,&fl)==-1) {
								       
                            perror("fcntl error");
							       
                          }

							            prevOffset = offset;
						          }
						 }

					}
          while(num>0);
					 
          printf("File Receiving Complete\n");
					
          fl.l_type=F_UNLCK;
					 
          if(fcntl(fileno(fo1),F_SETLK,&fl)==-1){

						 perror("fcntl error");
					}
					 

          fclose(fo1);
			}


 }



// In downloading function we can pass three argument
//     1. new_fd : new_fd is a new file discriptor ....................................................................
//     2. fp1    : fp1 is a file pointer which use for downloading from the sharedstorage
//     3. f1     : f1 is a file pointer which store all the information of the user
//     4. pf     : pf is a file pointer which store video information
//     variable  : retry(if no data recieve form the sharedstorage then server retry until retry not equal)
//		   utry_again(waiting for data recive
// first of all we can open the file pf in read mode ...................................................................
// if the file is blank then print "File does not exist.............................................................
// else calculate the size of the file using fstat() function
// convert the return value of the function into double
// we use sprintf() function for converting double into string..........
// send the size of the video to the client.............................................................................
// after sending the size of the video, we can send the list of video client side one bye one
// client select the name of the video from list.then he/she select a video name to download and send to server then server recieve the video name by using recv() function
// then server open the sharedstorage file in read mode
// if file is blank the print "file does not exist 
// else set the len equal to size 
// we use retry variable for retrying for reading the block recoder for a specific period of time when the record does not read by the server from the sharedstorage
// set the lock structure variable values 
//	1. type  : specify the lock type means the lock is read lock or write lock
// 	2. start : specify the starting point of the block
//	3. len   : specify the length of the block 
// then set the record lock using fcnlt() function
// if the record lock is Acquire then read the data from the block and send to the client
// else try again and sleep the server for sometime
// if read data is less then the size then server wait until the readed data not equal to size
// after reading the record block,free the block lock
// if all process is done then close the file fp1
 


void video_downloading(int new_fd,FILE *fp1,FILE *f1,FILE *pf)
{
	char user_name[32],result[size],buffer1[size],recv_video_name[30],video_name[32];
	char download_path[20];
        
  int  len,i,len1=0;
	double num_byte_recv,num_byte_send,byte_to_send1,byte_to_read,m;
	struct flock fl;
	int utry_again,offset=0;
  

  if(pf==NULL){

      printf("File does not exist  \n");
                          
  } 
  else {  
             		 
      if(fstat(fileno(pf),&status)==0)//for calculating the size of file
        byte_to_send1=(double)(status.st_size);//assign the size of the file into the variable
                 		  
      sprintf(result,"%lf",byte_to_send1);
  		send(new_fd,result,sizeof(result),0);//send the size of the file to server
                            
      while(byte_to_send1>0) {             
         
         len=read(fileno(pf),buf,sizeof(buf)); //reading the file
         len1=send(new_fd,buf,len,0);//send data to se
         m=(double)(len1);
         byte_to_send1-=m;//subtract sending data from file size
      
      }
                         
  }

	recv(new_fd,video_name,sizeof(video_name),0);    //recevive the name of video which user want to download
         
  fseek(f1,0,SEEK_SET);
          
  while(fread(&user, sizeof(user),1,f1)==1) {	

					if(strcmp(user.recv_video_name1,video_name)==0) {
						
            break;
					
          }
	}
       	     
		printf("username: %s,\tpasswd: %s,\tvideo_name :%s\n",user.recv_user_name,user.recv_passwd,user.recv_video_name1);
    strcpy(download_path,"/home/sumit/temp/");
    strcat(download_path,video_name);
    printf("download_path:%s\n",download_path);

    fp1=fopen(download_path,"r");      //path(file) open in read mode 
     			
    if(fp1==NULL){                                    

        //if video does not  exit in sharable storage then exit
         			
        printf("file does not exit\n");
         			
        exit(1);
    }
    else                              //if video exits
    {
        offset = 0;
        len = size;
        int retry = 50;

				while(retry) {

              utry_again=20;
				      fl.l_type=F_RDLCK;
					    fl.l_whence=SEEK_SET;
					    fl.l_start=offset;
					    fl.l_len=len;
					
              int lockAcquired = 1;
					

              while(fcntl(fileno(fp1),F_SETLK,&fl)<0){

      						if(utry_again){
      							
                    printf("Waiting for Lock \n");
      							sleep(4);
      							utry_again--;
      						
                  }
      						else {
      							
                    lockAcquired = 0;
      							break;
      						}
    				  }
              
              if (lockAcquired == 0)
						    break;
					

              len=read(fileno(fp1),buffer1,sizeof(buffer1)); //reading the file
					     
              send(new_fd,buffer1,len,0);//send data to server
					
              if(len < size) {
						
                  retry--;
					    
              }


					  offset+=len;
				  
      }
				
			

      fl.l_type=F_UNLCK;


			fcntl(fileno(fp1),F_SETLK,&fl);
			printf("File Sending Complete\n");
    }


    fclose(fp1);
		
}


// 1. enter the name,passwd of the user which want to delete the video by the client and recieve bye server
// 2. server check the file for the particular user present in the list or not
// if the user present then enter the video name 
// 3. check the particulat video uploaded by this user 
// 4. if yes then give the permission for deleting 
//else display incoeerct username and passwd
// for deleting the video server compaire the name of the particular video to all video name in list
// and store all the other video in another file
// remove the list.txt (main file) and rename the tempriary file 
// the same step occur for file pf( video file)
// after removing the main file unlink the video from sharedstorage


void video_delete(int new_fd,FILE *fo1,FILE *f1,FILE * pf)
{

	char delete_video_name[32],delete_path[100],user_name[32],passwd[32];
	
  int flag=0;
  
  FILE * ft,* temp;

	recv(new_fd,user_name,32,0); //recv the video name which client want to delete.
	recv(new_fd,passwd,32,0);
	recv(new_fd,delete_video_name,32,0);//recv the video name which client want to delete.

	printf("deleted video name : %s\n",delete_video_name);       
       
  f1=fopen("/home/sumit/temp/list.txt","r");  
	   

  while(fread(&user,sizeof(user),1,f1))//find the position where the particular item present
	{	
			if((strcmp(user.recv_user_name,user_name)==0) && (strcmp(user.recv_passwd,passwd)==0)) {

    				if(strcmp(user.recv_video_name1,delete_video_name)==0) {
    					
                flag=1;
    					  break;

    				}
    				else {
    				    
                printf("username and passwd are incorrect");
    				    exit(1);

    				}
			}
	}
	 
	ft=fopen("/home/sumit/temp/temp1.txt","a+");
	temp=fopen("/home/sumit/temp/temp.txt","a+");
        

  if(flag==1) {
		
		  rewind(f1);
		  
      while(fread(&user, sizeof(user),1,f1)==1)//find the position where the particular item present
		  {	
			
          if(strcmp(delete_video_name,user.recv_video_name1)!=0) {
					   
                fwrite(&user,sizeof(user),1,ft);
			    }

	    }


      pf=fopen("/home/sumit/temp/video.txt","r");
		  
      rewind(pf);
		
      while(fread(&video, sizeof(video),1,pf)==1)//find the position where the particular item present
		  {	
			
        if(strcmp(delete_video_name,video.dvideo_name)!=0) {
					fwrite(&video,sizeof(video),1,temp);
			  }
	    }
		
			 
			fclose(f1);	
			fclose(ft);
			fclose(pf);
			fclose(temp);
			
      unlink("/home/sumit/temp/list.txt");
	      					
			rename("/home/sumit/temp/temp1.txt","/root/temp/list.txt");
      unlink("/home/sumit/temp/video.txt");			
			rename("/home/sumit/temp/temp.txt","/root/temp/video.txt");
                        
			f1=fopen("/home/sumit/temp/list.txt","r");
			pf=fopen("/home/sumit/temp/video.txt","r");
	}
                

    strcpy(delete_path,"/home/sumit/temp/");
    strcat(delete_path,delete_video_name);
    printf("%s\n",delete_path);
  		

    if(unlink(delete_path)==0) { // delete the video

		  	printf("video deleted\n");
		    send(new_fd ,"video deleted",1024,0);
		}
		else {
		         
             printf("not deleted\n");
		         send(new_fd,"not deleted",1024,0);
    
    }

}
    
void termination(int sockfd,char s[INET6_ADDRSTRLEN])
{
        printf("client of this ip address %s terminated  is  ",s);
        exit(1);
   
}
