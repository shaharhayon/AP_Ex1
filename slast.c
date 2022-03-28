#include <stdio.h>
#include <utmp.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>

bool is_positive_integer(char arg[]){
    if(arg[0] == '-') return false;
    for(int i = 0; arg[i] != 0; i++){
        if(!isdigit(arg[i])) return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("Error: wrong syntax. \nInput X (number of rows to be printed).\n");
        exit(-1);
    }

    int X = atoi(argv[1]);

    int fd;
    struct utmp utmp_record, disconnect_record;
    int size = sizeof(struct utmp);
    bool finished = false;
    bool show_all = false;

    // verify argument integrity
    if(!is_positive_integer(argv[1])) {
        perror("Error: wrong input. \nX must be a positive integer.\n");
        exit(-1);
    }

    if (X == 0) show_all = true;
    

    // open file descriptor
    fd = open(WTMP_FILE, O_RDONLY);
    if (fd == -1)
    {
        perror("Error: cant open file");
        exit(-1);
    }

    // move fd pointer to last entry
    lseek(fd, -size, SEEK_END);
    // iterate entries
    for (int i = 0; ((show_all) && (!finished)) || ((i < X) && (!finished)); i++)
    {
        // check if reading the first entry
        if(lseek(fd, 0, SEEK_CUR) == 0){
            finished = true;
        }

        if (read(fd, &utmp_record, size) == size)
        {
            // process only entries that are user logins
            if (utmp_record.ut_type == USER_PROCESS)
            {
                // user
                printf("%-8.8s ", utmp_record.ut_user);

                // line
                printf("%-12.12s ", utmp_record.ut_line);

                // ip address
                struct in_addr ip;
                ip.s_addr = utmp_record.ut_addr_v6[0];
                printf("%-17.17s", inet_ntoa(ip));

                // connect time
                time_t start_time = utmp_record.ut_tv.tv_sec;
                struct tm *start_tm = localtime(&start_time);
                char buffer_start[17];
                strftime(buffer_start, 17, "%a %b %d %H:%M", start_tm);
                printf("%s - ", buffer_start);

                // disconnect time
                off_t current_offset = lseek(fd, 0, SEEK_CUR);
                int found = 0;
                // iterate forward to find the entry's disconnect entry
                while((read(fd, &disconnect_record, size) == size) && (!found)){
                    if(strncmp(disconnect_record.ut_line, utmp_record.ut_line, sizeof(utmp_record.ut_line)) == 0){
                        found = 1;
                        time_t end_time = disconnect_record.ut_tv.tv_sec;
                        struct tm *end_tm = localtime(&end_time);
                        char buffer_end[6];
                        strftime(buffer_end, 6, "%H:%M", end_tm);
                        printf("%s ", buffer_end);

                // time difference
                        time_t diff = difftime(end_time, start_time);
                        int minutes = diff / 60;
                        int hours = minutes / 24;
                        minutes %= 60;
                        printf("(%.2d:%.2d)\n", hours, minutes);
                    }
                }
                if(!found){
                    printf("Logout not found\n");
                }

                // return the offset to its current offset
                lseek(fd, current_offset, SEEK_SET);
            }

            // if not a user login entry, dont count it
            else{
                i--;
            }

            // move fd pointer to next entry
            lseek(fd, -size * 2, SEEK_CUR);
        }


    }
    return 0;
}