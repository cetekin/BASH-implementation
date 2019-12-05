#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

/* TODOs */
//comm line buffer limit?
//cd command (chdir)
//rm exec?
//shell exit


/* Fonksiyon Prototipleri */
int check_command();
void execute_command();
void parse_command_line(char* arguments)


/* Fonksiyon Tanimlari */


void parse_command_line(char* arguments) {


}


void execute_command() {

}

/* Bu fonksiyon ile; cd,rm komutlari ile ilgili durumlar ele alinmistir */
int check_command() {

    return 0;

}





int main(int argc, char const *argv[]) {

    char command_line[500];
    char* arguments[50];
    char* arg;
    int i;

        while (1) {
            printf("17011603-shell$ ");
            fgets(command_line, 500, stdin);

            arg = strtok(command_line, " \n");
            i = 0;

            while (arg != NULL) {
                    arguments[i] = arg;
                    arg = strtok (NULL, " \n");
                    i++;
            }

            //TODO: kontrol!!
            arguments[i] = NULL;

            check_command(arguments[0]);

            /* Child Process */
            if (fork() == 0) {
                    /* Komut, tum parametreleriyle calistirilir */
                    execvp(command_line, arguments);
                    /* stdout buffered oldugundan, sonucun depolanmadan gosterilmesi icin buffer flushlanir */
                    fflush(stdout);
            }

            else {
                    /* Parent proses, child processde kosulan komutun tamamlanmasini bekliyor */
                    wait(NULL);
            }



    }
    return 0;
}
