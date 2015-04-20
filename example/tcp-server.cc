#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#include <sys/time.h>

#include <ctime>

#define SERVER_PORT 2000

int
main (int argc, char *argv[])
{
  int sock;
  sock = socket (PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons (SERVER_PORT);
  addr.sin_addr.s_addr = INADDR_ANY;

  int status;
  status = bind (sock, (const struct sockaddr *) &addr, sizeof(addr));
  status = listen (sock, 1);

  int fd = accept (sock, 0, 0);
  std::cout << " accept -> " << fd << std::endl;

  uint8_t buf[10240];

  memset (buf, 0, 10240);
  ssize_t tot = 0;
  double tot_time = 0;
  for (uint32_t i = 0; ; i++)
    {
      ssize_t n = 10240;
      struct  timeval  start;
      gettimeofday(&start,NULL);
      static ssize_t old_tot = 0;
      ssize_t incbytes = 0;
      while (n > 0)
        {
          ssize_t bytes_read = read(fd, buf, 1024);//read (fd, &buf[10240 - n], n);
          if (bytes_read > 0)
            {
              n -= bytes_read;

           //   std::cout << "read:" << bytes_read << " n:" << n << std::endl;

              tot += bytes_read;
            }
          else
            {
              break;
            }
        }
      //   sleep (1);

        incbytes = tot - old_tot;
        old_tot = tot;

        struct  timeval stop;
        gettimeofday(&stop,NULL);
        double during =  1000000 * (stop.tv_sec-start.tv_sec) + stop.tv_usec-start.tv_usec;
        during = during / 1000000;
        tot_time = tot_time + during;
       // std::cout << "time: " << during << " s   rate: " << incbytes/during << " byte/s" << std::endl;

      if (tot >= 1024000)
      {
        break;
      } 
    }

  std::cout << "did read all buffers tot:" << tot << std::endl;
  std::cout << "total time :" << tot_time << " s " << "  rate: " << tot / tot_time << " byte/s" << std::endl;



  close (sock);
  close (fd);

  return 0;
}
