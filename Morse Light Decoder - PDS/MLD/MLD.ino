#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ctype.h> // Biblioteca para manipulação de caracteres

// ------------------ LCD e Pinos --------------------------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int ledPin = 13;
const int sensorPin = A0;

// ------------------ Tempos Morse -------------------------------
const int dotDuration = 100;   
const int dashDuration = 300;  
const int symbolSpace = 100;   
const int letterSpace = 300;   
const int wordSpace = 900;     

// ------------------ Threshold LDR ------------------------------
const int sensorThreshold = 880;

// ------------------ Filtro IIR ---------------------------------
float b0 = 0.06745527388907;
float b1 = 0.13491054777814;
float b2 = 0.06745527388907;
float a1 = -1.14298050254;
float a2 = 0.4128015980962;

// Atrasos do filtro
float x1=0, x2=0;
float y1=0, y2=0;

// Função IIR
float iirFilter(float x0) {
  float y0 = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2;
  x2 = x1;  x1 = x0;
  y2 = y1;  y1 = y0;
  return y0;
}

// ------------------ Tabela Morse -------------------------------
const char* morseCode[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--.."
};

// ------------------ Mensagem e estados --------------------------
char message[100] = "";
int msgIndex = 0;
int morseIndex = 0;
const char* currentMorse = nullptr;

enum { TX_DOT_DASH, TX_SYMBOL_SPACE, TX_LETTER_SPACE, TX_WORD_SPACE } txState;
unsigned long stateStartTime = 0;
unsigned long currentDuration = 0;

// ------------------ LDR / Descodificação --------------------------
bool previousState = false;
unsigned long lastChangeTime = 0;
char symbolBuffer[8] = {0};
int symbolPos = 0;

// ------------------ Posicionamento no LCD ------------------------
int cursorPos = 0; // de 0 a 31 (0..15 = linha 0, 16..31 = linha 1)

// ------------------ Flag de transmissão concluída ----------------
bool transmissionComplete = false;

// ------------------ Funções LCD ----------------------------------

void clearLCDLine(int line) {
  lcd.setCursor(0, line);
  lcd.print("                ");
  lcd.setCursor(0, line);
}

// Apaga todo o LCD
void clearLCDAll() {
  lcd.clear();
  cursorPos = 0;  
}

// Função que imprime um char no LCD, usando cursorPos
//  - Se cursorPos < 16 => linha 0
//  - Se cursorPos >=16 => linha 1 (até 31)
//  - Se passar de 31, continua sobrescrevendo na linha 1
void printDecodedChar(char c) {
  int line = cursorPos / 16;     // 0 ou 1
  if (line > 1) line = 1;        // se passar de 31, fica na linha 1
  int col = cursorPos % 16;      // 0..15
  lcd.setCursor(col, line);
  lcd.print(c);
  cursorPos++;
}

void updateLCD(const char* text, int col, int row) {
  lcd.setCursor(col, row);
  lcd.print(text);
}

// ------------------ Descodificação Morse --------------------------
void decodeSymbol() {
  if (symbolPos == 0) return;  // Se não há símbolo, sai

  symbolBuffer[symbolPos] = '\0';
  for (int i=0; i<26; i++){
    if (strcmp(morseCode[i], symbolBuffer)==0){
      printDecodedChar((char)('A'+i));
      return;
    }
  }
  // Se não reconheceu, imprime '?'
  printDecodedChar('?');
}

// Prepara a próxima letra
void prepareNextLetter() {
  char currentChar = message[msgIndex];

  // Ignora qualquer caracter que não seja letra ou espaço
  if (!isalpha(currentChar) && currentChar != ' ') {
    msgIndex++;
    if (message[msgIndex] == '\0') return; // Chegou no fim
    prepareNextLetter();
    return;
  }

  // Se for espaço
  if (currentChar == ' ') {
    txState = TX_WORD_SPACE;
    stateStartTime = millis();
  } else {
    currentChar = toupper(currentChar);
    currentMorse = morseCode[currentChar - 'A']; 
    morseIndex = 0;
  }
}

// Inicia ponto ou traço
void startNextSymbol(bool isDot) {
  digitalWrite(ledPin, HIGH);
  currentDuration = isDot ? dotDuration : dashDuration;
  stateStartTime = millis();
  txState = TX_DOT_DASH;
}

// Espaço entre símbolos / letras / palavras
void startSpace(unsigned long duration, int newState) {
  digitalWrite(ledPin, LOW);
  currentDuration = duration;
  stateStartTime = millis();
  txState = newState;
}

// ------------------ SETUP ----------------------------------------
void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inicio");

  Serial.begin(9600);
  delay(2000);

  clearLCDAll();
  updateLCD("Aguarde mensagem", 0, 0);
  Serial.println("Insira mensagem");
}

// ------------------ LOOP -----------------------------------------
void loop() {
  // 1) Ler mensagem do Serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();

    if (input.length() > 0 && input.length() < 100) {
      input.toCharArray(message, sizeof(message));
      msgIndex = 0;
      symbolPos = 0;
      cursorPos = 0; // Reinicia posicao do LCD
      clearLCDAll(); // Apaga LCD
      lcd.print("Msg Atualizada");
      Serial.println("Mensagem Atualizada:");
      Serial.println(message);

      delay(2000);
      clearLCDAll();

      // Reinicia transmissão para uma só vez
      transmissionComplete = false;

      // Prepara primeira letra
      prepareNextLetter();
      if (message[msgIndex] != '\0') {
        // Se não está vazia, inicia Morse
        startNextSymbol(currentMorse[0] == '.');
      }
    } else {
      Serial.println("mensagem nao apropriada (0 < len < 100)");
    }
  }

  // Se mensagem vazia, sai
  if (message[0] == '\0') return;

  // Se já terminou de transmitir, LED OFF e aguarda nova mensagem
  if (transmissionComplete) {
    digitalWrite(ledPin, LOW);
    return;
  }

  unsigned long now = millis();

  // 2) Ler e filtrar LDR
  float raw = analogRead(sensorPin);
  float filtered = iirFilter(raw);

  // (Opcional) plotar no Serial Plotter:
  Serial.print(raw);
  Serial.print(" ");
  Serial.println(filtered);

  bool ledState = (filtered > sensorThreshold);

  if (ledState != previousState) {
    unsigned long duration = now - lastChangeTime;
    lastChangeTime = now;

    if (!ledState) {  // LED apagou
      if (duration < symbolSpace * 2 && symbolPos < (int)sizeof(symbolBuffer) - 1) {
        symbolBuffer[symbolPos++] = '.';
      } else if (symbolPos < (int)sizeof(symbolBuffer) - 1) {
        symbolBuffer[symbolPos++] = '-';
      }
    } 
    else if (duration > wordSpace) { 
      // Espaço entre palavras
      if (symbolPos > 0) {
        symbolBuffer[symbolPos] = '\0';
        decodeSymbol();
        symbolPos = 0;
      }
      printDecodedChar(' '); // imprime espaço no LCD
    } 
    else if (duration > letterSpace) {
      // Espaço entre letras
      if (symbolPos > 0) {
        symbolBuffer[symbolPos] = '\0';
        decodeSymbol();
        symbolPos = 0;
      }
    }
    previousState = ledState;
  }

  // Estados de transmissão (só se não concluímos ainda)
  if ((now - stateStartTime) >= currentDuration) {
    switch (txState) {

      case TX_DOT_DASH:
        startSpace(symbolSpace, TX_SYMBOL_SPACE);
        break;

      case TX_SYMBOL_SPACE:
        morseIndex++;
        if (currentMorse[morseIndex] == '\0') {
          startSpace(letterSpace, TX_LETTER_SPACE);
        } else {
          startNextSymbol(currentMorse[morseIndex] == '.');
        }
        break;

      case TX_LETTER_SPACE:
        msgIndex++;
        // ---> Verifica se chegamos no fim da mensagem
        if (message[msgIndex] == '\0') {
          // Força descodificar último símbolo pendente, se existir
          if (symbolPos > 0) {
            symbolBuffer[symbolPos] = '\0';
            decodeSymbol();
            symbolPos = 0;
          }
          // Transmissão concluída
          transmissionComplete = true;
          digitalWrite(ledPin, LOW);
          break;
        }
        prepareNextLetter();
        if (txState != TX_WORD_SPACE) {
          startNextSymbol(currentMorse[0] == '.');
        }
        break;

      case TX_WORD_SPACE:
        if ((now - stateStartTime) >= wordSpace) {
          msgIndex++;
          // Se acabou
          if (message[msgIndex] == '\0') {
            // Também força descodificar se tiver símbolo
            if (symbolPos > 0) {
              symbolBuffer[symbolPos] = '\0';
              decodeSymbol();
              symbolPos = 0;
            }
            transmissionComplete = true;
            digitalWrite(ledPin, LOW);
            break;
          }
          prepareNextLetter();
          startNextSymbol(currentMorse[0] == '.');
        }
        break;
    }
  }

  delay(5);
}
