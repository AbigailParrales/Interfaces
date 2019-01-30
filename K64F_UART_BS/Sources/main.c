#include "derivative.h" /* include peripheral declarations */

#define cnt_104 2181

unsigned char THR;
unsigned char cont;
unsigned char OC_referencia;

void FTM_PT_init(){
	//FTM
	/*Configuration of the pin PTA1
	 * Mux=ALT3					//page 244
	 * 100 pin-table LQFP=35
	 * FTM0_CH6
	 */
	//Clock enabling of PTA
	SIM_SCGC5=1<<9;
	//Mux selection
	PORTA_PCR1=3<<8;
}

void FTM_init(){
	//Clocking
	SIM_SCGC6=1<<24;			//Enable clock for FTM0
	FTM0_SC=1<<3;				//System clock enabling pre-escaler=1, fclk = 32768*640

	FTM_PT_init();

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
	FTM0_C6SC=(1<<6)+(5<<2);			//Interruption enable, output compare toggle
	//CHIE=1
	//MSB=0
	//MSA=1
	//ELSB=0
	//ELSA=1


} 

void vUART_config (void)
{
	//configurar pin TXD, valor inicial=1
	//configurar timer->base de tiempo: 9600 bps, tb=104bs
	//configurar output compare
}

void vUART_send (unsigned char dato)
{
	THR=dato;
	TXD=0;	//start bit
	OC_referencia=timer+cnt_104;
	//Hab_intr_OC
	THRE=0;
	if(dato&1){}		//OC_action=set
	else OC_action=set
}

void main (void)
{
	vUART_config();
	vUART_send(0x25);
}

void FTM0_IRQHandler (void)
{
	OC_referencia=FTM0_C6V;
	if (cont>1)	//data [0-7]
  {	
		if(dato&1) TXD=1;
		else TXD=0;
    //104 ns = 2181 counts
		OC_referencia+=cnt_104;
		dato>>=1;
		cont--;
	}
	else
  {
		if(cont==1)	//last data [8]
    {
			TXD=1;
			OC_referencia+=cnt_104;
			cont--;
		}
	}
	if(cont==0)	//stop bit
  {
		//desabilitar OC_intr
    //Turn off the FTM0_C5 CHF flag
		/*
		 * Read CSC reg
		 * Write a 0 in CHF (pos:7)
		 */
    (void)FTM0_C6SC;
		FTM0_C6SC&=~(1<<7);
		//THRE=1
	}
}
