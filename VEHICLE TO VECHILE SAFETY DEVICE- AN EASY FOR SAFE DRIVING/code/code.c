#include "LPC214x.H"
#include <string.h>

#define rs (1<<16)
#define en (1<<17)
#define data_pins (0xff<<18)
#define motor (1<<2)
#define fire (1<<23)
#define ir (1<<21)
#define vibration (1<<22)

#define buzzer (1<<10)
#define mot (1<<18)

void lcd_ini(void);
void delay(unsigned int);
void cmd(unsigned char);
void dis(unsigned char);
void str(char *);
void ser_ini(void);
void tx_str0(unsigned char *ptr); 
void tx0(unsigned char dat);
unsigned char rcv(void);
unsigned char rcv1(void);
void tx_str1(unsigned char *ptr); 
void tx1(unsigned char dat);
void adc_ini(void);
unsigned int adc_read(unsigned char);
unsigned char msg_no[]="9000391929";
void send_sms(char *);
unsigned char dat[50];
unsigned char lat[]="1748335",lon[]="7838706";
 
int main(void)
{
//	unsigned char i;
	unsigned int x;
	PINSEL0 = 0;
	PINSEL1 = 0; 
	PINSEL2=0;	
	IO1DIR |= (unsigned int)0X03FF0000;
	IO0DIR |= motor+buzzer;
	IO0CLR |=motor+buzzer;
	IO0DIR &=~(vibration+ir+fire);
	lcd_ini();
	delay(15);      
	str("VCL TO VCL");
	adc_ini();
	cmd(0x01);
	ser_ini();
	tx_str0("AT+CMGF=1\r");
	while(rcv()!='K');
	str("Modem Initialized");
	while(1)           //loop forever
          {
						IO0CLR|=motor;
						cmd(0x01);
						x=adc_read(4);
						dis(x/1000+'0');
						dis(x%1000/100+'0');
						dis(x%100/10+'0');
						dis(x%10+'0');
						if ((x>950)||(x<850))
						{
							//cmd(0x01);
							IO0SET|=buzzer;
							IO0SET|=motor;
							do
							{
								x=adc_read(4);
								send_sms("driver fatigue at ");
							}
							while((x>950)||(x<850));
							IO0CLR|=buzzer;
							IO0CLR|=motor;
						}
						cmd(0xc0);
						if (!(IO0PIN&vibration))
						{
							str("Vibration");
							IO0SET|=buzzer;
							IO0SET|=motor;
							send_sms("Accident Occured at ");
							while (1);
						}
						else
							str("No Vibration");
						delay(500);
						cmd(0x01);
						if (IO0PIN&fire)
							str("No Fire");
						else
						{
							str("Fire detected");
							IO0SET|=buzzer;
							IO0SET|=motor;
							send_sms("fIRE Occured at ");
							while(!(IO0PIN&fire));
							IO0CLR|=buzzer;
							IO0CLR|=motor;
						}
						cmd(0xc0);
						if (!(IO0PIN&ir))
						{
							str("Obstacle");
							IO0SET|=buzzer;
							IO0SET|=motor;
							
							while(!(IO0PIN&ir));
							IO0CLR|=buzzer;
							IO0CLR|=motor;
						}
						else
							str("No Obstacle");
						delay(500);
				
	}						
}
		    
      

 
void cmd(unsigned char data)
{
	        IO1CLR |= data_pins+rs;
						IO1SET |= (data)<<18;
						IO1CLR |= rs;
						IO1SET |= en;
						delay(30);
						IO1CLR |= en;
}

void dis(unsigned char data)
{
	IO1CLR |= data_pins+rs;
						IO1SET |= (data<<18)|rs;
						IO1SET |= en;
						delay(30);
						IO1CLR |= en;
}

void lcd_ini(void)
{
 cmd(0x38);
	cmd(0x06);
	cmd(0x0e);
   
}
 
void delay(unsigned int n)
{
           int i,j;
 
           for(i=0;i<n;i++)
              for(j=0;j<0x2700;j++);
}  
 
void str(char *msg)
{
   while(*msg!='\0')
   {
     dis(*msg);
		 msg=msg+1;
	 }          
} 
 
void ser_ini(void)
{
		
		PINSEL0|=0x50005;//configure tx and rcv
	U0LCR|=(1<<7);//access divisor latches
	U1LCR|=(1<<7);
	U0DLL=71;//set baud rate
  U0DLM=0;//set baud rate
  U0FDR=0x83;//mul=12,div=1
  U0LCR|=0x03;//8 data bits
	U0LCR&=~(1<<7);//access transmit register
	U1DLL=71;//set baud rate
  U1DLM=0;//set baud rate
  U1FDR=0x83;//mul=12,div=1
  U1LCR|=0x03;//8 data bits
	U1LCR&=~(1<<7);//access transmit register
	U1TER|=0x80;//enable tx	

}

void tx_str0(unsigned char *ptr)
{
	while (*ptr)
	{
		tx0(*ptr);
		ptr=ptr+1;
	}
}


void tx0(unsigned char dat)
{
	while (!(U0LSR&0x20));
	U0THR=dat;
}

void tx_str1(unsigned char *ptr)
{
	while (*ptr)
	{
		tx1(*ptr);
		ptr=ptr+1;
	}
}


void tx1(unsigned char dat)
{
	while (!(U1LSR&0x20));
	U1THR=dat;
}



unsigned char rcv(void)
{
	while (!(U0LSR&0x01));
	return U0RBR;
}

unsigned char rcv1(void)
{
	while (!(U1LSR&0x01));
	return U1RBR;
}
void adc_ini(void)
{
	PINSEL1|=(1<<24)|(1<<26)|(1<<18)|(1<<28);
	AD0CR|=(4<<8);
}
	

unsigned int adc_read(unsigned char port)
{
	unsigned int val;
	AD0CR &= ~0xff;
	AD0CR |=(1<<port)+(1<<21);
	delay(10);
	ADGSR |= (1<<24);
	while ((AD0STAT&(1<<16))==0);
	val=(AD0GDR>>6)&0x3ff;
	AD0CR &= ~((1<<21)+(7<<24));
	ADGSR &= ~(1<<24);
	return val;	
}

void send_sms(char *msg)
{
	tx_str0("AT+CMGS=\"");
	tx_str0(msg_no);
	tx_str0("\"\r");
	while(rcv()!=' ');
	tx_str0(msg);
	tx_str0("Lat ");
	tx_str0(lat);
	tx_str0("Long:");
	tx_str0(lon);	
	tx0(0x1a);
	while(rcv()!='+');
	cmd(0x01);
	str("Msg sent");
}

void gps(void)
{
	unsigned char d=0,i;
while(1)
	{
	dat[d]=rcv1();
		if(dat[d]=='$')
		{
			dat[d]=rcv1();
			if(dat[d]=='G')
			{

				dat[d]=rcv1();
				if(dat[d]=='P')
				{
					dat[d]=rcv1();
					if(dat[d]=='R')
					{
						for(d=0;d<40;d++)
						{
							dat[d]=rcv1();
						}
						break;
					 }//if R
				}//if P
			}//if G
		}//if $	
	}//while
	if(dat[14]=='A')
 	{
 	cmd(0x01);
	str("LAT:");dis(dat[16]);dis(dat[17]);dis(223);dis(dat[18]);dis(dat[19]);dis(dat[20]);dis(dat[21]);dis(dat[22]);str("min");
		for(i=0;i<7;i++)
		lat[i]=dat[i+16];
	cmd(0xc0);
	str("                ");cmd(0xc0);
		for(i=0;i<7;i++)
		lon[i]=dat[i+29];
	str("LOG:");dis(dat[29]);dis(dat[30]);dis(223);dis(dat[31]);dis(dat[32]);dis(dat[33]);dis(dat[34]);dis(dat[35]);str("min");
 	}
	else
	{
		cmd(0x01);
		str("No Signal");
	}
	delay(500);

}
