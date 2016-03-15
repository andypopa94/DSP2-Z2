//////////////////////////////////////////////////////////////////////////////
// *
// * VEŽBA 2 – Banke filtara
// * Godina: 2016
// *
// * Zadatak: Banke filtara
// * Autor:
// *
// *
//////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "fir.h"
#include "filter_coeff.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 8000L
#define GAIN 0

//Random konstante
#define M1 2
#define BUF_SIZE 128

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(InputBufferL,4)
Int16 InputBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(InputBufferR,4)
Int16 InputBufferR[AUDIO_IO_SIZE];

#pragma DATA_ALIGN(InputBufferMono,4)
Int16 InputBufferMono[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(OutputBufferMono,4)
Int16 OutputBufferMono[AUDIO_IO_SIZE];

//Random bufferi

Int16 z0[N_COEFF] = { 0 };
Int16 z1[N_COEFF] = { 0 };
Int16 z2[N_COEFF] = { 0 };
Int16 z3[N_COEFF] = { 0 };
/*
Int16 z4[N_COEFF] = { 0 };
Int16 z5[N_COEFF] = { 0 };
Int16 z6[N_COEFF] = { 0 };
Int16 z7[N_COEFF] = { 0 };
Int16 z8[N_COEFF] = { 0 };
Int16 z9[N_COEFF] = { 0 };
Int16 z10[N_COEFF] = { 0 };
Int16 z11[N_COEFF] = { 0 }; */

Int16 buffer0[BUF_SIZE/2];
Int16 buffer1[BUF_SIZE/2];

//treba jos bar 2 buffera za varijantu sa rekurzivnim filter bankama


void stereoToMono(Int16* inputL, Int16* inputR, Int16* output, int size)
{
	int i;
	for(i = 0; i < size; i++)
	{
		output[i] = ( inputL[i]>>1 ) + ( inputR[i]>>1 );
	}
}

void downsample(Int16 *input, Int16 *output, int M, int N)
{
	int i;
	for (i = 0; i < N / M; ++i)
		output[i] = input[M * i];
}

void upsample(Int16 *input, Int16 *output, int L, int N)
{
	int i;
	for (i = 0; i < N; ++i)
	{
		int j;

		output[L * i] = input[i];

		for (j = 1; j < L; ++j)
			output[L * i + j] = 0;
	}
}


void filter_bank_d(Int16 *input, Int16 *output0, Int16 *output1, int n, Int16 *z0_d, Int16 *z1_d)
{


	Int16 tmp0[128];
	Int16 tmp1[128];


	fir_basic(input, H0, z0_d, N_COEFF, n, tmp0);
	downsample(tmp0, output0, M1, n);

	fir_basic(input, H1, z1_d, N_COEFF, n, tmp1);
	downsample(tmp1, output1, M1, n);

}

void filter_bank_u(Int16 *input0, Int16 *input1, Int16 *output, int n, Int16 *z0_u, Int16 *z1_u)
{
	Int16 tmp0[128];
	Int16 tmp1[128];
	int i;

	upsample(input0, tmp0, M1, n);
	fir_basic(tmp0, H0, z0_u, N_COEFF, n, tmp0);

	upsample(input1, tmp1, M1, n);
	fir_basic(tmp1, H1, z1_u, N_COEFF, n, tmp1);

	for(i = 0; i < n; i++)
	{
		output[i] = tmp0[i] + tmp1[i];
	}
}


void main( void )
{
	int i;
    int state=0;
    int n = 128;


    /* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );

    /* Inicijalizacija kontrolera za ocitavanje vrednosti dugmeta*/
    EZDSP5535_SAR_init();

	printf("\n DTMF Predajnik\n");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, GAIN);


 	while(1)
	{

 		/* Promeniti stanje na osnovu pritisnutog tastera */
 		/* Za dobavljanje vrednosti tastera koristiti funkciju EZDSP5535_SAR_getKey */
 		/* Your code here */

 		aic3204_read_block(InputBufferL, InputBufferR);
 		stereoToMono(InputBufferL, InputBufferR, InputBufferMono, AUDIO_IO_SIZE);

 		/* Izvrsiti analizu signala InputBufferMono upotrebom filter banke 4. reda */
 		/* Your code here */
		
		 filter_bank_d(InputBufferMono, buffer0, buffer1, n, z0, z1);

 		/* U zavisnosti od trenutnog stanja nulirati odredjene komponente signala */
 		/* Stanje 0 - nema obrade (komponente signala na ulazu nalaze se i na izlazu) */
 		/* Stanje 1 - samo frekvencijske komponente iz prvog podopsega */
 		/* Stanje 2 - samo frekvencijske komponente iz drugog podopsega */
 		/* Stanje 3 - samo frekvencijske komponente iz treceg podopsega */
 		/* Stanje 4 - samo frekvencijske komponente iz cetvrtog podopsega */
 		/* Your code here */

 		/* Izvrsiti sintezu signala i rezultat smestiti u OutputBufferMono */
 		/* Your code here */

 		filter_bank_u(buffer0, buffer1, OutputBufferMono, n, z2, z3);

		aic3204_write_block(OutputBufferMono, OutputBufferMono);
	}


	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}


