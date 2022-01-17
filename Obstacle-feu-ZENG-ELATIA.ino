// Gestion des Traces sur la console // pour debuguer 
#define DEBUG

// ====================================   Initialisation des valeurs de L'algorithme  =======================================================================
#define ALGO_OBSTACLE 'A'
#define ALGO_FEU     'V'
#define ALGO_STOP      'S'

#define P_DROITE 0x01 // pour connaître la ligne de l'Obstacle
#define P_MEDIUM 0x02
#define P_GAUCHE 0x04

char modd   = ALGO_FEU ; // Mode par defaut 
char mode   = ALGO_STOP ; 
int  pos    = P_MEDIUM ; // Position sur la ligne d'Obstacle
int  nbchgt = 0 ; // Nombre de changement de voie 

#define WAIT_STOP_CONTINUE 250 // stop je redemarre un peu et j'avance tout droit 

// Pour l'algo FEU (en ms)
#define WAIT_STOP 3000  // si je mets un stop 

// Pour l'algo OBSTACLE 
// Temps de rotattion pour se détourner de l'obstacle
#define WAIT_DEVIATION 500
#define WAIT_AVANCE    300
#define WAIT_STOP_OBSTACLE 1500

// ==================================   Gestion des moteurs =============================================================================
#define moteurGauche 5
#define moteurDroite 6
#define moteurGaucheA 7
#define moteurGaucheR 8
#define moteurDroiteR 9
#define moteurDroiteA 11
#define carSPEED 150
int carSpeed = 150 ; 

#define STOP  'S'
#define FWD   'F'
#define LEFT  'L'
#define RIGHT 'R'
#define BCK   'B'
char dir = 'S' ; 

// --------------- Gestion des moteurs : Avancer
void forward(){
  analogWrite(moteurGauche, carSpeed);
  analogWrite(moteurDroite, carSpeed);
  digitalWrite(moteurGaucheA, HIGH);
  digitalWrite(moteurGaucheR, LOW);
  digitalWrite(moteurDroiteR, LOW);
  digitalWrite(moteurDroiteA, HIGH);
#ifdef DEBUG 
  if (dir != FWD) // DUBUG que si changement de direction
  { dir = FWD ; Serial.println("go forward!"); }
#endif
}

// --------------- Gestion des moteurs : reculer
void back(){
  analogWrite(moteurGauche, carSpeed);
  analogWrite(moteurDroite, carSpeed);
  digitalWrite(moteurGaucheA, LOW);
  digitalWrite(moteurGaucheR, HIGH);
  digitalWrite(moteurDroiteR, HIGH);
  digitalWrite(moteurDroiteA, LOW);
#ifdef DEBUG
  if (dir != BCK) // DEBUG que si changement de direction
  {  dir = BCK ; Serial.println("go back!"); }
#endif
}

// --------------- Gestion des moteurs : pivoter à gauche
void left()
{
  analogWrite(moteurGauche, carSpeed);
  analogWrite(moteurDroite, carSpeed);
  digitalWrite(moteurGaucheA, LOW);
  digitalWrite(moteurGaucheR, HIGH);
  digitalWrite(moteurDroiteR, LOW);
  digitalWrite(moteurDroiteA, HIGH);
#ifdef DEBUG
  if (dir != LEFT)
  { dir = LEFT ; Serial.println("go left!"); }
#endif
}

// --------------- Gestion des moteurs : pivoter à droite
void right()
{
  analogWrite(moteurGauche, carSpeed);
  analogWrite(moteurDroite, carSpeed);
  digitalWrite(moteurGaucheA, HIGH);
  digitalWrite(moteurGaucheR, LOW);
  digitalWrite(moteurDroiteR, HIGH);
  digitalWrite(moteurDroiteA, LOW); 
#ifdef DEBUG
  if (dir != RIGHT)
  {  dir = RIGHT ; Serial.println("go right!"); }
#endif
} 

// --------------- Arrêt du véhicule
void stop()
{
   digitalWrite(moteurGauche, LOW);
   digitalWrite(moteurDroite, LOW);
#ifdef DEBUG
   if (dir != STOP)
   { dir = STOP ; Serial.println("Stop!"); }
#endif
}

// ====================================   Gestion du capteur à Ultra son  ================================================================
#include <Servo.h>  // Librairie de gestion du servo moteur qui pilote la position du capteur à ultra son
Servo myservo;      // Objet de contrôle du servo moteur 
int Echo = A4;  //reçoit l'impulsion
int Trig = A5;  //envoie une onde sonore

#define SERVO 3 

int distance = 0;
// Distance d'arret en cm => En mode Obstacle, il faut la place pour que la voiture a le temps d'effectuer la déviation sans toucher l'obstacle
#define STOPDIST 15

// ---------------- Gestion ultra sons : Mesure la distance "droit devant la voiture" 
// Retourne la valeur en cm 
int mesureDistance() 
{
  digitalWrite(Trig, LOW);   
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);  
  delayMicroseconds(20);
  digitalWrite(Trig, LOW);   // On est sûr . d'avoir une impulsion de 20 ms
  float Fdistance = pulseIn(Echo, HIGH);  // intervalle mesure le temps que ca prend pour que Echo reçoit l'onde
  Fdistance= Fdistance / 58; // on transforme notre intervalle de temps en distance en cm      
  return (int)Fdistance;  // notre intervalle est un float et notre variable est un int, donc on perd la virgule qui et dans intervalle
}  

// ---------------- Gestion ultra sons : Obstacle droit devant => ARRET
#if TOTO //Utilisation pour débugger: tester si la voiture détecte un obstacle
int detectionObstacleFeu() 
{
   distance = mesureDistance(); 
#ifdef DEBUG 
   traceUltraSon(distance) ; 
#endif
   if (distance < STOPDIST) // Emergency Stop
   {  
      stop() ; 
      return 0 ; 
   }
   return 1 ; 
}
#endif

void detectionObstacleFeu()
{
   distance = mesureDistance(); 
#ifdef DEBUG 
   traceUltraSon(distance) ;   
#endif
   while (distance < STOPDIST) // Emergency Stop
   {  
      stop() ; 
      distance = mesureDistance(); delay(500) ; // on recalcule toutes les 500 ms 
#ifdef DEBUG      
      traceUltraSon(distance) ; 
#endif

   }
}

// ---------------- Gestion des ultra sons : Debug 
#ifdef DEBUG
void traceUltraSon(int dst)
{
   Serial.print("Dist ") ; Serial.print(dst) ; Serial.println("") ; 
}
#endif


// ==================================   Gestion du suivi de ligne =======================================================================
#define capteurDroite 10
#define capteurMilieu  4
#define capteurGauche  2

#define etatCapteurDroite !digitalRead(capteurDroite)  //Les capteurs ne détectent rien par défaut
#define etatCapteurMilieu !digitalRead(capteurMilieu)
#define etatCapteurGauche !digitalRead(capteurGauche)

#define LT_STOP     0x7 //les champs de bit des capteurs
#define LT_GAUCHE   0x4
#define LT_MILIEU   0x2
#define LT_DROITE   0x1
#define LT_AUTO     0x0

// ---------------------  Gestion suivi de ligne : Les 3 capteurs dans un champs de bits 0x04 Pour gauche, 0x02 pour milieu , 0x01 pour droite
// Retourne en entier "champ de bit"
int capteurLine ; 
int getLT()
{
   capteurLine = (etatCapteurGauche << 2) + (etatCapteurMilieu << 1) + etatCapteurDroite ; 
   return capteurLine ; 
}

// ---------------------  Gestion suivi de ligne : Debug  
#ifdef DEBUG
void traceLT() 
{
  Serial.print("LT_Right ")  ; Serial.print(etatCapteurDroite) ; Serial.print(" - ") ; 
  Serial.print("LT_Medium ") ; Serial.print(etatCapteurMilieu) ; Serial.print(" - ") ; 
  Serial.print("LT_Left ")   ; Serial.print(etatCapteurGauche) ; Serial.println("")  ; 
}

void traceIntLT(int cpLine)
{ 
   Serial.print("Line 0x") ; Serial.println(cpLine, HEX) ; //bit en hexadecimal, pour les valeurs associés aux boutons de manette
}

#endif

// ---------------------  Gestion suivi de ligne : Gestion du Stop (Double ligne pour augmenter chance de détection)
// Commun à l'Obstacle et à la Feu 

// --------------------- Gestion suivi de ligne : Suivi de ligne
void suivreLigneEtStop()
{
  if(etatCapteurMilieu){
    forward();
  }
  else if(etatCapteurDroite) { 
    right();
    while(etatCapteurDroite);                             
  }   
  else if(etatCapteurGauche) {
    left();
    while(etatCapteurGauche);  
  }
  
}


// --------------------- Gestion suivi de ligne : déviation ou évitement sur l'Obstacle
//deviationDroite(&right,LT_GAUCHE,&left,P_DROITE) ;
void deviationDroite(int newPos) 
{
// On evite en tourant coté f1
   right() ; 
   delay(WAIT_DEVIATION) ; // Avec vitesse à 150, environ 45°

// On ralentit pour être certain de trouver la ligne
   carSpeed = 100 ; 
   
// On avance tant qu'on ne trouve pas la nouvelle ligne 
   while (getLT() != LT_GAUCHE) // on est dans le vide, on s'arrête dès qu'on la ligne
   { forward() ; }

// Ligne trouvée, on revient en vitesse normale    
   carSpeed = carSPEED ; 

// On indique la nouvelle position sur l'Obstacle
   pos = newPos ;  

// On facilite la reprise de ligne
   left() ; delay(250) ; 
}

void deviationGauche(int newPos) 
{
// On evite en tourant coté f1
   left() ; 
   delay(WAIT_DEVIATION) ; // Avec vitesse à 150, environ 45°

// On ralentit pour être certain de trouver la ligne
   carSpeed = 100 ; 
   
// On avance tant qu'on ne trouve pas la nouvelle ligne 
   while (getLT() != LT_DROITE) // on est dans le vide, on s'arrête dès qu'on la ligne
   { forward() ; }

// Ligne trouvée, on revient en vitesse normale    
   carSpeed = carSPEED ; 

// On indique la nouvelle position sur l'Obstacle
   pos = newPos ;  

// On facilite la reprise de ligne
   right() ; delay(250) ; 
}

// ==================================   Gestion du bluetooth =================================================================
#define LED 13

void ledMode() 
{
   if (mode == ALGO_OBSTACLE)
   {  digitalWrite(LED, HIGH) ; }
   else
   {  digitalWrite(LED, LOW) ; }
}

bool state = LOW;
char getstr;

// Les commandes émises par le bluetooth sont de simple caractère
void bluetooth()
{
  if(Serial.available())
    getstr = Serial.read();
    
  switch(getstr)
  {
    case 'f': mode = modd ; ledMode() ; forward() ; break;
    case 'b': back();  break ;
    case 'l': left();  delay(250) ; break;
    case 'r': right(); delay(250) ; break;
    case 's': stop();  mode = ALGO_STOP ; break;
    case 'a': stateChange(); break;
    case 'v': // v inversion du mode. Si mode == ALGO_STOP => On bascule en Feu   
              mode = (mode == ALGO_FEU)? ALGO_OBSTACLE : ALGO_FEU ;
              modd = mode ; 
              ledMode() ; 
          break ; 
    default:
#ifdef DEBUG
           //    Serial.print("bluetooth default ") ; Serial.println(getstr) ; 
#endif
         break;
  }
#ifdef DEBUG
       // Serial.print("BlueMode ") ; Serial.println(mode) ; 
#endif

}

void stateChange()
{
  state = !state;
  digitalWrite(LED, state) ;
#ifdef DEBUG  
  Serial.println("Light");  
#endif
}
// ===================================== SETUP de la carte =================================================
void setup() 
{
// Initialisation afin d'avoir des messages pour la mise au point
// Avec le bluetooth, il faut être à 9600 bauds
//   Serial.begin(115200) ;
   Serial.begin(9600) ; 
   
// Initialisation pour le capteur de suivi de ligne
   pinMode(capteurDroite,INPUT);
   pinMode(capteurMilieu,INPUT);
   pinMode(capteurGauche,INPUT);
   
// Initialisation pour le capteur à Ultra son  
   pinMode(Echo, INPUT);    
   pinMode(Trig, OUTPUT); 
   
// On met le capteur pour une vision droit devant 
   myservo.attach(SERVO);
   myservo.write(90); // On met le capeteur en postion droit devant -> 90°

// On arrête la voiture
   stop() ; 

// Affiche mode
   pinMode(LED, OUTPUT); 
   ledMode() ; 
}

// ===================================== ALGO : FEU ============================
// On suit la ligne, on arrête lorsque y a un feu (obstacle)
// ===================================== ALGO : OBSTACLE ============================
void algoObstacle()
{
   
// Detection d'obstacle 
   distance = mesureDistance(); 
#ifdef DEBUG 
   traceUltraSon(distance) ; 
   Serial.print("Position ") ; Serial.println(pos,HEX) ; 
#endif
   if (distance < STOPDIST) // Obstacle sur l'Obstacle
   {  
      switch(pos)
      {
         case P_MEDIUM: // On choisit "+/-" au hasard le changement de direction puisqu'on est sur la voie médiane
            nbchgt ++ ;
            if ((nbchgt % 2) == 0) // On dévie à droite
            { deviationDroite(P_DROITE) ; }
            else // On dévie à gauche
            { deviationGauche(P_GAUCHE) ; }
         break ; 
         case P_DROITE: // On est à droite => donc retour sur la ligne médiane
            { deviationGauche(P_MEDIUM) ; }
         break ; 
         case P_GAUCHE:
             deviationDroite(P_MEDIUM) ; 
         break ;
      }
       
   } 
   
// On a dévié, jusqu'à trouver une ligne, sans prendre en compte les obstacles. Donc on suit la ligne
   suivreLigneEtStop() ; 
  
}
// ========================================= BOUCLE PRINCIPALE ================================================
void loop() 
{
#ifdef DEBUG
   // delay(1000) ; // Pour avoir le temps de lire la console
#endif
   bluetooth() ; // On lit le bluetooth ou la console de PC 
   switch(mode)
   { case ALGO_OBSTACLE:
          algoObstacle() ; 
     break ;
     case ALGO_FEU:
          detectionObstacleFeu() ; // Arrêt si obstacle et on reste tant qu'il y a l'obstacle
          suivreLigneEtStop() ;                
     break;
     case ALGO_STOP:
          stop(); // Par sécurité
     break ;
   }
}
