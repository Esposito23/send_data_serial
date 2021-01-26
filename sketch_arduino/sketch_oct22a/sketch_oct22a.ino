//Francesco Esposito 
//22-10-20


#include <SerialFlash.h>
#include <SPI.h>

const int sizeBuffer = 1024;
byte arrayBuffer[sizeBuffer];
byte inizioSeq = 0x23; // #
byte typeTXT= 0x74; //f
int n=0;  // byte da consumare
int m=0;  // byte resuidi
int up=0; // 1* pos da riempire
int down=0; //1*pos da consumare


SerialFlashFile flashFile;            // oggetto su cui si carica il file per la lib SerialFlash


#define MOSI              13
#define MISO              14
#define SCK               12
#define CSPIN              30



void setup()
{
    Serial.begin(9600);
    Serial.println("Arduino in ascolto sulla seriale");
    // aspetta che si possa accedere alla memoria
    if (!SerialFlash.begin(CSPIN)) {
    while (1) {
      Serial.println("Unable to access SPI Flash chip");
      delay(1000);
    }
  }




//    for (int i = 0; i<sizeBuffer; i++)                // E' necessario resettare il buffer?
//    {                     
//      arrayBuffer[i]='\x00';
//    }
}


void chiudiFile(){                  // Verifica che sia ultimato la scrittura del file , chiude.
  if (m<1){
      Serial.println("Chiudi file");
       flashFile.close();
//      controlloFile();            // Richiama metodo di controllo scrittura(vedi codice sotto)
      }
}


//void controlloFile(){              // Metodo di verifica scrittura
//    char filename[64];            // Stampa il file scritto con il filename attuale
//    uint32_t filesize;
//    SerialFlash.opendir();
//    while (1) {
//      if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
//        Serial.println(filename);
//        Serial.println(filesize);
//        
//        char buffer[1];
//        SerialFlashFile file;
//        file = SerialFlash.open(filename);
//        if (file) {    
//          while (file.available()) {
//          file.read(buffer, 1); 
//    
//         Serial.write((char)*buffer);
//          }      
//    } else {
//      break; // no more files
//    }
//  }
// }
//}

  




  


void ascolta_seriale(){

  if (Serial.available() and n<sizeBuffer) {            // Se presente info sulla seriale
               
    if (m>0){                                           // Consumo i byte residui
    
      char writeByte[0];
      writeByte[0]=Serial.read();
      flashFile.write(writeByte,1);
      m--;
      chiudiFile();
    }
     else
    {                                               //Diversamente carico sul buffer
      arrayBuffer[up]=Serial.read();
      up=(up+1)%sizeBuffer;
      n++;
    }
  }
}



void crea_pack(){
  if (n>=20 and m==0){                             //Se sul Buffer sono presenti almeno 20 byte
    
    const int sizePack = 20;                        // Size del pack
    int sizeFile;                                   //size del file
    String fileType;                               //tipo di file
    byte arrayPack[sizePack];                      //Pack di controllo
    
    for (int idx = 0; idx<=20; idx++){             //Copio i primi 20 byte in un nuovo array          
      arrayPack[idx]=arrayBuffer[(down+idx)%sizeBuffer];
    }

    if (check_header(arrayPack)){                  //Verifico correttezza pacchetto attraverso Header
      
      char * mioPack=extract_name(arrayPack);       //Leggo il nome del file
      fileType = extract_type(arrayPack[2]);       //Leggo il tipo del file
      sizeFile = extract_size(arrayPack);          //Leggo lo spazio da occupare 
      m= sizeFile;
      n-=20;                                        //Aggiorno le variabili
      down=(down+20)%sizeBuffer;
      
       if (SerialFlash.exists(mioPack)){          // Controlla se esiste il file con quel nome
          SerialFlash.remove(mioPack);            // Se esiste viene rimosso
          }
        
        if (SerialFlash.createErasable(mioPack, sizeFile)) {       //Crea file
          flashFile = SerialFlash.open(mioPack);                   // Imposta il file in apertura
          
          if (!flashFile) {
            Serial.println("Error flash file open");              //Errore su apertura file
            return;
          }
        }
        else {
          Serial.println("Error flash create");                   // Errore creazione file
          return;
        }
    }
    else
    {                                                             //Se il pacchetto non e' corretto andiamo avanti di 1
      down=(down+1)%sizeBuffer;
      n--;
    }
  }
}




int extract_size(byte arrayPack[]){                               //Converto i 4 byte al corrispettivo intero
  int sizeTemp=arrayPack[15] << 24 | arrayPack[16] << 16  | arrayPack[17] << 8 | arrayPack[18];
  Serial.print("Lo spazio da allocare : ");
  Serial.println(sizeTemp);    
  return(sizeTemp);
}


char * extract_name(byte arrayPack[]){         //Estraggo il nome del file e lo ritorna
  char nome [12];
  int temp = 0;
  for (int i = 3 ; i<=14 ; i++ ){             //grezzo
    if (arrayPack[i] != ' '){                 //Copia il filename in un nuovo array
      nome[temp]=arrayPack[i];
      temp++;
      }
  }
  return(nome);
}


String extract_type(byte b){          //Estrai il tipo di file ( da sistemare la politica del terzo byte)
  if (b==typeTXT){
    Serial.println("Il file e' di tipo testo");
    return(".txt");
  }
  else{
    Serial.println("Il file non e' supportato");
    return("null");
  }
}



boolean check_header(byte arrayPack []){                 //Verifico i primi 2 byte che siano quelli di start (da definire)
  if (arrayPack[0]==inizioSeq and arrayPack[1]==inizioSeq){
    Serial.println("Header corretto");
    return (true);
  }
  else{
    Serial.print("Header Sbagliato");
    return(false);
  }
}



void consumeBuffer(){                //se sono presenti byte nel buffer li consumo 
  if (m>0 and n>0)  {
    Serial.write(arrayBuffer[down]);
    flashFile.write(arrayBuffer,down ); 
    down=(down+1)%sizeBuffer;
    n--;      
    m--;
    chiudiFile();
  }
}



void loop() {
  consumeBuffer();
  ascolta_seriale();
  crea_pack();
}
