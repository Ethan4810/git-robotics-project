/*
   Green Light Red Light Game

   Game Progress
   1. Game starts with main theme playing.
   2. Green light with game theme playing.
   3. Red light and initiate movement detection.
   4. Gane Won  (Reaches safe area)
   5. Game Lost (Out of counts or movement detected)
   6. Game reset with arduino reset button.

   Things to Do
   - adjust min diff and max diff (movement detection)
   - change game lost theme
*/

// adjustable variables
const float music_speed = 1.1;      // adjust
const int max_game_cnt = 5;         // adjust
const int min_diff = 10;            // adjust
const int max_diff = 15;            // adjust
const int win_distance = 35;        // adjust
const int stop_read_time = 100;     // adjust
const int move_read_time = 66;      // adjust
const int max_read_cnt = 30;        // adjust

// passive buzzer pins and variables
#include "pitches.h"
const int musicPin = 2;             // pin
const int alarmPin = 3;             // pin

// led pins
const int greenPin = 4;             // pin
const int redPin = 5;               // pin

// hc-sr04 sensor pins and variables
const int trigPin = 6;              // pin
const int echoPin = 7;              // pin
long read_duration;
int distance; // cm
int stop_distance;
int move_distance;
int diff; // cm

// servo pin and variables
#include <Servo.h>
Servo servo;
const int servoPin = 8;             // pin
const int posToWall = 0;
const int posToPlayer = 180;

// main theme (squid game)
int main_melodies[] = {
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_DS5, NOTE_B4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4,
  NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4, NOTE_B4,
  NOTE_B4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_E4
};
int main_theme_durations[] = {
  4, 4, 2, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2,
  4, 4, 2, 4, 4, 2,
  4, 4, 4, 4, 4, 4, 2
};
int main_theme_length = sizeof(main_melodies) / sizeof(int);

// game theme (green light red light)
int game_melodies[] = {
  NOTE_E4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_A4, NOTE_A4, NOTE_E4, NOTE_E4, NOTE_G4
};
int game_theme_durations[] = {
  4, 4, 2, 2, 2,
  4, 4, 4, 4, 2
};
int game_theme_length = sizeof(game_melodies) / sizeof(int);

// win theme (mario victory)
int win_melodies[] = {
  NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5,
  NOTE_G5, NOTE_E5,
  NOTE_GS3, NOTE_C4, NOTE_DS4, NOTE_GS4, NOTE_C5, NOTE_DS5,
  NOTE_GS5, NOTE_DS5,
  NOTE_AS3, NOTE_D4, NOTE_F4, NOTE_AS4, NOTE_D5, NOTE_F5,
  NOTE_AS5, NOTE_AS5, NOTE_AS5, NOTE_AS5, NOTE_C6
};
int win_theme_durations[] = {
  6, 6, 6, 6, 6, 6,
  2, 2,
  6, 6, 6, 6, 6, 6,
  2, 2,
  6, 6, 6, 6, 6, 6,
  2, 6, 6, 6, 1
};
int win_theme_length = sizeof(win_melodies) / sizeof(int);

// lost theme
int lost_melodies[] = {
  NOTE_C5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4,
  NOTE_A4, NOTE_G4, NOTE_F4, NOTE_C4
};
int lost_theme_durations[] = {
  6, 6, 6, 6, 6, 6,
  6, 6, 6, 2
};
int lost_theme_length = sizeof(lost_melodies) / sizeof(int);

void setup() {
  Serial.begin(9600);

  // hc-sr04 sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // servo setup
  servo.attach(servoPin, 500, 2500);
  servo.write(posToPlayer);

  // passive buzzer setup
  pinMode(musicPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);

  // led setup
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  allLightsOn();

  // play main theme
  Serial.println("***********************");
  Serial.println("Playing main music ...");
  playMusic(main_melodies, main_theme_durations, main_theme_length);
  delay(2000);
}

void loop()
{
  // loop through game counts
  for (int game_cnt = 0; game_cnt < max_game_cnt ; game_cnt++)
  {
    // print game count
    Serial.println("***********************");
    Serial.print("\t");
    Serial.print("Game ");
    Serial.println(game_cnt + 1);

    // green light
    servo.write(posToWall);
    greenLightOn();
    Serial.println("-----------------------");
    Serial.println("Green Light ~");
    playMusic(game_melodies, game_theme_durations, game_theme_length);

    // red light
    Serial.println("Red   Light !");
    servo.write(posToPlayer);
    redLightOn();
    delay(1000);

    // read stop distance
    Serial.println("-----------------------");
    Serial.print("Stop ");
    stop_distance = readDistance(stop_read_time);
    Serial.println("-----------------------");

    // read move distances (takes (max_read_cnt x move_read_time) seconds)
    for (int read_cnt = 0; read_cnt < max_read_cnt; read_cnt ++)
    {
      Serial.print("Move ");
      move_distance = readDistance(move_read_time);

      // player move (game over)
      if (isPlayerMove())
      {
        Serial.println("-----------------------");
        Serial.println("Player 324 eliminated!");
        Serial.println("***********************");
        allLightsOff();
        redLightOn();
        playAlarm();
        delay(1000);
        playMusic(lost_melodies, lost_theme_durations, lost_theme_length);
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
  Serial.println("Out of game counts.");
  Serial.println("-----------------------");
  Serial.println("Player 324 eliminated!");
  Serial.println("***********************");
  allLightsOff();
  redLightOn();
  playMusic(lost_melodies, lost_theme_durations, lost_theme_length);
  exit(0);
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

  // player reach safe area (game won)
  if (distance <= win_distance)
  {
    // game count over (game over)
    Serial.println("");
    Serial.println("-----------------------");
    Serial.println("You are in safe area.");
    Serial.println("-----------------------");
    Serial.println("Player 324 won!");
    Serial.println("***********************");
    allLightsOff();
    greenLightOn();
    playMusic(win_melodies, win_theme_durations, win_theme_length);
    exit(0);
  }

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
  if (min_diff <= diff && diff <= max_diff) // noise removal
  {
    Serial.println("-----------------------");
    Serial.println("Movement detected.");
    return true;
  }

  // player stay still (game continues)
  else
  {
    return false;
  }
}

void playMusic(int melodies[], int music_durations[], int music_length)
{
  for (int i = 0; i < music_length; i++)
  {
    int music_duration = 1000 / music_durations[i];
    int pause_time = music_duration * music_speed;

    tone(musicPin, melodies[i], music_duration);
    delay(pause_time);
    noTone(musicPin);
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

void greenLightOn()
{
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
}

void redLightOn()
{
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
}

void allLightsOn()
{
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, HIGH);
}

void allLightsOff()
{
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
}
