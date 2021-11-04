/*
   Green Light Red Light Game

   Things to consider:
   1. Noise detection (hc-sro4)
   2. Hardware malfunction (wires)
   3.

   Things to implement:
   1. Game start and quit (win) button
   2. Game win/lost music
   3. RGB LED

   Naming Conventions:
   snake_case: general variables
   camelCase: pin variables, functions
   PascalCase: file names
*/


// button pins and variables
#include <ezButton.h>
ezButton button(12);
int btnState;

// servo pin and variables
#include <Servo.h>
int pos = 0;
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
const int musicPin = 2;
const int alarmPin = 3;

// green light red light theme
char game_notes[] = "eaaagaaeeg";
int game_beats[] = {1, 1, 2, 2, 2, 1, 1, 1, 1, 1};

// squid game theme
char main_notes[] = "bbbbbbbbbagabbbbbbbbagagee";
int main_beats[] = {1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2};


void setup() {
  // hc-sr04 sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // led setup
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);

  // button setup
  button.setDebounceTime(100);

  // passive buzzer setup
  pinMode(musicPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  digitalWrite(musicPin, LOW);
  digitalWrite(alarmPin, LOW);

  // servo setup
  servo.attach(servoPin, 500, 2500);
  servo.write(0);
  Serial.begin(9600);

  playTheme(main_notes, main_beats);
  delay(2000);
}

void loop()
{
  // green light
  servo.write(0);
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
  playTheme(game_notes, game_beats);

  // red light
  servo.write(180);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
  delay(1000);

  // read stop distance
  // TODO: fix (first distance = 0 cm error)
  Serial.println("***********************");
  Serial.print("Stop ");
  stop_distance = readDistance(300);
  Serial.println("-----------------------");

  // read move distances (takes (read_cnt x read_time) seconds)
  for (int read_cnt = 0; read_cnt < 20; read_cnt ++)
  {
    Serial.print("Move ");
    move_distance = readDistance(100);

    // player move
    if (isMove())
    {
      playAlarm();
      break;
    }

    // player stay still
    else
    {
      continue;
    }
  }
}

int frequency(char note)
{
  char names[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C', 'D', 'E'};
  int num_notes = strlen(names);
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523, 587, 659};

  for (int i = 0; i < num_notes; i++)
  {
    if (names[i] == note)
    {
      return (frequencies[i]);
    }
  }

  return 0;
}

void playTheme(char notes[], int beats[])
{
  int tempo = 280;
  int song_length = strlen(notes);
  int music_duration;

  // loop through song
  for (int i = 0; i < song_length; i++)
  {
    music_duration = beats[i] * tempo;

    // pause sound
    if (notes[i] == ' ')
    {
      delay(music_duration);
    }

    // play sound
    else
    {
      tone(musicPin, frequency(notes[i]), music_duration);
      delay(music_duration);
    }

    // brief pause between notes
    delay(tempo / 10);
  }
}

int readDistance(int read_time)
{
  // send ping from trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  // read ping from echo pin
  read_duration = pulseIn(echoPin, HIGH);
  distance = (read_duration * 0.0343) / 2.;

  // show distance and button state
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm  ");
  delay(read_time);

  return distance;
}

bool isMove()
{
  diff = abs(stop_distance - move_distance);

  // player move
  // check only in this range (noise prevention)
  // for human: 10 < diff < 15
  // for object: 2 < diff < 15
  if (2 <= diff && diff < 15)
  {
    return true;
  }

  // player stay still
  else
  {
    return false;
  }
}

bool isArrived()
{
  
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
