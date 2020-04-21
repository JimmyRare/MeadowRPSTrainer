#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum Actions { ROCK, PAPER, SCISSORS };
#define NUM_ACTIONS 3

typedef struct {
	double frequency[NUM_ACTIONS];
} strategy_t;

typedef struct {
	int count[NUM_ACTIONS];
} strategy_sum_t;

typedef struct{
	int action[NUM_ACTIONS];
	int cumulative;
} regrets_t;

typedef struct {
	regrets_t regrets;
	strategy_t strategy;
	strategy_sum_t strategy_sum;
} player_t;

int get_action(player_t* player) {
	double r = (double)rand() / RAND_MAX;
	int i = 0;
	double cumulative_probability = 0;

	// Add probabilitys for each action until it's higher than the random value,
	// then return the current iterator value representing the action.
	while (i < NUM_ACTIONS - 1) {
		cumulative_probability += player->strategy.frequency[i];
		if (r < cumulative_probability) {
			break;
		}
		i++;
	}

	return i;
}

// Get current mixed strategy through regret-matching
strategy_t get_strategy(player_t* player) {
	double normalizer = 0;

	// Add each player positive regret to the player strategy
	for (int i = 0; i < NUM_ACTIONS; i++) {
		player->strategy.frequency[i] = player->regrets.action[i] > 0 ? player->regrets.action[i] : 0;
		normalizer += player->strategy.frequency[i];
	}

	// Normalize player strategy
	for (int i = 0; i < NUM_ACTIONS; i++) {
		if (normalizer > 0) {
			player->strategy.frequency[i] /= normalizer;
		}
		else {
			player->strategy.frequency[i] = 1.0f / NUM_ACTIONS;
		}
		player->strategy_sum.count[i] += player->strategy.frequency[i];
	}

	return player->strategy;
}

strategy_t get_average_strategy(player_t* player) {
	strategy_t average_strategy;
	double normalizer = 0;
	for (int i = 0; i < NUM_ACTIONS; i++) {
		normalizer += player->strategy_sum.count[i];
	}
	for (int i = 0; i < NUM_ACTIONS; i++) {
		if (normalizer > 0) {
			average_strategy.frequency[i] = player->strategy_sum.count[i] / normalizer;
		}
		else {
			average_strategy.frequency[i] = 1.0 / NUM_ACTIONS;
		}
	}
	return average_strategy;
}

void train(player_t* player, player_t* opponent, int iterations) {
	double actionUtility[NUM_ACTIONS];
	for (int i = 0; i < iterations; i++) {
		// Get regret-matched mixed-strategy actions
		player->strategy = get_strategy(player);
		int player_action = get_action(player);
		int opponent_action = get_action(opponent);

		// Compute action utilities
		actionUtility[opponent_action] = 0;
		actionUtility[opponent_action == SCISSORS ? 0 : opponent_action + 1] = 1;
		actionUtility[opponent_action == ROCK ? SCISSORS : opponent_action - 1] = -1;

		// Accumulate action regrets
		for (int i = 0; i < NUM_ACTIONS; i++) {
			player->regrets.action[i] += actionUtility[i] - actionUtility[player_action];
		}
	}
}

int main() {
	//========================================================
	//						SETUP
	//========================================================
	// Initialize randomizer
	srand(time(0));

	// Init regrets and strategy
	regrets_t regret_init;
	strategy_t strategy_init;
	strategy_sum_t strategy_sum_init;
	for (int i = 0; i < NUM_ACTIONS; i++) {
		regret_init.action[i] = 0;
		strategy_init.frequency[i] = 0;
		strategy_sum_init.count[i] = 0;
	}
	regret_init.cumulative = 0;

	// Init players
	player_t player1;
	player_t player2;
	player1.regrets = regret_init;
	player2.regrets = regret_init;
	player1.strategy = strategy_init;
	player2.strategy.frequency[0] = 0.4;
	player2.strategy.frequency[1] = 0.3;
	player2.strategy.frequency[2] = 0.3;
	player1.strategy_sum = strategy_sum_init;
	player2.strategy_sum = strategy_sum_init;

	//========================================================
	//						TRAIN
	//========================================================
	int iterations = 1000000;
	train(&player1, &player2, iterations);
	strategy_t avg = get_average_strategy(&player1);

	//========================================================
	//				   OUTPUT SOLUTION
	//========================================================
	printf("The optimal solution was calculated by using %d iterations.\n", iterations);

	printf("Opponent strategy: ROCK: %.2f, PAPER: %.2f, SCISSORS: %.2f\n",
		player2.strategy.frequency[ROCK],
		player2.strategy.frequency[PAPER],
		player2.strategy.frequency[SCISSORS]
	);

	printf("Your optimal play vs this opponent is: ROCK: %.2f, PAPER: %.2f, SCISSORS: %.2f\n",
		avg.frequency[ROCK],
		avg.frequency[PAPER],
		avg.frequency[SCISSORS]
	);

	//========================================================
	//						CLEAN
	//========================================================

	return 0;
}