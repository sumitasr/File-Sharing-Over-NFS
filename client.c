/* ******************************************************************************************************************
* program:   client.c -- a stream socket client 
*
* purpose:   1. The purpose of the client to share a file (video,audio,txtfile,.datfile,etc......)among the different users....
	     2. This file will be stored on the common storage.
	     3. Client can  
	     allocate a socket, connect to a server , 
*            uploading:read from a file , send the contents of the file 
             downloading :receive the contents of the file,write into the file
*/

/********************* *************************Define HeaderFiles******************************************************/
//1. #include<stdio.h>   used for I/O operations.........
//2. #include <unistd.h> define system call wrapper functions(fork,I/O()read,write,close)
//3. #include <errno.h>        //#define<error.h> defines several macros used to define & docunemt runtime errors
//4. #include <string.h>
//5. #include <netdb.h>       //#define<netdb.h> definitions for network database operations
//6. #include <sys/types.h>   //#define<sys/types.h> contains definitions of a no of data types used in system call,used in next two header files. 
//7. #include <netinet/in.h>  //includes defitions of structure needed for socket.
//8. #include <sys/socket.h>  //contains constants & structure needed for internet domain adderess
//9. #include <arpa/inet.h> 
//10. #include<time.h> 
//11. #include <sys/stat.h>
//12. #include <fcntl.h>

//#define size 1024      size of buffer
//#define PORT "3490"     // the port client will be connecting to server
/*****************************************************************************************************************************************************/
#include <stdio.h>        //used for I/O operation
#include <stdlib.h>
#include <unistd.h>       // define system call wrapper functions(fork,I/O()read,write,close)
#include <errno.h>        //defines several macros used to define & docunemt runtime errors
#include <string.h>
#include <netdb.h>       // definitions for network database operations
#include <sys/types.h>   // contains definitions of a no of data types used in system call,used in next two header files. 
#include <netinet/in.h>  //includes defitions of structure needed for socket.
#include <sys/socket.h>  //contains constants & structure needed for internet domain adderess
#include <arpa/inet.h> 
#include <time.h> 
#include <sys/stat.h>
#include <fcntl.h>

#define size 1024
#define PORT "3490"     // the port client will be connecting to 

// get sockaddr, IPv4 or IPv6:
//******************************************************************************************************************************************************************/
// buffer[size]= use for storing the video in packets(some specific bytes)....... 
 //int sockfd = socket file discriptor 
//   *****************************************************************************************************************************************************************/
char buffer[size];
int sockfd;
void video_uploading(int,FILE *fp);
void video_downloading(int,FILE *fo);
void video_delete(int,FILE *fp); 
void termination(int sockfd,char s[30]);
/****************************************************************************************************************************************************************/

//struct stat is a system struct that is defined to store information about files...............................................................................
// Information such as the file's size, access permissions, and the time it was last modified are all available using the stst()................................
//but we are using this function to calulate the size of file..................................................................................................

/****************************************************************************************************************************************************************/
struct stat status;
void *get_in_addr(struct sockaddr *sa)
{
	    if (sa->sa_family == AF_INET) 
    	    {
        		return &(((struct sockaddr_in*)sa)->sin_addr);
            }
    			return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
/****************************************************************************************************************************************************************/
// struct addrinfo hints, *servinfo, *p These are the basic structures for all syscalls and functions that deal with internet addresses. 
// struct sockaddr_storage their_addr  connector's address information
// struct sigaction sa;  sigaction - examine and change a signal action. system  call  is used to change the action taken by a process on a specific signal. 
// char * leftover If no conversion is performed, zero is returned and the value of buffer is stored in the location referenced by leftover.
// choice is used for the entering choice .......
//char s[INET6_ADDRSTRLEN] used to store ip address.......
//memset is used to fill memory with constant byte-function fills the hints bytes of the memory area pointed to by hints with constant byte 0.
//hints.ai_family desired address family for return address..................
//AF_UNSPEC return socket address for any address family (ipv4,ipv6)
// hints.ai_socktype defile the socket type (tcp,udp)
//SOCK_STREAM used for tcp socket....
/***************************************************************************************************************************************************/
int main(int argc, char *argv[])
{
  		  /********* Program Variable Define & Initialization **********/  
	    int sockfd,numbytes;         //socket file discriptor
	    struct addrinfo hints, *servinfo, *p;  
	    int rv,choice,len;     
	    char ch;
            FILE *fp,*fo;
	    char s[INET6_ADDRSTRLEN];     //hold the ipaddress
		    // if no command line argument supplied
	    if (argc != 2) 
	    {
		        fprintf(stderr,"usage: client hostname\n");
		        exit(1);     //just exit
	    }

	    memset(&hints, 0, sizeof hints);   // fill memmory with a constant byte and returns a pointer to the memmory area hints
	    hints.ai_family = AF_UNSPEC;       // allow IpV4 or IpV6 adderess domain
	    hints.ai_socktype = SOCK_STREAM;   // fo socket stream

/*******************************************************************************************************************************************************************/	   
   // getaddrinfo()=   returns a list of address structure...................................
   //Given  node(NULL)  and  service(Port),  which  identify an Internet host and a service, getaddrinfo() returns one or    more addrinfo structures, each of which
   // contains an Internet address that can be specified in a call to bind or connect
   //argv[1]= return 1 ipaddress
/*******************************************************************************************************************************************************************/
           if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
	     {
		        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		        return 1;
	     }

/*****************************************************************************************************************************************************************/
//socket is to create an endpoint for communcation.
// servinfo->ai_family argument select the protocol family which will be used for communcation.............
// servinfo->ai_socktype, type of socket...................................................................
// servinfo->ai_protocol specify the particular to be used with socket.....................................
/****************************************************************************************************************************************************************/

            if (sockfd = socket(servinfo->ai_family, servinfo->ai_socktype,servinfo->ai_protocol))
	    {
		    perror("client: socket");

	    }
/************************************************************************************************************************************************************/
 //inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
 // their_addr.ss_family This argument shall specify the family of the address. This can be AF_INET [IP6] [Option Start]  or AF_INET6. [Option End]
 //  get_in_addr((struct sockaddr *)&their_addr  argument points to a buffer holding an IPv4 address if the af argument is AF_INET, [IP6] [Option Start]  or an IPv6 address if the af argument is AF_INET6; [Option End] the address must be in network byte order
 //s,argument points to a buffer where the function stores the resulting text string; it shall not be NULL.
 // sizeof(s) argument specifies the size of this buffer, which shall be large enough to hold the text string (INET_ADDRSTRLEN characters for IPv4, [IP6] [Option Start]  INET6_ADDRSTRLEN characters for IPv6). [Option End]
/**********************************************************************************************************************************************************/

	    inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr),s, sizeof(s));
	    printf("client: connecting to: %s\n", s);

/*************************************************************************************************************************************************************/
//    The connect() system call connects the socket referred to by the file descriptor sockfd to the address specified by addr. The addrlen  argument  specifies the size of addr.  The format of the address in addr is determined by the address space of the socket sockfd;
//If  the connection or binding succeeds, zero is returned.  On error, -1 is returned, and errno is set appropriately.

/**************************************************************************************************************************************************************/ 	    
           if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) 
	    {
		    close(sockfd); 
		    perror("client: connect");
		    return 0;            
	    }

            freeaddrinfo(servinfo);
       
           while(1)
	    {                            
		  printf("\n *****************************************************\n");
		  printf(" Enter your choice:\n");
		  printf(" For uploading: 1\n For downloading: 2\n For delete: 3\n For exit: 4\n ");
		  scanf("%d",&choice);
     		  switch(choice)
	 	  {
			   	case 1:          
					send(sockfd,"1",1,0);
                                        video_uploading(sockfd,fp);        
					break;
				case 2:
                         		send(sockfd,"2",1,0);
                                        video_downloading(sockfd,fo);
                                                        
					break;
				case 3:           
					send(sockfd,"3",1,0);
					video_delete(sockfd,fp);
                                        break;
				case 4:
					send(sockfd,"4",1,0);
					termination(sockfd,s);
                                        exit(1);
                                        break;
			        default:
					printf("wrong choice");
                                        break;
                                                       
		 } 		
	   }			
          close(sockfd);  
    	   return 0;
} 
/************************************************* uploading************************************************************/
/*      char path[100] = array for store the path where video is store
*      char result[size] = array for store the size of video 
*      char username[32]=  array for store the username
*      char video_name[32] = array for store the videoname
*      int len = this variable takes the (512 bytes)
*      int len1 = this variable takes the bytes which is send to server 
*      double byte_to_send =contains size os file after calculate  
*/   
//       1. first of all client enter the user name, password,videoname                                                                                                                                         
//      2.then send all the information to the server side......                                                                                                                   
//     3. server will store all the informatiom in the file but before storing in a file it checks......
//      **  that particular video is present or not
//       ** if video is already present in the file then server send the client side again enetr the video name....                                                                                                       
//     ** if not present then  store this information in a file f1.
//        ** and also store that video name in another file...                                                                                                             
//     4.now client enter the path(file) where the video is stored...
//       5.open the file in a read mode 
//       6.client calculate the size of file 
//         ** assign this size into a variable 
//         ** convert this double to string
//         ** send to the server
//    7. client read the data from file (512 bytes) 
//         send to the server 
//         subtract 512 bytes into the file size(byte_to_send)
//       untill byte_to_send is 0
//      8. finally close the file                                                                                                                         
/***********************************************************************************************************************************************************/
  

void  video_uploading(int sockfd,FILE *fp)
{
	      char path[100],result[size],username[32],passwd[32],video_name[32];
	      int flag=0;
	      int len1,len;
	      double byte_to_send,m;
	      printf("\n*************************Upload****************************\n");
	      printf("\nclient want to upload:\n");               	                                                                                         
	      printf("Enter user name: ");
	      scanf("%s",username);
	      send(sockfd,username,32,0);
	      printf("Enter password: ");
	      scanf("%s",passwd);
	      send(sockfd,passwd,32,0);
              do
              {
	      		printf("Enter the video-file name: ");
              		scanf("%s",video_name);
              		send(sockfd,video_name,sizeof(video_name),0);
             	        recv(sockfd,buffer,32,0);
              		if(strcmp(buffer,"Video Exists")==0)
               		{
              		   printf("Video alredy exists on server");
                  		flag=1;
               	        }
               		else
               		{
                   		flag=0;
                	}
               }while(flag==1);
              while(1)
	      {  
	      		printf("Enter the path of video-file:\n ");   // give the path of the video  where the video is store 
	      		scanf("%s",path);
              		fp=fopen(path,"r"); 
		       //open the file in read mode specify by this path
              		if(fp==NULL)
              		{
                        	  printf("File does not exist  \n");  
              		} 
              		else
              		{     
             			 if(fstat(fileno(fp),&status)==0)//for calculating the size of file
                	  	 byte_to_send=(double)(status.st_size);//assign the size of the file into the variable
               		  	 sprintf(result,"%lf",byte_to_send);//convert double into string
			         send(sockfd,result,sizeof(result),0);//send the size of the file to server
                                 while(byte_to_send>0) 
                         	{             
                                	       len=read(fileno(fp),buffer,sizeof(buffer)); //reading the file
                                	       len1=send(sockfd,buffer,len,0);//send data to server
					       strcpy(buffer,"\0");
                                	       m=(double)(len1);
                                	       byte_to_send-=m;//subtract sending data from file size
                                	       printf("byte_to_send =%lf\t",byte_to_send); 
                         	}
                  	break;         
                }
                
         }
	      fclose(fp);
}

/**************************************************Downloading**************************************************************
*     char path_name[100]=array for storing the path of downloading
*     char buf1[32]=array for store the size of file
      char video_name[32]=array for store the videoname
*     int data_recv= in this variable recv the size of data 
*     int numtry=10 ;if data not recieve then client wait 10 times
*     fcntl(sockfd, F_SETFL, O_NONBLOCK)=is used for make the record lock to nonblock
*     F_SETFL= Set  the  file status flags to the value specified by arg. 
      1. when client want to download a video 
        **then firstly list of videos will be displayed on the screen that was uploaded
*       **client choose any video which they want to download 
        **enter the video name 
*       **and path where they want to store that video
*     send the video name sever side if video exist then download the video  
*     for concurrent uploading and downloading using file locking(fcntl) & set nonblock 
*     
*/
/************************************************************************************************************************************************************/







     void video_downloading(int sockfd,FILE *fo)
     {
       		double converter,remainder,temp=0;
                int data_recv;
                char *leftover,path_name[100],buf1[30],buffer1[size],video_name[32];
                printf("\n*********************Download********************\n");
		printf("\nclient want to download:\n");
		printf("list of the videos :\n");		
		recv(sockfd,buffer,sizeof(buffer),0);//recieve file size in string
		remainder = strtod(buffer, &leftover);	 //convert string into double
				
		do
		{
                          if(data_recv=recv(sockfd,buf1,sizeof(buf1),0))
                          { 	
				if(data_recv>0)
				{
					printf("%s\n",buf1); 
					converter=(double)(data_recv);
		                   	remainder-=converter;
				}
			  }
		}while(remainder>0);                                               
	      	printf("Enter the video-file name from the list: ");
       		scanf("%s",video_name);
		printf("Enter the path, where to save the video-file: "); 
        	scanf("%s",path_name);
                send(sockfd,video_name,sizeof(video_name),0); 	
        	fo=fopen(path_name,"w");         // open the file in write mode for downloading   
         	if(fo==NULL)
         	{                                   
        		      printf("file does not exist");
           		      exit(1);
         	}
         	else
       		{      
			int numTry = 20;
			fcntl(sockfd, F_SETFL, O_NONBLOCK);  // set to non-blocking
			while(numTry)
			{
			      if(data_recv=recv(sockfd,buffer1,sizeof(buffer1),0))
		       	      {
					if(data_recv>=0)
					{
						fwrite(buffer1,data_recv,1,fo);//write data in the file
					}
					else 
					{
						printf("waiting for 3 seconds\n");
						sleep(5);
						numTry--;						
					}
			      }
			}
		}
         		fclose(fo);
}


/********************************************************************************************************************************************************/
/*                                                                                                                                                      
 *   for delete a video ....                                                                                                                               
 *   1.enter the user name and passwd which will be assigend by the client at the time of uploading                                                                                     
 *   2.enter the video name which client want to delete a video ......                                                                                               
 *   3.send the user name,passwd and video name to the server side 
 *   4.server send a msg to the client side that video deleted........
 *   char delete_videoname[32] = array for storing the videoname which user want to delete
 *   char user_name[32] = array for storing the user name
 *   char passwd[32] = array for storing the passwd of user                                                                                                                                                             
 */                                                                                                                                                     
/********************************************************************************************************************************************************/
void video_delete(int sockfd,FILE *fp)
{
             char delete_videoname[32],user_name[32],passwd[32];
             printf("\n***********************Delete**********************\n");
             printf("\nuser want to delete a video\n");
	     printf("enter the user name:");
             scanf("%s",user_name);
	     send(sockfd,user_name,32,0);
	     printf("enter the passwd:");
             scanf("%s",passwd);
	     send(sockfd,passwd,32,0);
             printf("enter the video name which user want delete\n");
             scanf("%s",delete_videoname);
             send(sockfd,delete_videoname,32,0);
             recv(sockfd,buffer,sizeof(buffer),0);
	     printf("%s",buffer);
}

/******************************************************************************************************************************* ********************
*   for terminate the client....
*   char s[INET6_ADDRSTRLEN]=contain IPV4 & IPV6 address
***************************************************************************************************************************************/
void termination(int sockfd,char s[INET6_ADDRSTRLEN])
{
	     strcat(s,"client want to terminate\n");
             printf("%s",s);					
}



