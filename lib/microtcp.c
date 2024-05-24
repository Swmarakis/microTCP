/*
 * microtcp, a lightweight implementation of TCP for teaching,
 * and academic purposes.
 *
 * Copyright (C) 2015-2017  Manolis Surligas <surligas@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*George Somarakis
  csd4797

  HY-335a: Project Phase A
*/


#include "microtcp.h"
#include "../utils/crc32.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>

microtcp_sock_t microtcp_socket(int domain, int type, int protocol) {
  microtcp_sock_t sock;

 
  sock.sd = socket(domain, type, protocol);
  if (sock.sd < 0) {
    perror("Socket creation failed!\n");
    
    exit(EXIT_FAILURE);
  }

  return sock;
}


int microtcp_bind(microtcp_sock_t *socket, const struct sockaddr *address, socklen_t address_len)
{
    if (bind(socket->sd, address, address_len) < 0)
    {
        perror("Failed to bind the socket");
        return -1;
    }

    socket -> state = LISTEN; 
    return 0;
}


//Done
int microtcp_connect (microtcp_sock_t *socket, const struct sockaddr *address, socklen_t address_len)
{
  //microtcp_header_t *send_header = malloc(sizeof(microtcp_header_t));
  //microtcp_header_t *receive_header = malloc(sizeof(microtcp_header_t));

  microtcp_header_t send_header;
  microtcp_header_t receive_header;
  
  initialize_random_number_generator();
  //Setup SYN Packet 
  send_header.seq_number = htonl(generate_random_sequence_number());
  send_header.ack_number = 0;
  send_header.control = htons(0x2); //SYN IS TRIGGERD
  send_header.window = 0;
  send_header.data_len = 0;
  send_header.future_use0 = 0;
  send_header.future_use1 = 0;
  send_header.future_use2 = 0;
  send_header.checksum = 0;
  uint32_t calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
  send_header.checksum = htonl(calculatedChecksum);
  // Print the header fields
  printf("\n\n");
  printf("1) \n");
  printf("Sending SYN: \n");
  printf("seq_number: %u\n", ntohl(send_header.seq_number));
  printf("ack_number: %u\n", ntohl(send_header.ack_number));
  printf("control: %u (", ntohs(send_header.control));
  // Print each bit in the control field
  for (int i = 15; i >= 0; i--) {
    uint8_t bit = (ntohs(send_header.control) >> i) & 1;
    printf("%u", bit);
  }
  printf(")\n");
  printf("window: %u\n", send_header.window);
  printf("data_len: %u\n", send_header.data_len);
  printf("future_use0: %u\n", send_header.future_use0);
  printf("future_use1: %u\n", send_header.future_use1);
  printf("future_use2: %u\n", send_header.future_use2);
  printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing
  printf("\n\n");

  if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0, address, address_len) < 0){
    socket -> state = INVALID;
    perror("Failed to send SYN packet in Connection\n");
  
    return -1;
  }


  //Receive SYN + ACK packet 
  if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)address, &address_len) < 0){
      socket -> state = INVALID;
      perror("Failed to receive SYN ACK packet Microtcp connect\n");
      
      return -1;
  }

  // Print the header fields
  printf("\n\n");
  printf("4) \n");
  printf("Printing Received SYN+ACK: \n");
  printf("seq_number: %u\n", ntohl(receive_header.seq_number));
  printf("ack_number: %u\n", ntohl(receive_header.ack_number));
  //printf("control: %u\n", receive_header->control);
  printf("control: %u (", ntohs(receive_header.control));

  // Print each bit in the control field
  for (int i = 15; i >= 0; i--) {
    uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
    printf("%u", bit);
  }

  printf(")\n");
  
  
  printf("window: %u\n", receive_header.window);
  printf("data_len: %u\n", receive_header.data_len);
  printf("future_use0: %u\n", receive_header.future_use0);
  printf("future_use1: %u\n", receive_header.future_use1);
  printf("future_use2: %u\n", receive_header.future_use2);
  printf("checksum: %u\n", ntohl(receive_header.checksum));

  printf("\n\n");


  //Check is SYN ACK response is valid
  if(ntohs(receive_header.control) == 0xA){
    printf("Success SYN ACK packet\n");
    
    socket -> curr_win_size = ntohs(receive_header.window);
    //Set up ACK packet 
    send_header.seq_number = htonl(ntohl(receive_header.ack_number) + 1);
    send_header.ack_number = htonl(ntohl(receive_header.seq_number) + 1);
    send_header.control = htons(0x2); //ACK is triggerd
    send_header.window = 0;
    send_header.data_len = 0;
    send_header.future_use0 = 0; 
    send_header.future_use1 = 0;
    send_header.future_use2 = 0;
    send_header.checksum = 0;
    uint32_t calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
    send_header.checksum = htonl(calculatedChecksum);

    if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0, address, address_len) < 0){
      socket -> state = INVALID;
      perror("Failed to send ACK packet in Connection\n");
      return -1;
    }
    printf("\n\n");
    printf("5) \n");
    printf("Sending ACK: \n");
    printf("seq_number: %u\n", ntohl(send_header.seq_number));
    printf("ack_number: %u\n", ntohl(send_header.ack_number));
    printf("control: %u (", ntohs(send_header.control));

    // Print each bit in the control field
    for (int i = 15; i >= 0; i--) {
      uint8_t bit = (ntohs(send_header.control) >> i) & 1;
      printf("%u", bit);
    }
 

    printf(")\n");
    printf("window: %u\n", send_header.window);
    printf("data_len: %u\n", send_header.data_len);
    printf("future_use0: %u\n", send_header.future_use0);
    printf("future_use1: %u\n", send_header.future_use1);
    printf("future_use2: %u\n", send_header.future_use2);
    printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing
    printf("\n\n");
    

  }else{
    printf("The packet is brokem (not a SYN ACK packet)\n");
    return -1;
  }

  socket -> state = ESTABLISHED;
  return 0;
}





int microtcp_accept(microtcp_sock_t *socket, struct sockaddr *address, socklen_t address_len){

  microtcp_header_t send_header;
  microtcp_header_t receive_header;




  if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0,  (struct sockaddr *)address, &address_len) < 0){
    socket -> state = INVALID;
    perror("Failed to receive SYN in Accept\n");
    printf("Error code: %d\n", errno);

    exit(EXIT_FAILURE);
    return -1;
  }
  printf("\n\n");
  // Print the header fields
  printf("2) \n");
  printf("Printing Received SYN: \n");
  printf("seq_number: %u\n", ntohl(receive_header.seq_number));
  printf("ack_number: %u\n", ntohl(receive_header.ack_number));
  printf("control: %u (", ntohs(receive_header.control));

  // Print each bit in the control field
  for (int i = 15; i >= 0; i--) {
    uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
    printf("%u", bit);
  }

  printf(")\n");
  printf("window: %u\n", receive_header.window);
  printf("data_len: %u\n", receive_header.data_len);
  printf("future_use0: %u\n", receive_header.future_use0);
  printf("future_use1: %u\n", receive_header.future_use1);
  printf("future_use2: %u\n", receive_header.future_use2);
  printf("checksum: %u\n", ntohl(receive_header.checksum));
  printf("\n\n");

 
  //Set up SYN + ACK packet 
  send_header.seq_number = htonl(generate_random_sequence_number());
  send_header.ack_number = htonl(ntohl(receive_header.seq_number) + 1);
  send_header.control = htons(0xA); //SYN ACK IS TRIGGERD
  send_header.window = htons(MICROTCP_WIN_SIZE);
  send_header.data_len = 0;
  send_header.future_use0 = 0;
  send_header.future_use1 = 0;
  send_header.future_use2 = 0;
  send_header.checksum = 0;
  uint32_t calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
  send_header.checksum = htonl(calculatedChecksum);

  printf("\n\n");
  // Print the header fields
  printf("3) \n");
  printf("Sending SYN+ACK: \n");
  printf("seq_number: %u\n", ntohl(send_header.seq_number));
  printf("ack_number: %u\n", ntohl(send_header.ack_number));
  printf("control: %u (", ntohs(send_header.control));

  // Print each bit in the control field
  for (int i = 15; i >= 0; i--) {
    uint8_t bit = (ntohs(send_header.control) >> i) & 1;
    printf("%u", bit);
  } 

  printf(")\n");
  printf("window: %u\n", send_header.window);
  printf("data_len: %u\n", send_header.data_len);
  printf("future_use0: %u\n", send_header.future_use0);
  printf("future_use1: %u\n", send_header.future_use1);
  printf("future_use2: %u\n", send_header.future_use2);
  printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing


  if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0,  address, address_len) < 0){
    socket -> state = INVALID;
    perror("Failed to send SYN+ACK packet in Accept\n");
    
    return -1;
  }


  if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0,  (struct sockaddr *)address, &address_len) < 0){
    socket -> state = INVALID;
    perror("Failed to receive SYN in Accept\n");
    printf("Error code: %d\n", errno);

    exit(EXIT_FAILURE);
    return -1;
  }
  printf("\n\n");
  // Print the header fields
  printf("6) \n");
  printf("Printing Received ACK: \n");
  printf("seq_number: %u\n", ntohl(receive_header.seq_number));
  printf("ack_number: %u\n", ntohl(receive_header.ack_number));
  printf("control: %u (", ntohs(receive_header.control));

  // Print each bit in the control field
  for (int i = 15; i >= 0; i--) {
    uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
    printf("%u", bit);
  }

  printf(")\n");
  printf("window: %u\n", receive_header.window);
  printf("data_len: %u\n", receive_header.data_len);
  printf("future_use0: %u\n", receive_header.future_use0);
  printf("future_use1: %u\n", receive_header.future_use1);
  printf("future_use2: %u\n", receive_header.future_use2);
  printf("checksum: %u\n", ntohl(receive_header.checksum));
  printf("\n\n");
 
  socket -> state = ESTABLISHED;
  printf("Connection is ESTABLISHED\n");
  return 0;
}



int microtcp_shutdown(microtcp_sock_t *socket, int how) {

  microtcp_header_t send_header;
  microtcp_header_t receive_header;
    /*
        How == 0 -> client asking for termination
        How == 1 -> server answering fort termination 
    */
    

    // Check for a valid state, e.g., ESTABLISHED or CLOSING_BY_PEER
    if (socket->state != ESTABLISHED && socket->state != CLOSING_BY_PEER) {
        // Handle invalid state
        return -1;
    }
    //CLIENT
    if (how == 0 ) {
        if(socket->state == ESTABLISHED){
          //Setup FIN ACK Packet 
          send_header.seq_number = htonl(generate_random_sequence_number());
          send_header.ack_number = 0;
          send_header.control = htons(0x9); //FIN ACK IS TRIGGERD
          send_header.window = htons(socket -> curr_win_size);
          send_header.data_len = 0;
          send_header.future_use0 = 0;
          send_header.future_use1 = 0;
          send_header.future_use2 = 0;
          send_header.checksum = 0;
          uint32_t calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
          send_header.checksum = htonl(calculatedChecksum);

          // Print the header fields
          printf("\n\n");
          printf("7) \n");
          printf("Sending FIN ACK: \n");
          printf("seq_number: %u\n", ntohl(send_header.seq_number));
          printf("ack_number: %u\n", ntohl(send_header.ack_number));
          printf("control: %u (", ntohs(send_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(send_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", send_header.window);
          printf("data_len: %u\n", send_header.data_len);
          printf("future_use0: %u\n", send_header.future_use0);
          printf("future_use1: %u\n", send_header.future_use1);
          printf("future_use2: %u\n", send_header.future_use2);
          printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing
          printf("\n\n");

          if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0,(struct sockaddr *)&(socket->address), socket->address_len) < 0){
            socket -> state = INVALID;
            perror("Failed to send FIN ACK packet in Shutdown\n");
          return -1;
          }

          
          // Wait for ACK from server
          sleep(0.1);
          
          if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)&socket->address, &(socket->address_len)) < 0){
            socket -> state = INVALID;
            perror("Failed to receive ACK in Shutdown\n");
            printf("Error code: %d\n", errno);

            
            return -1;
          }
          
          // Print the header fields
          printf("11) \n");
          printf("Printing Received ACK: \n");
          printf("seq_number: %u\n", ntohl(receive_header.seq_number));
          printf("ack_number: %u\n", ntohl(receive_header.ack_number));
          printf("control: %u (", ntohs(receive_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", receive_header.window);
          printf("data_len: %u\n", receive_header.data_len);
          printf("future_use0: %u\n", receive_header.future_use0);
          printf("future_use1: %u\n", receive_header.future_use1);
          printf("future_use2: %u\n", receive_header.future_use2);
          printf("checksum: %u\n", ntohl(receive_header.checksum));
          printf("\n\n");


          socket -> state = CLOSING_BY_HOST;



          if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)&socket->address, &(socket->address_len)) < 0){
            socket -> state = INVALID;
            perror("Failed to receive FIN ACK in Shutdown\n");
            printf("Error code: %d\n", errno);         
            return -1;
          }
          
          // Print the header fields
          printf("12) \n");
          printf("Printing Received FIN ACK: \n");
          printf("seq_number: %u\n", ntohl(receive_header.seq_number));
          printf("ack_number: %u\n", ntohl(receive_header.ack_number));
          printf("control: %u (", ntohs(receive_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", receive_header.window);
          printf("data_len: %u\n", receive_header.data_len);
          printf("future_use0: %u\n", receive_header.future_use0);
          printf("future_use1: %u\n", receive_header.future_use1);
          printf("future_use2: %u\n", receive_header.future_use2);
          printf("checksum: %u\n", ntohl(receive_header.checksum));
          printf("\n\n");


          //Sending last ACK 
          send_header.seq_number = htonl(generate_random_sequence_number());
          send_header.ack_number = htonl(ntohl(receive_header.seq_number) + 1);
          send_header.control = htons(0x8); //ACK IS TRIGGERD
          send_header.window = htons(socket -> curr_win_size);
          send_header.data_len = 0;
          send_header.future_use0 = 0;
          send_header.future_use1 = 0;
          send_header.future_use2 = 0;
          send_header.checksum = 0;
          calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
          send_header.checksum = htonl(calculatedChecksum);

          // Print the header fields
          printf("\n\n");
          printf("13) \n");
          printf("Sending ACK: \n");
          printf("seq_number: %u\n", ntohl(send_header.seq_number));
          printf("ack_number: %u\n", ntohl(send_header.ack_number));
          printf("control: %u (", ntohs(send_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(send_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", send_header.window);
          printf("data_len: %u\n", send_header.data_len);
          printf("future_use0: %u\n", send_header.future_use0);
          printf("future_use1: %u\n", send_header.future_use1);
          printf("future_use2: %u\n", send_header.future_use2);
          printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing
          printf("\n\n");

          if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)&(socket->address), socket->address_len) < 0){
            socket -> state = INVALID;
            perror("Failed to send ACK packet in Shutdown\n");
          return -1;
          }



          return 0;
          
        }else{
          perror("In MicroTCP Shutdown the client's side socket was not ESTABLISHED\n");
          return -1;
        }

    }else if(how == 1){
          

          //Server Side Answering to the shutdown requests
          if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)&socket->address, &(socket->address_len)) < 0){
            socket -> state = INVALID;
            perror("Failed to receive FIN ACK in Shutdown\n");
            printf("Error code: %d\n", errno);    
            return -1;
          }
          printf("8) \n");
          printf("Printing Received FIN ACK: \n");
          printf("seq_number: %u\n", ntohl(receive_header.seq_number));
          printf("ack_number: %u\n", ntohl(receive_header.ack_number));
          printf("control: %u (", ntohs(receive_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", receive_header.window);
          printf("data_len: %u\n", receive_header.data_len);
          printf("future_use0: %u\n", receive_header.future_use0);
          printf("future_use1: %u\n", receive_header.future_use1);
          printf("future_use2: %u\n", receive_header.future_use2);
          printf("checksum: %u\n", ntohl(receive_header.checksum));
          printf("\n\n");

          socket -> state = CLOSING_BY_PEER;


          //Sending ACK to the client 
          send_header.seq_number = htonl(generate_random_sequence_number());
          send_header.ack_number = htonl(ntohl(receive_header.seq_number) + 1);
          send_header.control = htons(0x8); //ACK IS TRIGGERD
          send_header.window = htons(socket -> curr_win_size);
          send_header.data_len = 0;
          send_header.future_use0 = 0;
          send_header.future_use1 = 0;
          send_header.future_use2 = 0;
          send_header.checksum = 0;
          uint32_t calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
          send_header.checksum = htonl(calculatedChecksum);

          // Print the header fields
          printf("\n\n");
          printf("9) \n");
          printf("Sending ACK: \n");
          printf("seq_number: %u\n", ntohl(send_header.seq_number));
          printf("ack_number: %u\n", ntohl(send_header.ack_number));
          printf("control: %u (", ntohs(send_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(send_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", send_header.window);
          printf("data_len: %u\n", send_header.data_len);
          printf("future_use0: %u\n", send_header.future_use0);
          printf("future_use1: %u\n", send_header.future_use1);
          printf("future_use2: %u\n", send_header.future_use2);
          printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing
          printf("\n\n");

          if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)&(socket->address), socket->address_len) < 0){
            socket -> state = INVALID;
            perror("Failed to send ACK packet in Shutdown\n");
          return -1;
          }

          //Sending FIN ACK to the client 
          send_header.seq_number = htonl(generate_random_sequence_number());
          send_header.ack_number = 0;
          send_header.control = htons(0x9); //FIN ACK IS TRIGGERD
          send_header.window = htons(socket -> curr_win_size);
          send_header.data_len = 0;
          send_header.future_use0 = 0;
          send_header.future_use1 = 0;
          send_header.future_use2 = 0;
          send_header.checksum = 0;
          calculatedChecksum = crc32((uint8_t *)&send_header, sizeof(microtcp_header_t));
          send_header.checksum = htonl(calculatedChecksum);

          // Print the header fields
          printf("\n\n");
          printf("10) \n");
          printf("Sending FIN ACK: \n");
          printf("seq_number: %u\n", ntohl(send_header.seq_number));
          printf("ack_number: %u\n", ntohl(send_header.ack_number));
          printf("control: %u (", ntohs(send_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(send_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", send_header.window);
          printf("data_len: %u\n", send_header.data_len);
          printf("future_use0: %u\n", send_header.future_use0);
          printf("future_use1: %u\n", send_header.future_use1);
          printf("future_use2: %u\n", send_header.future_use2);
          printf("checksum: %u\n", ntohl(send_header.checksum)); // Convert back to host byte order for printing
          printf("\n\n");

          if(sendto(socket -> sd, &send_header, sizeof(microtcp_header_t), 0, (struct sockaddr *)&(socket->address), socket->address_len) < 0){
            socket -> state = INVALID;
            perror("Failed to send FIN ACK packet in Shutdown\n");
          return -1;
          }


          //waiting for answering the request 


          if(recvfrom(socket -> sd, &receive_header, sizeof(microtcp_header_t), 0,  (struct sockaddr *)&socket->address, &(socket->address_len)) < 0){
            socket -> state = INVALID;
            perror("Failed to receive ACK in Shutdown\n");
            printf("Error code: %d\n", errno);    
            return -1;
          }

          printf("14) \n");
          printf("Printing Received ACK: \n");
          printf("seq_number: %u\n", ntohl(receive_header.seq_number));
          printf("ack_number: %u\n", ntohl(receive_header.ack_number));
          printf("control: %u (", ntohs(receive_header.control));
          // Print each bit in the control field
          for (int i = 15; i >= 0; i--) {
            uint8_t bit = (ntohs(receive_header.control) >> i) & 1;
            printf("%u", bit);
          }
          printf(")\n");
          printf("window: %u\n", receive_header.window);
          printf("data_len: %u\n", receive_header.data_len);
          printf("future_use0: %u\n", receive_header.future_use0);
          printf("future_use1: %u\n", receive_header.future_use1);
          printf("future_use2: %u\n", receive_header.future_use2);
          printf("checksum: %u\n", ntohl(receive_header.checksum));
          printf("\n\n");

          return 0;
    }else{
      perror("Falsed manipulation with how parameter check the server.c or client.c files\n");
      return -1;
    }

  

    // Perform remaining operations, if any

    socket -> state = CLOSED;

    return 0; // Successful shutdown
    
}

// Function to send a single packet
ssize_t send_packet(microtcp_sock_t *socket, const void *data, size_t data_len, uint32_t seq_number, uint16_t flags) {
    microtcp_packet_t packet;
    packet.header.seq_number = htonl(seq_number);
    packet.header.ack_number = htonl(socket->ack_number);
    packet.header.control = htons(flags);
    packet.header.window = htons(socket->curr_win_size);
    packet.header.data_len = htonl(data_len);
    packet.header.future_use0 = 0;
    packet.header.future_use1 = 0;
    packet.header.future_use2 = 0;
    packet.header.checksum = 0; // Placeholder for checksum, should be calculated

    // Allocate memory for data and copy it
    packet.data = (uint8_t *)malloc(data_len);
    if (packet.data == NULL) {
        perror("Error allocating memory");
        return -1;
    }

    memcpy(packet.data, data, data_len);

    // Perform additional tasks like checksum calculation here if needed
    uint32_t calculatedChecksum = crc32((uint8_t *)&packet, sizeof(microtcp_header_t));
    packet.header.checksum = htonl(calculatedChecksum);

    // Use sendto() to send the packet
    ssize_t bytes_sent = sendto(socket->sd, &packet, sizeof(microtcp_packet_t), 0,
                                (struct sockaddr *)&socket->address, socket->address_len);
    if (bytes_sent < 0) {
        perror("Error Sending Packet\n");
    }

    free(packet.data); // Free allocated memory

    return bytes_sent;
}


// Function to receive a single packet
ssize_t receive_packet(microtcp_sock_t *socket, void *buffer, size_t buffer_len, uint16_t *flags) {
    microtcp_packet_t packet;
    ssize_t bytes_received = recvfrom(socket->sd, &packet, sizeof(microtcp_packet_t), 0,
                                      (struct sockaddr *)&socket->address, &socket->address_len);

    if (bytes_received > 0) {
        // Perform additional tasks like checksum verification here if needed
        // Update window, congestion control, etc.
        socket->curr_win_size = htons(ntohs(packet.header.window));
        socket->ack_number = htonl(ntohl(packet.header.seq_number) + ntohl(packet.header.data_len));
        *flags = htons(ntohs(packet.header.control));
        // Copy data to user buffer
        size_t copy_len = (buffer_len < packet.header.data_len) ? buffer_len : packet.header.data_len;
        memcpy(buffer, packet.data, copy_len);
    }

    return bytes_received;
}

// Function to update window size based on ACK
size_t update_window_size(size_t current_size, size_t acked_bytes) {
    // Update window size based on ACK and congestion control mechanisms
    // Implement your window size update logic here
    return (current_size > acked_bytes) ? (current_size - acked_bytes) : 0;
}

// Function to update congestion control based on ACK
size_t update_congestion_control(size_t cwnd, size_t ssthresh, size_t acked_bytes, int dup_ack_count) {
    if (dup_ack_count >= 3) {
        // Fast Retransmit: Handle multiple duplicate ACKs as a sign of packet loss
        ssthresh = cwnd / 2;
        cwnd = cwnd / 2 + 1;
    } else {
        // Update congestion control based on ACK and congestion control mechanisms
        // Basic implementation; adjust as needed
        if (cwnd < ssthresh) {
            // Slow start phase
            cwnd += acked_bytes;
        } else {
            // Congestion avoidance phase
            cwnd += (acked_bytes * MICROTCP_MSS) / cwnd;
        }
    }

    return cwnd;
}

ssize_t microtcp_send(microtcp_sock_t *socket, const void *buffer, size_t length, int flags) {
    size_t remaining = length;
    size_t data_sent = 0;
  
    while (data_sent < length) {

        size_t bytes_to_send = minimum(socket->curr_win_size, socket->cwnd, remaining);
        size_t chunks = bytes_to_send / MICROTCP_MSS;

        printf("Sending %zu bytes in %zu chunks\n", bytes_to_send, chunks);

        for (size_t i = 0; i < chunks; i++) {
            uint32_t seq_number = ntohl(socket->seq_number + i * MICROTCP_MSS);

            printf("Sending chunk %zu with sequence number %u\n", i, seq_number);

            ssize_t bytes_sent = send_packet(socket, buffer + i * MICROTCP_MSS, MICROTCP_MSS, seq_number, 0);

            printf("Sent %zd bytes\n", bytes_sent);
            if (bytes_sent < 0) {
                perror("Error sending packet");
                return -1;
            }

            

            int dup_ack_count = 0;
            struct timeval start_time, current_time;
            gettimeofday(&start_time, NULL);

            while (1) {
                ssize_t ack_received = receive_packet(socket, NULL, 0, NULL);

                if (ack_received > 0) {
                    socket->curr_win_size = update_window_size(socket->curr_win_size, bytes_sent);
                    socket->cwnd = update_congestion_control(socket->cwnd, socket->ssthresh, bytes_sent, dup_ack_count);

                    printf("ACK received, updated window size: %zu, updated cwnd: %zu\n",
                           socket->curr_win_size, socket->cwnd);

                    break;
                }

                gettimeofday(&current_time, NULL);
                long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000L +
                                    (current_time.tv_usec - start_time.tv_usec);

                if (elapsed_time > MICROTCP_ACK_TIMEOUT_US) {
                    dup_ack_count = 0;
                    printf("Timeout occurred, retransmitting\n");
                    break;
                }
            }
        }

        if (bytes_to_send % MICROTCP_MSS) {
            size_t semi_filled_size = bytes_to_send % MICROTCP_MSS;
            uint32_t seq_number = htonl(ntohl(socket->seq_number + chunks * MICROTCP_MSS));

            printf("Sending semi-filled chunk with sequence number %u\n", seq_number);

            ssize_t bytes_sent = send_packet(socket, buffer + data_sent, semi_filled_size, seq_number, 0);

            if (bytes_sent < 0) {
                perror("Error sending packet");
                return -1;
            }

            printf("Sent %zd bytes\n", bytes_sent);

            int dup_ack_count = 0;
            struct timeval start_time, current_time;
            gettimeofday(&start_time, NULL);

            while (1) {
                ssize_t ack_received = receive_packet(socket, NULL, 0, NULL);

                if (ack_received > 0) {
                    socket->curr_win_size = update_window_size(socket->curr_win_size, bytes_sent);
                    socket->cwnd = update_congestion_control(socket->cwnd, socket->ssthresh, bytes_sent, dup_ack_count);

                    printf("ACK received, updated window size: %zu, updated cwnd: %zu\n",
                           socket->curr_win_size, socket->cwnd);

                    break;
                }

                gettimeofday(&current_time, NULL);
                long elapsed_time = (current_time.tv_sec - start_time.tv_sec) * 1000000L +
                                    (current_time.tv_usec - start_time.tv_usec);

                if (elapsed_time > MICROTCP_ACK_TIMEOUT_US) {
                    dup_ack_count = 0;
                    printf("Timeout occurred, retransmitting\n");
                    break;
                }
            }
        }

        remaining -= bytes_to_send;
        data_sent += bytes_to_send;
    }

    return length;
}


// Function to send an ACK packet
ssize_t send_ack(microtcp_sock_t *socket, uint32_t ack_number, uint16_t flags) {
    microtcp_packet_t ack_packet;
    ack_packet.header.seq_number = htonl(ntohl(socket->seq_number));
    ack_packet.header.ack_number = htonl(ack_number);
    ack_packet.header.control = htons(flags);
    ack_packet.header.window = htons(ntohs(socket->curr_win_size));
    ack_packet.header.data_len = 0; // No data in ACK packets
    ack_packet.header.future_use0 = 0;
    ack_packet.header.future_use1 = 0;
    ack_packet.header.future_use2 = 0;
    ack_packet.header.checksum = 0; // Placeholder for checksum, should be calculated

    uint32_t calculatedChecksum = crc32((uint8_t *)&ack_packet, sizeof(microtcp_header_t));
    ack_packet.header.checksum = htonl(calculatedChecksum);

    // Use sendto() to send the ACK packet
    ssize_t bytes_sent = sendto(socket->sd, &ack_packet, sizeof(microtcp_packet_t), 0,
                                (struct sockaddr *)&socket->address, socket->address_len);

    return bytes_sent;
}


ssize_t microtcp_recv(microtcp_sock_t *socket, void *buffer, size_t length, int flags) {
    size_t remaining = length;
    size_t data_received = 0;

    while (remaining > 0) {
        // Receive a packet
        microtcp_packet_t received_packet;
        ssize_t bytes_received = recvfrom(socket->sd, &received_packet, sizeof(microtcp_packet_t), 0,
                                          (struct sockaddr *)&socket->address, &socket->address_len);

        if (bytes_received < 0) {
            perror("Error receiving packet");
            printf("Error receiving packet: %s\n", strerror(errno));
            return -1;
        }

        printf("Received packet with %zd bytes\n", bytes_received);

        // Perform checksum verification
        uint32_t receivedChecksum = ntohl(received_packet.header.checksum);
        received_packet.header.checksum = 0;  // Clear checksum field for verification
        uint32_t calculatedChecksum = crc32((uint8_t *)&received_packet, sizeof(microtcp_header_t));

        if (receivedChecksum != calculatedChecksum) {
            // Corrupted packet, ignore and send duplicate ACK
            printf("Corrupted packet detected, sending duplicate ACK\n");
            send_ack(socket, ntohl(socket->ack_number), 0x8);
            continue;
        }

        printf("Checksum verification successful\n");

        // Update window, congestion control, etc.
        size_t copy_len;
        //ntohl(socket->ack_number)
        if (ntohl(received_packet.header.seq_number) == ntohl(socket->ack_number)) {
            // Received the expected packet in order
            copy_len = minimum(remaining, ntohl(received_packet.header.data_len), remaining);
            memcpy(buffer + data_received, received_packet.data, copy_len);

            // Update acknowledgment number
            socket->ack_number += ntohl(received_packet.header.data_len);

            // Send an ACK for the received packet
            ssize_t ack_sent = send_ack(socket, ntohl(socket->ack_number), 0x8);

            if (ack_sent < 0) {
                perror("Error sending ACK");
                return -1;
            }

            printf("Received in-order packet, updated ACK number: %u, sent ACK\n", ntohl(socket->ack_number));

            // Update remaining, data_received, etc.
            remaining -= copy_len;
            data_received += copy_len;
        } else {
            // Out-of-order packet, ignore and send duplicate ACK
            printf("Out-of-order packet detected, sending duplicate ACK\n");
            send_ack(socket, ntohl(socket->ack_number), 0x8);
        }

        // Check for connection termination (FIN, ACK)
        if (received_packet.header.control & 0x9) {
            // Set the state of the microTCP connection to CLOSING_BY_PEER
            socket->state = CLOSING_BY_PEER;
            printf("Received FIN control, setting state to CLOSING_BY_PEER\n");
            return -1;
        }
    }

    printf("Exiting microtcp_recv, received %zu bytes\n", data_received);
    return data_received; // Return the total number of bytes received
}




//What I added 


//Generating A Random Sequense Number
void initialize_random_number_generator() {
    srand((unsigned int)time(NULL));
}

uint32_t generate_random_sequence_number() {
    uint32_t random_sequence_number = rand() % 1000 + 1;
    return random_sequence_number;
    
}

ssize_t minimum(size_t x, size_t y, size_t z){
	return MIN(MIN(x, y),z);
}


