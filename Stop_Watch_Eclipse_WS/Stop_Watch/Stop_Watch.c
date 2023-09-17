/*
 * Stop_Watch.c
 *
 *  Created on: Sep 16, 2023
 *  Author: Ayman_Mostafa
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Global Variables
unsigned char SEC1=0;
unsigned char SEC2=0;
unsigned char MIN1=0;
unsigned char MIN2=0;
unsigned char HOUR1=0;
unsigned char HOUR2=0;



void Timer1_CTC(unsigned short Counts)
{
	TCNT1 = 0;		/* Set timer1 initial count to zero */
	OCR1A = Counts;    /* Set the Compare value to Counts */
	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Interrupt */
	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */
	TCCR1A = (1<<FOC1A);
	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Prescaler = F_CPU/1024 CS10=1 CS11=0 CS12=1
	 */
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);
}

ISR(TIMER1_COMPA_vect)
{
	SEC1++;

	if (SEC1 >= 10)
	{
		SEC1 = 0;
		SEC2++;

		if (SEC2 >= 6)
		{
			SEC2 = 0;
			MIN1++;

			if (MIN1 >= 10)
			{
				MIN1 = 0;
				MIN2++;

				if (MIN2 >= 6)
				{
					MIN2 = 0;
					HOUR1++;

					if (HOUR1 >= 10)
					{
						HOUR1 = 0;
						HOUR2++;
					}

					if (HOUR2 == 2 && HOUR1 == 4)  // Limit to 24-hour format
					{
						HOUR2 = 0;
						HOUR1 = 0;
					}
				}
			}
		}
	}
}

//Function for Interrupt 0
void INT0_REST_BOTTON(void)
{
	SREG|=(1<<7);
	//The falling edge of INT0 ISC01=1 and ISC00=0
	MCUCR|=(1<<ISC01);
	MCUCR&=~(1<<ISC00);
	GICR|=(1<<INT0);//External Interrupt Request 0 Enable
}
//ISR For handling Interrupt 0
ISR(INT0_vect)
{
	SEC1=0;
	SEC2=0;
	MIN1=0;
	MIN2=0;
	HOUR1=0;
	HOUR2=0;
}

//Function for Interrupt 1
void INT1_PAUSED_BOTTON(void)
{
	SREG|=(1<<7);
	MCUCR|=(1<<ISC11) | (1<<ISC10);	//The rising edge of INT1 ISC11=1 and ISC10=1
	GICR|=(1<<INT1);//External Interrupt Request 1 Enable
}
//ISR For handling Interrupt 1
ISR(INT1_vect)
{
	//No clock source (Timer/Counter stopped)
	TCCR1B&=~(1<<CS10) & ~(1<<CS11) & ~(1<<CS12) ;
}

void INT2_RESUMED_BOTTON(void)
{
	SREG|=(1<<7);
	MCUCSR&=~(1<<ISC2);//The falling edge of INT2 ISC2=0
	GICR|=(1<<INT2);//External Interrupt Request 2 Enable
}
//ISR For handling Interrupt 2
ISR(INT2_vect)
{
	//Restart clock source (Timer/Counter start again).
	//Prescaler = F_CPU/1024 CS10=1 CS11=0 CS12=1
	TCCR1B&=~(1<<CS11);
	TCCR1B|=(1<<CS10) | (1<<CS12);
}

int main(void)
{
	Timer1_CTC(976);//Enable Timer 1 to count 1 second

	INT0_REST_BOTTON();//for Interrupt 0
	INT1_PAUSED_BOTTON();//for Interrupt 1
	INT2_RESUMED_BOTTON();//for Interrupt 2



	DDRC|=0x0F;	//Output to control 7-segment -> PORTC (First 4 pins)
	PORTC&=0xF0;// initialization

	DDRA|=0x3F;	//Output For enables for 7-segment -> PORTA (First 6 pins)
	PORTA&=0xC0;// initialization

	SFIOR&=~(1<<PUD);//for enable internal pull-up

	//PUSH BUTTON FOR External Interrupt INT0 with falling edge with the internal pull-up resistor(For REST)
	DDRD &= ~(1<<PD2);
	PORTD|=(1<<PD2);//enable internal pull-up for pin2 For PORTD

	//PUSH BUTTON FOR External Interrupt INT1 with Raising edge with the external pull-down resistor(For PAUSE)
	DDRD &= ~(1<<PD3);

	//PUSH BOTTON FOR External Interrupt INT2 with falling edge with the internal pull-up resistor(For RESUMED )
	DDRB &= ~(1<<PB2);
	PORTB|=(1<<PB2);//enable internal pull-up for pin2 For PORTB



	while(1)
	{
		//For the First two 7 Segment(SECONDS)
		PORTA&=0xC0;
		PORTA|=(1<<PA0);
		PORTC = (PORTC & 0xF0) | (SEC1 & 0x0F);
		_delay_us(5);
		PORTA&=0xC0;
		PORTA|=(1<<PA1);
		PORTC = (PORTC & 0xF0) | (SEC2 & 0x0F);
		_delay_us(5);

		//For the Second two 7 Segment(MINUTES)
		PORTA&=0xC0;
		PORTA|=(1<<PA2);
		PORTC = (PORTC & 0xF0) | (MIN1 & 0x0F);
		_delay_us(5);
		PORTA&=0xC0;
		PORTA|=(1<<PA3);
		PORTC = (PORTC & 0xF0) | (MIN2 & 0x0F);
		_delay_us(5);

		//For the Third two 7 Segment(HOURS)
		PORTA&=0xC0;
		PORTA|=(1<<PA4);
		PORTC = (PORTC & 0xF0) | (HOUR1 & 0x0F);
		_delay_us(5);
		PORTA&=0xC0;
		PORTA|=(1<<PA5);
		PORTC = (PORTC & 0xF0) | (HOUR2 & 0x0F);
		_delay_us(5);

	}
}


