#include<stdio.h>
#include<windows.h> 
#include <string.h>
#include<math.h>


#define MAX_LONGITUDE 180
#define MAX_LATITUDE 90
#define KNOTS_TO_KM 1.852
#define DELIMITER__ ','

float ltemp;
int b;
float c;
int counter;
char loglines[200];

struct GPS_Data
{
	float latitude;
	char latitude_hemisphere;
	float longitude;
	char longitude_hemisphere;
};

float convert_to_coordinates(float coordinates, const char* val)
{
	if ((*val == 'm') && (coordinates < 0.0 && coordinates > MAX_LATITUDE))
	{
		return 0;
	}
	if (*val == 'p' && (coordinates < 0.0 && coordinates > MAX_LONGITUDE))
	{
		return 0;
	}

	b = coordinates / 100;
	c = (coordinates / 100 - b) * 100;
	c /= 60;
	c += b;
	return c;
}

void string_parser(char* oneline, struct GPS_Data* gps)
{
	const char delimiter = DELIMITER__;
	char* token;
	char* next_token;
	rsize_t strmax = sizeof oneline;
	token = strtok_s(oneline, &strmax, delimiter, &next_token);
	counter = 0;
	token = strtok_s(oneline, &strmax, delimiter, &next_token);
	while (token != NULL)
	{
		if (counter == 2)
		{
			ltemp = atof(token);
			ltemp = convert_to_coordinates(ltemp, "m");
		}
		if (counter == 3)
		{
			if (*token == 'S')
			{
				gps->latitude_hemisphere = 'S';
			}
			else
			{
				gps->latitude_hemisphere = 'N';
			}
			gps->latitude = ltemp;
		}
		if (counter == 4)
		{
			ltemp = atof(token);
			ltemp = convert_to_coordinates(ltemp, "p");
		}
		if (counter == 5)
		{
			if (*token == 'W')
			{
				gps->longitude_hemisphere = 'W';
			}
			else
			{
				gps->longitude_hemisphere = 'E';

			}
			gps->longitude = ltemp;
		}
		token = strtok_s(NULL, &strmax, delimiter, &next_token);
		++counter;
	}
}


void get_Coordinates(char *line) {

	struct GPS_Data gps;
	char select[80];

	if (strstr(line, "$GPGGA"))
	{
		string_parser(line, &gps);
	}
	if (((gps.latitude > 0) && (gps.latitude < MAX_LATITUDE)) && ((gps.longitude > 0) && (gps.longitude < MAX_LONGITUDE)))
		printf("Latitude: %f %c\nLongitude: %f %c", gps.latitude, gps.latitude_hemisphere, gps.longitude, gps.longitude_hemisphere);
	else
		printf("invalid input");
}

void main() {


	printf("*****************************\n");
	printf("*   Reading From Port(14)   *\n");
	printf("*****************************\n\n");
	printf("Opening serial port 14\n");

	HANDLE hComm14;

	hComm14 = CreateFile("\\\\.\\COM14",//port name
		GENERIC_READ | GENERIC_WRITE, //Read/Write
		0,                            // No Sharing
		NULL,                         // No Security
		OPEN_EXISTING,// Open existing port only
		0,            // Non Overlapped I/O
		NULL);        // Null for Comm Devices

	if (hComm14 == INVALID_HANDLE_VALUE)
		printf("Error in opening serial port 14\n");
	else
		printf("Opening serial port com14 successful\n");

	//Setting Baud Rate...

	DCB dcbSerialParams = { 0 }; // Initializing DCB structure
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	GetCommState(hComm14, &dcbSerialParams);

	dcbSerialParams.BaudRate = CBR_9600;  // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
	//dcbSerialParams.StopBits = ONESTOPBIT;// Setting StopBits = 1
	dcbSerialParams.Parity = NOPARITY;  // Setting Parity = None


	SetCommState(hComm14, &dcbSerialParams);

	//Setting Timeouts...

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 50; // in milliseconds
	timeouts.ReadTotalTimeoutConstant = 1000; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier = 10; // in milliseconds
	timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds
	SetCommTimeouts(hComm14, &timeouts);

	//Reading from Com14
	
	
	//Setting an event
	printf("\nListening From Port(14)");

	SetCommMask(hComm14, EV_RXCHAR);
	DWORD dwEventMask;
	int size = 200000;
	//char* heapBuffer = (char*)malloc(sizeof(char) * size); /*Stored in heap segment*/
	
	
	int i = 0;
	int j = 0;
	WaitCommEvent(hComm14, &dwEventMask, NULL);
	char TempChar; //Temporary character used for reading

	DWORD NoBytesR = 0;
	DWORD NoBytesRead;

	do
	{
		ReadFile(hComm14,           //Handle of the Serial port
			&TempChar,       //Temporary character
			sizeof(TempChar),//Size of TempChar
			&NoBytesRead,    //Number of bytes read
			NULL);
		loglines[j] = TempChar;
		j++;


		//SerialBuffer[i] = TempChar;// Store Tempchar into buffer
		if (TempChar == 10) {
			 
			
			printf("Line number : %d, Length of Line : %d\n", i, j);

			get_Coordinates( loglines );
			
			for (int v = 0;v < j;v++) {
				printf("%c", loglines[v]);
				
			}
			printf("\n");
			
			//i++;
			j = 0;
		}
		//*(heapBuffer + i) = TempChar;
		
		//printf("%c\n", TempChar);
	}

	while (NoBytesRead > 0);

	
	
	/*
	char oneline[200];

	for (int o = 0;o < 1000;o++) {
			for (int v = 0;v < 200;v++){
				oneline[v] = loglines[o];
				
			}
			printf("%s\n", oneline);
			get_Coordinates(oneline);
	}
	*/


	//SerialBuffer[i] = '\0';
	//*(heapBuffer + i) = '\0';

	//printf("\nThe Data of LogLine:\n");

	
	//printf("Data Recieved from port com14:\n\nLines(%d) \nLastLine( %s )\n", i, loglines[i]);

	//Trimming Data...
	printf("trimming data...\n");
	/*char *ptr = strstr(heapBuffer,"GPGGA");
	if (ptr != NULL) //Substring found
	{
		printf("GPGGA position is '%i'\n",ptr - heapBuffer );
	}
	else // Substring not found 
	{
		printf("doesn't contain \n");
	}*/
	//int j = strstr(SerialBuffer, "GPGGA") - SerialBuffer;


	//printf("GPGGA position is '%i'\n", j);
	printf("\n");
	

	printf("*****************************\n");
	printf("*   Writing through Port(15)   *\n");
	printf("*****************************\n\n");
	printf("Opening serial port 15\n");
	HANDLE hComm15;
	hComm15 = CreateFile("\\\\.\\COM15",//port name
		GENERIC_READ | GENERIC_WRITE, //Read/Write
		0,                            // No Sharing
		NULL,                         // No Security
		OPEN_EXISTING,// Open existing port only
		0,            // Non Overlapped I/O
		NULL);        // Null for Comm Devices

	if (hComm15 == INVALID_HANDLE_VALUE)
		printf("Error in opening serial port 15\n");
	else
		printf("Opening serial port com15 successful\n");

	//Setting Baud Rate...

	DCB dcbSerialParams15 = { 0 }; // Initializing DCB structure
	dcbSerialParams15.DCBlength = sizeof(dcbSerialParams15);

	GetCommState(hComm15, &dcbSerialParams15);

	dcbSerialParams15.BaudRate = CBR_9600;  // Setting BaudRate = 9600
	dcbSerialParams15.ByteSize = 8;         // Setting ByteSize = 8
	dcbSerialParams15.StopBits = ONESTOPBIT;// Setting StopBits = 1
	dcbSerialParams15.Parity = NOPARITY;  // Setting Parity = None

	SetCommState(hComm15, &dcbSerialParams15);

	//Setting Timeouts...

	COMMTIMEOUTS timeouts15 = { 0 };
	timeouts15.ReadIntervalTimeout = 50; // in milliseconds
	timeouts15.ReadTotalTimeoutConstant = 50; // in milliseconds
	timeouts15.ReadTotalTimeoutMultiplier = 10; // in milliseconds
	timeouts15.WriteTotalTimeoutConstant = 1000; // in milliseconds
	timeouts15.WriteTotalTimeoutMultiplier = 10; // in milliseconds


	//Writing data to com port...


	//char lpBuffer[8];
	//fgets(lpBuffer, 128, stdin);
	DWORD dNoOFBytestoWrite;         // No of bytes to write into the port
	DWORD dNoOfBytesWritten = 0;     // No of bytes written to the port
	dNoOFBytestoWrite = 200000;



	//Writing into com15 Alhasan
	printf("Writing into com15\n");
	WriteFile(hComm15,        // Handle to the Serial port
		"asa",     // Data to be written to the port
		dNoOFBytestoWrite,  //No of bytes to write
		&dNoOfBytesWritten, //Bytes written
		NULL);


	printf("\nNumber of bytes written %d\n", dNoOfBytesWritten);
	printf("\nClosing com14\n");
	CloseHandle(hComm14);//Closing the Serial Port
	printf("\nClosing com15\n");
	CloseHandle(hComm15);//Closing the Serial Port
	char c;
	printf("\nPress Enter to Conitinue");
	scanf_s("%c", &c);
	return 0;

}
