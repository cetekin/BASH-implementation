#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define STDIN 0
#define STDOUT 1

/* TODOs */
//pipe buffer problem
//rm exec?
//shell exit


/* Fonksiyon Prototipleri */
void execute_command();
void parse_command_line(char* command_line);
void set_pipe_attributes(int fd[2]);
void change_std_descriptors(int fd[2], int* original_stdin, int* original_stdout);
void restore_std_descriptor(int original_stddesc, int pipe_flag, int stddesc);
void init_pipe_components(int* original_stdin, int* original_stdout);


/* Fonksiyon Tanimlari */
/*************************/


/* Pipe bufferina yonlendirilen stdin ve stdout bu fonksiyon ile eski haline donusturulur */
void restore_std_descriptor(int original_stddesc, int pipe_flag, int stddesc) {

    if (pipe_flag) {
        /* STDOUT eski haline getiriliyor (pipe descriptorlari da kapatiliyor ayni zamanda) */
        dup2(original_stddesc, stddesc);
        close(original_stddesc);
    }

}

/* Bu fonksiyon, stdin stdout betimleyicilerinin pipe uclarina baglanmasini saglamaktadir */
void connect_std_to_pipe(int pipe_fd, int std_choice) {
    dup2(pipe_fd, std_choice);
    close(pipe_fd);
}

/* Bu fonksiyon, uygun formatte gelen argumanlar ile birlikte, belirtilen komutu calistirir */
void execute_command(char** arguments) {

    /* Tilde karakteri cd programi tarafindan taninmadigi icin, HOME path environment degiskeninden cekilmistir */
    if ( (arguments[0] != NULL) && (!strcmp(arguments[0],"cd")) ) {
        if (!strcmp(arguments[1],"~")) {
            chdir(getenv("HOME"));
        }
        else{
            chdir(arguments[1]);
        }
    }

    else {
        /* Child Process */
        if (fork() == 0) {
                /* Komut, tum parametreleriyle calistirilir */
                execvp(arguments[0], arguments);
        }

        else {
                /* Parent proses, child processde kosulan komutun tamamlanmasini bekliyor */
                wait(NULL);
        }
    }

}


/*
 * Komut satirini tum argumanlar ile birlikte parcalarina ayirir.
 * Komut satirinda pipe operatoru kullanilmissa, komutlar ayri ayri calistirilip ciktilari aktarilir.
*/
void parse_command_line(char* command_line) {

    char* arg;
    int i;
    char** arguments;
    int pipe_flag;
    int fd[2];
    int original_stdin;
    int original_stdout;
    int prev_pipe_read_end;


    arguments = (char**)calloc(50, sizeof(char*));


    arg = strtok(command_line, " \n");
    i = 0;
    pipe_flag = 0;

    /* Mevcut stdin ve stdout referanslari saklaniyor */
    original_stdin = dup(STDIN);
    original_stdout = dup(STDOUT);

    /* Komut ve argumanlar tek tek parcalaniyor */
    while (arg != NULL) {

        /* Eger pipe operatoru kullanilmissa mevcut komutun ciktisi bir pipe bufferina aktarilir (STDOUT yerine) */
        if (!strcmp(arg,"|")) {


            /* Yeni pipe olusturuluyor */
            pipe(fd);

            /* stdout, pipe yazma ucuna baglaniyor */
            connect_std_to_pipe(fd[1], STDOUT);

            /* Eger onceden pipe operatoru ile karsilasildiysa */
            if (pipe_flag) {
                /* stdin, Bir onceki komut icin olusturulan pipe in okuma ucuna baglaniyor. Boylece onceki komut ciktisi aliniyor */
                connect_std_to_pipe(prev_pipe_read_end, STDIN);
            }

            /* Olusturulan pipe in okuma ucu bir sonraki komut tarafindan kullanilacagi icin saklaniyor */
            prev_pipe_read_end = fd[0];
            pipe_flag = 1;

            arguments[i+1] = NULL;

            /* Komut calistirilir */
            execute_command(arguments);

            /* Pipe operatorunden sonra gelen siradaki komut argumanlari icin gerekli alan ayrilir, degiskenler ilklendirilir */
            free(arguments);
            arguments = (char**)calloc(50, sizeof(char*));

            i = 0;
            arguments[i] = strtok (NULL, " \n");

        }
        else {
            arguments[i] = arg;
        }

        /* Aradaki bosluklara gore parse edilen komut satirinda, bir sonraki arguman alinir */
        arg = strtok (NULL, " \n");
        i++;

    }


    arguments[i] = NULL;

    /* Pipe operatoru kullanilmissa, stdin ve stdout dosya betimleyicileri degistirildigi icin, son komutun
    ciktisinin terminale basilabilmesi icin stdout eski haline getiriliyor */
    restore_std_descriptor(original_stdout, pipe_flag, STDOUT);

    /* Eger onceden pipe operatoru ile karsilasildiysa */
    if (pipe_flag) {
      /* stdin, Bir onceki komut icin olusturulan pipe okuma ucuna baglaniyor. Boylece onceki komut ciktisi aliniyor */
      connect_std_to_pipe(prev_pipe_read_end, STDIN);
    }

    /* Komut satirindaki son komut (pipe kullanilmissa) veya pipesiz tek komut calistirilir */
    execute_command(arguments);

    /* Shelle girilecek sonraki komutlar stdin uzerinden alinacagi icin, stdin de eski haline getirilir */
    restore_std_descriptor(original_stdin, pipe_flag, STDIN);
}


int main(int argc, char const *argv[]) {

    char command_line[1000];


        while (1) {
            printf("17011603-shell$ ");
            fgets(command_line, 1000, stdin);

            /* Komut satiri parcalanir ve uygun sekilde calistirilir */
            parse_command_line(command_line);

    }
    return 0;
}
