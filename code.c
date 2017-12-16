#include <fcntl.h> //what is this
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aio.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_aio;

int buffer_sizes[] = {0,0,0,0,0,0,0,0,0,0};
int offsets[]      = {0,0,0,0,0,0,0,0,0,0};
int file_size;

struct arg_struct{
    int source_fd;
    int dest_fd;
    int op_offset;
    int buffer_size;
};

void* asynch_copy(void* arguments){

    struct aiocb aio;
    char* buffer;
    int err, ret;
    struct arg_struct *args = (struct arg_struct*) arguments;
    int source_fd = args->source_fd;
    int dest_fd   = args->dest_fd ;
    int temp_offset = args->op_offset;
    //sem_wait(&semaphore);
    //sem_post(&semaphore);
    buffer = malloc(args->buffer_size*sizeof(char));

    memset(&aio, 0, sizeof(struct aiocb));//buffer da memsetlenmicek miydi?
    memset(buffer, 0 ,sizeof(buffer));
    aio.aio_fildes = source_fd;
    aio.aio_buf    = buffer;
    aio.aio_nbytes = args->buffer_size;

    while(0){
        aio.aio_offset = temp_offset;

        aio_read(&aio);

        printf("read process has started\n");

        while(aio_error(&aio) == EINPROGRESS){}

        err = aio_error(&aio);
        ret = aio_return(&aio);

        if(err != 0)
        {
          printf("Error at aio_error():\n", strerror(err));
          close(source_fd);
          exit(2);
        }


        printf("ret value %d\n",ret );// bu degerle napiyoruz ? duzgun output vermek lazim

        //write section

        aio.aio_fildes = dest_fd;
        aio_write(&aio);

        printf("write process has started\n");

        while(aio_error(&aio) == EINPROGRESS){}

        err = aio_error(&aio);
        ret = aio_return(&aio);

        if(err != 0)
        {
          printf("Error at aio_error():\n", strerror(err));
          close(dest_fd);
          exit(2);
        }


        printf("ret value %d\n",ret );


        temp_offset+=args->op_offset;
        if(temp_offset>file_size){
            break;
        }
    }
    pthread_exit(NULL);
}

void create_fill_file(int file_size, int thread_number){

  FILE *file;

  char buffer[file_size+1];

  int i; int char_value = 97;
  int thread_portion = 0;
  int letter_change_limit = 0;
  int counter_of_thread = 2;

  int temp_remainder = 0;
  int remainder = 0;

  remainder = file_size%thread_number;
  if(remainder>1){
      temp_remainder = file_size%(thread_number - 1);
      thread_portion = (file_size - temp_remainder)/(thread_number-1);
  }
  else if(remainder==1) thread_portion = (file_size - remainder)/thread_number ;
  else thread_portion = file_size/thread_number;

  letter_change_limit = thread_portion;
  //Offsetleri yukle
  //buffer_sizes yukle
  offsets[0] = 0;
  buffer_sizes[0] = thread_portion;

  for (i = 0; i < thread_number+1; i++) {
      offsets[i] = offsets[i-1] + thread_portion;
      buffer_sizes[i] = thread_portion;
  }
  if(temp_remainder!=0 || remainder !=0){
      i++;
      offsets[i] = offsets[i-1]+thread_portion +1;
      buffer_sizes[i] = thread_portion + 1;
  }
  for (i = 0; i < file_size; i++) {
      if(i > thread_portion-1  && counter_of_thread != thread_number){//Will start to enter when 2. thread section comes
          char_value++;
          thread_portion=letter_change_limit+thread_portion;
          counter_of_thread++;
      }
      buffer[i] = (char)char_value;//a'yi 97 yapip dene
  }

  buffer[file_size] = '\0';

  file = fopen("source.txt","w");

  fprintf(file, buffer);

  fclose(file);

}


int main(int argc, char* argv[])
{

  struct arg_struct args;

  int destination_fd;
  int source_fd = 0, dest_fd = 0, thread_count = 0;
  int file_size;//inputtan gelmeli
  int size_type = 0;
  char* abs_path_source = malloc(250*sizeof(char)); char* abs_path_dest = malloc(250*sizeof(char));

  char *source_filename = "source.txt";
  char *dest_filename   = "destination.txt";

  if(strcmp(argv[1],"-")!=0){
      strcpy(abs_path_source, argv[1]);
      strcat(abs_path_source, source_filename);
  }else{
      strcpy(abs_path_source,source_filename);

  }

  if(strcmp(argv[2],"-")!=0){
      strcpy(abs_path_dest, argv[2]);
      strcat(abs_path_dest, dest_filename);
  }else{
      strcpy(abs_path_dest,dest_filename);
  }

  thread_count = atoi(argv[3]);
  size_type    = atoi(argv[4]);
  file_size    = atoi(argv[5]);

  if(size_type == 1) file_size = file_size*1000000;

  pthread_t threads[thread_count];
  //int offsets[thread_count];
  create_fill_file(file_size,thread_count);

  mode_t file_mode      = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  source_fd = open(abs_path_source, O_RDONLY); //O_RDONLY gerek varmi?
  dest_fd   = open(abs_path_dest, O_WRONLY | O_CREAT | O_TRUNC, file_mode);

  if(source_fd == -1)
  {
      perror("open source");//duzgun error bastir, perror arastir.
      return EXIT_FAILURE;
  }
  if(dest_fd == -1)
  {
      perror("open dest");//duzgun error bastir, perror arastir.
      return EXIT_FAILURE;
  }

  args.source_fd = source_fd;
  args.dest_fd   = dest_fd;



  // data.dat inputtan gelicek
  //sem_init(&sem_aio, 0, 1);

//------------------------------------------------///
  int i, status = 0;

  for (i = 0; i < thread_count; i++) {
     args.op_offset = offsets[i];
     args.buffer_size = buffer_sizes[i];
     status = pthread_create(&threads[i], NULL, asynch_copy, (void*)&args );
     if(status != 0){ printf("Thread creation error\n");}
  }

  for (i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }

  close(source_fd);
  close(dest_fd);
  printf("Copying is finished\n" );
  return EXIT_SUCCESS;
}
