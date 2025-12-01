/*
* Author: Jade Thong
* Created: October 20, 2025, 6:33 PM
* Purpose: To simulate a Blackjack game
*/

// System Libraries Here
#include <iostream>
#include <string>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
#include <stdexcept>
#include <limits>
#include <sstream>
#include <iomanip> // For output formatting

// User Libraries Here
// Global Constants Only, No Global Variables
// Constants like PI, e, Gravity, Conversions, 2D array size only!

// Card Structure and Constants

// Card Structure
struct Card {
    std::string rank; // "A", "2", ..., "K"
    std::string suit; // "\u2660" (Spades), "\u2665" (Hearts), "\u2666" (Diamonds), "\u2663" (Clubs)
    int value; // Primary value (Ace = 11)

    // Overloaded operators for list/set comparisons and unique checks
    // Required for std::find and comparisons
    bool operator==(const Card& oth) const {
        // Two cards are equal if both rank and suit match
        return rank == oth.rank && suit == oth.suit;
    }
    bool operator<(const Card& oth) const { // For sorting
        if (rank != oth.rank) return rank < oth.rank; // Sort by rank first
        return suit < oth.suit; // Then by suit
    }
    // For easy printing
    friend std::ostream& operator<<(std::ostream& os, const Card& c) {
        return os << c.rank << c.suit; // e.g., "A♠"
    }
};

// Map: Stores rank to its primary value
const std::map<std::string, int> CRDVALS = {
    // Ace is 11 by default; adjusted in scoring logic
    {"A", 11}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}, {"6", 6},
    {"7", 7}, {"8", 8}, {"9", 9}, {"10", 10}, {"J", 10}, {"Q", 10}, {"K", 10}
};
// List: Stores all suits
const std::list<std::string> SUITS = {"\u2660", "\u2665", "\u2666", "\u2663"};

// Set: Stores all valid ranks for validation
const std::set<std::string> VALRANKS = {
    "A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K" // Valid ranks
};

// Hand Structures
struct Hand {
    std::list<Card> cards; // Hand is a list of cards
    int bet = 0; // Bet amount for this hand
    bool isplit = false; // Flag to indicate if this hand is a result of a split
    bool ddown = false; // Flag for double down
};

// Player Structure
struct Player {
    int id; // Player ID
    std::string name; // Player Name
    int chips = 1000; // Starting chips
    // List: Player can have multiple hands (for splits)
    std::list<Hand> hands;
    // Operator for sorting by chips (descending)
        bool operator<(const Player& oth) const {
            return chips > oth.chips; // Descending order
        }
};

// Deck Management

// Function to simulate random access on a std::list
// Returns a pointer to the card at the nth position (0-indexed)
// Note: This is O(N) complexity, not O(1) like std::vector
Card* getNthCard(std::list<Card>& deck, int n) {
    // Check for out-of-bounds access
    if (n < 0 || n >= deck.size()) {
        return nullptr; // Return null if index is invalid
    }
    
    // Iterator to the beginning of the list
    auto it = deck.begin();
    
    // std::advance moves the iterator 'n' steps forward
    std::advance(it, n);
    
    // Return a pointer to the element the iterator points to
    return &(*it);
}

// Global Containers for Game State
std::list<Card> deck;          // Deck of cards
std::stack<Card> disPile;      // LIFO discard
std::queue<Player*> playQue;   // Tracks turn order

// Function Prototypes Here

// Clear input buffer
void clear_in() {
    std::cin.clear(); // Clear any error flags
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
}

// Scoring Functions
// Calculates the best Blackjack score for a hand.
int calcScr(const Hand& hand) {
    if (hand.cards.empty()) { // No cards means score is 0
        return 0;
    }

    // Use std::accumulate to sum card values and count Aces
    auto result = std::accumulate(hand.cards.cbegin(), hand.cards.cend(), std::make_pair(0, 0),
        // Pair: first = sum of non-Ace values, second = count of Aces
        [](std::pair<int, int> acc, const Card& c) {
            if (c.rank == "A") { // Ace handling
                acc.second += 1; // Count Aces
            } else {
                acc.first += c.value; // Sum non-Ace values
            }
            return acc;
        }
    );

    // Unpack results
    int initial = result.first; // Sum of non-Ace values
    int ace_cnt = result.second; // Number of Aces
    int score = initial; // Start with non-Ace sum

    // Add Aces as 11 initially
    score += ace_cnt * 11;

    // Adjust Ace values
    while (score > 21 && ace_cnt > 0) { // While busting and have Aces to adjust
        score -= 10; // Change 11 to 1 (11 - 1 = 10 difference)
        ace_cnt--; // One less Ace to adjust
    }

    // Final score
    return score; // Final score
}

// Checks if a hand is a natural Blackjack (21 with 2 cards).
bool is_nat(const Hand& hand) {
    // Natural only if 2 cards and not a split hand
    return calcScr(hand) == 21 && hand.cards.size() == 2 && !hand.isplit;
}

// Deck Management
// Creates a standard deck with the specified number of decks.
void createDk(int num_dk) { // Number of decks to create
    deck.clear(); // Clear existing deck
    while(!disPile.empty()) disPile.pop(); // Clear discard pile

    // Nested loops to create the deck(s)
    for (int d = 0; d < num_dk; ++d) {
        // Nested iteration over map and list
        for (const auto& r_pair : CRDVALS) {  // r_pair: (rank, value)
            for (const std::string& suit : SUITS) {
                Card c = {r_pair.first, suit, r_pair.second}; // Create card
                deck.push_back(c); // Add to deck
            }
        }
    }
}

// Shuffles the deck using a custom algorithm with list iterators
void shufDk() {
    if (deck.empty()) return; // Don't shuffle an empty deck

    // Temporary list for shuffling
    std::list<Card> tmpDk;
    tmpDk.splice(tmpDk.begin(), deck); // Efficiently moves all elements

    // Random number generator setup
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 g(seed);

    int n = tmpDk.size(); // Number of cards
    if (n == 0) return;
    
    // Custom list-based shuffle
    for (int i = 0; i < n * 2; ++i) { // Shuffle iterations n*2
        auto it_from = tmpDk.begin(); // Iterator to select card
        // Iterator: std::advance to a random position
        std::advance(it_from, g() % n);

        // Remove card from current position
        Card c = *it_from; // Copy card
        it_from = tmpDk.erase(it_from); // Erase returns iterator to next element
        
        // Insert card at new random position
        int trgPos = g() % n; // Target position
        auto it_to = tmpDk.begin(); // Iterator to insert position
        std::advance(it_to, trgPos); // Move to target position

        // Insert card
        tmpDk.insert(it_to, c);
    }
    // Move shuffled cards back to deck
    deck.splice(deck.begin(), tmpDk);
}

// Deals a card from the deck to the hand, reshuffling if necessary
// Uses list iterators for deck management
void dealCrd(std::list<Card>& trgLst) { // Target list to receive card
    // Reshuffle if deck is empty
    if (deck.empty()) {
        std::cout << "\n--- Reshuffling Discard Pile ---\n";
        
        // Move cards from stack to list for reshuffling
        while (!disPile.empty()) { // While discard pile not empty
            deck.push_back(disPile.top()); // List push_back
            disPile.pop(); // Stack pop
        }
        shufDk(); // Shuffle the deck
        if (deck.empty()) { // Still empty after reshuffle
             throw std::runtime_error("No cards left to deal or shuffle!");
        }
    }
    
    // Iterator: deck.begin() gives an iterator to the first element
    trgLst.push_back(*deck.begin());
    deck.pop_front();
}

// Moves all cards from a Hand to the discard pile.
void discHnd(Hand& hand) {
    // STL Algorithm: std::for_each to iterate and push to stack
    std::for_each(hand.cards.begin(), hand.cards.end(), [](const Card& c) {
        disPile.push(c); // Stack push
    });
    hand.cards.clear(); // List clear
    hand.bet = 0; // Reset bet
}

// Display Functions
// Print hand, optionally hiding second card

void prntHnd(const Hand& hand, bool hide_1 = false) {
    std::cout << "[ "; // Start hand display
    int count = 0; // Card counter
    
    // Iterator: std::list::const_iterator
    for (auto it = hand.cards.cbegin(); it != hand.cards.cend(); ++it) {
        // Hide second card if specified
        if (hide_1 && count == 1) {
            // Hidden card representation
            std::cout << "XX ";
        } else {
            std::cout << *it << " "; // Use overloaded operator
        }
        count++; // Increment card counter
    }
    std::cout << "]";
}

// Print player statistics and hands
void prntStat(const Player& p, bool inclHnd = true) {
    // Player header display with chips
    std::cout << "\n**Player " << p.id << " (" << p.name << ")** - Chips: $" << p.chips << "\n";
    // Include the hand details
    if (inclHnd) {
        int hndCnt = 1; // Hand counter
        // Iterator: Iterate over all hands
        for (const auto& hand : p.hands) {
            int score = calcScr(hand); // Calculate hand score
            std::cout << "  Hand " << hndCnt++; // Hand number
            if (hand.isplit) std::cout << " (Split)"; // Indicate split hand
            if (hand.ddown) std::cout << " (DD)"; // Indicate double down hand
            std::cout << " Bet: $" << hand.bet << " Score: (" << score << "): ";
            prntHnd(hand, false); // Print hand cards
            std::cout << "\n";
        }
    }
}

// Game Logic Functions

// Handles the "Hit" action for a specific hand.
// Reference to hand to modify
void playHit(Hand& hand) {
    std::cout << "Player hits. Dealing card.\n";
    dealCrd(hand.cards); // Deal card to hand
}

// Handles the "Split" action for a player.
// Modifies the player's hands list and the current hand iterator.
void playSplt(Player& p, std::list<Hand>::iterator& curIt) {
    Hand& origHnd = *curIt; // Reference to the original hand
    
    // Check if splitting is valid (two cards of the same rank)
    if (origHnd.cards.size() != 2 || origHnd.cards.front().rank != origHnd.cards.back().rank) { // Invalid split
        std::cout << "Cannot split this hand.\n";
        return;
    }

    // Check if player has enough chips to place the second bet
    if (p.chips < origHnd.bet) {
        std::cout << "Not enough chips to place a second bet for splitting.\n";
        return;
    }

    // Proceed with split
    std::cout << "Splitting Hand. Placing additional $" << origHnd.bet << " bet.\n";

    // Create the new hand (the second split hand)
    Hand newHnd; // New hand for the split
    newHnd.bet = origHnd.bet; // Same bet as original hand
    newHnd.isplit = true; // Mark as split hand
    
    // Move the second card from the original hand to the new hand
    // Iterator: Get iterator to the second card (list::begin() and advance)
    auto scndIt = origHnd.cards.begin();
    std::advance(scndIt, 1); // Move to second card
    
    // Move card to new hand
    newHnd.cards.push_back(*scndIt);
    origHnd.cards.erase(scndIt); // Remove from original hand

    // Insert the new hand *after* the original hand in the player's hands list
    // This allows the player to play the hands sequentially (original first, then new)
    curIt++; // Move iterator to the position AFTER the original hand
    p.hands.insert(curIt, newHnd); // Insert new hand
    curIt--; // Move iterator back to point to the original hand for continued play
    
    // Update chip count and deal second cards
    p.chips -= newHnd.bet;
    
    // Deal the second card to the original hand
    dealCrd(origHnd.cards);
    
    // Deal the second card to the new hand (now located one position forward)
    curIt++; // Iterator to the new hand
    dealCrd(curIt->cards);
    curIt--; // Iterator back to the original hand

    std::cout << "Split successful. Playing the first hand...\n";
}

// Handles the "Double Down" action for a player.
void playDD(Player& p, Hand& hand) { // Reference to player and hand
    // Check if Double Down is valid (only on initial two cards)
    if (hand.cards.size() != 2) { // If not initial two cards
        std::cout << "Double Down only allowed on initial two cards.\n";
        return;
    }
    if (p.chips < hand.bet) { // If not enough chips
        std::cout << "Not enough chips to Double Down.\n";
        return;
    }

    // Proceed with Double Down and output
    std::cout << "Player Doubles Down! Betting an additional $" << hand.bet << ".\n";
    p.chips -= hand.bet; // Deduct additional bet
    hand.bet *= 2; // Double the bet
    hand.ddown = true; // Mark hand as double down
    
    // Player gets exactly one card
    playHit(hand);
    
    // Show final hand and output score
    std::cout << "Final Hand Score: (" << calcScr(hand) << ")\n";
    prntHnd(hand);
    std::cout << "\n";
}

// Processes the outcome of a single hand against the dealer.
// Updates player chips based on the result.
// References to player, player's hand, and dealer's hand
void setHnd(Player& p, Hand& hand, const Hand& dlHnd) {
    int p_score = calcScr(hand); // Player's hand score
    int d_score = calcScr(dlHnd); // Dealer's hand score

    // Output settlement header
    std::cout << "\n--- Settlement for " << p.name << "'s hand (Score: " << p_score << ") ---\n";

    // Player bust
    if (p_score > 21) {  // If score is over 21
        std::cout << "Player BUSTS. Bet of $" << hand.bet << " lost.\n";
        // Chips already deducted at bet time
    }
    // Both have naturals
    else if (is_nat(hand) && is_nat(dlHnd)) {
        std::cout << "PUSH (Natural vs. Natural). Bet of $" << hand.bet << " returned.\n";
        p.chips += hand.bet; // Return original bet
    }
    // Natural blackjack for player
    else if (is_nat(hand)) {
        int winAmt = static_cast<int>(hand.bet * 1.5); // 1.5x winnings
        std::cout << "NATURAL BLACKJACK! Wins 1.5x. $" << winAmt << " won (Total return: $" << hand.bet + winAmt << ").\n";
        p.chips += hand.bet + winAmt; // Return original bet + winnings
    }
    // Dealer bust
    else if (d_score > 21) { // If score is over 21 for dealer
        std::cout << "Dealer BUSTS (" << d_score << "). Player wins $" << hand.bet << ".\n";
        p.chips += hand.bet * 2; // Return original bet + winnings
    }
    // Dealer has natural blackjack
    else if (is_nat(dlHnd)) {
        std::cout << "Dealer has NATURAL BLACKJACK. Bet of $" << hand.bet << " lost.\n";
    }
    // Compare scores
    else if (p_score > d_score) { // Player wins
        std::cout << "Player Wins (" << p_score << " > " << d_score << "). Wins $" << hand.bet << ".\n";
        p.chips += hand.bet * 2; // Return original bet + winnings
    }
    else if (p_score < d_score) { // Dealer wins
        std::cout << "Dealer Wins (" << d_score << " > " << p_score << "). Bet of $" << hand.bet << " lost.\n";
    }
    else { // Push
        std::cout << "PUSH (" << p_score << " vs. " << d_score << "). Bet of $" << hand.bet << " returned.\n";
        p.chips += hand.bet; // Return original bet
    }
    
    discHnd(hand);
}


// Game Logic Functions

// Handles the main player decision phase (Hit, Stand, Split, Double Down).
void hdlPlay(Player& p, Hand& dlHnd) {
    std::string choice; // Player choice input
    
    // Use a while loop with an iterator to manage the list of hands,
    // allowing for insertion (splitting) and safe iteration.
    auto it = p.hands.begin(); // Iterator to current hand
    while (it != p.hands.end()) { // While there are hands to play
        Hand& curHnd = *it; // Reference to current hand
        bool done = false; // Flag to indicate if done playing this hand

        // Output current hand header
        std::cout << "\n--- " << p.name << "'s Turn (Hand Bet: $" << curHnd.bet << ") ---\n";
        
        // Skip hands that were just completed by a Double Down
        if (curHnd.ddown) {
            ++it; // Move to the next hand
            continue;
        }

        // Handle a split pair of Aces
        if (curHnd.isplit && curHnd.cards.size() == 2 && curHnd.cards.front().rank == "A" && curHnd.cards.back().rank == "A") {
            std::cout << "Split Aces: Only one card is dealt to each. Must stand.\n";
            ++it; // Move to the next hand after standing
            continue;
        }

        // Inner loop for playing the current hand
        bool split = false;

        // Loop until the player stands, busts, or completes the hand
        while (!done) {
            int score = calcScr(curHnd); // Calculate current hand score
            std::cout << "Current Hand Score (" << score << "): ";
            prntHnd(curHnd); // Print current hand
            std::cout << "\n";

            // Check for bust or 21
            if (score > 21) { // If score exceeds 21
                std::cout << "Hand Busted!\n";
                break;
            }
            if (score == 21) { // If score is exactly 21
                std::cout << "Hand is 21! Standing.\n";
                break;
            }
            
            // Prompt for action
            std::cout << "Actions: (H)it / (S)tand";
            
            // Check if Split and Double Down are available
            // Can split if two cards of same rank and not already a split hand
            bool canSplt = (curHnd.cards.size() == 2 && curHnd.cards.front().rank == curHnd.cards.back().rank && !curHnd.isplit);
            bool canDbl = (curHnd.cards.size() == 2 && p.chips >= curHnd.bet); // Can double down if two cards and enough chips
            
            // Display available actions
            if (canSplt) std::cout << " / (P)lit"; // 'P' for sPlit to avoid confusion with 'S'tand
            if (canDbl) std::cout << " / (D)ouble Down"; // 'D' for Double Down
            std::cout << "\nChoose action: ";
            
            std::cout << " > ";

            // Player chooses action
            std::cin >> choice;

            // Standardize input to uppercase
            std::transform(choice.begin(), choice.end(), choice.begin(), ::toupper);
            clear_in();
            
            // Handle player choice
            if (choice == "H") { // Hit
                playHit(curHnd); // Deal card to hand
            } else if (choice == "S") { // Stand
                done = true; // End turn for this hand
            } else if (choice == "D" && canDbl) { // Double Down
                playDD(p, curHnd); // Handle Double Down
                done = true; // Double Down ends the turn for this hand
            } else if (choice == "P" && canSplt) {
                // The player_split function modifies the list and the iterator 'it'
                playSplt(p, it);
                split = true; // Mark that a split occurred
                break;
            } else {
                std::cout << "Invalid or unavailable action.\n"; // Prompt again
            }
        } // end while (!done_playing)
        
        // Handle iterator progression after hand completion
        if (split) {
            // After a split, stay on the current iterator to play the new hand next
            // 'it' already points to the original hand; increment to the new hand
        } else {
            // Normal progression: move to the next hand in the list
            ++it;
        }

    } // end while (it != p.hands.end())
}

// Plays a single round of Blackjack for all players and the dealer.
void playRnd(std::list<Player>& plyrs, Player& dealr) {
    
    // Bets and Initial Deal
    std::cout << "\n" << std::string(50, '=') << "\n"; // Round header
    std::cout << "                NEW ROUND STARTING\n";
    std::cout << std::string(50, '=') << "\n";

    // Reshuffle check
    if (deck.size() < 60) { // Threshold for reshuffle
        std::cout << "Deck size (" << deck.size() << ") is low. Performing full reshuffle.\n";
        createDk(4); // Recreate deck with 4 decks
        shufDk(); // Shuffle the deck
    }
    //  Store initial chips for all players (STL Map)
    std::map<std::string, int> initChp;

    // STL Algorithm: std::for_each to populate the map
    std::for_each(plyrs.cbegin(), plyrs.cend(), [&](const Player& p) {
        initChp[p.name] = p.chips; // Map insert/assignment
    });
    
    // Place Bets and set up Turn Queue (STL Container: std::queue)
    std::for_each(plyrs.begin(), plyrs.end(), [&](Player& p) {
        int betAmt = 0; // Bet amount input

        // Prompt for bet until valid
        while (betAmt < 1 || betAmt > p.chips) { // Invalid bet
            std::cout << p.name << " (Chips: $" << p.chips << "), place your bet: ";
            std::cin >> betAmt; // Input bet amount
            clear_in();
            if (betAmt < 1 || betAmt > p.chips) { // Invalid bet
                std::cout << "Invalid bet. Must be between $1 and $" << p.chips << ".\n";
            }
        }
        // Set up player's initial hand and deduct chips
        p.hands.emplace_back(); // Add initial hand
        p.hands.front().bet = betAmt; // Set bet for the hand
        p.chips -= betAmt; // Deduct bet from chips
        playQue.push(&p); // Add player to the turn order queue
    });
    
    // Initial Deal (Player, Dealer, Player, Dealer)
    std::cout << "\n--- Initial Deal ---\n";
    // Deal card 1 to all players (in order)
    std::queue<Player*> tempQ = playQue; // Copy queue for iteration
    while (!tempQ.empty()) { // While queue not empty
        dealCrd(tempQ.front()->hands.front().cards); // Deal to player's first hand
        tempQ.pop(); // Remove from temp queue
    }
    // Deal card 1 to dealer
    dealCrd(dealr.hands.front().cards);
    
    // Deal card 2 to all players
    tempQ = playQue;
    while (!tempQ.empty()) { // While queue not empty
        dealCrd(tempQ.front()->hands.front().cards); // Deal to player's first hand
        tempQ.pop(); // Remove from temp queue
    }
    // Deal card 2 to dealer
    dealCrd(dealr.hands.front().cards); // Dealer's hole card

    // Display initial hands
    std::cout << "\nDealer's upcard: ";
    prntHnd(dealr.hands.front(), true); // Hide the second card
    std::cout << "\n";
    
    // Check for naturals
    Hand& dealrH = dealr.hands.front(); // Dealer's hand
    bool dealrNat = is_nat(dealrH); // Check if dealer has natural

    if (dealrNat) {
        std::cout << "\n**DEALER NATURAL BLACKJACK!**\n";
    }

    // Player Actions Phase
    while (!playQue.empty()) { // While there are players to process
        Player* p = playQue.front(); // Queue: front()
        playQue.pop(); // Queue: pop()
        
        // If dealer has natural, only check for push, otherwise players play
        if (!dealrNat) {
            hdlPlay(*p, dealrH);
        } else {
            std::cout << "\n" << p->name << ": Dealer has a Natural. Skip action phase.\n";
        }
    }
    
    // Dealer Play
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "               DEALER'S PLAY\n";
    std::cout << std::string(50, '-') << "\n";
    
    int d_score = calcScr(dealrH); // Dealer's initial score
    std::cout << "Dealer reveals hole card. Full Hand (" << d_score << "): ";
    prntHnd(dealrH); // Print dealer's full hand
    std::cout << "\n";

    if (!dealrNat) { // Only play if dealer doesn't have natural
        while (d_score < 17) { // Dealer hits on soft 17
            std::cout << "Dealer Hits (score < 17).\n";
            dealCrd(dealrH.cards); // Deal card to dealer
            d_score = calcScr(dealrH); // Recalculate score
            std::cout << "Dealer's Hand (" << d_score << "): ";
            prntHnd(dealrH); // Print dealer's hand
            std::cout << "\n";
        }
        std::cout << "Dealer Stands at " << d_score << ".\n";
    }

    // Final Settlement Phase
    std::cout << "\n" << std::string(50, '-') << "\n";
    std::cout << "               FINAL SETTLEMENT\n";
    std::cout << std::string(50, '-') << "\n";

    // STL Algorithm: std::for_each to iterate over all players
    std::for_each(plyrs.begin(), plyrs.end(), [&](Player& p) {
        // Iterator: Iterate over all hands a player might have (original + split hands)
        for (auto& hand : p.hands) { // For each hand
            if (hand.bet > 0) { // Only settle hands that were bet on
                setHnd(p, hand, dealrH); // Settle the hand
            } else {
                discHnd(hand); // Clean up empty hands if any somehow remain
            }
        }
        // Cleanup: Use std::list::remove_if to clean up all empty hands
        p.hands.remove_if([](const Hand& h){ // Lambda to check if hand is empty
            return h.cards.empty(); // Remove if empty
        });
    });

    // Discard dealer's hand
    discHnd(dealrH);
}

// Main Game Loop
void runGame() {
    std::list<Player> plyrs; // List of players
    Player dealr = {0, "Dealer", 0}; // Dealer player
    dealr.hands.emplace_back(); // Dealer always has one hand

    std::cout << "### Welcome to Blackjack Casino ###\n";
    
    // Setup Players
    int numPlay = 0; // Number of players input
    while (numPlay < 1 || numPlay > 3) {
        std::cout << "Enter number of players (1-3): ";
        std::cin >> numPlay; // Input number of players
        clear_in();
    }

    for (int i = 1; i <= numPlay; ++i) { // Get player names
        std::string name; // Player name input
        std::cout << "Enter name for Player " << i << ": ";
        std::getline(std::cin, name); // Input player name
        plyrs.emplace_back(Player{i, name, 1000}); // Add player to list
    }

    // Initial Deck Setup
    createDk(4); // Create deck with 4 standard decks
    shufDk(); // Shuffle the deck

    // Play Again Loop
    std::string playAgn = "Y";

    while (playAgn == "Y") { // While player wants to play again
        try {
            // Check for players who are out of money
            // STL Algorithm: std::remove_if (using lambda) to manage player list
            plyrs.remove_if([&](const Player& p) { // Lambda to check if player is out of chips
                if (p.chips < 1) { // If player has no chips
                    std::cout << "\n" << p.name << " is out of chips and leaves the game.\n";
                    return true;
                }
                return false; // Keep player otherwise
            });
            
            // Check if any players remain
            if (plyrs.empty()) { // No players left
                std::cout << "\nAll players are out of chips. Game Over.\n";
                break;
            }
            
            // Check if dealer's hand needs reset
            if (!dealr.hands.empty() && dealr.hands.front().bet > 0) { // If dealer's hand has cards
                 discHnd(dealr.hands.front()); // Discard dealer's hand
            } else if (dealr.hands.empty()) { // If dealer has no hands
                dealr.hands.emplace_back(); // Create dealer's hand
            }

            // Play a round of Blackjack
            playRnd(plyrs, dealr);

        } catch (const std::exception& e) { // Catch any critical errors
            std::cerr << "CRITICAL GAME ERROR: " << e.what() << "\n";
            break;
        }

        // Round Summary after each round
        std::cout << "\n" << std::string(50, '*') << "\n";
        std::cout << "Round Summary:\n"; // Summarize player chips

        // STL Algorithm: std::for_each to display final chips
        std::for_each(plyrs.begin(), plyrs.end(), [](const Player& p) {
            prntStat(p, false); // Print player stats without hands

        });

        // Prompt to play again or not
        std::cout << "\n" << std::string(50, '*') << "\n";
        std::cout << "Play another round? (Y/N): ";
        std::cin >> playAgn; // Input choice
        std::transform(playAgn.begin(), playAgn.end(), playAgn.begin(), ::toupper);

        // Clear input buffer
        clear_in();

    }

    // If player chooses not to play again or all players are out
    std::cout << "\nThank you for playing Blackjack. Final Chip Counts:\n";
    std::for_each(plyrs.begin(), plyrs.end(), [](const Player& p) { // Final stats
        prntStat(p, false); // Print player stats without hands
    });

    std::cout << "Goodbye!\n";
}

int main (int argc, char** argv) {
    // Set Random Number Seed Here (System clock used in shuffle_deck)
    
    // Declare all Variables Here (Done within run_game_loop)
    
    // Input or initialize values Here
    
    // Process/Calculations Here
    // Setting fixed point notation for chips display
    std::cout << std::fixed << std::setprecision(0);

    runGame();
    
    // Output Located Here

    // Exit
    return 0;
}
