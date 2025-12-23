#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;

// ======================================================
// STRUCTS
// ======================================================

// Holds stats for a single game
struct GameStats {
    string date;    // YYYY-MM-DD (use this format so lexical sort by date works)
    int points;
    int rebounds;
    int assists;
    int steals;
    int blocks;

    int fgm, fga;   // Field Goals Made/Attempted
    int threem, threea; // 3-point Made/Attempted
    int ftm, fta;   // Free Throws Made/Attempted

    // A simple constructor for convenience
    GameStats() : date(""), points(0), rebounds(0), assists(0), steals(0), blocks(0),
        fgm(0), fga(0), threem(0), threea(0), ftm(0), fta(0) {
    }
};

// Holds a player's name and all their games
struct Player {
    string name;
    vector<GameStats> games;
};

// ======================================================
// HELPER I/O UTILITIES
// ======================================================

// Read an entire line after using >> for ints; used to sync newline
void clearInputLine() {
    string dummy;
    getline(cin, dummy);
}

// Read an integer safely with prompt
int readInt(const string& prompt) {
    int x;
    while (true) {
        cout << prompt;
        if (cin >> x) {
            clearInputLine();
            return x;
        }
        else {
            cout << "Invalid integer. Try again.\n"<<endl;
            cin.clear();
            clearInputLine();
        }
    }
}

// Read a string line (including spaces)
string readLine(const string& prompt) {
    cout << prompt;
    string s;
    getline(cin, s);
    return s;
}

// ======================================================
// CALCULATION HELPERS
// ======================================================

// Field goal % (returns double, 0 if attempts = 0)
double pct(int made, int att) {
    if (att == 0) return 0.0;
    return (double)made / att * 100.0;
}

// Simple classroom-style Player Efficiency Rating (PER)
// This is NOT the NBA's PER; it's a simplified, transparent formula useful for learning.
// Example formula used here:
//   raw = points + rebounds + assists + steals + blocks
//         - ( (fga - fgm) + (fta - ftm) )   // punishment for missed shots
// Then normalized by games (if passed gamesCount) to get per-game value.
double simplePER(const Player& p) {
    if (p.games.empty()) return 0.0;
    double totalRaw = 0.0;
    for (const auto& g : p.games) {
        double raw = g.points + g.rebounds + g.assists + g.steals + g.blocks;
        raw -= ((g.fga - g.fgm) + (g.fta - g.ftm)); // penalty for misses
        totalRaw += raw;
    }
    return totalRaw / (double)p.games.size();
}

// ======================================================
// PLAYER & GAME OPERATIONS
// ======================================================

// Add a new player and return its index in players vector
int addPlayer(vector<Player>& players) {
    string name = readLine("Enter new player's full name: ");
    if (name.empty()) {
        cout << "Player name cannot be empty.\n"<<endl;
        return -1;
    }
    // Check for duplicate names - optional
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i].name == name) {
            cout << "Player already exists at index " << i + 1 << ".\n"<<endl;
            return (int)i;
        }
    }

    Player p;
    p.name = name;
    players.push_back(p);
    cout << "Player '" << name << "' added (index " << players.size() << ").\n"<<endl;
    return (int)players.size() - 1;
}

// Select a player by showing a menu, returns index or -1 if none
int selectPlayer(const vector<Player>& players) {
    if (players.empty()) {
        cout << "No players available. Add a player first.\n"<<endl;
        return -1;
    }
    cout << "\nPlayers:\n"<<endl;
    for (size_t i = 0; i < players.size(); ++i) {
        cout << (i + 1) << ". " << players[i].name << " (" << players[i].games.size() << " games)\n"<<endl;
    }
    int choice = readInt("Select player number (0 to cancel): ");
    if (choice == 0) return -1;
    if (choice < 1 || choice >(int)players.size()) {
        cout << "Invalid selection.\n"<<endl;
        return -1;
    }
    return choice - 1;
}

// Enter a single game's stats interactively and append to player's games
void enterGameForPlayer(Player& p) {
    GameStats g;
    cout << "\nEntering new game for " << p.name << ". Use YYYY-MM-DD for date.\n"<<endl;
    g.date = readLine("Date (YYYY-MM-DD): ");
    g.points = readInt("Points: ");
    g.rebounds = readInt("Rebounds: ");
    g.assists = readInt("Assists: ");
    g.steals = readInt("Steals: ");
    g.blocks = readInt("Blocks: ");
    g.fgm = readInt("Field goals made (FGM): ");
    g.fga = readInt("Field goals attempted (FGA): ");
    g.threem = readInt("3-pointers made (3PM): ");
    g.threea = readInt("3-pointers attempted (3PA): ");
    g.ftm = readInt("Free throws made (FTM): ");
    g.fta = readInt("Free throws attempted (FTA): ");

    // Basic validation: ensure subcounts don't exceed totals
    if (g.threem > g.fgm) {
        cout << "Warning: 3PM > FGM. Adjusting FGM to be at least 3PM.\n";
        g.fgm = g.threem;
    }

    p.games.push_back(g);
    cout << "Game added for " << p.name << " (" << g.date << ").\n"<<endl;
}

// Edit an existing game for a player by index (1-based shown to user)
void editGame(Player& p) {
    if (p.games.empty()) { cout << "No games to edit.\n"; return; }

    cout << "\nGames for " << p.name << ":\n"<<endl;
    for (size_t i = 0; i < p.games.size(); ++i) {
        cout << (i + 1) << ". " << p.games[i].date << " - " << p.games[i].points << " pts\n";
    }
    int idx = readInt("Enter game number to edit (0 to cancel): ");
    if (idx == 0) return;
    if (idx < 1 || idx >(int)p.games.size()) { cout << "Invalid game number.\n"; return; }

    GameStats& g = p.games[idx - 1];
    cout << "Editing Game " << idx << " (" << g.date << "). Press enter to keep current value.\n";

    // Helper lambda: read an int or keep current by empty line
    auto readIntKeep = [&](const string& prompt, int& field) {
        cout << prompt << " [" << field << "]: ";
        string line; getline(cin, line);
        if (line.empty()) return; // keep current
        stringstream ss(line);
        int v; if (ss >> v) field = v;
        else cout << "Invalid input; keeping previous value.\n"<<endl;
        };

    cout << "Date (current " << g.date << "): "<<endl;
    string newDate; getline(cin, newDate);
    if (!newDate.empty()) g.date = newDate;

    readIntKeep("Points", g.points);
    readIntKeep("Rebounds", g.rebounds);
    readIntKeep("Assists", g.assists);
    readIntKeep("Steals", g.steals);
    readIntKeep("Blocks", g.blocks);
    readIntKeep("FGM", g.fgm);
    readIntKeep("FGA", g.fga);
    readIntKeep("3PM", g.threem);
    readIntKeep("3PA", g.threea);
    readIntKeep("FTM", g.ftm);
    readIntKeep("FTA", g.fta);

    cout << "Game updated.\n"<<endl;
}

// Delete a game by number (1-based)
void deleteGame(Player& p) {
    if (p.games.empty()) { cout << "No games to delete.\n"; return; }

    cout << "\nGames for " << p.name << ":\n"<<endl;
    for (size_t i = 0; i < p.games.size(); ++i) {
        cout << (i + 1) << ". " << p.games[i].date << " - " << p.games[i].points << " pts\n";
    }
    int idx = readInt("Enter game number to delete (0 to cancel): ");
    if (idx == 0) return;
    if (idx < 1 || idx >(int)p.games.size()) { cout << "Invalid game number.\n"; return; }

    string confirm = readLine("Type 'DELETE' to confirm deletion: ");
    if (confirm == "DELETE") {
        p.games.erase(p.games.begin() + (idx - 1));
        cout << "Game deleted.\n";
    }
    else {
        cout << "Deletion cancelled.\n"<<endl;
    }
}

// ======================================================
// SORTING FUNCTIONS
// ======================================================

// Sort a player's games by date (ascending). Assumes date strings are YYYY-MM-DD
void sortGamesByDate(Player& p) {
    sort(p.games.begin(), p.games.end(), [](const GameStats& a, const GameStats& b) {
        return a.date < b.date;
        });
    cout << "Games sorted by date (oldest -> newest).\n"<<endl;
}

// Sort a player's games by points (descending)
void sortGamesByPoints(Player& p) {
    sort(p.games.begin(), p.games.end(), [](const GameStats& a, const GameStats& b) {
        return a.points > b.points;
        });
    cout << "Games sorted by points (highest -> lowest).\n"<<endl;
}

// ======================================================
// STATS REPORTS: totals, averages, best game, ASCII chart
// ======================================================

// Show totals and shooting percentages
void showTotals(const Player& p) {
    if (p.games.empty()) { cout << "No games to report.\n"<<endl; return; }

    int totalPts = 0, totalReb = 0, totalAst = 0, totalStl = 0, totalBlk = 0;
    int totalFGM = 0, totalFGA = 0, total3M = 0, total3A = 0, totalFTM = 0, totalFTA = 0;

    for (const auto& g : p.games) {
        totalPts += g.points;
        totalReb += g.rebounds;
        totalAst += g.assists;
        totalStl += g.steals;
        totalBlk += g.blocks;
        totalFGM += g.fgm;
        totalFGA += g.fga;
        total3M += g.threem;
        total3A += g.threea;
        totalFTM += g.ftm;
        totalFTA += g.fta;
    }

    cout << fixed << setprecision(2);
    cout << "\n=== TOTALS for " << p.name << " ===\n"<<endl;
    cout << "Games: " << p.games.size() << "\n" << endl;
    cout << "Points: " << totalPts << "\n" << endl;
    cout << "Rebounds: " << totalReb << "\n" << endl;
    cout << "Assists: " << totalAst << "\n" << endl;
    cout << "Steals: " << totalStl << "\n" << endl;
    cout << "Blocks: " << totalBlk << "\n" << endl;
    cout << "FG%: " << pct(totalFGM, totalFGA) << "% (" << totalFGM << "/" << totalFGA << ")\n" << endl;
    cout << "3P%: " << pct(total3M, total3A) << "% (" << total3M << "/" << total3A << ")\n" << endl;
    cout << "FT%: " << pct(totalFTM, totalFTA) << "% (" << totalFTM << "/" << totalFTA << ")\n" << endl;
}

// Show per-game averages
void showAverages(const Player& p) {
    if (p.games.empty()) { cout << "No games to report.\n"; return; }

    double totalPts = 0, totalReb = 0, totalAst = 0, totalStl = 0, totalBlk = 0;
    for (const auto& g : p.games) {
        totalPts += g.points;
        totalReb += g.rebounds;
        totalAst += g.assists;
        totalStl += g.steals;
        totalBlk += g.blocks;
    }
    cout << fixed << setprecision(2);
    cout << "\n=== AVERAGES for " << p.name << " ===\n" << endl;
    cout << "PPG: " << totalPts / p.games.size() << "\n" << endl;
    cout << "RPG: " << totalReb / p.games.size() << "\n" << endl;
    cout << "APG: " << totalAst / p.games.size() << "\n" << endl;
    cout << "SPG: " << totalStl / p.games.size() << "\n" << endl;
    cout << "BPG: " << totalBlk / p.games.size() << "\n" << endl;
    cout << "Simple PER: " << simplePER(p) << "\n" << endl;
}

// Find and show the best scoring game(s)
void showBestScoringGames(const Player& p) {
    if (p.games.empty()) { cout << "No games to report.\n"; return; }
    int bestPts = p.games[0].points;
    for (const auto& g : p.games) bestPts = max(bestPts, g.points);

    cout << "\n=== Best Scoring Game(s): " << bestPts << " pts ===\n";
    for (size_t i = 0; i < p.games.size(); ++i) {
        if (p.games[i].points == bestPts) {
            auto& g = p.games[i];
            cout << (i + 1) << ". " << g.date << " - " << g.points << " pts, "
                << "FG%=" << fixed << setprecision(1) << pct(g.fgm, g.fga) << "%, "
                << "3P=" << pct(g.threem, g.threea) << "%\n";
        }
    }
}

// ASCII bar chart of points per game. Each '*' represents 2 points (adjust scale if desired)
void showAsciiChart(const Player& p) {
    if (p.games.empty()) { cout << "No games to chart.\n"; return; }
    cout << "\n=== ASCII Chart: Points per Game (each '*' = 2 points) ===\n" << endl;
    for (size_t i = 0; i < p.games.size(); ++i) {
        int stars = (int)round(p.games[i].points / 2.0);
        cout << setw(3) << (i + 1) << " [" << p.games[i].date << "] "
            << setw(3) << p.games[i].points << " | ";
        for (int s = 0; s < stars; ++s) cout << '*';
        cout << '\n';
    }
}

// ======================================================
// FILE I/O: Save/Load all players, Export CSV for a player
// ======================================================

// Save all players and their games to a text file in a simple format.
// Format:
// <numPlayers>
// For each player:
//   <name>
//   <numGames>
//   For each game: date points rebounds assists steals blocks fgm fga threem threea ftm fta
void saveAllPlayersToFile(const vector<Player>& players, const string& filename = "players_data.txt") {
    ofstream out(filename);
    if (!out) {
        cout << "Error opening '" << filename << "' for writing.\n" << endl;
        return;
    }
    out << players.size() << '\n';
    for (const auto& p : players) {
        // Escape newline issues by writing name on a single line (no internal newlines allowed)
        out << p.name << '\n';
        out << p.games.size() << '\n';
        for (const auto& g : p.games) {
            // Write each field separated by spaces; date stays as string (YYYY-MM-DD)
            out << g.date << ' '
                << g.points << ' '
                << g.rebounds << ' '
                << g.assists << ' '
                << g.steals << ' '
                << g.blocks << ' '
                << g.fgm << ' ' << g.fga << ' '
                << g.threem << ' ' << g.threea << ' '
                << g.ftm << ' ' << g.fta << '\n';
        }
    }
    out.close();
    cout << "Saved all players to '" << filename << "'.\n" << endl;
}

// Load players from file created by saveAllPlayersToFile
void loadAllPlayersFromFile(vector<Player>& players, const string& filename = "players_data.txt") {
    ifstream in(filename);
    if (!in) {
        cout << "No saved file '" << filename << "' found.\n" << endl;
        return;
    }
    players.clear();
    size_t numPlayers = 0;
    in >> numPlayers;
    clearInputLine(); // consume '\n' after number
    for (size_t i = 0; i < numPlayers; ++i) {
        Player p;
        p.name = readLine(">> (loading) temporary read to sync - not used"); // workaround - we'll actually read from stream
        // The above line is a placeholder to align with console IO; we'll instead read directly from 'in'
    }
    // Reset and do proper reading without mixing cin/getline and ifstream
    in.clear();
    in.seekg(0);
    in >> numPlayers;
    string dummy;
    getline(in, dummy); // consume rest of first line

    for (size_t i = 0; i < numPlayers; ++i) {
        Player p;
        getline(in, p.name); // player's name line
        size_t numGames = 0;
        in >> numGames;
        getline(in, dummy); // consume newline
        for (size_t j = 0; j < numGames; ++j) {
            GameStats g;
            in >> g.date
                >> g.points
                >> g.rebounds
                >> g.assists
                >> g.steals
                >> g.blocks
                >> g.fgm >> g.fga
                >> g.threem >> g.threea
                >> g.ftm >> g.fta;
            getline(in, dummy); // consume endline
            p.games.push_back(g);
        }
        players.push_back(p);
    }
    in.close();
    cout << "Loaded " << players.size() << " players from file.\n" << endl;
}

// Export a single player's games to CSV (useful for importing into Excel)
void exportPlayerToCSV(const Player& p, const string& filename) {
    ofstream out(filename);
    if (!out) {
        cout << "Error opening '" << filename << "' for CSV export.\n" << endl;
        return;
    }
    // CSV header
    out << "Date,Points,Rebounds,Assists,Steals,Blocks,FGM,FGA,3PM,3PA,FTM,FTA,FG%,3P%,FT%\n";
    for (const auto& g : p.games) {
        out << g.date << ','
            << g.points << ','
            << g.rebounds << ','
            << g.assists << ','
            << g.steals << ','
            << g.blocks << ','
            << g.fgm << ','
            << g.fga << ','
            << g.threem << ','
            << g.threea << ','
            << g.ftm << ','
            << g.fta << ','
            << fixed << setprecision(2) << pct(g.fgm, g.fga) << ','
            << fixed << setprecision(2) << pct(g.threem, g.threea) << ','
            << fixed << setprecision(2) << pct(g.ftm, g.fta) << '\n';
    }
    out.close();
    cout << "Exported " << p.name << " to CSV file '" << filename << "'.\n" << endl;
}

// ======================================================
// PLAYER MENU: All per-player operations centralized here
// ======================================================
void playerMenu(Player& p) {
    int choice;
    do {
        cout << "\n=== Menu for " << p.name << " ===\n" << endl;
        cout << "1. Add a game\n" << endl;
        cout << "2. Edit a game\n" << endl;
        cout << "3. Delete a game\n" << endl;
        cout << "4. Sort games by date\n" << endl;
        cout << "5. Sort games by points\n" << endl;
        cout << "6. Show totals\n" << endl;
        cout << "7. Show averages & PER\n" << endl;
        cout << "8. Show best scoring game(s)\n" << endl;
        cout << "9. ASCII chart: points per game\n" << endl;
        cout << "10. Export player to CSV\n" << endl;
        cout << "0. Back to main menu\n" << endl;
        choice = readInt("Choice: ");

        switch (choice) {
        case 1: enterGameForPlayer(p); break;
        case 2: editGame(p); break;
        case 3: deleteGame(p); break;
        case 4: sortGamesByDate(p); break;
        case 5: sortGamesByPoints(p); break;
        case 6: showTotals(p); break;
        case 7: showAverages(p); break;
        case 8: showBestScoringGames(p); break;
        case 9: showAsciiChart(p); break;
        case 10: {
            string fname = readLine("Filename for CSV (e.g., player.csv): ");
            if (fname.empty()) fname = p.name + ".csv";
            exportPlayerToCSV(p, fname);
            break;
        }
        case 0: break;
        default: cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

// ======================================================
// MAIN MENU: Player-level and global actions
// ======================================================

int main() {
    vector<Player> players;
    int choice;

    cout << "Advanced Basketball Statistics Program (CSCI I concepts)\n" << endl;
    cout << "Features: multiple players, save/load, CSV export, edit/delete, sorting, PER, ASCII charts.\n" << endl;

    do {
        cout << "\n=== MAIN MENU ===\n" << endl;
        cout << "1. Add a player\n" << endl;
        cout << "2. Select player (open player menu)\n" << endl;
        cout << "3. Save all players to file\n" << endl;
        cout << "4. Load players from file\n" << endl;
        cout << "5. Export all players to individual CSV files\n" << endl;
        cout << "6. Quick report: list all players and averages\n" << endl;
        cout << "0. Exit\n" << endl;

        choice = readInt("Choice: ");
        switch (choice) {
        case 1:
            addPlayer(players);
            break;

        case 2: {
            int idx = selectPlayer(players);
            if (idx >= 0) playerMenu(players[idx]);
            break;
        }

        case 3:
            saveAllPlayersToFile(players);
            break;

        case 4:
            loadAllPlayersFromFile(players);
            break;

        case 5:
            // Export each player to a CSV named "<playername>.csv" (spaces replaced with underscores)
            for (const auto& p : players) {
                string fname = p.name;
                // sanitize filename: replace spaces with underscores
                replace(fname.begin(), fname.end(), ' ', '_');
                fname += ".csv";
                exportPlayerToCSV(p, fname);
            }
            cout << "All players exported to CSV files.\n" << endl;
            break;

        case 6:
            cout << "\n=== Quick Player Summary ===\n" << endl;
            for (const auto& p : players) {
                cout << p.name << " - Games: " << p.games.size() << endl;
                if (!p.games.empty()) {
                    cout << ", PPG: " << fixed << setprecision(2)
                        << ([&]() {
                        double totalPts = 0;
                        for (const auto& g : p.games) totalPts += g.points;
                        return totalPts / p.games.size();
                            }());
                    cout << ", PER: " << simplePER(p) << endl;
                }
                cout << '\n' << endl;
            }
            break;

        case 0:
            cout << "Exiting program. Tip: save your data (option 3) before quitting.\n" << endl;
            break;

        default:
            cout << "Invalid choice.\n" << endl;
        }

    } while (choice != 0);

    return 0;
}
