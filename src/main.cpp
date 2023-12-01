#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>
#include "semphr.h"
#define GREEN 6        
#define YELLOW 5
#define RED 4
#define buzzer 13
#define buttonA 2
#define buttonB 3

volatile bool pedestrian = false, priority = false;
const TickType_t _1000ms = pdMS_TO_TICKS(1000);
const TickType_t _500ms = pdMS_TO_TICKS(500);
const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);  // initialize the library with the numbers of the interface pins
TaskHandle_t red_handle, yellow_handle, green_handle,pedes_handle, prio_handle; 
SemaphoreHandle_t ledSemaphore, interruptPeSemaphore, interruptPrSemaphore;

void setup() {
  Serial.begin(9600); //for debugging
  lcd.begin(16, 2);
  pinMode(buzzer,OUTPUT);
  pinMode(buttonA,INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  ledSemaphore = xSemaphoreCreateBinary();
  interruptPeSemaphore = xSemaphoreCreateBinary(); //interrupt 1 for pedestrian crossing ONLY
  interruptPrSemaphore = xSemaphoreCreateBinary(); //interrupt 2 for pedestrian + priority crossing 
    xTaskCreate(redTaskController,      //task name
                "red light toggle",     //task description
                100,                    // stack depth set to 100 instead of 128 so that no overflow
                NULL,  // any parameters to pass
                3,                      // priority
                &red_handle                 // taskhandle for qms.
    );
    xTaskCreate(yellowTaskController,           //task name
                "yellow light toggle",       //task description
                100,                         // stack depth
                NULL,  // any parameters to pass
                1,                           // priority
                &yellow_handle                      // taskhandle for qms.
    );
    xTaskCreate(greenTaskController,           //task name
                "green light toggle",        //task description
                100,                         // stack depth
                NULL,  // any parameters to pass
                2,                           // priority
                &green_handle                         // taskhandle for qms.
    );
    xTaskCreate(PedesController,           //task name
                "for button press",        //task description
                100,                         // stack depth
                NULL,  // any parameters to pass
                3,                           // priority
                &pedes_handle                         // taskhandle for qms.
    );
    xTaskCreate(PrioController,           //task name
                "for button press",        //task description
                100,                         // stack depth
                NULL,  // any parameters to pass
                3,                           // priority
                &prio_handle                         // taskhandle for qms.
    );
  attachInterrupt(digitalPinToInterrupt(buttonA), buttonAPress, LOW); //attach interrupt to pin 2
  attachInterrupt(digitalPinToInterrupt(buttonB), buttonBPress, LOW); // attach interrupt to pin 3
  xSemaphoreGive(ledSemaphore);
  vTaskStartScheduler();
 
}

void redTaskController(void *pvParameters){
  while (1){
  vTaskPrioritySet(NULL,3);
  pinMode(RED,OUTPUT);
   xSemaphoreTake(ledSemaphore, portMAX_DELAY);
   if (pedestrian == true){
     for (int timer = 9; timer>0; timer--)
      {
        lcd.clear();                      
        lcd.setCursor(0,0);
        lcd.print("Pedestrian");
        lcd.setCursor(0,1);
        lcd.print("Timer: ");
        digitalWrite(RED, HIGH);
        lcd.setCursor(6,1);
        lcd.print(timer);
        if (timer < 9)
        {
          tone(buzzer, 800); 
          vTaskDelay(5);
          noTone(buzzer);
        }
        if (timer < 5)
        {
          tone(buzzer, 800);
          digitalWrite(RED,digitalRead(RED)^1);
          vTaskDelay(5);                        //to make buzzer sound twice when timer <5s left
          digitalWrite(RED,digitalRead(RED)^1);
          noTone(buzzer);
        }
        vTaskDelay(_1000ms);
      }
    
    }

  else if (priority == true)
    {
      for (int timer = 12; timer>0; timer--)
      {
        lcd.clear(); //needed here cause two digit number
        lcd.setCursor(0,0);
        lcd.print("PrioPedestrian");
        lcd.setCursor(0,1);
        lcd.print("Timer: ");
        digitalWrite(RED, HIGH);
        lcd.setCursor(8,1);
        lcd.print(timer);
        if (timer < 12)
        {
          tone(buzzer, 800); 
          vTaskDelay(5);
           noTone(buzzer);
        }
        if (timer < 5)
        {
          tone(buzzer, 800);
          digitalWrite(RED,digitalRead(RED)^1);
          vTaskDelay(5);                            //to make buzzer sound twice when timer <5s left
          digitalWrite(RED,digitalRead(RED)^1);
          noTone(buzzer);
        }
        vTaskDelay(_1000ms);
        
      }
    }
  else {
    for (int timer = 6; timer>0; timer--)
    {

        lcd.setCursor(0,0);
        lcd.print("Red Light");
        lcd.setCursor(0,1);
        lcd.print("Timer: ");
        digitalWrite(RED, HIGH);
        lcd.setCursor(6,1);
        lcd.print(timer);
        vTaskDelay(_1000ms);
    }
      }
  pedestrian = false;
  priority = false;  // reset the flags
  digitalWrite(RED,LOW);
  xSemaphoreGive(ledSemaphore);
  lcd.clear();
  vTaskPrioritySet(red_handle,2);
  vTaskPrioritySet(green_handle, 3);
  }
}



void yellowTaskController(void *pvParameters){
  while (1){
  vTaskPrioritySet(NULL,3);
  pinMode(YELLOW,OUTPUT);

  xSemaphoreTake(ledSemaphore, portMAX_DELAY);
  
  for (int timer = 2; timer>0; timer--)
  {
    lcd.setCursor(0,0);
    lcd.print("Yellow Light");
    lcd.setCursor(0,1);
    lcd.print("Timer: ");
    digitalWrite(YELLOW, HIGH);
    lcd.setCursor(6,1);
    lcd.print(timer);
    vTaskDelay(_1000ms);
  }
  digitalWrite(YELLOW,LOW);
  xSemaphoreGive(ledSemaphore);
  lcd.clear();
  vTaskPrioritySet(NULL,2);
  vTaskPrioritySet(green_handle, 3);

  }
}
  
void greenTaskController(void *pvParameters){
  while (1){
  vTaskPrioritySet(NULL,3);
  pinMode(GREEN,OUTPUT);
  Serial.print(pedestrian);
  Serial.print(",");
  Serial.println(priority);
  xSemaphoreTake(ledSemaphore, portMAX_DELAY);
  lcd.setCursor(0,0);
  lcd.print("Green Light");
  lcd.setCursor(0,1);
  lcd.print("Timer: ");
  for (int timer = 4; timer>0; timer--)
  {
    digitalWrite(GREEN, HIGH);
    lcd.setCursor(6,1);
    lcd.print(timer);
    vTaskDelay(_1000ms);
  }
  digitalWrite(GREEN,LOW);
  xSemaphoreGive(ledSemaphore);
  lcd.clear();
  vTaskPrioritySet(NULL,2);
  vTaskPrioritySet(green_handle, 3);

}
}

void PedesController(void *pvParameters){
  while (1)
  {
  if (xSemaphoreTake(interruptPeSemaphore, portMAX_DELAY) == pdPASS) {
  pedestrian = true;
  Serial.println("something happened"); //debugging
  }
}
}

void PrioController(void *pvParameters){
  while (1)
  {
    if (xSemaphoreTake(interruptPrSemaphore, portMAX_DELAY) == pdPASS) { //if semaphore is taken successfully
    priority = true;
    Serial.println("something extra happened"); //debugging
  }
  }
}

void buttonAPress(){              //interrupt handler for normal pedestrian press 
  xSemaphoreGiveFromISR(interruptPeSemaphore, NULL);
  }

void buttonBPress(){              //interrupt handle for priority press
  xSemaphoreGiveFromISR(interruptPrSemaphore, NULL);
}
  

void loop() {
  //nothing here
}