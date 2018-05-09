/*
 * Date: 2018-05-04
*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ncurses.h>
#include<pthread.h>

struct TmpFile
{
    char *errfile;
    char *resfile;
};

// Print help message
void help(char* name);

// Empty file `ERRFILE` and `RESFILE`
void emptyFile(char *ERRFILE, char *RESFILE);

// Print `length` characters of '─'
void printLine(int length);

// wait to press 'n' and show more message
void wait2showmore();

// Exit program when user press 'q'
void* waitInput(void *args);

int main(int argc, char* argv[])
{
	char c;
	
	while((c=getopt(argc, argv, "h")) != -1)
	{
		switch(c)
		{
			case 'h':
				help(argv[0]);
				return 0;
				break;
			default:
				printf("in default\n");
		}
	}

	if(argc != 2)
	{
		printf("Error usage!\n");
		help(argv[0]);
		return 1;
	}

	// Test if can open file argv[1]
	if(fopen(argv[1], "r") == NULL)
	{
		printf("Error: Can't open file: '%s'\n", argv[1]);
		return 2;
	}

        // Create temporary file
        char ERRFILE[30] = {0,};
        char RESFILE[30] = {0,};
        FILE *tmp = popen("mktemp", "r");
        fgets(ERRFILE, sizeof(ERRFILE), tmp);
        pclose(tmp);

        tmp = popen("mktemp", "r");
        fgets(RESFILE, sizeof(RESFILE), tmp);
        pclose(tmp);


        // Remove trailing newline character
        char *pos;
        if((pos = strchr(ERRFILE, '\n')) != NULL)
        {
            *pos = '\0';
        }
        if((pos = strchr(RESFILE, '\n')) != NULL)
        {
            *pos = '\0';
        }

        // printf("errfile: %s\n", ERRFILE);
        // printf("resfile: %s\n", RESFILE);

        


	initscr();
	cbreak();
	noecho();
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);


        // Create thread to wait user input
        struct TmpFile tmpfile = {ERRFILE, RESFILE};
        pthread_t tid;
        if(pthread_create(&tid, NULL, waitInput, &tmpfile) != 0)
        {
            fprintf(stderr, "Create thread failed!\n");
            return 1;
        }


 
        clear();
	printw("wait a little ...");
	refresh();
	char cmd[100];
	sprintf(cmd, "fping -f %s 2> %s > %s", argv[1], ERRFILE, RESFILE);
	while(1)
	{
		// Empty file `ERRFILE` and `RESFILE`
		emptyFile(ERRFILE, RESFILE);
		system(cmd);

		// Get current time
		FILE* fin = popen("date '+%F %T'", "r");
		char time[30];
		if(fgets(time, sizeof(time), fin) == NULL)
		{
			printf("Error: Can't get current time!\n");
			return 3;
		}
		fclose(fin);

		clear();

		// Print time
		mvprintw(0, COLS-20, "%s", time);

		// print result
		attron(A_BOLD|COLOR_PAIR(3));
		printw("Result:\n");
		printLine(40);
		attroff(A_BOLD|COLOR_PAIR(3));
		fin = fopen(RESFILE, "r");
		if(fin == NULL)
		{
			printf("Can't open file: '%s'\n", "res");
			return 1;
		}

		char line[100];
		while(fgets(line, sizeof(line), fin) != NULL)
		{
			if(strstr(line, "alive"))
			{
				attron(COLOR_PAIR(2));
				printw("%s", line);
				attroff(COLOR_PAIR(2));
			}
			else
			{
				attron(COLOR_PAIR(1));
				printw("%s", line);
				attroff(COLOR_PAIR(1));
			}

			refresh();
			wait2showmore();
		}
		fclose(fin);

		// print error
		attron(A_BOLD|COLOR_PAIR(3));
		printw("\nError:\n");
		wait2showmore();
		printLine(40);
		wait2showmore();
		attroff(A_BOLD|COLOR_PAIR(3));
		fin = fopen(ERRFILE, "r");
		if(fin == NULL)
		{
			printf("Can't open file: '%s'\n", "error");
			return 1;
		}
		while(fgets(line, sizeof(line), fin) != NULL)
		{
			attron(COLOR_PAIR(4));
			printw("%s", line);
			attroff(COLOR_PAIR(4));

			refresh();
			int currX, currY;
			getyx(stdscr, currY, currX);
			wait2showmore();
		}
		fclose(fin);

		refresh();
		sleep(1);
	}

	
	endwin();

	return 0;
}

// Print help message
void help(char* name)
{
	printf("\nUsage:\n");
	printf("\t%s FILENAME\n", name);
	printf("\t%s -h\n\n", name);
        printf("Interactive Commands:\n");
	printf("\tn\tShow next page\n");
	printf("\tq\tQuit\n\n");
}

// Empty file `ERRFILE` and `RESFILE`
void emptyFile(char *ERRFILE, char *RESFILE)
{
	char cmd[100];
	sprintf(cmd, "> %s; > %s", ERRFILE, RESFILE);
	system(cmd);
}

// Print `length` characters of '─'
void printLine(int length)
{
	int counter = 0;
	while(counter < length)
	{
		addch(ACS_HLINE);
		++counter;
	}
	printw("\n");
}

// wait to press 'n' and show more message
void wait2showmore()
{
	int currX, currY;
	getyx(stdscr, currY, currX);
	if(currY == LINES-1)
	{
		printw("Press n to show more\n");
		refresh();
		while(true)
		{
			char input = getch();
			if(input == 'n')
			{
				clear();
				break;
			}
		}
	}
}

// Exit program when user press 'q'
void* waitInput(void *args)
{
    struct TmpFile *tmpfile = (struct TmpFile*)args;
    while(1)
    {
        // int ch = getch();
        int ch = getc(stdin);
        if(ch == 'q')
        {
            puts("Quit");
            endwin();

            // Remove temporary file
            unlink(tmpfile->errfile);
            unlink(tmpfile->resfile);
            exit(0);
        }
    }
}
