/*Header:

| 4 byte magic string | 1 byte version | 4 byte (uint32) # of records |

Record:

| 1 byte record type enum | 4 byte (uint32) Unix timestamp | 8 byte (uint64) user ID |

Record type enum:

* 0x00: Debit
* 0x01: Credit
* 0x02: StartAutopay
* 0x03: EndAutopay

Debit and Credit record types, there is an additional field, an 8 byte
(float64) amount in dollars, at the end of the record.

All multi-byte fields are encoded in network byte order. */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// processes unix epoch entries
int get_unix(char *buffer, int pos) {
	
	// big endian conversion four bytes to uint32
	int i, x;
	
	// uses a union to combine bit spaces
    union {
       int tmp32;
       char buf32[4];
    }u;
	
	for (i = 0; i < 4; i++) {
	    u.buf32[3-i] = buffer[pos + i];
	}
	
	x = u.tmp32;
	
	return x;
}

// processes 64 bit user id entries big endian
unsigned long long get_userid(char *buffer, int pos) {
	
	int i;
	unsigned long long x;
	
	// union to combine bytes to uint64
    union {
       unsigned long long tmp64;
       char buf64[8];
    }u;
	
	for (i = 0; i < 8; i++) {
	    u.buf64[7-i] = buffer[pos + i];
	}
	
	x = u.tmp64;
	
	return x;
}

// processes bytes to float64 big endian
double get_float(char *buffer, int pos) {
	
	int i;
	double d1;
	char bytes[8];
	
	for (i = 0; i < 8; i++) {
		bytes[7-i] = buffer[pos];
		pos++;
	}	
	
	// memcpy creates a float from bytes
	memcpy(&d1, bytes, 8);
	
	return d1;
};

int main()
{
	// declare global vars
	FILE *fp;
	int i, files, size, pos = 0, myUInt32, autopayStart = 0, autopayEnd = 0; 
	unsigned long long myUInt64;
	double myFloat64, creditTotal = 0, debitTotal = 0, userTotal = 0; 
	char *buffer;
	
	/* ------------------- PROCESS FILE TO BUFFER START ------------------- */
	
	// Open file binary mode for reading
	fp = fopen("txnlog.dat", "rb");
   
	// get size of file
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
   
	// malloc memory the size of file
	buffer = malloc(size);
   
	// read file into buffer
	fread(buffer, sizeof(char), size, fp);
	
	//close file
	fclose(fp);
	
	/* ------------------- FILE PROCESSED TO BUFFER END ------------------- */
	
	/* ------------------- HEADER START ------------------------ */
	
	// print file type
	printf("\nFile type: "); // file type
	for (i = 0; i < 4; i++){
		printf("%c", buffer[pos]);
		pos++;
	}
	
	// print version
	printf("\nVersion: %d\n", buffer[pos]); // version
	pos++;
	
	// capture total no of files
	for (i = 0; i < 1; i++){
		// big endian conversion
		myUInt32 = get_unix(buffer, pos);
		files = myUInt32 + 1; // save total no of records
		printf("Number of Records: %d\n", files);
		pos += 4;
	}
	
	/* ------------------- HEADER END --------------------------- */
	
	/* ------------------- RECORDS PARSE START ------------------------ */
	
	// run through each record, processing accordingly
	for (i = 0; i < files; i++){
		
		// 0x00: Debit
		if ( buffer[pos] == 0 ) {
			pos++;
			myUInt32 = get_unix(buffer, pos);
			pos += 4;
			myUInt64 = get_userid(buffer, pos);
			pos += 8;
			myFloat64 = get_float(buffer, pos);
			pos += 8;
			printf("\nRecord No. %d - DEBIT\n", i + 1);
			printf("unix epoch: %d\n", myUInt32);
			printf("userid: %llu\n", myUInt64);
			printf("amount: $%.2f\n", myFloat64);
			debitTotal += myFloat64; // adding to debit total
			if (myUInt64 == 1111111111111111111) {
				userTotal -= myFloat64;
			}
		}
		
		// 0x01: Credit
		else if ( buffer[pos] == 1) {
			pos++;
			myUInt32 = get_unix(buffer, pos);
			pos += 4;
			myUInt64 = get_userid(buffer, pos);
			pos += 8;
			myFloat64 = get_float(buffer, pos);
			pos += 8;
			printf("\nRecord No. %d - CREDIT\n", i + 1);
			printf("unix epoch: %d\n", myUInt32);
			printf("userid: %llu\n", myUInt64);
			printf("amount: $%.2f\n", myFloat64);
			creditTotal += myFloat64; // adding to credit total
			if (myUInt64 == 1111111111111111111) {
				userTotal += myFloat64;
			}
		}
		
		// 0x02: StartAutopay
		else if ( buffer[pos] == 2 ) {
			pos++;
			myUInt32 = get_unix(buffer, pos);
			pos += 4;
			myUInt64 = get_userid(buffer, pos);
			pos += 8;
			autopayStart++; // increment autopay start count
			printf("\nRecord No. %d - AUTOPAY START\n", i + 1);
			printf("unix epoch: %d\n", myUInt32);
			printf("userid: %llu\n", myUInt64);
			printf("autopay start count: %d\n", autopayStart);
		}
		
		// 0x03: EndAutopay
		else {
			pos++;
			myUInt32 = get_unix(buffer, pos);
			pos += 4;
			myUInt64 = get_userid(buffer, pos);
			pos += 8;
			autopayEnd++; // increment autopay end count
			printf("\nRecord No. %d - AUTOPAY END\n", i + 1);
			printf("unix epoch: %d\n", myUInt32);
			printf("userid: %llu\n", myUInt64);
			printf("autopay end count: %d\n", autopayEnd);
		}
	}
	
	/* ------------------- RECORDS PARSE END ------------------------ */
	
	// totals: debits, credits, autopays started, autopays ended
	printf("\n\nTotal Debit: $%.2f\nTotal Credit: $%.2f\nTotal Autopays Started: %d\nTotal Autopays Ended: %d\n", \
	debitTotal, creditTotal, autopayStart, autopayEnd);
	// total debit/credit entries for user 1111111111111111111
	printf("\nUser 1111111111111111111 balance: $%.2f\n\n\n", userTotal); 

	return(0);
}