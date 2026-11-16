Stone & Spikes — asset HUD, layout 1280 × 720

background.png               1280 × 720   Cornice, 7 × 7, barra HP/AP e fondo End Turn
knight.png / skeleton.png      58 × 64    Pedine, ancoraggio al centro del bordo inferiore
*_portrait.png                 64 × 64    Usa a 64 × 64 se attivo, altrimenti a 56 × 56
pillar.png                     50 × 60    Colonna, centrata sulla casella
spikes.png                     60 × 60    Trappola, centrata sulla casella
health_badge.png               48 × 18    Cuore a sinistra, disegna la frazione a destra
action_menu.png               104 × 58    Attacca a sinistra, spingi a destra

In coordinate 1280 × 720, rettangolo giocabile indicativo: (387,134)–(890,624).
Dividi larghezza e altezza per 7. Punto di appoggio della pedina:
centro orizzontale della casella, 4 px sopra il bordo inferiore.
Per End Turn, centro indicativo (1084,364) e raggio cliccabile 62 px.
Il centro orizzontale dei ritratti è intorno a x=160; primo centro intorno a y=135.

I valori HP/AP, le frazioni sopra i nemici e la scritta del pulsante
vanno disegnati in gioco con un font comune. Se scali la scena, usa
la stessa trasformazione per geometria e input.
