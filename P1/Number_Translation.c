#include <stdio.h>
#include <math.h>

#define BINARY_STRING_LENGTH 33
#define HEX_STRING_LENGTH 11

void GetBinaryFromUser(char *binary);
void GetHexFromUser(char *hex);
void GetDecimalFromUser(unsigned int *x);

void ConvertBinaryToDecimal(const char *binary, unsigned int *decimal);
void ConvertHexToDecimal(const char *hex, unsigned int *decimal);
void ConvertDecimalToBinary(const unsigned int decimal, char *binary); 
void ConvertDecimalToHex(const unsigned int decimal, char *hex); 

int main(){
    char binary[BINARY_STRING_LENGTH] = "11011";
    char hex[HEX_STRING_LENGTH] = "0x3F";
	unsigned int decimal = 31;
	
	enum MENU {binary_to_decimal=1, hex_to_decimal, decimal_to_binary, decimal_to_hex};
	while (1) {
		printf("Which conversion would you like to do?\n");
		printf("  %d. Binary to Decimal\n", binary_to_decimal);
		printf("  %d. Hex to Decimal\n", hex_to_decimal);
		printf("  %d. Decimal to Binary\n", decimal_to_binary);
		printf("  %d. Decimal to Hex\n", decimal_to_hex);
		printf("  q to Quit\n");
		int choice;
		if (!scanf("%d", &choice)) return 0;  // exits if user enters something other than a digit
		
		switch (choice) {
			case binary_to_decimal: {
				GetBinaryFromUser(binary); 
				ConvertBinaryToDecimal(binary, &decimal); 
				printf("Binary: %s\nDecimal: %u\n\n", binary, decimal);
			} break;
			case hex_to_decimal: {
				GetHexFromUser(hex); 
				ConvertHexToDecimal(hex, &decimal); 
				printf("Hex: %s\nDecimal: %u\n\n", hex, decimal);
			} break;
			case decimal_to_binary: {
				GetDecimalFromUser(&decimal); 
				ConvertDecimalToBinary(decimal, binary); 
				printf("Decimal: %u\nbinary: %s\n\n", decimal, binary);
			} break;
			case decimal_to_hex: {
				GetDecimalFromUser(&decimal); 
				ConvertDecimalToHex(decimal, hex); 
				printf("Decimal: %u\nHex: %s\n\n", decimal, hex);
			} break;
			default: return 0; // exit if user enters an invalid number
		}
	}
	return 0;
}

void GetBinaryFromUser(char * binary) {
    // write your code here to complete this function
    printf("Please enter binary string here: ");
    scanf("%s", binary);
    return;
}

void GetHexFromUser(char * hex) {
    // this function has been written for you
    printf("Please enter hex string here (do include the 0x prefix): ");
    scanf("%s", hex);
    for (char *p=hex;*p;p++){
      if(*p>='a'){
	*p-='a'-'A';
      }
    }// to upper case
    return;
}

void GetDecimalFromUser(unsigned int *x) {
    // write your code here to complete this function
    printf("Please enter decimal number here: ");
    scanf("%u", x);
    return;
}

void ConvertBinaryToDecimal(const char * binary, unsigned int *decimal) {
    // write your code here to complete this function
    int numCount = 0;
    *decimal = 0;

    // counts how many digits there are in the binary
    while (*binary == '0' || *binary == '1'){
      numCount += 1;
      binary += 1;
    }

    // resets pointer position
    binary -= numCount;

    // converting to decimal
    for (int x = numCount; x > 0; x--){
      int power = x-1;
      *decimal += (*binary - 48) * (int)(pow(2.00, (double)power));
      binary += 1;
    }
    return;
}

void ConvertHexToDecimal(const char *hex, unsigned int *decimal) {
    // write your code here to complete this function
    int numCount = 0;
    *decimal = 0;

    // shifts pointer after the initial '0X'
    hex += 2;
  
    // counts how many digits there are in the hex
    while ((*hex >= 48 && *hex < 58) || (*hex >= 65 && *hex < 71)) {
      numCount += 1;
      hex += 1;
    }

    // resets pointer position
    hex -= numCount;

    // converting to decimal
    for (int x = numCount; x > 0; x--) {
      int power = x-1;

      // if the character is a digit or an alphabet
      if (*hex >= 48 && *hex < 58) {
        *decimal += (*hex - 48) * (int)(pow(16.00, (double)power));
      }
      else{
        *decimal += (*hex - 55) * (int)(pow(16.00, (double)power));
      }
      hex += 1;
    }
    return;
}

void ConvertDecimalToBinary(const unsigned int decimal, char *binary) {
    // write your code here to complete this function
    int numCount = 0;
    int result = 1;
  
    // counts how many binary digits it should take up
    if (decimal == 0) numCount = 1;

    else {
      while (result > 0) {
        result = decimal - (int)(pow(2.00, (double)numCount));

        if (result < 0) {
	  numCount -= 1;
        }
        numCount += 1;
      }
    }

    // moves pointer to relevant location and adds '\0' to determine where array ends
    binary += numCount;
    *binary = '\0';
    binary -= 1;

    // copy of decimal since decimal is a const
    result = decimal;

    // converts decimal into binary
    for (int x = 0; x < numCount; x++) {
      *binary = (result % 2)+48;
      result = result / 2;
      binary -= 1;
    }
    
    binary += 1;
    return;
}

void ConvertDecimalToHex(const unsigned int decimal, char *hex) {
    // write your code here to complete this function
    int numCount = 0;
    int result = 1;

    // counts how many hex digits it should take up
    if (decimal == 0) numCount = 1;

    else {
      while (result > 0) {
        result = decimal - (int)(pow(16.00, (double)numCount));

        if (result < 0) {
	  numCount -= 1;
        }
        numCount += 1;
      }
    }

    // moves pointer to relevant location and adds '\0' to determine where array ends
    hex += numCount+2;
    *hex = '\0';
    hex -= 1;

    // copy of decimal since decimal is a const
    result = decimal;

    // converts decimal into hex
    for (int x = 0; x < numCount; x++) {
      if (result % 16 > 9) {
        *hex = (result % 16)+55;
      }
      else {
        *hex = (result % 16)+48;
      }
      result = result / 16;
      hex -= 1;
    }

    // adding the initial '0X'
    *hex = 'X';
    hex -= 1;
    *hex = '0';
    return;
}










