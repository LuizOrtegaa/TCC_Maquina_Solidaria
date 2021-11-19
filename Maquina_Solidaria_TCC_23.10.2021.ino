/* ============================================================================

        SENAI "Antônio Ermírio de Morais"
        TCC Curso Técnico 4 Termo
        Tema: Máquina Solidária
        Versão 1.0
        Data: 10/2021
                                                    Integrantes do Grupo: 
                                                                Leonardo
                                                                Luana
                                                                Luiz 
                                                                Mayara
                                                                Nathan
                                                                Sabrina       
  
      Autor: Luiz Ortega
      email: Luiz.n.a@hotmail.com
  Instagram: @microcontrolando

============================================================================ */

/* ========================================================================= */
/* --- Bibliotecas  --- */

#include <Wire.h>                                               /*Comunicação I2C*/
#include <LiquidCrystal_I2C.h>                                  /*Comunicação do display através da comunicação I2C*/
#include <SPI.h>                                                /*Comunicação do RFID pelo protocolo SPI*/
#include <MFRC522.h>                                            /*Biblioteca que atua na biblioteca SPI*/
#include <Servo.h>                                              /*Biblioteca dos Servos*/
#include "ThingSpeak.h"                                         /*Comunicação com o THINGSPEAK*/
#include <ESP8266WiFi.h>                                        /*Conectividade WIFI*/

/* --- Parâmetros  --- */

#define SS_PIN  D3                                              /*Pino SDA RFID{houve alteração do pin D8 pelo D3*/
#define RST_PIN D0                                              /*Pino RST RFID*/
const char* ssid = "ortega";                                    /*Passa nome da rede*/
const char* password = "05/11/94";                              /*Passa senha da rede*/

  MFRC522::MIFARE_Key key;                                      /*Objeto "chave" utilizado para autentificação*/      
  LiquidCrystal_I2C lcd(0x27,16,2);                             /*Passa parametros do display (endereço, número de colunas e linhas (16x2))*/
  MFRC522 rfid(SS_PIN, RST_PIN);                                /*Passa parametros para função*/
  Servo servo_1;                                                /*Instancia Servo_1*/
  Servo servo_2;                                                /*Instancia Servo_2*/
  Servo servo_3;                                                /*Instancia Servo_3*/
  Servo servo_4;                                                /*Instancia Servo_4*/
/* ========================================================================= */

/* ========================================================================= */
/* --- Macros --- */
#define delay_Servo      5000                                   /*Tempo Servo on*/
#define pisca            500                                    /*Valor do delay do pisca backlight LCD no setup*/
#define led_pin          10                                     /*Pino do LED*/
#define bip              9                                      /*Pino do bip*/
#define timeOnBip        400                                    /*Time On Buzzer*/
#define timeOffBip       2100                                   /*Time Off Buzzer*/
#define postandoInterval 30000                                  /*Intervalo para realizar leitura e escrita no THINGSPEAK*/
#define valor_item1      1                                      /*Custo do item 1*/
#define valor_item2      2                                      /*Custo do item 2*/                      
#define valor_item3      3                                      /*Custo do item 3*/
#define valor_item4      4                                      /*Custo do item 4*/


/* ========================================================================= */
/* --- Variáveis Globais --- */
int     saldo  =  0,                                            /*Variável que recebe o saldo das TAG's*/
        card_1 = 100,                                           /*Saldo Card 1*/
        card_2 = 10,                                            /*Saldo Card 2*/  
item_escolhido =  0;                                            /*Variável que recebe Qual item foi escolhido*/ 
       
bool    acesso =  0,                                            /*Variável de controle de acesso*/
     _continue =  1;                                            /*Variável de controle que limita o funcionamento em caso de falta de saldo*/
                                                 
byte    item_1 =  1,                                            /*Variável que guarda o valor do item 1*/
        item_2 =  2,                                            /*Variável que guarda o valor do item 2*/
        item_3 =  3,                                            /*Variável que guarda o valor do item 3*/
        item_4 =  4;                                            /*Variável que guarda o valor do item 4*/

float   und_item_1 = valor_item1,                               /*Váriavel para controle da quantidade to item 1*/
        und_item_2 = valor_item2,                               /*Váriavel para controle da quantidade to item 2*/
        und_item_3 = valor_item3,                               /*Váriavel para controle da quantidade to item 3*/
        und_item_4 = valor_item4;                               /*Váriavel para controle da quantidade to item 4*/
      
unsigned long tempoAnterior =  0;                               /*Variável que armazena o tempo Anterior para soar o buzzer no processo (Retire o item)*/                
unsigned long channel = 1446032;                                /*Número de canal do THINGSPEAK*/

String      apiKey = "POU7C1109BWPQGE2";                        /*API Write do THINGSPEAK*/
const char* server = "api.thingspeak.com";                      /*Endereço server*/              
int         estado_1;                                           /*Variável que recebe resultado da leitura do field 5*/                                    
WiFiClient  client;                                             /*chama função WiFiclient*/
unsigned long lastConnectionTime = 0;                           /*Tempo da última conexão*/
unsigned long lastUpdateTime = 0;                               /*Tempo do último update*/

/* ========================================================================= */
/* --- Protótipo das funções --- */

    void leitura_RFID     ( );                                  /*Função responsável por ler as TAG's*/
    void imprime_LCD      ( );                                  /*Função responsável por Imprimir no LCD*/ 
    void le_botao         ( );                                  /*Função responsável por ler os botões*/ 
    void item_selecionado ( );                                  /*Função responsável por ligar o motor selecionado,ler o micro e finalizar processo*/
    void liga_servo       ( );                                  /*Função responsável por ligar os servos*/
    void alarme           ( );                                  /*Função responsável soar bip*/  
    void app              ( );                                  /*Função que recebe e envia dados para o o THINGSPEAK*/
    
/* ========================================================================= */
/* --- Função Principal --- */

    void setup( ){                                              /*Inicio da Função Setup*/
      
  pinMode(A0, INPUT);                                           /*Declara A0 como Entrada*/
  pinMode(bip, OUTPUT);                                         /*Declara bip como saída*/ 
  digitalWrite(bip, 0);                                         /*Bip nível baixo*/
  pinMode(led_pin, OUTPUT);                                     /*Declara led_pin como Saída*/
  digitalWrite(led_pin, 0);                                     /*led_pin nível baixo*/
  /*Serial.begin(115200);                                       -->Inicia Monitor Serial (utilizado para testes)*/
  lcd.init( );                                                  /*Inicia LCD*/
  lcd.backlight( );                                             /*Inicia Backlight*/
  SPI.begin( );                                                 /*Inicia SPI*/
  rfid.PCD_Init( );                                             /*Inicia RFID*/
  lcd.clear( );                                                 /*Limpa LCD*/
  lcd.setCursor(0,0);                                           /*Posiciona Cursor*/
  lcd.print("*****Maquina****");                                /*Exibe mensagem no LCD*/
  lcd.setCursor(0,1);                                           /*Posiciona Cursor*/
  lcd.print("****Solidaria***");                                /*Exibe mensagem no LCD*/
  delay(pisca);                                                 /*Delay de pisca*/
  lcd.setBacklight(LOW);                                        /*Backlight OFF*/
  delay(pisca);                                                 /*Delay de pisca*/
  lcd.setBacklight(HIGH);                                       /*Backlight ON*/
  delay(pisca);                                                 /*Delay de pisca*/              
  lcd.setBacklight(LOW);                                        /*Backlight OFF*/
  delay(pisca);                                                 /*Delay de pisca*/
  lcd.setBacklight(HIGH);                                       /*Backlight ON*/
  delay(pisca);                                                 /*Delay de pisca*/
  lcd.clear();                                                  /*Limpa LCD*/  
  /*Serial.println();                                           -->Pula linha*/
  /*Serial.println("Maquina Solidaria:");                       -->Exibe mensagem no Monitor Serial*/
  /*Serial.println("Aproxime seu cartao");                      -->Exibe mensagem no Monitor Serial*/
  servo_1.attach ( 2);                                          /*Instancia objeto Servo_1 pin  2*/
  servo_2.attach (15);                                          /*Instancia objeto Servo_2 pin 15*/
  servo_3.attach ( 3);                                          /*Instancia objeto Servo_3 pin  3*/
  servo_4.attach ( 1);                                          /*Instancia objeto Servo_4 pin  1*/
  delay(1000);                                                  /*Delay*/

  WiFi.begin(ssid, password);                                   /*Inicia WIFI(passa parametros)*/
  lcd.clear( );                                                 /*Limpa LCD*/
  lcd.setCursor(0,0);                                           /*Posiciona Cursor*/
  lcd.print("Conectando WiFi");                                 /*Exibe mensagem no LCD*/
  lcd.setCursor(0,1);                                           /*Posiciona Cursor*/
  
while(WiFi.status( ) != WL_CONNECTED && analogRead(A0) < 1000){ /*loop que espera conectar na rede*/  
        lcd.print(".");                                         /*Exibe mensagem no LCD*/
        delay(500);                                             /*delay*/
}/*end while*/

    lcd.clear( );                                               /*Limpa LCD*/  
    if(WiFi.status( ) == WL_CONNECTED)                          /*Testa se houve conexão*/
    {
      lcd.setCursor(1,0);                                       /*Posiciona Cursor*/
      lcd.print("WiFi CONECTADO ");                             /*Exibe mensagem no LCD*/
    }/*end if*/
    else                                                        /*Se não houve conexão*/
    {
      lcd.setCursor(1,0);                                       /*Posiciona Cursor*/
      lcd.print("NAO CONECTADO");                               /*Exibe mensagem no LCD*/
    }/*end else*/
  delay(5000);                                                  /*delay*/
  lcd.clear( );                                                 /*limpa LCD*/
  ThingSpeak.begin(client);                                     /*Inicia client ThingSpeak*/
  
    }/*end setup*/

/* ========================================================================= */

    void loop( ){                                               /*Início da Função loop*/
       
    if (_continue==1)                                           /*Se _continue verdadeiro chama funções dentro do if*/
    {       
      app         ( );                                          /*Função que recebe e envia dados para o o THINGSPEAK*/
      imprime_LCD ( );                                          /*Função responsável por Imprimir no LCD*/
      leitura_RFID( );                                          /*Função responsável por ler as TAG's*/

    }/*end if continue*/
      else
      {                                                         /*Se _continue falso Reseta variáveis, limpa LCD e reinicia loop*/
        item_escolhido = 0;                                     /*Zera item_escolhido*/
        acesso = 0;                                             /*Zera acesso*/
        saldo  = 0;                                             /*Zera saldo*/
        lcd.clear( );                                           /*Limpa LCD*/ 
        _continue=1;                                            /*Continue passa a ser verdadeiro e reinicia loop*/                                              
      }/*end else*/
    }/*end void loop*/

/* ========================================================================= */
/* --- Desenvolvimento das funções --- */
/*---------------------------------------------------------------------------*/
    void app ( )
    {
      
      if (millis ( ) - lastUpdateTime >= postandoInterval)      /*Testa se a diferença do tempo atual com o ultimo update é maior igual do que o tempo de intervalo*/
      {     
        lastUpdateTime = millis ( );                            /*atualiza último update*/
        estado_1 = ThingSpeak.readFloatField(channel, 5);       /*faz leitura do campo 5 e guarda em estado_1*/
          if(estado_1)                                          /*Se verdadeiro estado_1*/
          {
            lcd.clear( );                                       /*limpa LCD*/
            lcd.setCursor(5,0);                                 /*Posiciona cursor*/  
            lcd.print("Maquina");                               /*Imprime no LCD*/
            lcd.setCursor(4,1);                                 /*Posiciona cursor*/  
            lcd.print("Completa");                              /*Imprime no LCD*/
            delay(pisca);                                       /*Delay de pisca*/
            lcd.setBacklight(LOW);                              /*Backlight OFF*/
            delay(pisca);                                       /*Delay de pisca*/
            lcd.setBacklight(HIGH);                             /*Backlight ON*/
            delay(pisca);                                       /*Delay de pisca*/              
            lcd.setBacklight(LOW);                              /*Backlight OFF*/
            delay(pisca);                                       /*Delay de pisca*/
            lcd.setBacklight(HIGH);                             /*Backlight ON*/
            delay(pisca);                                       /*Delay de pisca*/
            und_item_1 = valor_item1;                           /*Reseta quantidade do und_item_1*/
            und_item_2 = valor_item2;                           /*Reseta quantidade do und_item_2*/
            und_item_3 = valor_item3;                           /*Reseta quantidade do und_item_3*/
            und_item_4 = valor_item4;                           /*Reseta quantidade do und_item_4*/
            delay(60000);                                       /*delay de 1 minuto*/
            lcd.clear( );                                       /*limpa LCD*/
          }       
 
          if (client.connect(server,80)) {                      /*Inicia um client TCP para o envio dos dados*/
            String postStr = apiKey;                            /*Cria string postStr e passa apiKey*/
            postStr +="&field1=";                               /*Concatena field1*/
            postStr += String(und_item_1);                      /*Concatena und_item_1*/
            postStr +="&field2=";                               /*Concatena field2*/
            postStr += String(und_item_2);                      /*Concatena und_item_2*/
            postStr +="&field3=";                               /*Concatena field3*/
            postStr += String(und_item_3);                      /*Concatena und_item_3*/
            postStr +="&field4=";                               /*Concatena field4*/
            postStr += String(und_item_4);                      /*Concatena und_item_4*/
            postStr += "\r\n\r\n";                              /*Caractere nulo, encerra strg*/
 
     client.print("POST /update HTTP/1.1\n");                   /*Faz update da string, protocolo HTTP*/
     client.print("Host: api.thingspeak.com\n");                
     client.print("Connection: close\n");                       
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");         
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
  }
  client.stop();                                                /*Encerra client*/
  }
}

/*---------------------------------------------------------------------------*/
    void leitura_RFID( )                                        /*Função responsável por ler as TAG's*/
    {  
    if (! rfid.PICC_IsNewCardPresent( ) )    return;            /*Se não houver leitura de novo cartão, reinicia loop*/       
    if (! rfid.PICC_ReadCardSerial  ( ) )    return;            /*Se não houver leitura de cartão na //Serial, reinicia loop*/  

String strID = "";                                              /*Cria variável String nula*/
    for (byte i = 0; i < 4; i++)                                /*Concatena Bytes e monta a String da TAG em HEXA*/
    {                        
      strID +=  (rfid.uid.uidByte[i] < 0x10 ? "0" : "")+          
                      String (rfid.uid.uidByte[i], HEX)+          
                                      (i!=3 ? ":" : "");          
    strID.toUpperCase( );                                       /*Converte string para sua versão em Letras Maiúsculas*/
    }/*end for*/
  
    /*Serial.println("==========================");             -->Imprime no monitor Serial*/    
    /*Serial.println("      TAG DETECTADA");                    -->Imprime no monitor Serial*/    
    /*Serial.println("==========================");             -->Imprime no monitor Serial*/    
 
    if (strID == "2A:C0:36:59")                                 /*Compara TAG lida com TAG 1*/
    {     
      saldo = card_1;                                           /*Atualiza variável saldo para valor de card_1*/
      acesso=1;                                                 /*Concede acesso*/
    }/*end if*/
  
      else if (strID == "F4:F0:10:2A")                          /*Compara TAG lida com TAG 2*/
      {
        saldo  = card_2;                                        /*Atualiza variável saldo para valor de card_2*/
        acesso = 1;                                             /*Concede acesso*/
      }/*end else if*/
        else                                                    /*Se a TAG lida não for igual as TAG'S Cadastradas*/
        { 
          lcd.clear( );                                         /*limpa LCD*/                     
          /*Serial.println("Cartão INVÁLIDO     ");             -->Imprime no monitor Serial*/
          /*Serial.println( );                                  -->Pula linha*/
          lcd.setCursor(3,0);                                   /*Posiciona cursor*/  
          lcd.print("Cartao nao");                              /*Imprime no LCD*/
          lcd.setCursor(3,1);                                   /*Posiciona cursor*/  
          lcd.print("CADASTRADO");                              /*Imprime no LCD*/
          delay(5000);                                          /*delay*/
          lcd.clear( );                                         /*limpa LCD*/
         }/*end else*/
     }/*end leitura_RFID*/
/*---------------------------------------------------------------------------*/
    void imprime_LCD( )                                         /*Função responsável por Imprimir no LCD*/
    {
      if(acesso)                                                /*Se Acesso for concedido*/
      {
        digitalWrite(led_pin, HIGH);                            /*led_pin ON*/
        lcd.clear( );                                           /*limpa LCD*/
        lcd.setCursor(4,0);                                     /*Posiciona cursor*/  
        lcd.print("Creditos:");                                 /*Imprime no LCD*/
        lcd.setCursor(7,1);                                     /*Posiciona cursor*/  
        lcd.print(saldo);                                       /*Imprime no LCD*/
        delay(5000);                                            /*delay*/
        lcd.clear( );                                           /*limpa LCD*/
        lcd.setCursor(0,0);                                     /*Posiciona cursor*/  
        lcd.print("Escolha o item:");                           /*Imprime no LCD*/
        le_botao    ( );                                        /*Chama função de leitura dos botoes*/
        acesso = 0;                                             /*reseta acesso*/
        lcd.clear( );                                           /*limpa LCD*/  
      }/*end while*/
        else                                                    /*Se não foi concedido acesso*/
        {         
          digitalWrite(led_pin, LOW);                           /*led_pin OFF*/
          lcd.setCursor(2,0);                                   /*Posiciona cursor*/  
          lcd.print("Aproxime seu");                            /*Imprime no LCD*/
          lcd.setCursor(0,1);                                   /*Posiciona cursor*/  
          lcd.print("Cartao solidario");                        /*Imprime no LCD*/
        }/*end else*/  
     }/*end imprime_LCD*/
/*---------------------------------------------------------------------------*/
    void le_botao    ( )                                        /*Função que faz leitura dos botões e Micro*/
    {                                                           
byte  n=0;                                                      /*Variável que se altera ao selecionar item. Usara para imprimir "item_n" no LCD*/
bool  leitura_botao = 1;                                        /*Variável de condição para execurar while de leitura de botão*/
   
      while(leitura_botao)                                      /*Faz leitura de botão enquanto leitura_botao for verdadeiro*/
      {   
int  le_analog = analogRead(A0);                                /*Armazena leitura de A0 na variável le_analog*/
      delay(50);
         
      if(le_analog >= 1000  &&   le_analog <= 1024)             /*Faixa de leitura de A0 para que item seja item_1*/
      { 
        if(saldo >= item_1 && und_item_1 > 0)                   /*Se item igual a 1 e und_item_1 for maior que zero*/
        {
          /*Serial.println("Item 1");                           -->Imprime na Serial*/
          n=1;                                                  /*Atribui n = 1*/
          item_escolhido = item_1;                              /*Atribui item_1 ao item_escolhido*/
          leitura_botao=0;                                      /*Reseta leitura_botao*/
          und_item_1 = und_item_1 - 1;                          /*Debita und_item_1*/   
         }/*end if*/
            else if (saldo < item_1)                            /*Se o saldo for menor que o item_1*/
            {
              lcd.clear( );                                     /*Limpa LCD*/
              lcd.setCursor(4,0);                               /*Posiciona cursor*/  
              lcd.print("Credito");                             /*Imprime no LCD*/
              lcd.setCursor(2,1);                               /*Posiciona cursor*/ 
              lcd.print("Insuficiente");                        /*Imprime no LCD*/
              delay(2000);                                      /*Delay*/
              lcd.clear( );                                     /*Limpa LCD*/
              lcd.setCursor(4,0);                               /*Posiciona cursor*/  
              lcd.print("Procure");                             /*Imprime no LCD*/
              lcd.setCursor(2,1);                               /*Posiciona cursor*/ 
              lcd.print("Assistencia");                         /*Imprime no LCD*/
              delay(5000);                                      /*Delay*/
              _continue = 0;                                    /*Se saldo insuficiente, encerra if do loop e reinicia*/
              leitura_botao = 0;                                /*Reseta leitura_botao*/
              item_escolhido = 0;                               /*Reseta item_escolhido*/
             }/*end else if*/
                else if (und_item_1 == 0)                       /*Testa se acabou und_item_1*/
                {
                  lcd.clear( );                                 /*Limpa LCD*/
                  lcd.setCursor(6,0);                           /*Posiciona cursor*/  
                  lcd.print("Item 1");                          /*Imprime no LCD*/
                  lcd.setCursor(2,1);                           /*Posiciona cursor*/ 
                  lcd.print("Insuficiente");                    /*Imprime no LCD*/
                  delay(5000);                                  /*Delay*/
                  lcd.clear( );                                 /*Limpa LCD*/
                  _continue = 0;                                /*Se saldo insuficiente, encerra if do loop e reinicia*/
                  leitura_botao = 0;                            /*Reseta leitura_botao*/
                  item_escolhido = 0;                           /*Reseta item_escolhido*/
                 }/*end else if*/
      }/*end if*/
      
      else if(le_analog >= 960  &&   le_analog < 1000)          /*Faixa de leitura de A0 para que item seja item_2*/
      {
        if(saldo >= item_2 && und_item_2 > 0)                   /*Se item igual a 2, testa se saldo é maior que item_2*/
        {
          /*Serial.println("Item 2");                           -->Imprime na Serial*/
          n=2;                                                  /*Atribui n = 2*/
          item_escolhido = item_2;                              /*Atribui item_2 ao item_escolhido*/
          leitura_botao = 0;                                    /*Encerra leitura_botao*/  
          und_item_2 = und_item_2 - 1;                          /*Decrementa und_item2*/
        }/*end if*/        
            else if (saldo < item_2)                            /*Testa se saldo é menor que o item_2*/
            {    
                lcd.clear( );                                   /*Limpa LCD*/
                lcd.setCursor(4,0);                             /*Posiciona cursor*/  
                lcd.print("Credito");                           /*Imprime no LCD*/
                lcd.setCursor(2,1);                             /*Posiciona cursor*/                                  
                lcd.print("Insuficiente");                      /*Imprime no LCD*/
                delay(2000);                                    /*Delay*/
                lcd.clear( );                                   /*Limpa LCD*/
                lcd.setCursor(4,0);                             /*Posiciona cursor*/  
                lcd.print("Procure");                           /*Imprime no LCD*/
                lcd.setCursor(2,1);                             /*Posiciona cursor*/               
                lcd.print("Assistencia");                       /*Imprime no LCD*/
                delay(5000);                                    /*Delay*/  
                _continue = 0;                                  /*Encerra _continue (if principal do void loop)*/ 
                leitura_botao = 0;                              /*Encerra leitura_botao*/
                item_escolhido = 0;                             /*Encerra item_escolhido*/
            }/*end else if*/
             
                else if (und_item_2 == 0)                       /*Testa se acabou und_item_2*/
                {
                  lcd.clear( );                                 /*Limpa LCD*/
                  lcd.setCursor(6,0);                           /*Posiciona cursor*/  
                  lcd.print("Item 2");                          /*Imprime no LCD*/
                  lcd.setCursor(2,1);                           /*Posiciona cursor*/ 
                  lcd.print("Insuficiente");                    /*Imprime no LCD*/
                  delay(5000);                                  /*Delay*/
                  lcd.clear( );                                 /*Limpa LCD*/
                  _continue = 0;                                /*Encerra _continue (if principal do void loop)*/
                  leitura_botao = 0;                            /*Encerra leitura_botao*/
                  item_escolhido = 0;                           /*Encerra item_escolhido*/
                }/*end else if*/
      }/*end else if*/
    
      else if(le_analog >= 870  &&   le_analog < 960)           /*Faixa de leitura de A0 para que item seja item_3*/
      {
        if(saldo>=item_3 && und_item_3 > 0)                     /*Se item igual a 2 e und_item_2 for maior que zero*/                     
        {
          /*Serial.println("Item 3");                           -->imprime na serial*/ 
          n=3;                                                  /*Atribui n = 3*/
          item_escolhido = item_3;                              /*Atribui item_3 ao item_escolhido*/                              
          leitura_botao=0;                                      /*Encerra leitura_botao*/ 
          und_item_3 = und_item_3 - 1;                          /*Decrementa und_item3*/
        }/*end if*/
            else if (saldo < item_3)                            /*Testa se saldo é menor que o item_3*/
            {
              lcd.clear( );                                     /*Limpa LCD*/ 
              lcd.setCursor(4,0);                               /*Posiciona cursor*/  
              lcd.print("Credito");                             /*Imprime no LCD*/
              lcd.setCursor(2,1);                               /*Posiciona cursor*/  
              lcd.print("Insuficiente");                        /*Imprime no LCD*/
              delay(2000);                                      /*Delay*/ 
              lcd.clear( );                                     /*Limpa LCD*/ 
              lcd.setCursor(4,0);                               /*Posiciona cursor*/  
              lcd.print("Procure");                             /*Imprime no LCD*/
              lcd.setCursor(2,1);                               /*Posiciona cursor*/ 
              lcd.print("Assistencia");                         /*Imprime no LCD*/
              delay(5000);                                      /*Delay*/    
              _continue = 0;                                    /*Encerra _continue (if principal do void loop)*/ 
              leitura_botao = 0;                                /*Encerra leitura_botao*/
              item_escolhido = 0;                               /*Encerra item_escolhido*/
            }
                else if (und_item_3 == 0)                       /*Testa se acabou und_item_2*/
                {
                  lcd.clear( );                                 /*Limpa LCD*/
                  lcd.setCursor(6,0);                           /*Posiciona cursor*/  
                  lcd.print("Item 3");                          /*Imprime no LCD*/
                  lcd.setCursor(2,1);                           /*Posiciona cursor*/ 
                  lcd.print("Insuficiente");                    /*Imprime no LCD*/
                  delay(5000);                                  /*Delay*/
                  lcd.clear( );                                 /*Limpa LCD*/
                  _continue = 0;                                /*Encerra _continue (if principal do void loop)*/
                  leitura_botao = 0;                            /*Reseta leitura_botao*/
                  item_escolhido = 0;                           /*Reseta item_escolhido*/
                }/*end else if*/
      }/*end else if*/
      
      else if(le_analog >= 650  &&   le_analog < 870)           /*Faixa de leitura de A0 para que item seja item4_*/
      {
        if(saldo >= item_4 && und_item_4 > 0)                   /*Se item igual a 2 e und_item_2 for maior que zero*/
        {
          /*Serial.println("Item 4");                           -->Imprime na Serial*/ 
          n=4;                                                  /*Atribui n = 4*/    
          item_escolhido = item_4;                              /*Atribui item_4 ao item_escolhido*/
          leitura_botao=0;                                      /*Encerra leitura_botao*/
          und_item_4 = und_item_4 - 1;                          /*Decrementa und_item4*/
        }/*end if*/
            else if (saldo < item_4)                            /*Testa se saldo é menor que o item_4*/
            {
              lcd.clear( );                                     /*Limpa LCD*/ 
              lcd.setCursor(4,0);                               /*Posiciona cursor*/  
              lcd.print("Credito");                             /*Imprime no LCD*/
              lcd.setCursor(2,1);                               /*Posiciona cursor*/ 
              lcd.print("Insuficiente");                        /*Imprime no LCD*/
              delay(2000);                                      /*Delay*/ 
              lcd.clear( );                                     /*Limpa LCD*/ 
              lcd.setCursor(4,0);                               /*Posiciona cursor*/  
              lcd.print("Procure");                             /*Imprime no LCD*/
              lcd.setCursor(2,1);                               /*Posiciona cursor*/  
              lcd.print("Assistencia");                         /*Imprime no LCD*/
              delay(5000);                                      /*Delay*/    
              _continue = 0;                                    /*Encerra _continue (if principal do void loop)*/
              leitura_botao  = 0;                               /*Encerra leitura_botao*/
              item_escolhido = 0;                               /*Encerra item_escolhido*/                               
            }/*end else if*/         
                else if (und_item_4 == 0)                       /*Testa se acabou und_item_4*/
                  {
                    lcd.clear( );                               /*Limpa LCD*/                                         
                    lcd.setCursor(6,0);                         /*Posiciona cursor*/  
                    lcd.print("Item 4");                        /*Imprime no LCD*/
                    lcd.setCursor(2,1);                         /*Posiciona cursor*/ 
                    lcd.print("Insuficiente");                  /*Imprime no LCD*/
                    delay(5000);                                /*Delay*/
                    lcd.clear( );                               /*Limpa LCD*/
                    _continue = 0;                              /*Se saldo insuficiente, encerra if do loop e reinicia*/
                    leitura_botao = 0;                          /*Reseta leitura_botao*/
                    item_escolhido = 0;                         /*Reseta item_escolhido*/
                  }
      }/*end else if*/   
      }/*end while*/

      if (item_escolhido != 0)                                  /*Testa se algum item foi escolhido*/ 
      {
        lcd.clear( );                                           /*Limpa LCD*/
        lcd.setCursor(5,0);                                     /*Posiciona cursor*/  
        lcd.print("Item ");                                     /*Imprime no LCD*/
        lcd.print(n);                                           /*Imprime no LCD*/
        lcd.setCursor(2,1);                                     /*Posiciona cursor*/
        lcd.print("Creditos: ");                                /*Imprime no LCD*/    
        lcd.print(item_escolhido);                              /*Imprime no LCD*/  
        delay(5000);                                            /*Delay*/
        
            if(saldo == card_1)                                 /*Se o saldo for igual ao card_1*/
            {
              card_1 = card_1 - item_escolhido;                 /*Decrementa item_escolhido saldo do card_1*/
              saldo  = card_1;                                  /*Atribuí card_1 a variável saldo*/
            }/*end if*/        
                else if(saldo == card_2)                        /*Se o saldo for igual ao card_2*/
                {
                  card_2 = card_2 - item_escolhido;             /*Decrementa item_escolhido saldo do card_2*/
                  saldo  = card_2;                              /*Atribuí card_1 a variável saldo*/        
                }/*end else if*/
        lcd.clear( );                                           /*Limpa LCD*/ 
        lcd.setCursor(2,0);                                     /*Posiciona cursor*/  
        lcd.print("Saldo Atual");                               /*Imprime no LCD*/
        lcd.setCursor(2,1);                                     /*Posiciona cursor*/  
        lcd.print(saldo);                                       /*Imprime no LCD*/
        lcd.print(" Creditos");                                 /*Imprime no LCD*/
        delay(5000);                                            /*Delay*/
        item_selecionado( );                                    /*Chama função item_selecionado*/
        }/*end if*/
    }/*end le_botao*/
/*---------------------------------------------------------------------------*/
    void item_selecionado( )                                    /*Função responsável por ligar o motor selecionado, ler o micro e finalizar processo*/
    {   
      liga_servo( );                                            /*Chama Função responsável por ligar os servos*/
      lcd.clear( );                                             /*Limpa LCD*/  
      lcd.setCursor(4,0);                                       /*Posiciona cursor*/  
      lcd.print("Retire o");                                    /*Imprime no LCD*/
      lcd.setCursor(6,1);                                       /*Posiciona cursor*/  
      lcd.print("Item");                                        /*Imprime no LCD*/
      delay(1000);                                              /*Delay*/
             
bool micro_acionado = 0;                                        /*Variável de controle do Micro*/

        while(!micro_acionado)                                  /*Se o micro for acionado*/
        {
int       le_analog = analogRead(A0);                           /*Variável que recebe leitura analógica A0*/
          delay(50);                                            /*Delay*/
            if(le_analog >= 500  &&   le_analog < 650)          /*Faixa de leitura de A0 para que Micro seja acionado*/
            {
            /*Serial.println("MICRO");                          -->Imprime na Serial*/                  
            micro_acionado = 1;                                 /*Atribuí condição verdadeira a micro_acionado e encerra while*/
            digitalWrite(bip, 0);                               /*Garante que o buzzer permaneça desligado*/
            lcd.setBacklight(HIGH);                             /*Garante que o backlight permaneça ligado*/
            
            }/*end else if*/
                else                                            /*Se Micro não for acionado*/
                {
                  alarme ( );                                   /*Chama função responsável soar bip*/
                }/*end else*/
          
        }/*end while*/
        lcd.clear( );                                           /*Limpa LCD*/
        lcd.setCursor(3,0);                                     /*Posiciona cursor*/  
        lcd.print("OBRIGADO!!!");                               /*Imprime no LCD*/
        delay(5000);                                            /*Delay*/
        item_escolhido = 0;                                     /*Encerra item_escolhido*/                                     
        acesso = 0;                                             /*Encerra acesso*/        
        lcd.clear( );                                           /*Limpa LCD*/

     }/*end item_selecionado( )*/
/*---------------------------------------------------------------------------*/
    void liga_servo( )                                          /*Função responsável por ligar os servos*/
    {
      if(item_escolhido == item_1)                              /*Se o item_escolhido for o item 1*/
      {
        servo_1.write(50);                                      /*Liga Servo 1*/
        delay(delay_Servo );                                            /*Delay*/
        servo_1.write(90);                                      /*Desliga Servo 1*/
        delay(delay_Servo );                                            /*Delay*/
      }/*end if*/ 

          if(item_escolhido == item_2)                          /*Se o item_escolhido for o item 2*/
          {
            servo_2.write(50);                                  /*Liga Servo 2*/
            delay(delay_Servo );                                        /*Delay*/
            servo_2.write(90);                                  /*Desliga Servo 2*/
            delay(delay_Servo );                                        /*Delay*/
          }/*end if*/

              if(item_escolhido == item_3)                      /*Se o item_escolhido for o item 3*/
              {
                servo_3.write(50);                              /*Liga Servo 3*/
                delay(delay_Servo );                                    /*Delay*/
                servo_3.write(90);                              /*Desliga Servo 3*/
                delay(delay_Servo );                                    /*Delay*/
              }/*end if*/
                  if(item_escolhido == item_4)
                  {
                    servo_4.write(50);                          /*Liga Servo 4*/
                    delay(delay_Servo );                                /*Delay*/
                    servo_4.write(90);                          /*Desliga Servo 4*/
                    delay(delay_Servo );                                /*Delay*/
                  }/*end if*/      
    }/*end liga_servo*/

    void alarme ( )                                             /*Função responsável soar bip*/ 
    {
unsigned long tempoAtual = millis( );                           /*Atribuí millis( ) a variável tempoAtual*/
bool buzzer = 1;                                                /*Variável de controle para do buzzer. Inicia em nível alto*/
      if(tempoAtual - tempoAnterior > timeOnBip)                /*Enquanto tempoAtual - tempoAnterior for menor que timeOnBip, buzzer ligado*/
      {
        buzzer = 0;                                             /*Quando a condição for verdadeira, buzzer fica em nível baixo*/
      }/*end if*/
          if(tempoAtual - tempoAnterior > timeOffBip)           /*Enquanto tempoAtual - tempoAnterior for menor que timeOffBip, buzzer desligado*/
          {
            buzzer = 1;                                         /*Quando a condição for verdadeira, buzzer fica em nível alto*/
            tempoAnterior = tempoAtual;                         /*Atribuí tempoAtual à tempoAnterior*/
          }/*end if*/
      digitalWrite (bip, buzzer);                               /*Alterna nível lógico de bip conforme valor de buzzer*/
      lcd.setBacklight (!buzzer);                               /*Alterna condição do backlight LCD conforme a negação de buzzer*/
    }/*end alarme*/


    
/* ============================================================================  
                                                           
        *******   ***      **     **     ***   *******
      ******   *****       *********      *****    *****
    ******  ********       *********       ******    *****
   ****   **********       *********       *********   *****
  ****  **************    ***********     ************   ****
 ****  *************************************************  ****
****  ***************************************************  ****
****  ****************************************************  ****
****  ****************************************************  ****
 ****  ***************************************************  ****
  ****  *******     ****  ***********  ****     *********  ****
   ****   *****      *      *******      *      ********  ****
    *****   ****             *****             ******   *****
      *****   **              ***              **    ******
       ******   *              *              *   *******
         *******                                *******
            ********                         *******
               *********************************
                    ***********************
                                                              
                                                              
============================================================================ */
                  /* --- Final do Programa --- */
