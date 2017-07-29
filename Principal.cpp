/*
 * Principal.cpp
 *
 *  Created on: 26/08/2016
 *      Author: jesica
 */

#include "Principal.h"
#include <map>
#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

//defino el tipo ulong
#ifndef ulong  //Este debería llamarse uint pero le pongo ulong porque tengo todo el código de tipo ulong
#define ulong unsigned int
#endif

///////// cantidad de elementos que pediremos en cada asignacion de memoria
#define numberElementsPilaToRequest 8
#define numberNodesToRequest 10
#define numberLeavesToRequest 10
#define numberPositionCloseToRequest 10

/// no usamos BORRAR!
#define bit8 128
#define bit7 64
#define bit6 32
#define bit5 16
#define bit4 8
#define bit3 4
#define bit2 2
#define bit1 1
/////////


#define error(msg){	exit(1);}
/*	printf("\n error, ");
	printf(msg);
	printf("\n");
	*/

//-----------------------------------------------------------------
///------------------------------> PILA
typedef struct tipoPila {
	ulong positionArray;
	ulong nodes;
	ulong leaves;
	ulong positionBit;
	ulong positionInitial;
//	char subString;
} tipoPila;


tipoPila *pila;
long topePila;
ulong endIndexPila=0; //Marca el limite superior de la estructuras Pila de acuerdo a la cantidad de elementos pedidos con ralloc

///<------------------------------ fin PILA
//-----------------------------------------------------------------


///------------------------------> Arreglos con los resultados del findClose ------------------------------------------------------------
// arreglos por separados para guardar el resultado de cada nivel del findClose
//1) Arreglo que guarda la cantidad de nodos (OJO: recordar que no se llena consecutivamente puede llenarse primero la posicion 3 luego la 1, luego la 4 y luego la 2)
ulong *nodes;
ulong lastPositionNodesUsed;	//ultima posicion del arreglo de nodos que esta en uso, OJO no se llena consecutivamente se puede llenar la posicion 3, luego la 1 y luego la 2.
ulong totalCountNodes = 0; 		//cantidad total de nodos (usados + sin uso), cantidad de elementos asignados con el ralloc que no necesariamente significa que esten todos usados.
ulong totalCountNodesUsed = 0; 	//cantidad total de NODOS USADOS.

//2) Arreglo que guarda la cantidad de hojas
ulong *leaves;
ulong lastPositionLeavesUsed;	//ultima posicion del arreglo de hojas que esta en uso, OJO no se llena consecutivamente.
ulong totalCountLeaves = 0; 	//cantidad total de hojas (usadas + sin uso), cantidad de elementos asignados con el ralloc que no necesariamente significa que esten todos usados.
ulong totalCountLeavesUsed = 0; //cantidad total de HOJAS USADAS.


//3) Arreglo que guarda las posiciones de cierre
ulong *positionClose;
ulong lastPositionPositionCloseUsed;	//ultima posicion del arreglo de posiciones de cierre que esta en uso, OJO no se llena consecutivamente.
ulong totalCountPositionClose = 0; 		//cantidad total de posiciones de cierre (usadas + sin uso), cantidad de elementos asignados con el ralloc que no necesariamente significa que esten todos usados.
ulong totalCountPositionCloseUsed = 0; 	//cantidad total de POSICIONES DE CIERRE USADAS
///<------------------------------------------------------------------------------------------------------------------------

ulong positionArrayClose = 0; //equiv a Posnivel
ulong lastPosArrayUsed = 0;

map<ulong,ulong> mapa;

Principal::Principal() {
	// TODO Auto-generated constructor stub
}

Principal::~Principal() {
	// TODO Auto-generated destructor stub
}

void verif(void *p)
{
	if (p == NULL)
		error ("sin memoria\n");
}

//==============================================================================================================
//================================> INICIO METODOS para las estructuras con el RESULTADO DEL FINDCLOSE

//-----------------------------------------------------------------
/** Asignar espacio si hace falta para estructura de tipo ulong (Nodes, Leaves, PositionClose)
//-----------------------------------------------------------------
 * arrayULong: apuntador a estructura para asignar memoria
 * totalCount: cantidad de elementos totales en la estructura
 * usedCount: cantidad de lugares ya usados para la estructura ptr //TODO creo que no se usa, si es asi sacar
 * countToRequest: cantidad de bloques de asignación de memoria a solicitar
 * posToInsert: posicion en la que se quiere insertar
 */
void addSpaceStructUlong(ulong **arrayULong, ulong *totalCount, ulong *usedCount, int countToRequest, ulong posToInsert)
{
	int i= *totalCount;
	while(posToInsert >= *totalCount)
	{
		*totalCount=*totalCount+countToRequest;
		*arrayULong=(ulong *)realloc(*arrayULong,((*totalCount)*sizeof(ulong)));
		verif(*arrayULong);
	}

	//lleno con 0 los lugares agregados a partir de la 'ultima posicion disponible antes de asignar mas lugares' hasta la 'ultima posicion luego de agregar lugares'
	while (i < *totalCount)//TODO ver si es < o <=
	{
		(*arrayULong)[i]=0;
		i++;
	}
}
//<================================ fin METODOS para las estructuras con el RESULTADO DEL FINDCLOSE



//==============================================================================================================
//================================> INICIO METODOS PILA

//-----------------------------------------------------------------
//verifica memoria pedida
//-----------------------------------------------------------------

void verifySpace(void *p)
{
	if (p == NULL)
		error ("sin memoria\n");
}


//-----------------------------------------------------------------
//Asignar espacio si hace falta para estructura Pila
//-----------------------------------------------------------------
//ptr apuntador a estructura para asignar memoria
//n cantidad de elementos actuales en la estructura
//end_index cantidad de lugares ya asignados para la estructura ptr
//blk_pila cantidad de bloques de asignación de memoria a solicitar
void addSpacePila(tipoPila ***ptr, ulong n, ulong *end_index, int blk_pila)
{
	if( *end_index == n )
	{
		int i=0;
		*end_index=*end_index+blk_pila;//
		**ptr=(tipoPila *)realloc(**ptr,((*end_index)*sizeof(tipoPila)));
		verifySpace(**ptr);
		while (i<blk_pila) //Inicializamos los campos de la pila recién asignados
		{
			(**ptr)[n].positionArray=0;
			(**ptr)[n].nodes=0;
			(**ptr)[n].leaves=0;
			(**ptr)[n].positionBit=0;
			(**ptr)[n].positionInitial=0;
			i++;
			n++;
		}
	}
}

void push(tipoPila **l, tipoPila v){
	int bloques = numberElementsPilaToRequest;
	addSpacePila(&l, topePila+1, &endIndexPila, bloques);
	topePila++;
	(*l)[topePila].positionArray= v.positionArray;
	(*l)[topePila].nodes = v.nodes;
	(*l)[topePila].leaves = v.leaves;
	(*l)[topePila].positionBit = v.positionBit;
	(*l)[topePila].positionInitial = v.positionInitial;

};

int pop() {
	topePila--;
	return 1;
};

int empty () {
	if (topePila == -1)
		return 1;
	else
		return 0;
};

void copyPreviousElement(tipoPila **l, bool lastWasZero)
{
	//Solo en la ultima posicion de la pila se incrementa con un "1" mas.
	if(topePila == 1 || lastWasZero)
	{
		(*l)[topePila-1].nodes = (*l)[topePila-1].nodes + (*l)[topePila].nodes + 1;
	}
	else
	{
		(*l)[topePila-1].nodes = (*l)[topePila-1].nodes + (*l)[topePila].nodes;
	}
	(*l)[topePila-1].leaves = (*l)[topePila-1].leaves + (*l)[topePila].leaves;
	(*l)[topePila-1].positionBit = (*l)[topePila].positionBit;


	if(topePila >1 && ( ((*l)[topePila-1].positionArray)==0 ) )
	{
		if(lastPosArrayUsed > positionArrayClose) //pila[topePila].positionArray > positionArrayClose)
		{
			//nodoPila.positionArray = pila[topePila].positionArray + 1;
			lastPosArrayUsed = lastPosArrayUsed + 1;
		}
		else
		{
			lastPosArrayUsed = positionArrayClose + 1;
		}
		(*l)[topePila-1].positionArray = lastPosArrayUsed;
	}

	//(*l)[topePila-1].positionArray = (*l)[topePila].positionArray;
}

void initNode(tipoPila *nodoPila)
{
	nodoPila->nodes=0;
	nodoPila->leaves=0;
	nodoPila->positionBit=0;
	nodoPila->positionInitial=0;
	if(topePila >= 0)
	{
		if(pila[topePila].positionArray > positionArrayClose)
		{
			nodoPila->positionArray= pila[topePila].positionArray +1;
		}
		else
		{
			nodoPila->positionArray= positionArrayClose +1;
		}
	}
	else
	{
		nodoPila->positionArray=0;
	}
	//nodoPila->subString='';
}
//TODO metodo no usado
void setInfoNode(tipoPila *nodoPila, int positionBit, int positionArray, char subString)
{
	nodoPila->nodes=0;
	nodoPila->leaves=0;
	nodoPila->positionBit=positionBit;
	nodoPila->positionArray=positionArray;
}
//<================================ fin METODOS PILA


/**
 * text: arreglo de caracteres que contiene el texto en bytes.
 * size: tamaño del arreglo de caracteres text.
 * bitsCount: cantidad de bits porque puede que el texto tenga una cantidad de bits que no necesariamente sea multiplo de 8 (cant de bits en un byte)
 * 		por ejemplo el texto = 11001001110 tiene 11 bits que seran recibidos como 2 bytes (en text[]) un byte = 11001001, otro byte = 11000000 este ultimo se completa con 0's
 * level: indica el nivel hasta donde queremos calcular.
 */
void buildFindClose(unsigned char text[], ulong size, ulong bitsCount, ulong level)
{
	ulong arrayPos = 0;
	ulong bitToByte = 0;
	bool lastWasZero=false;
	topePila=-1;

	//declaro un nodo para la pila
	tipoPila nodoPila;

	//itero cada bit segun la cantidad de bits que me pasaron por parametro
	for(ulong eachBit=0; eachBit < bitsCount; eachBit++)
	{
		arrayPos = (eachBit >> 3);
		bitToByte = (eachBit & 7); //va de 0 a 7

		cout << "eachBit:   " << eachBit << endl;
		//cout << "bitsCount: " << bitsCount;
		cout << "arrayPos:  " << arrayPos << endl;
		cout << "bitToByte: " << bitToByte << endl;

		//cout << "text[arrayPos]: " << text[arrayPos];

		//evaluo si en esa posicion hay un '1' o un '0'
		// 128 >> bitToByte = 128>>0 o 128>>1 o 128>>2 o 128>>3 o .... 128>>7. Osea va corriendo el 1 de 10000000 (128)
		if(text[arrayPos] & (128 >> bitToByte)) // este "&" devuelve un 'true' si en esa posicion hay un '1' sino devuelve false
		{
			cout << "es un UNO !!!! "<< endl<< endl;
			//inicializo nodos y hojas
			initNode(&nodoPila);

			//copio la posicion del bit dentro de todo el array
			nodoPila.positionBit = eachBit;

			//seteo donde comienza el nodo
			nodoPila.positionInitial = eachBit;

			//subo en la pila
			push(&pila, nodoPila);

			lastWasZero = false;
		}
		else
		{
			if(lastWasZero)
			{// si anterior fue un cero
if(topePila<=level)
{
				cout << "es un CERO !!!! y el ultimo fue un CERO"<< endl << endl;
				//1- guardo la posicion del bit que estoy mirando
				pila[topePila].positionBit = eachBit;

				//2- guardo en los arreglos de resultados de findClose cada valor
				//a) guardo en arreglo de nodos
				addSpaceStructUlong(&nodes, &totalCountNodes, &totalCountNodesUsed, numberNodesToRequest, nodoPila.positionArray);
				lastPositionNodesUsed = nodoPila.positionArray; //TODO si no se usa eliminar declaracion y referencias
				totalCountNodesUsed ++;
				nodes[pila[topePila].positionArray] = pila[topePila].nodes;

				//b) guardo en arreglo de hojas
				addSpaceStructUlong(&leaves, &totalCountLeaves, &totalCountLeavesUsed, numberLeavesToRequest, nodoPila.positionArray);
				lastPositionLeavesUsed = nodoPila.positionArray;
				totalCountLeavesUsed ++;
				leaves[pila[topePila].positionArray] = pila[topePila].leaves;

				//c) guardo la posicion del bit
				addSpaceStructUlong(&positionClose, &totalCountPositionClose, &totalCountPositionCloseUsed, numberPositionCloseToRequest, nodoPila.positionArray);
				lastPositionPositionCloseUsed = nodoPila.positionArray;
				totalCountPositionCloseUsed ++;

				positionClose[pila[topePila].positionArray] = pila[topePila].positionBit;

				//3-Guardo en el mapa
				mapa[pila[topePila].positionInitial] = pila[topePila].positionArray;
				//mapa.add(pila[topePila].positionInitial, pila[topePila].positionArray);

				cout << "inicia en = " << pila[topePila].positionInitial << " y el resultado esta en = " << pila[topePila].positionArray << endl;

				//4- actualizo la ultima posicion que hemos completado en los arreglos resultantes del findClose
				if(positionArrayClose < pila[topePila].positionArray)
					positionArrayClose = pila[topePila].positionArray;
}
				if(topePila > 0)
				{
					//5- Bajo la informacion a la posicion anterior en la pila
					copyPreviousElement(&pila, lastWasZero);

					//6- Pop pila
					pop();
				}
			}
			else
			{// si anterior fue un uno

				cout << "es un CERO !!!! y el ultimo fue un UNO"<< endl<< endl;

				//incremento nodos y hojas
				nodoPila.nodes = nodoPila.nodes + 1;
				nodoPila.leaves = nodoPila.leaves + 1;

				//incremento posbit
				nodoPila.positionBit = eachBit;

				//agregamos la info a la ultima posicion de la pila
				pila[topePila].nodes = nodoPila.nodes;
				pila[topePila].leaves = nodoPila.leaves;
				pila[topePila].positionBit = nodoPila.positionBit;

				if(topePila > 0)
				{
					//Anexo la informacion a la posicion anterior en la pila
					copyPreviousElement(&pila, lastWasZero);

					//Pop pila
					pop();
				}
			}
			lastWasZero = true;
		}
	}

	cout << endl << "NODES" << endl;
	for(int i =0; i< totalCountNodesUsed ; i++)
	{
		cout << "nodes[" << i << "] = " << nodes[i] << endl;
	}
	cout << endl << "LEAVES" << endl;
	for(int i =0; i< totalCountLeavesUsed ; i++)
	{
		cout << "leaves[" << i << "] = " << leaves[i] << endl;
	}
	cout << endl << "POSITIONES CLOSE" << endl;
	for(int i =0; i< totalCountPositionCloseUsed ; i++)
	{
		cout << "positionClose[" << i << "] = " << positionClose[i] << endl;
	}
}

/**
 * Me dice si en esa posicion del texto hay un 1 o un 0.
 */
int es_un_uno(unsigned char text[], ulong pos)
{
	ulong arrayPos = (pos >> 3);
	ulong bitToByte = (pos & 7); //va de 0 a 7

	if(text[arrayPos] & (128>> bitToByte))
		return 1;
	else
		return 0;
}

/**
 * Me dice si esa posicion es o no una hoja.
 */
int es_hoja(unsigned char text[], ulong pos)
{
	pos++;
	if(es_un_uno(text, pos))
		return 0; //No es hoja
	else
		return 1; //es hoja
}

ulong findClose(ulong currentPosition, ulong currentLevel, ulong givenLevel)
{
	ulong eachPositionClose;
//	if(currentLevel < givenLevel)
//	{
		//usar estructura mia para saber donde cierra X nodo
		//return positionClose[estruc_q_falta[currentPosition]]
		eachPositionClose = positionClose[mapa[currentPosition]];
/*	}
	else
	{
		//usar el findClose de Dario
		//eachPositionClose =
	}*/
	return eachPositionClose;
}

/**
 * text		: contiene el texto a evaluar
 * eachBit	: indica la posicion dentro del texto
 * statusNode: nos indica si un nodo esta cerrado o abierto ('a'=abierto, 'c'=cerrado)
 *
 * Hace lo siguiente:
 	 - si en esa posicion hay un 1 y esta abierto ('a'=1) -> incremento nivel (no modifica la condicion)
  	 - si en esa posicion hay un 1 y esta cerrado ('c'=0) -> lo abro ('a')
 	 - si en esa posicion hay un 0 y esta abierto ('a'=1) -> lo cierra ('c')
  	 - si en esa posicion hay un 0 y esta cerrado ('c'=0) -> decrementa nivel
 */

ulong getCurrentLevel(unsigned char text[], ulong currentPosition, ulong *statusNode, ulong currentLevel)
{
//	ulong arrayPos = (currentPosition >> 3);
//	ulong bitToByte = (currentPosition & 7);

	if(currentPosition==0)
	{
		*statusNode = 1;
		return 0;
	}

	//evaluo si en esa posicion hay un '1' o un '0'
	//if(text[arrayPos] & (128 >> bitToByte)) // este "&" devuelve un 'true' si en esa posicion hay un '1' sino devuelve false
	if(es_un_uno(text, currentPosition))
	{
		if(*statusNode == 1)
			currentLevel++;
		else
			*statusNode = 0;
	}
	else
	{
		if(*statusNode == 1)
			*statusNode = 0;
		else
			currentLevel--;
	}
	return currentLevel;
}

void getStatics(unsigned char text[], ulong size, ulong bitsCount, ulong givenLevel)
{
	ulong currentLevel = 0;
	ulong statusNode = 0; //0=cerrado, 1=abierto
	ulong eachPositionClose;

	for(ulong eachBit=0; eachBit < bitsCount; eachBit++)
	{
		if(es_un_uno(text, eachBit))
		{
			currentLevel = getCurrentLevel(text, eachBit, &statusNode, currentLevel);

			if(es_hoja(text, eachBit))
				eachPositionClose = (eachBit+1);
			else
				eachPositionClose = findClose(eachBit, currentLevel, givenLevel);

			cout << "La posicion " << eachBit << " cierra en -> " << eachPositionClose << endl;
		}
	}
}

int main (int argc, char *argv[])
{
	//original
	//111011010100100110100110101101010000
	unsigned char allText[5];
	allText[0] = 237; 	//11101101 = 128+64+32+0+8+4+0+1 =237
	allText[1] = 73; 	//01001001 = 0+64+0+0+8+0+0+1 = 73
	allText[2] = 166;	//10100110 = 128+0+32+0+0+4+2+0 = 166
	allText[3] = 181;	//10110101 = 128+0+32+16+0+4+0+1 = 181
	allText[4] = 0; 	//00000000 = 0
	buildFindClose(allText, 5, 36, 3);

	//findClose(allText);
	getStatics(allText, 5, 36, 3);
/*
	unsigned char allText[8];
	allText[0] = 244;
	allText[1] = 181;
	allText[2] = 26;
	allText[3] = 122;
	allText[4] = 102;
	allText[5] = 134;
	allText[6] = 228;
	allText[7] = 0;
	findClose(allText, 8, 58);
*/



}



