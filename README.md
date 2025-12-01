# CIS17C – Project 1
# Blackjack Simulation 

A robust, multi-player, console-based Blackjack simulation implemented in standard C++ that utilizes STL containers and algorithms to manage game state, player actions, and card shuffling logic.

## Project Overview

This program simulates a full game of casino-style Blackjack against a dealer, supporting multiple human players. The core objective was to implement a complex, interactive game using advanced C++ features, specifically focusing on the performance and flexibility of **Standard Template Library (STL)** containers like `std::list`, `std::map`, `std::queue`, and `std::stack`.

## Features

### Core Game Logic

  * **Multi-Deck Management:** Uses a **4-deck** shoe for realistic play.
  * **Dynamic Reshuffle:** Automatically moves the **Discard Pile (`std::stack`)** back into the **Deck (`std::list`)** and reshuffles when the deck count falls below a threshold (60 cards).
  * **Multi-Player Support:** Allows 1 to 3 players to compete against the dealer.
  * **Dealer Rules:** Dealer hits on any score $\le 16$ and stands on all scores $\ge 17$ (Hard or Soft 17).

### Player Actions & Hand Management

  * **Standard Actions:** **Hit** (H) and **Stand** (S).
  * **Splits (P):** Allows splitting a pair of cards (same rank) into two separate hands, including handling the special rule for **Split Aces** (one card draw only).
      * *Implementation Detail:* Splitting is managed by dynamically inserting a new `Hand` into the player's `std::list<Hand>` using **list iterators**.
  * **Double Down (D):** Allows doubling the bet and receiving exactly one additional card.
  * **Payouts:** Handles standard Blackjack payouts (3:2 for Natural Blackjack) and manages **Pushes**.
  * **Game State:** Utilizes a **Turn Queue (`std::queue<Player*>`)** to manage player order during the action phase.

### Technical Implementation

  * **Score Calculation (`calcScr`):** Uses **`std::accumulate`** with a custom lambda function to efficiently calculate the optimal hand score, correctly handling the flexible value of **Aces** (1 or 11).
  * **Deck Shuffle:** Implements a custom, list-safe shuffle algorithm using a random number generator and **list iterators** for element selection and insertion (`std::advance`, `std::erase`, `std::insert`).
  * **Unicode Support:** Uses **Unicode characters** for card suits for enhanced console display.

## Getting Started

### Prerequisites

  * A C++ compiler.
  * A terminal/console that supports Unicode characters for proper display of card suits.

### Installation & Execution

1.  **Clone the repository**

    ```bash
    git clone https://github.com/jadethong/CIS17C_Blackjack_Project/
    ```

2.  **Compile the program**

    - Open your C++ compiler
    - Open the main.cpp file

3.  **Run the program**
    - Follow the on-screen prompts to enter the number of players and their bets.

##  Code Structure Notes

| Feature | C++ Container/Algorithm Used | Purpose |
| :--- | :--- | :--- |
| **Deck** | `std::list<Card>` | Allows efficient card dealing (`pop_front()`) and insertion during reshuffling. |
| **Discard Pile** | `std::stack<Card>` | Enforces **LIFO (Last-In, First-Out)** order for collecting cards before a reshuffle. |
| **Player Hands** | `std::list<Hand>` | Allows dynamic insertion and deletion of hands to support the **Split** action using iterators. |
| **Turn Order** | `std::queue<Player*>` | Manages the sequence of player turns. |
| **Scoring** | `std::accumulate` | Calculates the initial sum and Ace count in one pass for score optimization. |

-----

**Author**: Jade Thong
**Instructor**: Dr. Lehr
**Institution**: Riverside City College
**Course**: CIS 17-C Section 47469
