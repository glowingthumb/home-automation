/*
*	Project Name: Home Automation
*	Email: subhajit@glowingthumb.com, subhajit8@gmail.com
*/


#include<reg51.h>
#include<intrins.h>
#include<string.h>
#include<ctype.h>
#define ACK 1
#define NACK 0

/*INITIALIZATION*/
/*There are 8 LEDs connected to P1 header on the Dev Board*/

sbit LED1 = P1^0;
sbit LED2 = P1^1;
sbit LED3 = P1^2;
sbit LED4 = P1^3;
sbit LED5 = P1^4;
sbit LED6 = P1^5;
sbit LED7 = P1^6;
sbit LED8 = P1^7;

/*SMS Strings*/
unsigned char message[22];
unsigned char data1[]="DEV";
unsigned char data2[]="SET";
unsigned char data3[]="TIME";
unsigned char data4[]="TIME SET SUCCESS";
unsigned char errormessage[]="INVALID CHOICE";
unsigned char cellno[]= "AT+CMGS=\"0415945397\"";
unsigned char timetext[36];
unsigned char toggletext[9];
unsigned char dev[]="DEV";
unsigned char on[]="ON";
unsigned char off[]="OFF";
unsigned char space[]=" ";
unsigned char one[]="1",two[]="2",three[]="3",four[]="4",five[]="5",six[]="6",seven[]="7",eight[]="8";

/*Emulating I2C by Bit banging as this 8051 chip does not have in-built I2C.*/
sbit scl=P2^7;
sbit sda=P2^6;
bit acknowledge,b;

/*Date strings for the LCD*/
unsigned char year,month,date,hour,min,sec;
unsigned char year_text[3],month_text[3],date_text[3],hour_text[3],min_text[3],sec_text[3];
sbit en=P2^2;
sbit rs=P2^1;
sbit rw=P2^0; 

/*Function declarations*/
void delay(unsigned char);
void lcd_init();
void lcd_command(unsigned char);
void lcd_data(unsigned char *);
void start();
void stop();
void ack(bit);
void send_serial_byte(unsigned char);
unsigned char rec_serial_byte(void);
void set_time(unsigned char*,unsigned char*,unsigned char*, unsigned char*, unsigned char*, unsigned char*);
unsigned char ascii_bcd(unsigned char* );
unsigned char* bcd_ascii(unsigned char );
unsigned char* get_year();
unsigned char* get_month();
unsigned char* get_date();
unsigned char* get_hour();
unsigned char* get_min();
unsigned char* get_sec();
void Toupper(unsigned char *);

void matchdata(void);
void toggledevice(unsigned char);

/* 5*7, 2 LINES NO CURSOR BLINK, DISLAY ON */
void lcd_init()						
{
	lcd_command(0x38);
	delay(1);
	lcd_command(0x0C);
	delay(1);
	lcd_command(0x80);
	delay(1);									  
}

/*LCD COMMAND*/
void lcd_command(unsigned char d)			 
{
	rs=0;
	rw=0;
	P0= d;
	en=1;
	delay(1);
	en=0;
}

/*LCD DATA*/
void lcd_data(unsigned char *d)				   
{
	en=1;
	rs=1;
	rw=0;
	while(*d!='\0')
	{
		en=1;
		P0= *d;
		delay(1);
		en=0;
		d++;
	}
}

/*I2C Emulation*/
/*I2C START*/
void start()								 
{
	sda=1;
	scl=1;
	_nop_();
	_nop_();
	sda=0;
	_nop_();
	scl=0;
}

/*I2C STOP*/
void stop()								   
{
	sda=0;
	scl=1;
	_nop_();
	_nop_();
	sda=1;
	_nop_();
	scl=1;
}

/*I2C ACK/NACK*/
void get_ack(bit b)						  
{	
	if(b==ACK)
		sda=0;
	else
		sda=1;
	scl=0;
	scl=1;
	_nop_();
	acknowledge=sda;
	_nop_();
	scl=0;
}

/*I2C SEND DATA*/
void send_serial_byte(unsigned char clk_data_input)		  
{
	unsigned char i,deta;
	scl=0;
	deta=clk_data_input;
	for(i=0;i<8;i++)
	{
	 	sda=((deta & 0x80)?1:0);
		deta<<=1;
		scl=1;
		_nop_();
		_nop_();
		scl=0;
	}

}

/*I2C RECEIVE DATA*/
unsigned char rec_serial_byte(void)						
{

	unsigned char rec_byte=0;
	unsigned char i;
	sda=1;
	for(i=0;i<8;i++)
	{
		rec_byte=rec_byte<<1;
		scl=1;
		_nop_();
		_nop_();
	 	if(sda==1)
		rec_byte++;
		scl=0;
	}

	return rec_byte;
}


/*Function to set the time on the onboard battery backed RTC using I2C*/
void set_time(unsigned char* year,unsigned char* month,unsigned char* date, unsigned char* hour, unsigned char* min, unsigned char* sec)
{
	unsigned char year_b,month_b,date_b,hour_b,min_b,sec_b,day_b;

	year_b=ascii_bcd(year);
	month_b=ascii_bcd(month);
	date_b=ascii_bcd(date);
	hour_b=ascii_bcd(hour);
	min_b=ascii_bcd(min);
	sec_b=ascii_bcd(sec);
	day_b=0x01;
	
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x00);
	get_ack(ACK);
	send_serial_byte(sec_b);
	get_ack(ACK);
	send_serial_byte(min_b);
	get_ack(ACK);
	send_serial_byte(hour_b);
	get_ack(ACK);
	send_serial_byte(day_b);
	get_ack(ACK);
	send_serial_byte(date_b);
	get_ack(ACK);
	send_serial_byte(month_b);
	get_ack(ACK);
	send_serial_byte(year_b);
	get_ack(ACK);
	send_serial_byte(0x10);
	get_ack(ACK);
	stop();
} 

/*CONVERSION OF ASCII TO BCD AS SERIAL CHIP TAKES BCD DATA*/
unsigned char ascii_bcd(unsigned char* deta)	
{

	unsigned char byte1=0,byte2=0,byte3=0;
	byte1=(*deta)&0x0F;
	byte1<<=4;
	deta++;
	byte2=(*deta)&0x0F;
	byte3=byte2|byte1;
	return byte3;
}

/*BCD TO ASCII CONVERSION*/
unsigned char* bcd_ascii(unsigned char deta)		  
{
    static unsigned char byte[3];
	byte[0]=deta&0xF0;
	byte[0]>>=4;
	byte[0]|=0x30;
	byte[1]=deta&0x0F;
	byte[1]|=0x30;
	byte[2]='\0';
	return byte;
}

unsigned char* get_year()
{
    unsigned char a,*b;
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x06);
	get_ack(ACK);
	start();
	send_serial_byte(0xD1);
	get_ack(ACK);
	a= rec_serial_byte();
	get_ack(NACK);
	stop();
	b= bcd_ascii(a);
	return b;
}

unsigned char* get_month()
{
    unsigned char a,*b;
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x05);
	get_ack(ACK);
	start();
	send_serial_byte(0xD1);
	get_ack(ACK);
	a= rec_serial_byte();
	get_ack(NACK);
	stop();
	b= bcd_ascii(a);
	return b;
}

unsigned char* get_date()
{
    unsigned char a,*b;
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x04);
	get_ack(ACK);
	start();
	send_serial_byte(0xD1);
	get_ack(ACK);
	a= rec_serial_byte();
	get_ack(NACK);
	stop();
	b= bcd_ascii(a);
	return b;
}

unsigned char* get_hour()
{
    unsigned char a,*b;
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x02);
	get_ack(ACK);
	start();
	send_serial_byte(0xD1);
	get_ack(ACK);
	a= rec_serial_byte();
	get_ack(NACK);
	stop();
	b= bcd_ascii(a);
	return b;
}

unsigned char* get_min()
{
    unsigned char a,*b;
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x01);
	get_ack(ACK);
	start();
	send_serial_byte(0xD1);
	get_ack(ACK);
	a= rec_serial_byte();
	get_ack(NACK);
	stop();
	b= bcd_ascii(a);
	return b;
}

unsigned char* get_sec()
{
    unsigned char a,*b;
	start();
	send_serial_byte(0xD0);
	get_ack(ACK);
	send_serial_byte(0x00);
	get_ack(ACK);
	start();
	send_serial_byte(0xD1);
	get_ack(ACK);
	a= rec_serial_byte();
	get_ack(NACK);
	stop();
	b= bcd_ascii(a);
	return b;
}

/*SMS can be sent in any Letter case*/
void Toupper(unsigned char *a)        
{
	unsigned char i;
	for(i=0;a[i]!='\0';i++)
		{
		if(a[i]>=97 && a[i]<=122)
			a[i]=a[i]-32;
		}

}

/*Delay routine.*/
void delay(unsigned char x)  
{
	unsigned int i,j;
	for(i=0;i<x;i++)
		for(j=0;j<2000;j++);
}

/*Returning the content of Serial Buffer.*/
unsigned char sbufdata(void) 
{
	RI=0;
	while(RI==0);
	return(SBUF);
	RI=0;
}

/*Initializing the serial port with 9600 8N1 (mode 2, 8 bit auto reload).*/
void serialinit(void) 
{
	TMOD= 0x20;
	TH1= 0xFD;
	SCON=0x50;
	TR1=1;
}

/*Sending command to the SIM 300 GSM board.*/
void gsmcmdsend(unsigned char *a)		
{
	unsigned char i;

	for(i=0;a[i]!='\0';i++)
	{
		SBUF=a[i];
		while(TI==0);
		TI=0;
	}
	delay(4);

	SBUF=0x0A; // LF
	while(TI==0);
	TI=0;
	
	SBUF=0x0D; // CR
	while(TI==0);
	TI=0;

	while(RI==0); // SIM 300 Module sends "OK", but we are not making use of that, just receiving a character for confirmation.
	RI=0;
}

/*SIM 300 Init sequence*/
void gsminit(void)
{											  
	gsmcmdsend("AT");    // Sending AT to initialize.
	gsmcmdsend("ATE0");  // Turning Character Echo Off.
	gsmcmdsend("AT&W");
	gsmcmdsend("AT+CMGF=1");  
	gsmcmdsend("AT+CNMI=2,2,0,0,0"); 
}

/*Sending predefined text message to the receipient.*/
void sendtextmessage(unsigned char text[])  
{
	unsigned char i;

	for(i=0;cellno[i]!='\0';i++)
	{
		SBUF=cellno[i];
		while(TI==0);
		TI=0;
	}
	delay(16);
	
	SBUF=0x0D;
	while(TI==0);
	TI=0;

	while(RI==0);
	RI=0;

	for(i=0;text[i]!='\0';i++)
	{
		SBUF=text[i];
		while(TI==0);
		TI=0;
		delay(6);
	}
	
	delay(21);
	SBUF=0x1A;
	while(TI==0);
	TI=0;

	while(RI==0);
		RI=0;
}

/*Update time on LCD.*/
void update_time() interrupt 0
{
   
	
	lcd_command(0x80);
	lcd_data("Y:20");
	lcd_data(get_year());
	lcd_data(" M:");
	lcd_data(get_month());
	lcd_data(" D:");
	lcd_data(get_date());

	lcd_command(0xC0);
	lcd_data("Time: ");
	lcd_data(get_hour());
	lcd_data(":");
	lcd_data(get_min());
	lcd_data(":");
	lcd_data(get_sec());

}

/*Running in polling mode, called from main(). Have to test with interrupt 4*/
void receivedata()
{
	while(1)
	{
	unsigned char count;
	     	
		while(sbufdata()!='+'); // Look for this string which is the initialization of SMS.		  
		while(sbufdata()!='C');
        while(sbufdata()!='M');
        while(sbufdata()!='T');
        while(sbufdata()!=':');
        while(sbufdata()!='"');
				while(sbufdata()!=0x0D);
				while(sbufdata()!=0x0A);

		for(count=0;count<24;count++)	
		{										  
   			message[count]=sbufdata();
   			if(message[count]==0x0D)
			{
			 	message[count+1]=sbufdata();
			 	if(message[count+1]==0x0A)
					break;
				else
					count++;
			}
		}
	    message[count]='\0';
			
   		Toupper(message); // Convert the SMS to Upper Case.
	
		IE&=0xFE;
		matchdata(); // Analize the SMS and take action.
		IE|=0x01;
	}
}

/*Match the SMS with inbuilt commands and perform function.*/
void matchdata() 
{	
	unsigned char devid,i,j;
	bit errorflag=0;
		if(strncmp(message,data1,3)==0) // check for keyword for DEVICE SWITCHING FUNCTION.
		{
			if(message[3]=='\0' || message[3]<49 || message[3]>57) //check for device no.
			{
				errorflag=1; //wrong string will send back error msg.
				goto errorexit;
			}
			else
			{
				devid=message[3]-0x30; //get the device no. from sms
				toggledevice(devid); //toggle the device.
			}
 		}
		
		else if(strncmp(message,data2,3)==0) //Clock setting function.
		{
			for(i=4;i<17;i+=3)
			{
				if(message[i]>57 || message[i+1]<48 || message[i+2]!=' ')
				{
					errorflag=1;
					goto errorexit;
				} 			
			}
			i=19;
			if(message[i]>57 || message[i+1]<48)
			{
				errorflag=1;
				goto errorexit;
			}
		
			i=4;
			
			for(j=0;j<2;j++)
				year_text[j]=message[i++];
			year_text[j]='\0';
			i++;
				
			for(j=0;j<2;j++)
				month_text[j]=message[i++];
			month_text[j]='\0';
			i++;

			for(j=0;j<2;j++)
				date_text[j]=message[i++];
			date_text[j]='\0';
			i++;

			for(j=0;j<2;j++)
				hour_text[j]=message[i++];
			hour_text[j]='\0';
			i++;

			for(j=0;j<2;j++)
				min_text[j]=message[i++];
			min_text[j]='\0';
			i++;

			for(j=0;j<2;j++)
				sec_text[j]=message[i++];
			sec_text[j]='\0';
			
			set_time(year_text,month_text,date_text,hour_text,min_text,sec_text); //Year Month Date Hour Min Sec.
			sendtextmessage(data4);
		
		}
		else if(strncmp(message,data3,4)==0) // SMS Clock Time
		{			   
			timetext[0]='\0';
			strcat(timetext,"Y: ");
			strcat(timetext,"20");
			strcat(timetext,get_year());
			strcat(timetext," M:");
			strcat(timetext,get_month());
			strcat(timetext," D:");
			strcat(timetext,get_date());
			strcat(timetext," TIME NOW ");
			strcat(timetext,get_hour());
			strcat(timetext,":");
			strcat(timetext,get_min());
			strcat(timetext,":");
			strcat(timetext,get_sec());
			sendtextmessage(timetext);		
		}
		else
			sendtextmessage(errormessage); // Send error for wrong SMS string.		
		
		errorexit:
			if(errorflag==1)
				sendtextmessage(errormessage);
				errorflag=0;
}

/*Toggle the LEDs on the Dev board.*/
void toggledevice(unsigned char a)
{
	toggletext[0]='\0';
	switch(a)
	{
		case 1:
			LED1=~LED1;
			_nop_();
			_nop_();
			if(LED1==0)
			{
				strcat(toggletext,dev);
				strcat(toggletext,one);
				strcat(toggletext,space);
				strcat(toggletext,on);
				sendtextmessage(toggletext);
			}
			else
			{
				strcat(toggletext,dev);
				strcat(toggletext,one);
				strcat(toggletext,space);
				strcat(toggletext,off);
				sendtextmessage(toggletext);
			}					
			break;

			case 2:
				LED2=~LED2;
				_nop_();
				_nop_();
				if(LED2==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,two);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,two);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			case 3:
			   	LED3=~LED3;
				_nop_();
				_nop_();
			   	if(LED3==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,three);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,three);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			case 4:
			   	LED4=~LED4;
				_nop_();
				_nop_();
				if(LED4==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,four);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,four);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			case 5:
			   	LED5=~LED5;
				_nop_();
				_nop_();
			   	if(LED5==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,five);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,five);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			case 6:
			   	LED6=~LED6;
				_nop_();
				_nop_();
			   	if(LED6==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,six);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,six);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			case 7:
				LED7=~LED7;
				_nop_();
				_nop_();
			   	if(LED7==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,seven);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,seven);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			case 8:
			   	LED8=~LED8;
				_nop_();
				_nop_();
			   	if(LED8==0)
				{
					strcat(toggletext,dev);
					strcat(toggletext,eight);
					strcat(toggletext,space);
					strcat(toggletext,on);
					sendtextmessage(toggletext);
				}
				else
				{
					strcat(toggletext,dev);
					strcat(toggletext,eight);
					strcat(toggletext,space);
					strcat(toggletext,off);
					sendtextmessage(toggletext);
				}
			break;

			default:
				sendtextmessage(errormessage);
			break;		
		}
}

/*Main*/
void main()
{
	serialinit(); //UART Init
	gsminit(); //SIM 300 Init
	lcd_init(); //LCD Init
	IE|=0x81; //Interrupt Enable
	IT0|=1;
	receivedata(); //Start polling UART.	
}
