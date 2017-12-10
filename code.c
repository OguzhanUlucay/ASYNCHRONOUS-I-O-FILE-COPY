#include <fcntl.h> //what is this
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aio.h>
#include <errno.h>
#include <pthread.h>

void* asynch_copy(){



}


int main(int argc, char* argv[])
{

  struct aiocb aio;

  int pord_fd = 0, thread_count = 0;
  int length  = 200000;//inputtan gelmeli
  char *data  = (char*) malloc(length);//buffer. Ismi buffer olabilir

  pthread_t threads[thread_count];

  port_fd = open("data.dat", O_RDONLY)//O_RDONLY gerek varmi?
  // data.dat inputtan gelicek

  if(port_fd == -1)
  {
      perror("open");//duzgun error bastir, perror arastir.
      return EXIT_FAILURE;
  }

  memset(&aio, 0, sizeof(struct aiocb));//buffer da memsetlenmicek miydi?

  aio.aio_fildes = port_fd;
  aio.aio_buf    = data;
  aio.aio_nbytes = length;

  int i;
  for (i = 0; i < thread_count; i++) {
    pthread_create(threads[i], NULL, asynch_copy, NULL ){

    }
  }

//------------------------------------------------///
  aio_read(&aio);//why the fuck is this.

  printf("read process has started\n");

  while(aio_error(&aio) == EINPROGRESS){}

  err = aio_error(&aio);
  ret = aio_return(&aio);

  if(err != 0)
  {
    printf("Error at aio_error():\n", strerror(err));
    close(port_fd);
    exit(2);
  }

  printf("ret value %d\n",ret );// bu degerle napiyoruz ? duzgun output vermek lazim
  close(port_fd);
  printf("Reading is finished\n" );
  return EXIT_SUCCESS;
}
