#include <fcntl.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aio.h>
#include <errno.h>
#include <pthread.h>

/*Oguzhan Ulucay 2014510070 CME3205 ASSINGMENT 3 */

long buffer_sizes[] = {0,0,0,0,0,0,0,0,0,0};  //Filled in the create_fill_file function.
long offsets[]      = {0,0,0,0,0,0,0,0,0,0};  //Filled in the create_fill_file function.
long file_size;

//Thread Parameters
struct arg_struct{
    int which_thread;
    int source_fd;
    int dest_fd;
    long op_offset;  //Special offset of the thread.
    long buffer_size;  //Special buffer size of the thread
};
/*
    Thread Function.
    Takes parameters, starts to read, completes read op, starts write op, completes write op.

*/
void* asynch_copy(void* arguments){

    struct aiocb aio;
    struct arg_struct *args = (struct arg_struct*) arguments;

    char* buffer;  int err, ret;
    int source_fd = args->source_fd; int dest_fd   = args->dest_fd ;
    int temp_offset = args->op_offset; int which_thread = args->which_thread;

    printf("\n[%d].Thread -----> Welcomes you \n",which_thread);


    buffer = malloc(args->buffer_size*sizeof(char));
    memset(&aio, 0, sizeof(struct aiocb));
    memset(buffer, 0 ,sizeof(buffer));

    aio.aio_fildes = source_fd;
    aio.aio_buf    = buffer;
    aio.aio_nbytes = args->buffer_size;
    aio.aio_offset = temp_offset;

    aio_read(&aio);

    printf("[%d].Thread: read process has started                  Progress: ----->%%0<-----\n",which_thread);

    while(aio_error(&aio) == EINPROGRESS){}

    err = aio_error(&aio);
    ret = aio_return(&aio);

    if(err != 0)
    {
         printf("\nError at aio_error():%s\n", strerror(err));
         close(source_fd);
         exit(2);
    }


    printf("\n[%d].Thread: read operation completed                  Progress: ----->%%48<-----\n",which_thread);


    aio.aio_fildes = dest_fd; //Destination file's FILE DESCRIPTOR

    aio_write(&aio);  //Starts Write operation

    printf("\n[%d].Thread: write process has started at Offset: %d    Progress: ----->%%50<-----\n",which_thread,temp_offset);

    while(aio_error(&aio) == EINPROGRESS){}

    err = aio_error(&aio);

    if(err != 0)
    {
         printf("Error at aio_error():%d\n", err);
         close(dest_fd);
         exit(2);
    }

    ret = aio_return(&aio);

    printf("\n[%d].Thread: write operation completed at Offset: %d    Progress: ----->%%100<-----\n",which_thread,(temp_offset+args->buffer_size-1) );
    printf("\n[%d].Thread: returning to home\n\n",which_thread );

    pthread_exit(NULL);

}


/*
    Creates source.txt file and then fills it.

    @thread_portion  = Base size of all threads.
    @remainder       = from modulo.
    @char_value      = starts with "a".
*/
void create_fill_file(char *abs_path_source,int file_size, int thread_number){

  FILE *file;

  char *buffer = malloc((file_size+1)*sizeof(char));

  int i = 0; int char_value = 97;
  int thread_portion = 0;
  int remainder = 0;

  printf("Source File Creation is started\n");
  remainder = file_size%thread_number;
  thread_portion = (file_size - remainder)/thread_number;

  //Fills the buffers of all threads with base size.
  for ( i = 0; i < thread_number; i++) {
      buffer_sizes[i] = thread_portion;
  }

  //Add the extra sizes to thread's buffer with preceding order.
  for (i = 0; i < thread_number && remainder > 0; i++) {
      buffer_sizes[i] = buffer_sizes[i]+1;
      remainder--;
  }


  offsets[0] = 0;// First threads offset must be 0

  //Fills other offsets.
  for (i = 1; i < thread_number; i++) {
      offsets[i] = offsets[i-1] + buffer_sizes[i-1];
  }


  int j, file_counter = 0;

  //Fills the buffer which will be written to the source.txt
  for (i = 0; i < thread_number && file_counter <file_size; i++) {
      for (j = 0; j < buffer_sizes[i]; j++) {
          buffer[file_counter] = (char)char_value;
          file_counter++;
      }
      char_value++;//Changes the letter according to thread number.
  }


  buffer[file_size] = '\0';

  file = fopen(abs_path_source,"w");

  fprintf(file, buffer);
  printf("\nFile Creation Completed\n");
  fclose(file);

}

int main(int argc, char* argv[])
{
  int destination_fd;
  int source_fd = 0, dest_fd = 0, thread_count = 0;
  int file_size;//inputtan gelmeli
  int size_type = 0;
  char* abs_path_source = malloc(250*sizeof(char)); char* abs_path_dest = malloc(250*sizeof(char));

  char *source_filename = "source.txt";
  char *dest_filename   = "destination.txt";

  //-------------------Error Checking---------------------------//
  if(argc < 6)
  {
      printf("You have entered missing number of arguments, Please try again.\n");
      return(0);
  }
  if(argc > 6)
  {
      printf("You have entered too much number of arguments, Please try again.\n");
      return(0);
  }

  //   Takes arguments
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
  //-------------------Error Checking---------------------------//
  thread_count = atoi(argv[3]);
  if(thread_count < 1 || thread_count >10){
      printf("You have entered wrong number of THREAD, Please try again.\n");
      return(0);
  }

  size_type    = atoi(argv[4]);
  if(size_type < 0 || size_type >1){
      printf("You have entered wrong type of SIZE TYPE, Please try again.\n");
      return(0);
  }

  file_size    = atoi(argv[5]);
  if(file_size < 1 || file_size >200){
      printf("You have entered wrong amount of FILE SIZE, Please try again.\n");
      return(0);
  }

  printf("\n----------WELCOME----------\n");
  printf("\nYour thread number of choice:   %d\n",thread_count);
  printf("Your size type  of choice:      %d\n",size_type);
  printf("Your file size  of choice:      %d\n\n",file_size);

  if(size_type == 1) file_size = file_size*1000000; // Calculate MB size of the file.

  struct arg_struct args[thread_count];
  pthread_t threads[thread_count];
  //int offsets[thread_count];

  create_fill_file(abs_path_source,file_size,thread_count); // Invokes to create source File.

  mode_t file_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  source_fd = open(abs_path_source, O_RDONLY); //O_RDONLY gerek varmi?
  dest_fd   = open(abs_path_dest, O_WRONLY | O_CREAT | O_TRUNC, file_mode);


  if(source_fd == -1)
  {
      perror("open source");
      return EXIT_FAILURE;
  }
  if(dest_fd == -1)
  {
      perror("open dest");
      return EXIT_FAILURE;
  }


    int i = 0;
    for (i = 0; i < thread_count; i++) {

        args[i].source_fd    = source_fd;
        args[i].dest_fd      = dest_fd;
        args[i].op_offset    = offsets[i];
        args[i].buffer_size  = buffer_sizes[i];
        args[i].which_thread = i+1;

    }

    //-----------------THREAD CREATING----------------------------///
      int status = 0;

      //Creates threads
      for (i = 0; i < thread_count; i++) {

         status = pthread_create(&threads[i], NULL, asynch_copy, (void*)&(args[i]));
         if(status != 0){ printf("Thread creation error\n");}
      }

      for (i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
      }

      close(source_fd);
      close(dest_fd);
      printf("Copying Operation is finished. All File descriptors are closed. Have a good day.\n\n" );
      return EXIT_SUCCESS;
}
