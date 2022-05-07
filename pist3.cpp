// BONOLOTO.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <process.h>
#include <errno.h>
#include <signal.h>
#include "windows.h"

INT(*PISTinicio) (INT,INT,INT);
INT(*PISTnuevoPistolero) (CHAR);
INT(*PISTdisparar) (CHAR);



DWORD WINAPI funcionHijo(LPVOID param);
void crearHijos(int numeroHijos);
void SignalHandler(int signal);


char hola[27] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
int hijoss[26] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};
char mensaje[7];
int nPistoleros,rapidez,semilla;
int letM = 0, letV = 0, letC = 0, letW = 0, nPist = 0;


HANDLE hijos[26];
HANDLE evento,evento2;
HANDLE semaforito1,semaforito2,semaforito3;
CRITICAL_SECTION sc1,sc2,sc3,sc4,sc5,sc6,sc7; 


HANDLE memoria = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, sizeof(int) * 2 + sizeof(char), "memoria1");
LPINT ref = (LPINT)MapViewOfFile(memoria, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * 2 + sizeof(char)) + 250;
HINSTANCE libreria = LoadLibrary("pist3.dll");

FARPROC PIST_inicio = GetProcAddress(libreria, "PIST_inicio");
FARPROC PIST_nuevoPistolero = GetProcAddress(libreria, "PIST_nuevoPistolero");
FARPROC PIST_fin = GetProcAddress(libreria, "PIST_fin");
FARPROC PIST_disparar = GetProcAddress(libreria, "PIST_disparar");
FARPROC PIST_morirme = GetProcAddress(libreria, "PIST_morirme");
FARPROC PIST_vIctima = GetProcAddress(libreria, "PIST_vIctima");


int main(int argc, char* argv[]) {

	if (argc >= 2) {
		nPistoleros = atoi(argv[1]);
		if (nPistoleros<2 || nPistoleros>26) {
			printf("El parametro de entrada debe ser menor o igual que 26 y mayor o igual que 2\n");
			return -1;
		}
	}
	else {
		printf("El programa necesita pistoleros para funcionar\n");
		return -1;
	}
	if (argc >= 3) {
		rapidez = atoi(argv[2]);
		if (rapidez<0 || rapidez>26) {
			printf("El parametro de entrada debe ser menor o igual que 26 y mayor o igual que 0");
			return -1;
		}
		if (argc >= 4) {semilla = atoi(argv[3]);}
	}
	if (argc == 3) {semilla = 0;}

	if (memoria == NULL) {return -1;}
	if (ref == NULL) {return -1;}

	if (libreria == NULL) {return-1;}
	if (PIST_inicio == NULL) {return-1;}
	if (PIST_nuevoPistolero == NULL) {return-1;}
	if (PIST_fin == NULL) {return-1;}
	if (PIST_disparar == NULL) {return-1;}
	if (PIST_morirme == NULL) {return-1;}
	if (PIST_vIctima == NULL) {return-1;}

	PISTinicio = (INT(*)(INT, INT, INT)) GetProcAddress(libreria, "PIST_inicio");
	PISTnuevoPistolero = (INT(*)(CHAR)) GetProcAddress(libreria, "PIST_nuevoPistolero");
	PISTdisparar = (INT(*)(CHAR)) GetProcAddress(libreria, "PIST_disparar");
	typedef void (*SignalHandlerPointer)(int);

	for(int i = 0; i < nPistoleros; i++) {
		CopyMemory(ref + i, &hola[i], sizeof(char));
	}

	evento = CreateEvent(NULL, TRUE, FALSE, NULL);

	semaforito1 = CreateSemaphore(NULL, 0, 2, "semaforo1");
	semaforito2 = CreateSemaphore(NULL, 0, 1, "semaforo2");
	semaforito3 = CreateSemaphore(NULL, 0, nPistoleros-1, "semaforo3");


	InitializeCriticalSection(&sc1);InitializeCriticalSection(&sc2);InitializeCriticalSection(&sc3);InitializeCriticalSection(&sc4);InitializeCriticalSection(&sc5);InitializeCriticalSection(&sc6);InitializeCriticalSection(&sc7);
	SignalHandlerPointer previousHandler;
	previousHandler = signal(SIGABRT, SignalHandler);
	PISTinicio(nPistoleros, rapidez, semilla);
	crearHijos(nPistoleros);

	
	PIST_fin();
	return (*(ref - 1));

}

DWORD WINAPI funcionHijo(LPVOID param) {

	ReleaseSemaphore(semaforito1, 1, NULL);

	EnterCriticalSection(&sc2);
		(*(ref + 200)) = letM;
		(*(ref - 4)) = nPistoleros;
	LeaveCriticalSection(&sc2);
	
	int numH = (int)param;
	char letP = *(ref + numH);

	PISTnuevoPistolero(letP);


	for (;;) {


		char letra = *(ref);

		for (int d = 0; d < nPistoleros; d++)
		{
			EnterCriticalSection(&sc6);
				if (*(ref + d) < letra)
				{
					letra = *(ref + d);
				}
			LeaveCriticalSection(&sc6);
		}

			
		if (letP == letra) {
			WaitForSingleObject(semaforito2, INFINITE);
			ReleaseSemaphore(semaforito3, (*(ref - 4)) - 1, NULL);

		}
		else {
			EnterCriticalSection(&sc1);
			letM = (*(ref + 200));
			letM++;
			(*(ref + 200)) = letM;

			if (letM == ((*(ref - 4)) - 1))
			{
				ReleaseSemaphore(semaforito2, 1, NULL);
				letM = 0;
				nPist = *(ref - 4);
				(*(ref + 200)) = letM;
				(*(ref + 440)) = letM;
				(*(ref + 400)) = letM;
			}
			LeaveCriticalSection(&sc1);

			WaitForSingleObject(semaforito3, INFINITE);
		}

		char victima = PIST_vIctima();
		PISTdisparar(victima);

		EnterCriticalSection(&sc4);
			for (int i = 0; i < nPistoleros; i++) {
				if (*(ref + 230 + i) == 0) {

					*(ref + 230 + i) = victima;
					break;
				}
			}
		LeaveCriticalSection(&sc4);



		if (letP == letra) {
			WaitForSingleObject(semaforito2, INFINITE);
			ReleaseSemaphore(semaforito3, (*(ref - 4)) - 1, NULL);
		}
		else {
			EnterCriticalSection(&sc1);
				letM = (*(ref + 200));
				letM++;
				(*(ref + 200)) = letM;

				if (letM == ((*(ref - 4)) - 1))
				{
					ReleaseSemaphore(semaforito2, 1, NULL);
					letM = 0;
					nPist = *(ref - 4);
					(*(ref + 200)) = letM;
					(*(ref + 400)) = letM;
					(*(ref + 440)) = letM;

				}
			LeaveCriticalSection(&sc1);

			WaitForSingleObject(semaforito3, INFINITE);
		}



		for (int j = 0; j < nPistoleros; j++) {

			if (*(ref + 230 + j) == letP) {

				PIST_morirme();
				EnterCriticalSection(&sc5);
					*(ref + numH) = 'z';
					*(ref + 230 + j) = 0;

					letV = (*(ref - 4));
					letV--;
					(*(ref - 4))= letV;

					letC = (*(ref + 440));
					letC++;
					(*(ref + 440)) = letC;

					if (((*(ref + 440)) + (*(ref + 400))) == nPist) {
						PulseEvent(evento);
					}
				LeaveCriticalSection(&sc5);

				return 0;
			}	
		}
		EnterCriticalSection(&sc7);
			letW = (*(ref + 400));
			letW++;
			(*(ref + 400)) = letW;

			if (((*(ref + 400)) + (*(ref + 440))) == nPist) {
				PulseEvent(evento);
			}
		LeaveCriticalSection(&sc7);

		WaitForSingleObject(evento, INFINITE);
			

		if ((*(ref - 4)) == 1) {
			(*(ref - 1)) = letP;
			return 0;
		}
	}		
}

void crearHijos(int numeroHijos)
{
	

	for (int b = 0; b < numeroHijos; b++) {
		hijos[b] = CreateThread(NULL, 0, funcionHijo, LPVOID(hijoss[b]), 0, NULL);
		WaitForSingleObject(semaforito1, INFINITE); 
	}

	WaitForMultipleObjects(nPistoleros, hijos, TRUE, INFINITE); 
}

void SignalHandler(int signal)
{
	printf("hola");
	abort();
}

void perrorExit(char* mensaje, int retorno) {

	perror(mensaje);
	exit(retorno);

}