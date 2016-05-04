#  Implementation Of File Sharing Over NFS
Video Blogging & Implementation Of File Sharing Over NFS

*********************************************************************
Software Requirements:
Operating System: Linux
Language Used: C  
**********************************************************************

	INSTALLATION PROCEDURE:

STEP 1.
***********************************************************
  1.    In this project we uses 5 computers.
                 2 computers for client
                 2 computers for server
                 1 computer for Sharedstorage
  2.    All the computers connected by LAN.

************************************************************

STEP 2.
************************************************************
            (This process will be on the only that computer which u want to make as a common storage)
                                          
                        ------------------------------NFS SERVER----------------------------------

    1.          check the ipaddress of all computers by using command
                ifconfig

    2.          create the NFS Enviornment between 3 computers (one for Sharedstorage & 2 server)
                  
                 **    first step is open the terminal.

                 **   First check the NFS is already installed on the  three computers or not by using
                        the command   /etc/init.d/nfs start
                        if (NFS is not installed)
                        {
                               install the NFS by internet then proceeed further
                        }
                        else
                        {
                              come to the next step
                        }  

                 **   Make a directory on the comman storage by using command mkdir sharedstorage
                 
                 **   create a file by enter on the terminal   vim  /etc/exports
                     
                 **  Now file will be  open then press i for insert  in the file and write
                           /root/sharedstorage *(rw,sync)
             
                 **  save this file.
    
                 ** write the command on command prompt
                             /etc/init.d/nfs restart
                  
                  ** NFS configration is complete.

                  **  stop the  firewalls by using the command
                       service iptables stop

                       //ubuntu
                       sudo ufw disable
************************************************************************

STEP 3.
************************************************************************
               (This process will on those computers which u want to make as a server)

        --------------------------------------NFS CLIENT --------------------------------------------------

    1.    Now Sharedstorage is NFS server & 2 servers are NFS client.

    2.    Now Mount the NFS client computer to NFS server.

                **   first stop the firewall by using the command
                      service iptables stop

                * *  Mount the computers by using command
                      mount -t nfs -o nfsvers=3 ipaddress :/root/sharedstorage /root/temp
                  
                ** now we can use the harddisk of a NFS server as a common storage.

*******************************************************************************

STEP 4.
******************************************************************************
    1.      Now first compile the  server program on the NFS client computer                   
                   run the program by using command 
                  ** gcc server.c
                  ** ./a.out

    2.      Now run the client program on the client computer 
                **  gcc client.c
                **  ./a.out ipadress  (EX- ./a.out 127.0.0.1)


    3.      Then these option will be display on client side.
                    1. uploading
                    2. Downloading
                    3. Delete
                    4. Exit
                    printf(enter the choice);

    4.      According to the choice of client software will work........
                **video will be stored on the common storage
                ** client can download the video from the common storage....
                ** Multiple user can upload and download the videos at the same time...
                **self user can delete the video..
                ** if enter 4 by the user will be disconnect from the server...
*******************************************************************************
