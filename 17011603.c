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
//comm line buffer limit?
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


/* Bu fonksiyonda kullanilan pipe bufferinin nonblock olmasi icin gerekli islemler yapilmistir */
void set_pipe_attributes(int fd[2]) {

    int pipe_flags;

    /* Pipe read ucu ayarlari */
    pipe_flags = fcntl(fd[0], F_GETFL);
    pipe_flags |= O_NONBLOCK;
    fcntl(fd[0], F_SETFL, pipe_flags);

    /* Pipe write ucu ayarlari */
    pipe_flags = fcntl(fd[1], F_GETFL);
    pipe_flags |= O_NONBLOCK;
    fcntl(fd[1], F_SETFL, pipe_flags);

}

/* Komut satirinda pipe operatoru kullanilmasi durumunda yapilmasi gereken islemler bu fonksiyonda toplanmistir */
void init_pipe_components(int* original_stdin, int* original_stdout) {

    int fd[2];

    /* Komutlarin haberlesmesi icin pipe olusturulur */
    pipe(fd);

    /* Pipe blocking ozelligi kapatilir */
    set_pipe_attributes(fd);

    /* stdin ve stdout olusturulan pipein bufferina baglanir */
    change_std_descriptors(fd, original_stdin, original_stdout);

}

/* stdin ve stdout, olusturulan pipe bufferina yonlendirilir */
void change_std_descriptors(int fd[2], int* original_stdin, int* original_stdout) {

    /* Mevcut komuttan sonra gelen komutlarin inputlarini pipe bufferindan alabilmeleri icin, STDIN pipe bufferina baglanir */
    *original_stdin = dup(STDIN);
    dup2(fd[0], STDIN);

    /* Mevcut komutlarin ciktisi, STDOUT yerine pipe bufferina yonlendirilir */
    *original_stdout = dup(STDOUT);
    dup2(fd[1], STDOUT);

}

/* Pipe bufferina yonlendirilen stdin ve stdout bu fonksiyon ile eski haline donusturulur */
void restore_std_descriptor(int original_stddesc, int pipe_flag, int stddesc) {

    if (pipe_flag) {
        /* STDOUT eski haline getiriliyor */
        dup2(original_stddesc, stddesc);
        close(original_stddesc);
    }

}

/* Bu fonksiyon, uygun formatte gelen argumanlar ile birlikte, belirtilen komutu calistirir */
void execute_command(char** arguments) {

    /* Tilde karakteri cd programi tarafindan taninmadigi icin, HOME path environment degiskeninden cekilmistir */
    if (!strcmp(arguments[0],"cd")) {
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
                /* STDOUT buffered oldugundan, sonucun depolanmadan gosterilmesi icin buffer flushlanir */
                fflush(stdout);
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
    int original_stdout;
    int original_stdin;


    arguments = (char**)calloc(50, sizeof(char*));


    arg = strtok(command_line, " \n");
    i = 0;
    pipe_flag = 0;

    /* Komut ve argumanlar tek tek parcalaniyor */
    while (arg != NULL) {

        /* Eger pipe operatoru kullanilmissa mevcut komutun ciktisi bir pipe bufferina aktarilir (STDOUT yerine) */
        if (!strcmp(arg,"|")) {

            /* Komut akisinda pipe operatoru varsa; bundan sonra komutlarin, inputu pipedan almasi saglanacaktir */
            if (!pipe_flag) {
                pipe_flag = 1;

                /* Gerekli input & output yonlendirmeleri ve deklarasyonlar yapilir */
                init_pipe_components(&original_stdin, &original_stdout);
            }

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
