# 🎮 Stack Wars: A Two-Player Competitive Stack Game

StackWars is a fast-paced, 2-player competitive stacking game built with Arduino, LED matrices, an I2C LCD, and tact buttons.

Players race to stack moving blocks with precision — miss too much, and it’s game over!
Perfect for friendly duels and quick reflex challenges.

Both the name and the overall design — including the custom case and immersive sound effects — are inspired by Star Wars, giving the game a distinctive, intergalactic flair.

## ✨ Features
- Two simultaneous players on separate tact buttons and LED matrix displays
- LCD scoreboard for scores, game status, and high score
- Features different difficulty levels (1-8)
- Variable block speed (gets faster as you progress)
- Buzzer sound effects inspired by star wars for feedback, victory and game over
- Special button for extra functions (e.g., hold to reset, press to select difficulty)


## 🛠 Hardware Requirements
- Arduino Uno (or any compatible board)
- 8 × MAX7219 LED matrix modules (4 per player)
- 2 × I2C 16×2 LCD display 
- 2 × Tact switches (Player 1 Red & Player 2 Blue)
- 1 × Special tact switch black
- 1 × Passive buzzer
- 2 × 18650 lithium-ion battery with battery holder
- 1 × Power switch
- Jumper wires and breadboard or custom PCB
- Chassis materials

## 🎯 How to Play
1. Press the special button to start the game with the current selected difficulty.
2. Blocks move left and right for both players — press the respective player buttons to **stop** the block on that position.
3. Each block must align with the one below except the first one:
   - **Perfect alignment** → full block size retained
   - **Partial alignment** → block gets smaller
4. The higher you go, the faster it gets!
5. If your block does not align with any block below → **Game Over**, meanwhile if you reach the top first → **Win**

## 📸 Documentation

## 📸 Documentation

**Circuit Design (Made in Cirkit Designer):**  
<p align="center">
  <img src="Circuit%20Image.png" alt="Circuit Design" width="400">
</p>

**Actual Photos:**  
<p align="center">
  <img src="StackWarsRealPhoto.gif" alt="Actual Photos" width="400">
</p>

**Demonstration:**  
<p align="center">
  <img src="DemoPlay.gif" alt="Gameplay Demo" width="400">
</p>



**Paper Documentation:**
[Full Documentation (PDF)](/StackWarsPaper.pdf)

**3D Model:**
[3D Model](/StackWars3D.skp)

## 📜 License
This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.
