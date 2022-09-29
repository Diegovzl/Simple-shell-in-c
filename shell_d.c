#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>


#define PROMPT "#Diego_V> "
#define MAX 51

char SHELL[MAX];
char PATH[MAX];
char HOME[MAX];
char PWD[MAX];

void Imprimir_el_prompt();
void Leer_ordenes_desde_teclado();
void Analizar_orden_y_diferenciar_sus_campos(char cadena[], int plano);
void syst(char orden[]);
void cd(char directorio[]);
int cat(int argc, char *argv[]);
void comando(char* argumento[MAX], int plano);
void redirecSalida1(char cadena[MAX]);
void redirecSalida2(char cadena[MAX]);
void tuberias(char* argumento1[MAX], char* argumento2[MAX]);
void despedida();

int main(int argc, char const *argv[]){



    int salida = dup(1); // Guardo la salida estandar.



	system("clear");
	signal(SIGINT, despedida);
	while (1){

        strcpy(PATH,getenv("PATH"));  /////// arreglar
        strcpy(HOME,getenv("HOME"));// Obteniendo el directorio home y cargando en HOME
        strcpy(SHELL,getenv("SHELL"));// Obteniendo el directorio de shell y cargando en SHELL
        getcwd(PWD,MAX);

        close(1);/* Cierro la salida actual, en caso de que sea un fichero*/
        dup(salida); // Asigno la salida est�ndar, es decir, la consola.

		Imprimir_el_prompt();
		Leer_ordenes_desde_teclado();

	}
		return 0;
}

void Imprimir_el_prompt(){
   	char cwd[1024];
   	if (getcwd(cwd, sizeof(cwd)) != NULL)
       	fprintf(stdout, "%s", cwd);
   	else
       	perror("getcwd() error");

	printf(PROMPT);
}

void Leer_ordenes_desde_teclado(){
    int i, segplano = 0;
	char cadena[MAX];
	scanf("\n%[^\n]",cadena);
    segplano=0;
    for(i=0;cadena[i] != '\0'; i++){
        if(cadena[i] == '&') {
                    cadena[i] = '\0';
                    segplano = 1;
                    }
    }
	Analizar_orden_y_diferenciar_sus_campos(cadena, segplano);
}

void Analizar_orden_y_diferenciar_sus_campos(char cadena[], int plano){
    char cadlsa[] = "lsa";
    char cadclr[] = "clr";
    char cadcd[] = "cd";
    char caddir[] = "ls";
    char cadcat[] = "cat";
    int cambdir = 0, concat = 0, pcont = 0, k = 0, redireccion1 = 0, redireccion2 = 0, echo = 0, pipe = 0;
    char cadena2[MAX];
    char delimitador[] = " "; 
    char cadenafinal[MAX];
    char argumentoInd[MAX][MAX];
    char argumentoInd2[MAX][MAX];
    char *argumento[MAX];
    char *argumento2[MAX];
    argumento[0] = NULL;    

     strcpy( cadena2, cadena );
     
    char *token = strtok(cadena, delimitador); // para contar la cantidad de palabras.
    if(token != NULL){
        while(token != NULL){
            token = strtok(NULL, delimitador);
            pcont++;
        }
    }


    char *token2 = strtok(cadena2, delimitador); // Para separar los campos de la cadena.
    if(token2 != NULL){
        while(token2 != NULL){
            // Sólo en la primera pasamos la cadena; en las siguientes pasamos NULL
            if(0 == strcmp(token2, cadlsa)) // lsa
            {
                strcpy( argumentoInd[k], "ls" ); // conveniencia: si se escribe algo antes de de "lsa", ej :dfs lsa, se reemplazaŕa todo por ls -l. 
                argumento[k] = argumentoInd[k];  //agregamos la cadena al puntero
                k++;
                strcpy( argumentoInd[k], "-l" ); // conveniencia: si se escribe algo antes de de "lsa", ej :dfs lsa, se reemplazaŕa todo por ls -l. 
                argumento[k] = argumentoInd[k];  //agregamos la cadena al puntero
                k++;

            }else if (0 == strcmp(token2, cadclr)) //clear
            {
                strcpy( argumentoInd[k], "clear" );
                argumento[k] = argumentoInd[k];  //agregamos la cadena al puntero
                k++;
                
            }else if (0 == strcmp(token2, cadcd)) //cd
            {
                cambdir = 1;
                if (pcont == 1) return;
                pcont--;
                
            }else if (0 == strcmp(token2, cadcat)) // cat
            {
                concat = 1;
                strcpy( argumentoInd[k], token2 );
                argumento[k] = argumentoInd[k];  /* Y finalmente una vez creada la cadena, se la pasamos al puntero */

                k++;
            }else if (0 == strcmp(token2, "echo"))// echo
            {
                strcpy(cadenafinal, token2);
                strcat( cadenafinal, " " ); // añade un espacio luego de añadir la palabra.
                echo = 1;
                if (pcont == 1) return;
                pcont--;

            }else if (0 == strcmp(token2, ">")) //REDIRECCIÓN1 
            {
                
                redireccion1 = 1;
                pcont = pcont-2;

            }else if (0 == strcmp(token2, ">>")) //REDIRECCIÓN2
            {
                
                redireccion2 = 1;
                pcont = pcont-2;

            }else if (redireccion1 == 1) /*REDIRECCION1 Si encuentra un > cortar� la cadena que ser� el fichero que quiere utilizar
        para la salida */
            {
                strcpy(cadenafinal, token2);           
                redirecSalida1(cadenafinal);
                break;
            }else if (redireccion2 == 1) /*REDIRECCION2 Si encuentra un >> cortar� la cadena que ser� el fichero que quiere utilizar
        para la salida */
            {
                strcpy(cadenafinal, token2);            
                redirecSalida2(cadenafinal);
                break;

            }else if (0 == strcmp(token2, "|")) //TUBERIAS
            {
                pipe = 1;    

            } else if (pipe == 1)
            {
                strcpy( argumentoInd2[k], token2 );
                argumento2[k] = argumentoInd2[k];  /* Y finalmente una vez creada la cadena, se la pasamos al puntero */

                k++;

            }else // si no es una de las palabras reservadas, se añade normalmente al string
            {


                if (cambdir == 1 || echo == 1) // si se enviará a la función cd o se escribió la instrucción echo
                {
                    strcat(cadenafinal, token2 ); // Añade la palabra a la cadena final.
                
                    pcont--; // resta 1 a la cantidad de palabras que quedan por agregar a la cadenafinal.
                    //printf("cantidad de palabras = %d\n", pcont);

                    if (pcont == 0 ) break; /* Si el contador de palabras lega a cero o se hizo redireccionamiento
                    , rompe el ciclo. Éste break es importante para no añadir un espacio al final de cada instrucción.*/
                    strcat( cadenafinal, " " ); // añade un espacio luego de añadir la palabra.
                }else  
                {
                    strcpy(argumentoInd[k], token2);

                    argumento[k] = argumentoInd[k];  /* Y finalmente una vez creada la cadena, se la pasamos al puntero 
                    argumento que ser� el que se ejecute con la funci�n adecuada */

                    k++;

                    //pcont--; // resta 1 a la cantidad de palabras que quedan por agregar a la cadenafinal.
                }
            }
            token2 = strtok(NULL, delimitador);
        }
    }
    argumento[k] = NULL; // Asignamos NULL al �ltimo comando a ejecutar para el EXECV

	if (cambdir == 1)
	{
		cd(cadenafinal);

    }else if (echo == 1)
    {
        syst(cadenafinal);

	}else if (concat == 1)
    {
  
        cat (k, argumento);

    }else if (pipe == 1)
    {
        argumento[k] = NULL; // Asignamos NULL al �ltimo comando a ejecutar para el EXECV
        tuberias(argumento,argumento2);

    }else
    {
        comando (argumento, plano);     
    }

	strcpy(cadenafinal,"");
}


void syst(char orden[]){
	system(orden); // soporta pwd, dir, clear, 
}


void cd(char directorio[]){ // Cambia directorio con el uso de chdir.
	if (chdir(directorio) != 0)  {
		// Si no existe el directorio, retorna -1
		perror("Error");  //Muestra el mensaje de error arrojado por chdir.
	} 

}

int cat(int argc, char *argv[])
{ 
    int fd, i, ch; 
    
    for (i = 1; i < argc; i++) {                    /*iterate the loop for total argument count.*/
        
        fd = open(argv[1],O_RDONLY);            /*open the file in READONLY mode*/
        
        if(fd < 0) { 
            perror("open"); 
            goto OUT; 
        } 
        
        while(read(fd,&ch,1))                   /*Read one byte at a time*/
            
            write(STDOUT_FILENO,&ch,1);     /*Write the byte on Stdout*/
        
        close(fd);                              /*close the file discriptor*/
    }
 
    return 0;
OUT: 
        return -1;
}

void comando(char* argumento[MAX], int plano)

{   /* INICIO DE LA FUNCI�N CrearProceso */
    int estado=0;
    pid_t hijo; 
    hijo=fork();
/* Comprobamos si el hijo se cre� bien */
if (hijo==-1) printf("ERROR Creacion de proceso"); 
else if (hijo==0) {
            /* Y en caso de que el hijo est� bien creado, ejecutamos el proceso,
            si el programa comando a ejecutar no existe, nos da un error.*/
            execvp(argumento[0],argumento);
            perror("Error");
            exit(estado);
           }
else  { 
        if (plano == 0) hijo=wait(&estado);
        }

} /* FIN DE LA FUNCI�N CrearProceso */


/* Con esta funci�n si el usuario introduce una redirecci�n de salida a un 
fichero, en lugar de mostrar el comando en pantalla, lo guardar� en fichero
por ejemplo: man -k file > lista.file */

void redirecSalida1(char cadena[MAX])

{ /* INICIO DE LA FUNCI�N redirecSalida */
    char *cadenaPtr;
    cadenaPtr = cadena; // Puntero a la cadena
    close (1); // Cierro la salida est�ndar       
    open (cadenaPtr,O_CREAT | O_WRONLY,0777); // Asigno a la salida el fichero

} /* FIN DE LA FUNCI�N redirecSalida */

/* Con esta funci�n si el usuario introduce una redirecci�n de salida a un 
fichero, en lugar de mostrar el comando en pantalla, 
agrega contenido al final del fichero, si es que existe.
por ejemplo: cat file.file >> lista.file */

void redirecSalida2(char cadena[MAX])
{ /* INICIO DE LA FUNCI�N redirecSalida2 */
    char *cadenaPtr;
    cadenaPtr = cadena; // Puntero a la cadena
    close (1); // Cierro la salida est�ndar       
    open (cadenaPtr,O_APPEND | O_WRONLY,0777); // Asigno a la salida el fichero

} 


void tuberias(char* argumento1[MAX], char* argumento2[MAX])

{ /* INICIO DE LA FUNCI�N tuberias */
  int fd[2],estado;
    pid_t hijo; 
    hijo=fork();
    

    if (hijo==-1) printf("ERROR Creacion de proceso"); 
    else if (hijo==0) {
           pipe(&fd[0]); /* Funci�n pipe encargada de crear la tuber�a */
            if (fork()!=0) {
                    close(fd[0]); /* cerramos el lado de lectura del pipe */
                    close(1);
                    dup(fd[1]); /* STDOUT = extremo de salida del pipe */
                    close(fd[1]);/* cerramos el descriptor de fichero que sobra 
                                tras el dup(fd[1])*/
                    execvp(argumento1[0],argumento1);
                }
            else {
                    close(fd[1]);/* cerramos el lado de lectura del pipe */
                    close(0);
                    dup(fd[0]);/* STDOUT = extremo de salida del pipe */
                    close(fd[0]);/* cerramos el descriptor de fichero que sobra 
                                tras el dup(fd[0])*/
                    execvp(argumento2[0],argumento2);
      
                 }
           }
    else  hijo=wait(&estado);
} 

void despedida() {
	printf("\n------------------------\n");
	printf("  Ejecución finalizada\n");
	printf("------------------------\n");
	signal(SIGINT, SIG_DFL); // Indicamos al programa que use la acción por defecto
	raise(SIGINT);
}
