#include "derivative.h" /* include peripheral declarations */

#define counts_p_bit 2185
#define baud_rt	9600
#define sys_clk 32768*640

unsigned char TxD;
unsigned char THR;		//El dato adentro de UART
unsigned char THRE;		//FLAG: Hey, we have data inside!!!
unsigned char cont;
unsigned char FTM_counts;

void FTM_init(){
	//Clocking
	SIM_SCGC6=1<<24;			//Enable clock for FTM0
	FTM0_SC=1<<3;				//System clock enabling pre-escaler=1, fclk = 32768*640

	/*	//Global configuration
	FTM0_MODE = 3<<2;
	duty_cycle= 10486;
	FTM0_C6V=duty_cycle;*/

	//Interrupt configuration
	NVICICER1=(1<<(42%32))+(1<<(58%32));	//NVIC configuration
	NVICISER1=(1<<(42%32))+(1<<(58%32));	//NVIC configuration

	/*We use the desired channel, in this case ch6, you just check in the manual if it is available 
	 In the microcontroller the port PTA1 its available, and it stands for FTM0_ch6
	 */
}

void vUART_config (void)
{
	/*
	 * GPIOs
	 * Tx	PTA1		el mismo que para el FTM
	 * Rx	---
	 * 
	 * Port C		SIM_SCGC5		pg. 323
	 */
	//TxD
	SIM_SCGC5 |= (1<<9);		//clk PORTA
	PORTA_PCR1 = (1<<8);		//Mux alt=1	>>	GPIO
	GPIOA_PDDR |= (1<<1);		//Output config
	GPIOA_PDOR |= (1<<1);		//Set 1 for default	

	//RxD
	//-------

	//configurar timer->base de tiempo: 9600 bps, tb=104bs

	//configurar output compare
	FTM_init();
}

void vUART_send (unsigned char dato)
{
	THR=dato;

	if(THR&1) 
	{
		//TxD=1;
		//Interruption enable, output compare set
		FTM0_C6SC|=(3<<3);			
		/*
		 *CHIE=1
		 *MSB=0
		 *MSA=1
		 *ELSB=0
		 *ELSA=1
		 */
	}
	else 
	{
		//TxD=0;
		//Interruption enable, output compare clear
		FTM0_C6SC|=(11<<1);			
		/*
		 *CHIE=1
		 *MSB=0
		 *MSA=1
		 *ELSB=1
		 *ELSA=0
		 */
	}
	THR>>=1;
	GPIOA_PCOR = (1<<1);
	FTM0_C6V=FTM_counts+counts_p_bit;
	FTM0_C6SC |= (1<<6);

	//Give control of PTA1 to the FTM
	PORTA_PCR1=3<<8;

	cont=8;

	THRE=0;	//Hey, we have data inside!!!

	//Hab_intr_OC para que se manden los 10 bits (start bit + 8data + end bit=10bits)	
	FTM0_C6SC |= (1<<6);
}

int main (void)
{
	vUART_config();
	vUART_send(0xAA);
	while(1){

	}
	return 0;
}

void FTM0_IRQHandler (void)
{
	if(FTM0_C6SC&(1<<7) && FTM0_C6SC &(1<<6))
	{
		/* Turn off the FTM0_C6 CHF flag
		 * Read CSC reg
		 * Write a 0 in CHF (pos:7)
		 */
		(void)FTM0_C6SC;
		FTM0_C6SC&=~(1<<7);

		//Read the counts register
		FTM_counts=FTM0_C6V;
		if (cont>1)	//data [0-6]
		{
			if(THR&1) 
			{
				//TxD=1;
				//GPIOC_PDOR |= (1<<1);
				//Interruption enable, output compare set
				FTM0_C6SC|=(1<<3);			
				/*
					//CHIE=1
					//MSB=0
					//MSA=1
					//ELSB=1
					//ELSA=1
				 * 
				 */
			}
			else 
			{
				//TxD=0;
				//GPIOC_PDOR |= (0<<1);
				//Interruption enable, output compare clear
				FTM0_C6SC&=~(1<<3);			
				/*
					//CHIE=1
					//MSB=0
					//MSA=1
					//ELSB=1
					//ELSA=0
				 * 
				 */
			}
			//104 ns = 2181 counts
			FTM_counts+=counts_p_bit;
			FTM0_C6V=FTM_counts;
			THR>>=1;
			cont--;
		}
		else
		{
			if(cont==1)	//stop bit
			{
				//TxD=1;
				//Interruption enable, output compare set
				FTM0_C6SC|=(1<<3);			
				/*
				//CHIE=1
				//MSB=0
				//MSA=1
				//ELSB=1
				//ELSA=1
				 * 
				 */
				FTM_counts+=counts_p_bit;

				THRE=1;
				//desabilitar OC_intr
				FTM0_C6SC &=~(1<<6);
			}
		}

	}

}
