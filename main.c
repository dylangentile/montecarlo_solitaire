#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#undef NDEBUG /*force asserts to stay enabled in release builds*/
#include <assert.h> 





/*
Fisher-Yates Shuffle
https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm
*/
void
shuffle_in_place(uint32_t* const deck, const uint32_t deck_size)
{
	/*
	on any modern operating system this should be a suitable prng
	*/
	FILE* rng_file = fopen("/dev/urandom", "r");
	assert(rng_file && "failed to open /dev/urandom");

	/*
	Read in a table of random numbers from /dev/urandom in advance
	faster than waiting on blocking fread calls every time we need a random number
	*/

	const uint32_t random_array_size = deck_size;
	uint32_t* const random_array = calloc(random_array_size, sizeof *random_array);
	assert(random_array);

	assert(
		fread(random_array, 1, random_array_size * sizeof *random_array, rng_file)
			== random_array_size * sizeof *random_array
	);

	fclose(rng_file);


	for(uint32_t i = 0; i < deck_size; i++)
	{
		const uint32_t scratch_range = deck_size - i;

		/*
		random index in range [0, scratch_range)
		*/
		const uint32_t random_index = random_array[i] % scratch_range;
		const uint32_t end_idx = scratch_range - 1;

		/*
		swap randomly chosen element and last element of "scratch" (array within range)
		*/
		const uint32_t end_element = deck[end_idx];
		deck[end_idx] = deck[random_index];
		deck[random_index] = end_element;

	}


	free(random_array);
}


void
print_deck(uint32_t* deck, uint32_t deck_size)
{
	for(uint32_t* ptr = deck; ptr != deck + deck_size; ptr++)
		printf("%.2d\n",*ptr);
}

//assumes 52 card deck, for debugging
void
print_hand(uint32_t* hand)
{
	for(uint32_t i = 0; i < 52; i++)
	{
		if(hand[i] == 52)
		{
			printf("__ ");
			continue;
		}
		
		const uint32_t suit = hand[i]%4;
		const uint32_t face = hand[i]%13;

		
		switch(face)
		{
			case 0:
				printf("A");
			break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				printf("%d", face + 1);
			break;
			case 9:
				printf("~");
			break;
			case 10:
				printf("J");
			break;
			case 11:
				printf("Q");
			break;
			case 12:
				printf("K");
			break;			
			default: printf("_");
		}

		switch(suit)
		{
			case 0:
				printf("H");
			break;
			case 1:
				printf("D");
			break;
			case 2:
				printf("C");
			break;
			case 3:
				printf("S");
			break;
			default: printf("_");
		}
		printf(" ");
	}
	//printf("\n");
}

int
main()
{
	
	/*
	Set the parameters
	*/
	const uint32_t monte_carlo_iterations = 1000000;
	const uint32_t suit_count = 4;
	const uint32_t face_count = 13;
	const uint32_t deck_size = suit_count * face_count;
	assert((deck_size % 2) + 1 && "Decks with odd number of cards are unsolvable!");


	/*
	Allocate and initialize the deck
	*/

	uint32_t* const deck = calloc(deck_size, sizeof *deck);
	assert(deck && "bad allocation!");
	for(uint32_t i = 0; i < deck_size; i++)
		deck[i] = i;

	/*
	sanity check for shuffling 
		-- pipe the program into 'sort', make sure all of the numbers are still there
	*/
	/*	
		shuffle_in_place(deck, deck_size);
		print_deck(deck, deck_size); 
	*/


	/*
	Play array, reused in the various iterations
	Can't have more cards in play then in the deck
	*/

	uint32_t* const play_array = calloc(deck_size, sizeof *play_array);
	assert(play_array);

	

	uint32_t* const deck_end = deck + deck_size;
	//uint32_t* const play_end = play_array + deck_size;

	const int print_debug = 0;
	uint32_t successes = 0;
	for(uint32_t i = 0; i < monte_carlo_iterations; i++)
	{
		/*
		Shuffle the deck
		*/

		shuffle_in_place(deck, deck_size);

		/*
		set entire play area to 'unset'
		*/
		for(uint32_t i = 0; i < deck_size; i++)
			play_array[i] = deck_size; 
		
		uint32_t* deck_ptr = deck;
		uint32_t play_offset = 0;
		uint32_t discard_count = 0;


		while(1)
		{
			STOP:
			if(deck_ptr == deck_end)
				break;
			
			/*
			deal cards
			*/
			for(uint32_t i = 0; i < 4; i++)
			{
				if(play_array[play_offset + i] == deck_size)
					play_array[play_offset + i] = *(deck_ptr++);

				if(deck_ptr == deck_end && i != 3)
					goto STOP;
			}

			
			if(print_debug)
			{
				print_hand(play_array);
				printf("\t%d\t", play_offset);
			}

			/* Same suit? */
			if((play_array[play_offset+0] % suit_count) == (play_array[play_offset+3] % suit_count)) 
			{
				play_array[play_offset+1] = play_array[play_offset + 3];
				play_array[play_offset+2] = deck_size;
				play_array[play_offset+3] = deck_size;

				discard_count+=2;

				if(play_offset > 2)
					play_offset -= 2;
				else
					play_offset = 0;

				if(print_debug)
					printf("suit!!\n");
			}
			/* Same face? */
			else if((play_array[play_offset+0] % face_count) == (play_array[play_offset+3] % face_count)) 
			{
				for(uint32_t i = 0; i < 4; i++)
					play_array[play_offset+i] = deck_size;

				discard_count += 4;

				if(play_offset > 4)
					play_offset -= 4;
				else
					play_offset = 0;
				
				if(print_debug)
					printf("face!!\n");				
			}
			/*deal card, increase offset*/
			else
			{
				if(deck_ptr == deck_end)
					break;
				
				play_array[play_offset + 4] = *(deck_ptr++);
				play_offset++;


				if(print_debug)
					printf("deal..\n");
			}
		}

		if(discard_count == deck_size)
			successes++;
	}


	printf("%u/%u games won, %2.2f%% success rate\n", 
		successes, monte_carlo_iterations, 100.0*(double)successes/(double)monte_carlo_iterations);
	/*
	Cleanup
	*/
	free(deck);
	return 0;
}