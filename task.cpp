#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip> // For output formatting

using namespace std;

// Load ratings matrix from a CSV file
vector<vector<int>> loadRatingsMatrix(const string &filename) {
    vector<vector<int>> matrix;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }

    string line;
    size_t rowLength = 0; // To ensure all rows have the same length
    while (getline(file, line)) {
        stringstream ss(line);
        string value;
        vector<int> row;
        while (getline(ss, value, ',')) {
            try {
                row.push_back(stoi(value));
            } catch (const invalid_argument &e) {
                cerr << "Error: Non-numeric data encountered in file." << endl;
                exit(1);
            }
        }

        // Check for consistent row lengths
        if (matrix.empty()) {
            rowLength = row.size();
        } else if (row.size() != rowLength) {
            cerr << "Error: Inconsistent row lengths in the file." << endl;
            exit(1);
        }

        if (!row.empty()) {
            matrix.push_back(row);
        }
    }

    if (matrix.empty()) {
        cerr << "Error: Ratings matrix is empty." << endl;
        exit(1);
    }

    return matrix;
}

// Calculate similarity between two users using cosine similarity
double calculateSimilarity(const vector<int> &user1, const vector<int> &user2) {
    double dotProduct = 0.0, magnitude1 = 0.0, magnitude2 = 0.0;

    for (size_t i = 0; i < user1.size(); i++) {
        dotProduct += user1[i] * user2[i];
        magnitude1 += user1[i] * user1[i];
        magnitude2 += user2[i] * user2[i];
    }

    if (magnitude1 == 0 || magnitude2 == 0) {
        return 0.0;
    }

    return dotProduct / (sqrt(magnitude1) * sqrt(magnitude2));
}

// Predict ratings for a specific user
vector<pair<int, double>> predictRatings(const vector<vector<int>> &matrix, int userIndex, int topN) {
    vector<pair<int, double>> recommendations;
    vector<double> similarities(matrix.size(), 0.0);
    const vector<int> &targetUser = matrix[userIndex];

    // Calculate similarities between the target user and all other users
    for (size_t i = 0; i < matrix.size(); i++) {
        if (i != userIndex) {
            similarities[i] = calculateSimilarity(targetUser, matrix[i]);
        }
    }

    // Predict ratings for unrated movies
    for (size_t movie = 0; movie < targetUser.size(); movie++) {
        if (targetUser[movie] == 0) { // Unrated movie
            double weightedSum = 0.0;
            double similaritySum = 0.0;

            for (size_t otherUser = 0; otherUser < matrix.size(); otherUser++) {
                if (otherUser != userIndex && matrix[otherUser][movie] > 0) {
                    weightedSum += similarities[otherUser] * matrix[otherUser][movie];
                    similaritySum += fabs(similarities[otherUser]);
                }
            }

            if (similaritySum > 0) {
                recommendations.push_back({movie, weightedSum / similaritySum});
            }
        }
    }

    // Sort recommendations by predicted rating in descending order
    sort(recommendations.begin(), recommendations.end(), [](const pair<int, double> &a, const pair<int, double> &b) {
        return a.second > b.second;
    });

    // Keep only the top N recommendations
    if (recommendations.size() > static_cast<size_t>(topN)) {
        recommendations.resize(topN);
    }

    return recommendations;
}

// Main program
int main() {
    string filename;
    cout << "Enter the filename of the ratings CSV: ";
    cin >> filename;

    int targetUser, topN;
    cout << "Enter the target user index (0-based): ";
    cin >> targetUser;
    cout << "Enter the number of top recommendations to display: ";
    cin >> topN;

    // Load ratings matrix
    vector<vector<int>> ratingsMatrix = loadRatingsMatrix(filename);

    // Validate target user index
    if (targetUser < 0 || targetUser >= static_cast<int>(ratingsMatrix.size())) {
        cerr << "Error: Invalid user index." << endl;
        return 1;
    }

    // Predict ratings and get top N recommendations
    vector<pair<int, double>> recommendations = predictRatings(ratingsMatrix, targetUser, topN);

    // Display recommendations
    cout << "\nTop " << topN << " recommended movies for User " << targetUser + 1 << ":\n";
    for (const auto &rec : recommendations) {
        cout << "Movie " << rec.first + 1 << " with predicted rating " << fixed << setprecision(2) << rec.second << endl;
    }

    return 0;
}
