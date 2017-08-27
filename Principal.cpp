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
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* only for getTime() */
#include <sys/time.h>
#include <sys/resource.h>

#include <time.h>

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

/*
 * ###################################### INICIO CODIGO Findclose Dario.######################################### */
#define DIV(a) ((a)>> 3)
#define MOD(a) ((a) & 7)

// a/32 y a%32
#define DIVb(a) ((a)>> 5)
#define MODb(a) ((a) & 31)

// a/s y a%s
#define DIVs(a) ((a)>> 7)//Aquí asumimos que s=4 y b=32 por lo que s*b=128 --> 2^7=128.
#define MODs(a) ((a) & ((s*b)-1))

#define MIN(a,b) (a<b) ? a : b
#define error(msg) {printf("\n error, ");printf(msg); printf("\n");exit(1);}


typedef unsigned char byte;
#ifndef uchar
#define uchar unsigned char
#endif

byte RankTable[255];
char MinExcessTable[255];
char *MinExcessBitmap;  //estructura que mantiene los excesos minimos del bitmap que representa la estructura del árbol
char *MinExcessBitmap_RS;  //estructura que mantiene los excesos minimos del bitmap que representa la estructura del árbol para un total de (RS*b) bit
byte NumOfLeaves[255];

#define mask31 0x0000001F

#define max(x,y) ((x)>(y)?(x):(y))
#define min(x,y) ((x)<(y)?(x):(y))

/*numero de bits del entero de la maquina*/
#define W 32
/* W-1 */
#define Wminusone 31
/*numero de bits del entero de la maquina*/
#define WW 64
/*bits para hacer la mascara para contar mas rapido*/
#define bitsM 8
/*bytes que hacen una palabra */
#define BW 4

#ifndef uchar
#define uchar unsigned char
#endif

#define size_uchar 256

#define true 1

#define false 0

const unsigned char popcount_tab[] =
{
0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8,
};


ulong *bitmap_ulong, b=32,s=4; //Variable utilizada para mantener el arreglo de parentesis luego de convertirlo a ulong.


/**********************************************
        Procesos que estaban como inline
**********************************************/
ulong popLeaves (register int x, int *bandera) {
	ulong y;
	y =  NumOfLeaves[(x >>  0) & 0xff]  + NumOfLeaves[(x >>  8) & 0xff]  + NumOfLeaves[(x >> 16) & 0xff]  + NumOfLeaves[(x >> 24) & 0xff];

	//bandera es la variable que indica si el nro que se chequeo previamente terminaba en 1, si es así y además el ulong actual empieza con cero, estamos en presencia de una hoja.
	if (*bandera && (((x >>  24) & 0xff) < 128)) y=y+1;

	//Realizamos el & (and logico) con uno porque necesitamos saber si el nro es par o impar y con esta operación realizamos el modulo 2 de un nro.
	if ((((x >>  24) & 0xff) & 1) && (((x >>  16) & 0xff) < 128))
			y=y+1;

	if ((((x >>  16) & 0xff) & 1) && (((x >>  8) & 0xff) < 128))
				y=y+1;

	if ((((x >>  8) & 0xff) & 1) && (((x >>  0) & 0xff) < 128))
				y=y+1;

	//Si el byte menos significativo del ulong termina en 1 debemos ver si el byte mas significativo del ulong siguiente compienza con cero, en tal caso estamos en presencia de una hoja.
	if (((x >>  0) & 0xff) & 1)
		*bandera=1;
	else
		*bandera=0;

	return (y);
	}

ulong popcount (register int x) {
   return ( popcount_tab[(x >>  0) & 0xff]  + popcount_tab[(x >>  8) & 0xff]  +
            popcount_tab[(x >> 16) & 0xff]  + popcount_tab[(x >> 24) & 0xff] );
}




//#define error(msg) {printf("\n error, ");printf(msg); printf("\n");exit(1);}
//------------------------------------------------------------------------------------------
// Crea tablas con cantidad de unos, exceso de ceros y cantidad de hijos de los nro 0 al 255
//------------------------------------------------------------------------------------------
void initRankExcLeavesTables()
{
ulong i, j, aux;
char actualExcess;
int is_one;

      memset(RankTable,0, 255);
      memset(MinExcessTable,8, 255);
      memset(NumOfLeaves,0,255);
      for (i=0; i<256; i++)
      {	    actualExcess=0; aux=i; is_one=0;
            for (j=0; j<8; j++)
            {      if (aux&128)
                   {   RankTable[i]++; actualExcess++;is_one=1;}
                   else
                   {   actualExcess--;
                   	   if(is_one)
                   	   {  NumOfLeaves[i]++; is_one=0;}
                   }

                   if (actualExcess < MinExcessTable[i])
                        MinExcessTable[i]=actualExcess;
                   aux =aux<<1;
            }
      }

}

// FIN --- Crea tablas con cantidad de unos, exceso de ceros y cantidad de hijos de los nro 0 al 255
//------------------------------------------------------------------------------------------


// Convierte un arreglo de uchar a uno de ulong
//bitmap_char arreglo de uchar a convertir tener en cuenta que nos devuelve un arreglo de ulong sin modificar el
//arreglo de uchar
//bitmap_long arreglo de ulong en el que será devuelto el arreglo convertido
//cant_bits_bitmap cantidad de bit que tiene el arreglo de uchar
void uchartoulong(byte *bitmap_char, ulong **bitmap_long, ulong cant_bits_bitmap){
	int i, aux, aux2;
	ulong tam_bitmap_byte, tam_bitmap_ulong;
	byte *bitmap_char_aux;

	if MODb(cant_bits_bitmap){
		tam_bitmap_byte= DIV(cant_bits_bitmap)+4;
		tam_bitmap_ulong= DIVb(cant_bits_bitmap)+1;
	}
	else{
		tam_bitmap_byte= DIV(cant_bits_bitmap);
		tam_bitmap_ulong= DIVb(cant_bits_bitmap)+1;
	}

	bitmap_char_aux=(byte *)malloc(sizeof(byte)*tam_bitmap_byte);
	*bitmap_long=(ulong *)malloc(sizeof(ulong)*tam_bitmap_ulong);
	//*bitmap_long=(ulong *)realloc(*bitmap_long,tam_bitmap_ulong);

	memset(bitmap_char_aux,0, tam_bitmap_byte);
	memset(*bitmap_long,0, tam_bitmap_ulong);

	i=4;
	aux=0;
	aux2=i-1;
	while (i<= tam_bitmap_byte)
	{
		while (aux<i){
			bitmap_char_aux[aux2]=bitmap_char[aux];
			aux2-=1;
			aux+=1;
		}
		i=i+4;
		aux2=i-1;
	}

	*bitmap_long=(ulong *)bitmap_char_aux;
}



//------------------------------------------------------------------------------------------
// Crea tablas con exceso de ceros tomando de a 32 bit (4 Bytes) del Bipmap creado
// bitmap: es la estructura que mantiene los bit que conforman la estructura del árbol
// MinExcessBitmap: es un tabla global que mantiene los excesos minimos del bitmap
// last: Cantidad de bytes que tiene el bitmap
// b: variable global que nos da el tamaño del registro.
//------------------------------------------------------------------------------------------
void initExcBitmapTables(ulong *bitmap,ulong last)
{
	ulong i, j, aux, ulong_size_bitmap, cant_bit_sobrantes, Rs_size_bitmap, is=0, cur_is=0;
	char actualExcess;
	char actualExcess_Rs; //Mantiene el exceso minimo para la estructura Rs

	  ulong_size_bitmap=DIVb(last);  //Ver el tema si modificamos b #define DIVS(a) ((a)>> 5)
	  Rs_size_bitmap=DIVs(last);

	  MinExcessBitmap=(char *)realloc(MinExcessBitmap,ulong_size_bitmap+1);
      memset(MinExcessBitmap,8, ulong_size_bitmap+1);

      MinExcessBitmap_RS=(char *)realloc(MinExcessBitmap_RS,Rs_size_bitmap+1);
      memset(MinExcessBitmap_RS,8, Rs_size_bitmap+1);

      actualExcess_Rs=0;
	  for (i=0; i<ulong_size_bitmap; i++)
	  {	    actualExcess=0; aux=bitmap[i];
			if (is==s) //La estructura MinExcessBitmap_RS toma de a "s" ulong
			{
				cur_is+=1;
				actualExcess_Rs=0;
				is=0;
			}

			for (j=0; j<32; j++)
			{      if (aux&(1<<b-1))
					{ //2147483648 en binario  1000 0000 0000 0000 0000 0000 0000 0000  (2^(b-1))
					  actualExcess++;
					  actualExcess_Rs++;
					}
				   else
				   {
					 actualExcess--;
					 actualExcess_Rs--;
				   }

				   if (actualExcess < MinExcessBitmap[i])
						MinExcessBitmap[i]=actualExcess;
				   if (actualExcess_Rs < MinExcessBitmap_RS[cur_is])
					    MinExcessBitmap_RS[cur_is]=actualExcess_Rs;
				   aux=aux<<1;
			}
			is+=1;
	  }

	  if(MODs(last)<=32){cur_is+=1;actualExcess_Rs=0;}

      if (cant_bit_sobrantes=MODb(last)){ //En esta sección sacamos el exceso del los últimos bit, es decir puede suceder que nuestro bitmap tenga T modulos de 32bit y el último sólo resten de 0 a 31 bit por lo tanto debemos realizar un trabajo diferenciados sobre ellos
    	  // MODS(a) ((a) & 31)
    	  actualExcess=0; aux=bitmap[i];
    	  for (j=0; j<cant_bit_sobrantes; j++)//cant_bit_sobrantes es la cantidad de bits que no completan un ulong de 32 bits
    	  {   if (aux&(2^(cant_bit_sobrantes-1)))
    	  	  {
    		  	  actualExcess++;
    		  	  actualExcess_Rs++;
    	  	  }
    	  	  else
    	  	  {
    	  		  actualExcess--;
    	  		  actualExcess_Rs--;
    	  	  }
    	  	  if (actualExcess < MinExcessBitmap[i])
    	  		  MinExcessBitmap[i]=actualExcess;
    	  	  if (actualExcess < MinExcessBitmap_RS[cur_is])
    	  		  MinExcessBitmap_RS[cur_is]=actualExcess_Rs;
    	  	  aux =aux<<1;
    	  }
      }

}


int isleaf(byte *tree,ulong pos)
{
	pos++;
	if(tree[DIV(pos)] & (128>> MOD(pos++)))
		return 0; //No es hoja
	else
		return 1; //es hoja
}


ulong FindCloseOrig(ulong *bitmap_ulong, ulong pos, ulong last, ulong *nroNodo, ulong *nroHoja)
{
		ulong *A;

		A=bitmap_ulong;
		int E, is_one=0,bandera, break_byte=1, break_ulong=1, cont_s;
 	    ulong *aux_ulong, *aux_ulong2,pos_aux, exce_maximo=0;
 	    byte *aux_byte, *aux_byte2;
 	    int pos_bits, of_bits=0;
 	 	aux_ulong=(ulong *) malloc(sizeof(ulong)*1);
 	 	aux_ulong2=(ulong *) malloc(sizeof(ulong)*1);

 	 	*aux_ulong = A[DIVb(pos)];
 	 	aux_byte=(byte *)aux_ulong;

 	 	pos_bits=MODb(pos);

 	 	E=-1; pos++; pos_bits++;

		 if ((MODs(pos))) { //Si esta en posición modulo s*b = 0  directamente procesamos de a s*b bit

			if ((MODb(pos))) { //Si esta en posición modulo 32 = 0  directamente procesamos de a ulong

				 //********************   Procesa de a bits *******************//
				// procesamos los primeros bits hasta llegar al inicio de un byte

				 if (MOD(pos_bits))
				 {of_bits=1;
					 while ( MOD(pos_bits) && E!=0)
					 {     if (aux_byte[3-DIV(pos_bits)] & (128>> MOD(pos_bits++)) )
						   { E--; is_one=1; (*nroNodo)++;}
						   else
						   { E++;
							 if(is_one){(*nroHoja)++;is_one=0;}
						   }
						 pos++;
					 }
				 }
				 else{ //else MOD --Verificamos si el cero correspondiente al uno se encuentra el el inicio del
					 //byte. Estariamos en presencia de un hoja que tiene su 1 en un byte y su cero el inicio del
					 //proximo byte.
					 if (!(aux_byte[3-DIV(pos_bits)] & (128>> MOD(pos_bits)) ))
					 { E++; pos++; pos_bits++;}
				 }
				/*if ((!(MOD(pos))) && pos < last)
					if ( ((A[DIV(pos)-1]) & (128>> 7)) && (A[DIV(pos)] < 128) ) (*nroHoja)++;//Realizamos esta consulta para saber si el nro anterior termina en 1 y el siguiente empieza en 0, si este es el caso estamos en presencia de una hoja.
				*/
			}
			else{//else MODb -- Sólo ingresa cuando búscamos el cero para el uno que esta al final de un ulong y
				//verificamos si el siguiente ulong comienza con 0, podemos ver que es una hoja cuyo uno esta al
				//final de un ulong y su cero esta al inicio del siguiente ulong.
				*aux_ulong = A[DIVb(pos)];
				aux_byte=(byte *)aux_ulong;
				pos_bits=MODb(pos);
				if (!(aux_byte[3-DIV(pos_bits)] & (128>> MOD(pos_bits)) ))
				{ E++; pos++;pos_bits++;}
			}
		 }
		 else //else MODs  --
		 {  if (!(A[DIVb(pos)]&(1<<b-1)))
				{ E++; pos++;pos_bits++;}
		 }


        //********************   Procesa de a bytes *******************//

	 if ((MODs(pos))){  //Si esta en posición modulo s*b = 0  directamente procesamos de a s*b bit

        if ((MODb(pos))) {
			// se procesa de a bytes
        	// bandera=0;  Descomenar en caso de realizar el proceso de hojas llamando a pop_leaves
			while (pos+8 < last && E!=0 && MODb(pos))  //RECORDAR QUE PUSIMOS <= last DEBEMOS PROBAR SI FUNCIONA
			{
				  if (of_bits){//Si pos_bit dividido 8 da cero (0). Estamos en el situación donde pos_bit queda sobre el bit cero de nuestra estructura y por lo tanto no se ha hecho un procesamiento de a bits.
					  if ( ((aux_byte[3-DIV(pos_bits)+1]) & (128>> 7)) && (aux_byte[3-DIV(pos_bits)] < 128) ) (*nroHoja)++;//Realizamos esta consulta para saber si el nro anterior termina en 1 y el siguiente empieza en 0, si este es el caso estamos en presencia de una hoja.
				  }
				  else
					  of_bits=1;
				  if ( MinExcessTable[ aux_byte[3-DIV(pos_bits)] ] <= E){
					  break_byte=0;
					  break;double time, tot_time = 0;
				  }
				  else  { E -= 2 * RankTable[ aux_byte[3-DIV(pos_bits)] ] - 8 ; //exceso total
				  //      E -= 2 * popcount8(aux_byte[3-DIV(pos_bits)]) - 8 ; //exceso total
						  (*nroNodo)+= RankTable [ aux_byte[3-DIV(pos_bits)]];
						//(*nroNodo)+=popcount8(aux_byte[3-DIV(pos_bits)]);
						  (*nroHoja)+= NumOfLeaves[ aux_byte[3-DIV(pos_bits)]];
						//(*nroHoja)+=popLeaves8(aux_byte[3-DIV(pos_bits)], &bandera);
						}
				  pos+=8;
				  pos_bits+=8;
			  //if ( ((A[DIV(pos)-1]) & (128>> 7)) && (A[DIV(pos)] < 128) ) (*nroHoja)++;
			}
        }
	 }


		if (break_byte){
			if((aux_byte[0] & 128>> 7) && of_bits)
				bandera=1;
			else
				bandera=0;

			//********************   Procesa de a ulong *******************//
			while (pos+b < last && E!=0 && MODs(pos))
				// se procesa de a ulong
			{     if ( MinExcessBitmap[ DIVb(pos) ] <= E){
						break_ulong=0;
						break;
					}
				  else  { E -= 2 * popcount( A[DIVb(pos)] ) - b ; //exceso total b=32 en este caso
						  (*nroNodo)+= popcount ( A[DIVb(pos)]);
						  (*nroHoja)+= popLeaves( A[DIVb(pos)], &bandera);
						}
				  pos+=b;

			}


			if (break_ulong){
			//********************   Procesa de a (s*b) bit *******************//
				while (pos+(b*s) < last && E!=0)
				{
					//if(pos>=14011554 && pos<=14011810)
						//cont_s=0;
					  cont_s=0;
					  exce_maximo=0;
					  if ( MinExcessBitmap_RS[ DIVs(pos) ] <= E)
							break;
					  else
					  {
						  pos_aux=DIVb(pos);
						  while(cont_s<s){
							  exce_maximo+= popcount( A[pos_aux] );
							  (*nroNodo)+= popcount ( A[pos_aux]);
							  (*nroHoja)+= popLeaves( A[pos_aux], &bandera);
							  pos_aux+=1;
							  cont_s+=1;
						  }
						  E -= 2 * exce_maximo - b*s ; //exceso total s*b
					   }
					  pos+=b*s;
				}
			}


			//********************   Procesa de a ulong *******************//
			while (pos+b < last && E!=0)
				// se procesa de a ulong
			{     if ( MinExcessBitmap[ DIVb(pos) ] <= E)
						break;

				  else  { E -= 2 * popcount( A[DIVb(pos)] ) - b ; //exceso total b=32 en este caso
						  (*nroNodo)+= popcount ( A[DIVb(pos)]);
						  (*nroHoja)+= popLeaves( A[DIVb(pos)], &bandera);
						}
				  pos+=b;
			}


			if (E!=0)
				if (!(A[DIVb(pos)]&(1<<b-1)) && bandera) (*nroHoja)++;
				/*
				*aux_ulong = A[DIVb(pos)];
				*aux_ulong2 = A[DIVb(pos)-1];
				aux_byte=(byte *)aux_ulong;
				aux_byte2=(byte *)aux_ulong2;
				pos_bits=MODb(pos);
				if ( ((aux_byte2[0]) & (128>> 7)) && (aux_byte[3-DIV(pos_bits)] < 128) ) (*nroHoja)++; //Realizamos esta consulta para saber si el nro anterior termina en 1 y el siguiente empieza en 0, si este es el caso estamos en presencia de una hoja.
				*/
		}


		//********************   Procesa de a bytes *******************//

		//Esta pensado para trabajar con b=32 por lo tanto siempre que frene la iteración anterior es porque llegamos al final o no existen b bit pas procesar. En este último caso siempre queda sobre el comienzo de un byte
		// se procesa de a bytes
        *aux_ulong2=0; //Es utilizado para
        *aux_ulong = A[DIVb(pos)];
        aux_byte=(byte *)aux_ulong;
        pos_bits=MODb(pos);
		while (pos+8 < last && E!=0)
		{
			if (*aux_ulong2){ //Aquí se debe realizar esta pregunta luego de haber avanzado al menos una ves en el byte en cuestión
				if ( ((aux_byte[3-DIV(pos_bits)+1]) & (128>> 7)) && (aux_byte[3-DIV(pos_bits)] < 128) ) (*nroHoja)++; //Realizamos esta consulta para saber si el nro anterior termina en 1 y el siguiente empieza en 0, si este es el caso estamos en presencia de una hoja.

			}
			else
				*aux_ulong2=1;

			if ( MinExcessTable[ aux_byte[3-DIV(pos_bits)] ] <= E)
				  break;
			  else  { E -= 2 * RankTable[ aux_byte[3-DIV(pos_bits)] ] - 8 ; //exceso total
			        //E -= 2 * popcount8(aux_byte[3-DIV(pos_bits)]) - 8 ; //exceso total
					  (*nroNodo)+= RankTable [ aux_byte[3-DIV(pos_bits)]];
					//(*nroNodo)+=popcount8(aux_byte[3-DIV(pos_bits)]);
					  (*nroHoja)+= NumOfLeaves[ aux_byte[3-DIV(pos_bits)]];
					//(*nroHoja)+=popLeaves8(aux_byte[3-DIV(pos_bits)], &bandera);
					}
			  pos+=8;
			  pos_bits+=8;
		  //if ( ((A[DIV(pos)-1]) & (128>> 7)) && (A[DIV(pos)] < 128) ) (*nroHoja)++;
		}


		//********************   Procesa de a bits *******************//
		if (pos+8 >= last && E!=0){
				if ((pos<last) && ((aux_byte[3-DIV(pos_bits)+1]) & (128>> 7)) && (aux_byte[3-DIV(pos_bits)] < 128) ) (*nroHoja)++;}
		is_one=0;
        while (E!=0 && pos <last)
        {
        		if (aux_byte[3-DIV(pos_bits)] & (128>> MOD(pos_bits++)) )
               { E--; is_one=1; (*nroNodo)++;}
               else
               { E++;
               	 if(is_one){(*nroHoja)++;is_one=0;}
               }
        	pos++;
        }

        free(aux_ulong);
        free(aux_ulong2);
        //free(aux_byte);
        //free(aux_byte2);
        if (E!=0)
              error ("en FindClose, no lo encontro");
        return (pos-1);
}



/* #######################################   FIN CODIGO DARIO ###################################### */


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
ulong totalCountNodes = 0; 		//cantidad total de nodos (usados + sin uso), cantidad de elementos asignados con el ralloc que no necesariamente significa que esten todos usados.
ulong totalCountNodesUsed = 0; 	//cantidad total de NODOS USADOS.

//2) Arreglo que guarda la cantidad de hojas
ulong *leaves;
ulong totalCountLeaves = 0; 	//cantidad total de hojas (usadas + sin uso), cantidad de elementos asignados con el ralloc que no necesariamente significa que esten todos usados.
ulong totalCountLeavesUsed = 0; //cantidad total de HOJAS USADAS.


//3) Arreglo que guarda las posiciones de cierre
ulong *positionClose;
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

//<================================ fin METODOS PILA


/**
 * Metodo que calcula las posiciones de cierre, cantidad de nodos y cantidad de hojas.
 *
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

	tipoPila nodoPila;

	//itero cada bit segun la cantidad de bits que me pasaron por parametro
	for(ulong eachBit=0; eachBit < bitsCount; eachBit++)
	{
		arrayPos = (eachBit >> 3);
		bitToByte = (eachBit & 7); //va de 0 a 7

//		cout << "eachBit:   " << eachBit << endl;
//		cout << "arrayPos:  " << arrayPos << endl;
//		cout << "bitToByte: " << bitToByte << endl;
		//cout << "text[arrayPos]: " << text[arrayPos];

		//evaluo si en esa posicion hay un '1' o un '0'
		// 128 >> bitToByte = 128>>0 o 128>>1 o 128>>2 o 128>>3 o .... 128>>7. Osea va corriendo el 1 de 10000000 (128)
		if(text[arrayPos] & (128 >> bitToByte)) // este "&" devuelve un 'true' si en esa posicion hay un '1' sino devuelve false
		{
			//cout << "es un UNO !!!! "<< endl<< endl;
			initNode(&nodoPila); //inicializo nodos y hojas

			nodoPila.positionBit = eachBit; //copio la posicion del bit dentro de todo el array

			nodoPila.positionInitial = eachBit; //seteo donde comienza el nodo

			push(&pila, nodoPila);//subo en la pila

			lastWasZero = false;
		}
		else
		{
			if(lastWasZero)
			{// si anterior fue un cero
				if(topePila<=level)
				{
					//cout << "es un CERO !!!! y el ultimo fue un CERO"<< endl << endl;
					//1- guardo la posicion del bit que estoy mirando
					pila[topePila].positionBit = eachBit;

					//2- guardo en los arreglos de resultados de findClose cada valor
					//a) guardo en arreglo de nodos
					addSpaceStructUlong(&nodes, &totalCountNodes, &totalCountNodesUsed, numberNodesToRequest, nodoPila.positionArray);
					totalCountNodesUsed ++;
					nodes[pila[topePila].positionArray] = pila[topePila].nodes;

					//b) guardo en arreglo de hojas
					addSpaceStructUlong(&leaves, &totalCountLeaves, &totalCountLeavesUsed, numberLeavesToRequest, nodoPila.positionArray);
					totalCountLeavesUsed ++;
					leaves[pila[topePila].positionArray] = pila[topePila].leaves;

					//c) guardo la posicion del bit
					addSpaceStructUlong(&positionClose, &totalCountPositionClose, &totalCountPositionCloseUsed, numberPositionCloseToRequest, nodoPila.positionArray);
					totalCountPositionCloseUsed ++;
					positionClose[pila[topePila].positionArray] = pila[topePila].positionBit;

					//3-Guardo en el mapa
					mapa[pila[topePila].positionInitial] = pila[topePila].positionArray;
					//cout << "inicia en = " << pila[topePila].positionInitial << " y el resultado esta en = " << pila[topePila].positionArray << endl;

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

				//cout << "es un CERO !!!! y el ultimo fue un UNO"<< endl<< endl;

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

ulong findClose(ulong *textUlong, ulong currentPosition, ulong currentLevel, ulong bitsCount, ulong givenLevel, ulong *cantNodos, ulong *cantHojas)
{
	ulong eachPositionClose,positionArrays;
	if(currentLevel <= givenLevel)
	{
		//usar estructura mia para saber donde cierra X nodo
		//TODO no devolver un mapa sino usar la ultima posicion usada +1, Sacar mapa usando la ultima posicion usada, devuelvo la siguiente
		positionArrays = mapa[currentPosition];
		eachPositionClose = positionClose[mapa[currentPosition]];
		*cantNodos = nodes[positionArrays];
		*cantHojas = leaves[positionArrays];
	}
	else
	{
		//usar el findClose de Dario
		eachPositionClose = FindCloseOrig(textUlong, currentPosition, bitsCount, cantNodos, cantHojas);
	}
	return eachPositionClose;
}

/**
 * Me da el 'nivel actual' para la posicion del 'bit actual' dado por parametro.
 *
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
ulong getCurrentLevel(ulong currentPosition, ulong *statusNode, ulong currentLevel, ulong esUnUno)
{
	if(currentPosition==0)
	{
		*statusNode = 1;
		return 0;
	}

	//evaluo si en esa posicion hay un '1' o un '0'
	if(esUnUno)
	{
		if(*statusNode == 1)
			currentLevel++;
		else
			*statusNode = 1;
	}
	else
	{
		if(*statusNode == 1)
			*statusNode = 0;
		else
			currentLevel--;
	}
	//cout << "la posicion " << currentPosition << " esta en el nivel " << currentLevel << endl;
	return currentLevel;
}

void getStatics(unsigned char text[], ulong size, ulong bitsCount, ulong givenLevel, ulong * cantNodos, ulong * cantHojas, ulong *textUlong)
{
	ulong currentLevel = 0;
	ulong statusNode = 0; //0=cerrado, 1=abierto
	ulong eachPositionClose;

	for(ulong eachBit=0; eachBit < bitsCount; eachBit++)
	{
		ulong esUnUno = es_un_uno(text, eachBit);
		currentLevel = getCurrentLevel(eachBit, &statusNode, currentLevel, esUnUno);
		if(esUnUno)
		{
			ulong eshoja = es_hoja(text, eachBit);
			if(eshoja)
				eachPositionClose = (eachBit+1);
			else
				eachPositionClose = findClose(textUlong, eachBit, currentLevel, bitsCount, givenLevel, cantNodos, cantHojas);

			cout << "La posicion " << eachBit << " cierra en -> " << eachPositionClose;
			if(eshoja) cout << " y es una hoja" << endl;
			else  cout << ", y tiene " << *cantNodos << " nodos y " << *cantHojas << " hojas." << endl;
		}
	}
}

int main (int argc, char *argv[])
{
	double fc_total_time;
	double time, tot_time = 0;

// >>>>>>>>>>>>>>> vbles para medir tiempo de ejecucion de findclose y count.
	clock_t initTimeBuilding, finTimeBuilding;
	double totalTimeBuilding = 0;

	clock_t initTimeFindClose, finTimeFindClose;
	double totalTimeFindClose = 0;
// <<<<<<<<<<<<<<< vbles para medir tiempo de ejecucion de findclose y count.


	unsigned char allText[5];
	allText[0] = 237; 	//11101101 = 128+64+32+0+8+4+0+1 =237
	allText[1] = 73; 	//01001001 = 0+64+0+0+8+0+0+1 = 73
	allText[2] = 166;	//10100110 = 128+0+32+0+0+4+2+0 = 166
	allText[3] = 181;	//10110101 = 128+0+32+16+0+4+0+1 = 181
	allText[4] = 0; 	//00000000 = 0

		initTimeBuilding = clock();
	buildFindClose(allText, 5, 36, 3);
		finTimeBuilding = clock();
		totalTimeBuilding += (double)(finTimeBuilding - initTimeBuilding) / CLOCKS_PER_SEC;
		cout << "Tiempo total de construccion en milisegundos: " << (totalTimeBuilding* 1000.0) << endl;

	ulong cantNodos=0;
	ulong cantHojas=0;
	ulong posFindClose = 0;

	//convierto el unsigned char en unsigned long
	ulong *textUlong ;
	uchartoulong(allText, &textUlong, 36);

		initTimeFindClose = clock();
	getStatics(allText, 5, 36, 3, &cantNodos, &cantHojas, textUlong);
		finTimeFindClose = clock();
		totalTimeFindClose += (double)(finTimeFindClose - initTimeFindClose) / CLOCKS_PER_SEC;
		cout << "Tiempo total de FindClose: " << (totalTimeFindClose* 1000.0) << endl;

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



