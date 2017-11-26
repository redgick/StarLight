/*
   Firmware du projet "starlight" présenté lors de MétroMix 2017 à Rennes.
   La documentation est disponible sur http://www.wiki-rennes.fr/M%C3%A9troMix_2017/StarLight
   Et le code sur https://github.com/jlebunetel/StarLight
*/

// constantes
#define JOUEURA 1
#define JOUEURB 2

#define DEFAUT_FACTEUR_TOLERANCE  3
#define NUM_LEDS                  236

// librairie nécessaire au pilotage de la matrice de led HT1632c
// https://github.com/jlebunetel/HT1632_arduino/tree/ArduinoMega
#include <HT1632_arduino.h>

// création de l'objet afficheur
HT1632_arduino afficheur;

// création d'un objet "ecran" qui est un buffeur contenant l'image à afficher sur "afficheur"
Screen* ecran = new Screen();

// boutons
const int boutonA = 10;
const int boutonB = 11;

// librairies pour piloter la bande de led adressable Neopixel
// Adafruit NeoPixel version 1.1.3
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// broche sur laquelle est branchée la bande de led
#define PIN 3

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// variables globales
int taille_raquette = 7;
int balle_position = 0;
int balle_direction = 0; // direction = 0 -> de led 0 vers led max et direction = 1 -> de led max vers led 0
int main_joueur = 0;     // qui a la main ?
int scoreA = 0;
int scoreB = 0;

void setup() {
  // initialisation de la liaison série pour le débogage
  Serial.begin(115200);
  Serial.println("demarrage");

  // déclaration des boutons en entrée
  pinMode(boutonA, INPUT);
  pinMode(boutonB, INPUT);

  // initialisation de l'afficheur et message de bienvenue
  afficheur.begin(PWM_10);
  ecran->setLine("STAR", FIRST, CENTER, ORANGE);
  ecran->setLine("LIGHT", SECOND, CENTER, ORANGE);
  afficheur.display(ecran);

  // initialisation de la bande de led
  strip.begin();
  strip.show();
}


void loop() {
  // le jeu est en veille tant qu'aucun joueur n'a pris la main
  while (main_joueur == 0) {
    ecran->clear();
    ecran->setLine("PRESS", FIRST, CENTER, GREEN);
    ecran->setLine("START", SECOND, CENTER, RED);
    afficheur.display(ecran);

    // une petite animation pour patienter ...
    for (uint8_t i = 0; i < NUM_LEDS ; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();

    // on est en attente d'un appuis sur start
    while (digitalRead(boutonA) == 1 and digitalRead(boutonB) == 1) {
      theaterChase(strip.Color(0, 0, 127), 10);
    }

    // le jeu commence !
    if (digitalRead(boutonA) == 0) {
      // A viens d'appuyer
      main_joueur = JOUEURA;
    }
    else {
      // B viens d'appuyer
      main_joueur = JOUEURB;
    }

    // on lance la partie
    ecran->clear();
    ecran->setLine("READY?", FIRST, CENTER, ORANGE);
    afficheur.display(ecran);
    delay(1000);
    ecran->setLine("GO !!!", SECOND, CENTER, RED);
    afficheur.display(ecran);
    delay(1000);
  }


  // affichage des raquettes
  for (uint8_t i = 0 ; i < taille_raquette; i++) {
    // raquette verte
    strip.setPixelColor(i, strip.Color(0, 64, 0));
    // raquette rouge
    strip.setPixelColor(NUM_LEDS - i - 1, strip.Color(64, 0, 0));
  }
  strip.show();


  // affichage des scores
  ecran->clear();
  ecran->setLine("A: " + String(scoreA), FIRST, CENTER, GREEN);
  ecran->setLine("B: " + String(scoreB), SECOND, CENTER, RED);
  afficheur.display(ecran);

  // qui commence ?
  if (main_joueur == JOUEURA) {
    // le joueur A a la main
    balle_position = 0;
    balle_direction = 0;
    strip.setPixelColor(0, strip.Color(0, 0, 128));
    strip.show();
    while (digitalRead(boutonA) == 1) {
      // on attend un appuis sur le bouton A
      delay(10);
    }
  }
  else {
    // le joueur B a la main
    balle_position = NUM_LEDS - 1;
    balle_direction = 1;
    strip.setPixelColor(balle_position, strip.Color(0, 0, 128));
    strip.show();
    while (digitalRead(boutonB) == 1) {
      // on attend un appuis sur le bouton B
      delay(10);
    }
  }

  // c'est parti !
  bool partie_en_cours = true;

  while (partie_en_cours) {
    // on affiche la balle
    strip.setPixelColor(balle_position, strip.Color(0, 0, 64));
    strip.show();

    // on temporise un peu pour ralentir la balle
    //delay(20);

    // Le joueur A appuye
    if (digitalRead(boutonA) == 0) {
      if (balle_position < taille_raquette and balle_direction == 1) {
        // le joueur A renvoie la balle
        balle_direction = 1 - balle_direction;
      }
      else if (balle_position > taille_raquette * DEFAUT_FACTEUR_TOLERANCE and balle_direction == 1) {
        // le joueur B a gagné !
        scoreB++;
        main_joueur = JOUEURA;
        partie_en_cours = false;
        theaterChase(strip.Color(127, 0, 0), 50); // Green
      }
    }

    // Le joueur B appuye
    if (digitalRead(boutonB) == 0) {
      if ((balle_position > NUM_LEDS - taille_raquette - 1) and balle_direction == 0) {
        // le joueur B renvoie la balle
        balle_direction = 1 - balle_direction;
      }
      else if (balle_position < NUM_LEDS - taille_raquette * DEFAUT_FACTEUR_TOLERANCE - 1 and balle_direction == 0) {
        // le joueur A a gagné !
        scoreA++;
        main_joueur = JOUEURB;
        partie_en_cours = false;
        theaterChase(strip.Color(0, 127, 0), 50); // Red
      }
    }


    // on efface la balle
    if (balle_position < taille_raquette) {
      // raquette verte
      strip.setPixelColor(balle_position, strip.Color(0, 64, 0));
    }
    else if (balle_position > NUM_LEDS - taille_raquette - 1) {
      // raquette rouge
      strip.setPixelColor(balle_position, strip.Color(64, 0, 0));
    }
    else {
      // en dehors de la raquette
      strip.setPixelColor(balle_position, strip.Color(0, 0, 0));
    }
    strip.show();

    // la balle avance d'un cran
    if (balle_direction == 0) {
      balle_position++;
      balle_position++;
      if (balle_position >= NUM_LEDS - 1) {
        // le joueur A a gagné !
        scoreA++;
        main_joueur = JOUEURB;
        partie_en_cours = false;
        balle_position = NUM_LEDS - 1 ;
        theaterChase(strip.Color(0, 127, 0), 50); // Red
      }
    }
    else {
      balle_position--;
      balle_position--;
      if (balle_position <= 0) {
        // le joueur B a gagné !
        scoreB++;
        main_joueur = JOUEURA;
        partie_en_cours = false;
        balle_position = 0;
        theaterChase(strip.Color(127, 0, 0), 50); // Green
      }
    }
  }
}

// Theatre-style crawling lights.
// animation issue de la librairie de Adafruit
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < NUM_LEDS; i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();
      delay(wait);
      for (uint16_t i = 0; i < NUM_LEDS; i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}
