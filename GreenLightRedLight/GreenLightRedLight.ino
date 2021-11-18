/*
   Green Light Red Light Game

   Game Progress
   1. Game Start button
   2. Main music plays
   3. Game music plays
   4. Distance detection
   5. Gane Won/Lost

   Things to Do
   - adjust button sensitivity (debounce time)
   - adjust distance difference (human or object)
   - add win distance feature
*/

// button pins and variables
#include <ezButton.h>
ezButton button(12);
int btnState;

// servo pin and variables
#include <Servo.h>
int posToWall = 0;
int posToPlayer = 180;
int servoPin = 9;
Servo servo;

// hc-sr04 sensor pins and variables
// optimal range: 2 cm ~ 400 cm
const int trigPin = 5;
const int echoPin = 6;
long read_duration;
int distance;
int stop_distance;
int move_distance;
int diff;

// led pins
const int greenPin = 7;
const int redPin = 8;

// passive buzzer pins
#include "pitches.h"
const int musicPin = 2;
const int alarmPin = 3;

// main theme (squid game)
int main_melodies[] = {
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_DS5, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4,
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_E4
};
int main_durations[] = {
  4, 4, 2, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 2, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2
};
int main_theme_len = sizeof(main_melodies) / sizeof(int);

// game theme (green light red light)
int game_melodies[] = {
  NOTE_E4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_A4, NOTE_A4, NOTE_E4, NOTE_E4, NOTE_G4
};
int game_durations[] = {
  4, 4, 2, 2, 2,
  4, 4, 4, 4, 2
};
int game_theme_len = sizeof(game_melodies) / sizeof(int);

// win theme (mario victory)
int win_melodies[] = {
  NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5,
  NOTE_G5, NOTE_E5,
  NOTE_GS3, NOTE_C4, NOTE_DS4, NOTE_GS4, NOTE_C5, NOTE_DS5,
  NOTE_GS5, NOTE_DS5,
  NOTE_AS3, NOTE_D4, NOTE_F4, NOTE_AS4, NOTE_D5, NOTE_F5,
  NOTE_AS5, NOTE_AS5, NOTE_AS5, NOTE_AS5, NOTE_C6
};
int win_durations[] = {
  6, 6, 6, 6, 6, 6,
  2, 2,
  6, 6, 6, 6, 6, 6,
  2, 2,
  6, 6, 6, 6, 6, 6,
  2, 6, 6, 6, 1
};
int win_theme_len = sizeof(win_melodies) / sizeof(int);

// lost theme
int lost_melodies[] = {
  NOTE_C5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4,
  NOTE_A4, NOTE_G4, NOTE_F4, NOTE_C4
};
int lost_durations[] = {
  6, 6, 6, 6, 6, 6,
  6, 6, 6, 2
};
int lost_theme_len = sizeof(lost_melodies) / sizeof(int);

void setup() {
  Serial.begin(9600);

  // hc-sr04 sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // led setup
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);

  // button setup
  button.setDebounceTime(50);

  // servo setup
  servo.attach(servoPin, 500, 2500);
  servo.write(posToWall);

  // passive buzzer setup
  pinMode(musicPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  digitalWrite(musicPin, LOW);
  digitalWrite(alarmPin, LOW);
  Serial.println("***********************");
  Serial.println("Playing main music ...");
  playTheme(main_melodies, main_durations, main_theme_len);
  delay(2000);
}

void loop()
{
  // loop through game counts
  for (int game_cnt = 0; game_cnt < 5 ; game_cnt++)
  {
    if (isButtonPressed())
    {
      // button pressed (game won)
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);
      playTheme(win_melodies, win_durations, win_theme_len);
      delay(1000);
      exit(0);
    }

    // print game count
    Serial.println("***********************");
    Serial.print("\t");
    Serial.print("Game ");
    Serial.println(game_cnt + 1);

    // green light
    servo.write(posToWall);
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);
    Serial.println("-----------------------");
    Serial.println("Green Light ...");
    playTheme(game_melodies, game_durations, game_theme_len);

    // red light
    Serial.println("Red   Light !");
    servo.write(posToPlayer);
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    delay(1000);

    // read stop distance
    Serial.println("-----------------------");
    Serial.print("Stop ");
    stop_distance = readDistance(300);
    Serial.println("-----------------------");

    // read move distances (takes (read_cnt x read_time) seconds)
    for (int read_cnt = 0; read_cnt < 25; read_cnt ++)
    {
      Serial.print("Move ");
      move_distance = readDistance(80);

      // player move (game over)
      if (isPlayerMove())
      {
        Serial.println("***********************");
        playAlarm();
        delay(1000);
        playTheme(lost_melodies, lost_durations, lost_theme_len);
        exit(0);
      }

      // player stay still (game continues)
      else
      {
        continue;
      }
    }
  }
  // game count over (game over)
  Serial.println("-----------------------");
  Serial.println("Out of count. You lost!");
  Serial.println("***********************");
  playTheme(lost_melodies, lost_durations, lost_theme_len);
  exit(0);
}

void playTheme(int melodies[], int durations[], int theme_length)
{
  for (int i = 0; i < theme_length; i++)
  {
    int duration = 1000 / durations[i];
    int pause_time = duration * 1.13;

    tone(musicPin, melodies[i], duration);
    delay(pause_time);
    noTone(musicPin);
  }
}

bool isButtonPressed()
{
  button.loop();

  // button pressed
  if (button.isPressed())
  {
    Serial.println("-----------------------");
    Serial.println("Button is pressed!");
    Serial.println("***********************");
    return true;
  }

  // button not pressed
  Serial.println("-----------------------");
  Serial.println("Button is not pressed.");
  return false;
}

int readDistance(int read_time)
{
  // send ping from trigger pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);

  // read ping from echo pin
  read_duration = pulseIn(echoPin, HIGH);
  distance = (read_duration * 0.0343) / 2.;

  // show distance and button state
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(read_time);

  return distance;
}

bool isPlayerMove()
{
  diff = abs(stop_distance - move_distance);

  // player move (game lost)
  if (7 < diff && diff < 10) // noise removal
  {
    Serial.println("-----------------------");
    Serial.println("Player 324 eliminated !");
    return true;
  }

  // player stay still (game continues)
  else
  {
    return false;
  }
}

void playAlarm()
{
  tone(alarmPin, 600, 3000);
  delay(500);
  noTone(alarmPin);
  delay(500);
  tone(alarmPin, 600, 3000);
  delay(500);
  noTone(alarmPin);
  delay(500);
}
