#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

// enum pro vycet moznym operaci
enum{
  op_undefined, // neni zadana zadna operace
  op_init,      // init operace
  op_rotateRight, //rotace hor.
  op_rotateUp     // rotace ver.
}e_operation; 

// nasveni bytu, aby se zobrazil login
char login[64] = {0, -127, 102, 24, 24, 102, -127, 0, 0, -1, 8, 8, 8, 8, -16, 0, 0, -1, 2, 1, 1, 1, 2, 0, 0, 63, 64, -128, -128, 64, -1, 0,0, -1, -120, -120, -120, -120, -16, 0, 0, 3, 4, -8, 8, 4, 3, 0, -124, -62, -95, -111, -118, -124, 0, 0, 0, 16, 8, 4, 2, -1, 0};

// namapovana pamet display
unsigned char * display = (char *) 0xB0;

/* namapovany tlacitka
0.bit - init ( op_init )
1.bit - rotace hor. ( op_rotateRight )
2.bit - rotace ver. ( op_rotateUp )
*/
unsigned char * b_operations = (char *) 0xD0; 

// namapovano na bar rychlosti  
unsigned char * b_speed = (char *) 0xD1;


// funkce pro zjisteni stisknuteho tlaciska
// podle zmeny bitu ( na hodnotu 1 ) se pozna stisknute tlacitko
// podle bitu nastavym operaci
void getOperation(int *operation) {
    if(*b_operations & 0x01) { // testuji 0. bit
       *operation = op_init;
   } else if(*b_operations & 0x02) { // testuji 1. bit
       *operation = op_rotateRight;
   } else if(*b_operations & 0x04) { // testuji 2. bit
       *operation = op_rotateUp;
   } 
  
}

// inicializacni funkce, zobrazi se prvni 4 pismena z loginu
void init() {
  int i;
  
  // postupne aktivuji led na display
  for(i = 0; i < 32; i++) {
    display[i] = login[i];
 } 
}

// funkce pro simulaci zpozdeni
void delay(int delay, int *operation) {
  int i;
  int max = (256 - delay)*40; // vypocet cyklu
  
  // simulace zpozdeni
  for(i = 0; i < max; i++) {
      asm("nop"); 
      getOperation(operation);  // zjistim, jestli uzivatel mezitim nezmackl nejake tlacitko
  }
}

// rotace bytu o jeden bit
unsigned char rotateOneBit(unsigned char byte) {
    return (byte >> 1) + ((byte & 0b00000001) << 7);
}

// rotace bytu o N bitu
unsigned char rotateByNBits(unsigned char byte, int n) {
    int i;
    for(i = 0; i < n; i++){
        byte = rotateOneBit(byte); 
    }
    
    return byte;
}


// hlavni funkce
void main(void) 
{

// inicializace promennych
 int i = 0;
 int x = 64;
 int y = 0;
 int ofset = 0;
 int operation = op_undefined;
 *b_speed = 0;
 *b_operations = 0;
 
  // hlavni smycka programu 
  while(1) {
    
     __RESET_WATCHDOG(); // pri kazde smycce resetuji watchdog
     
     // zjistim si, jeslti nekdo nezmackl tlacitko
     getOperation(&operation);
     
     
     // rozhoduji se, kterou operaci vykonat
     if(operation == op_init){ // init
        init();
        
        // resetuji pomocne promenne
        i = 0;
        x = 0;
        y = 0;
        operation = op_undefined; // nastavim operaci na undefined, aby se porad nevykonaval init
     } else if (operation == op_rotateRight) { // rotace zleva doprava
     
        // prekreslim vsechny led
        for(i = 0; i < 32; i++) {
          display[i] = rotateByNBits(login[(i+x+64)%64], y%8); // vypocitam posuv
        }
          
        x--; // posunu se
        if(x < 1){ // login se uz vypsal cely, zacnu od zacatku
          x = 64;
        }
     } else if(operation == op_rotateUp) { // rotace zdola nahoru
     
        // prekreslim vsechny led
        for(i = 0; i < 32; i++) {
          display[i] = rotateByNBits(display[i], 1); // posunu byte o jeden bit
        }
       
        y++; // pripoctu vertikalni souradnici
        if(y == 8) { // prorotoval jsem vsech 8 bitu, zacnu znova
          y = 0; 
        }
     }
     
     // po kazdem prekresleni se vola zpozdeni ovladane barem rychlosti
     delay(*b_speed, &operation);
  }
}
